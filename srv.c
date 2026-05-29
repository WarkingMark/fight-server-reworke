/* 
 * modifed: igorpauk 2017-18
 */

#include "common.h"
#include "srvcmd.h"
#include "io.h"
#include "pers.h"
#include "fight.h"
#include "srv.h"


int                 fs_ipAddr;
int                 fs_ctrlSock, fs_clSock;
bool                fs_srvLoop = true;
int                 fs_ctime, fs_stime;

vector_t            *fs_workerVec = NULL;	// 'fs_worker_t' vector
vector_t            *fs_fightVec = NULL;	// 'fs_fight_t' vector
vector_t            *fs_persVec = NULL;		// 'fs_pers_t' vector
vector_t            *fs_persLua = NULL;		// 'Pers Lua Skill
vector_t            *fs_fightInfoVec = NULL;	// 'fs_fightInfo_t' vector (main and fight thread access)
vector_t            *fs_fightInfoVec1 = NULL;	// 'fs_fightInfo_t' vector (main and fight thread access)
pthread_mutex_t     fs_mutex;

struct sockaddr_in  fs_feedback_sa;
char                *fs_feedbackHttpHost = NULL, *fs_feedbackHttpPath = NULL, *fs_feedbackHttpAuth = NULL;

fs_srvOptionVal_t   fs_srvOptions[FS_SO_MAXCODE+1] = {
	3000,   // FS_SO_MAX_FIGHT_CNT 
};



errno_t fs_srvLockMutex(void) {
	if (pthread_mutex_lock(&fs_mutex) != 0) {
		WARN("pthread_mutex_lock() failed: %s",strerror(errno));
		return ERR_SYS_ERROR;
	}
	return OK;
}

errno_t fs_srvUnlockMutex(void) {
	if (pthread_mutex_unlock(&fs_mutex) != 0) {
		WARN("pthread_mutex_unlock() failed: %s",strerror(errno));
		return ERR_SYS_ERROR;
	}
	return OK;
}

// ========================================= client non-blocking I/O ========================================= //

fs_client_t *fs_clientCreate(int sock) {
	fs_client_t  *client;

	client = malloc(sizeof(fs_client_t));
	if (!client) {
		WARN("malloc() failed");
		return NULL;
	}
	memset(client,0,sizeof(fs_client_t));
	client->sock = sock;
	client->pidx = -1;
	client->iotime = fs_stime;
	client->inPacketVec = v_init(NULL);
	client->outPacketVec = v_init(NULL);
	return client;
}

errno_t fs_clientDelete(fs_client_t *client) {
	fs_packet_t  *packet;

	if (!client) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	fs_clientDisconnect(client);
	while ((packet = v_pop(client->inPacketVec))) fs_packetDelete(packet,true);
	while ((packet = v_pop(client->outPacketVec))) fs_packetDelete(packet,true);
	v_free(client->inPacketVec);
	v_free(client->outPacketVec);
	free(client->inBuf);
	free(client->outBuf);
	free(client);
	return OK;
}

errno_t fs_clientDisconnect(fs_client_t *client) {
	if (!client) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	if (client->sock) {
		shutdown(client->sock,SHUT_RDWR);
		close(client->sock);
		client->sock = 0;
		client->flags |= FS_CF_DISCONN;
	}
	if (client->pers) {
		client->pers->client = NULL;
		client->pers = NULL;
	}
	return OK;
}

errno_t fs_clientRead(fs_client_t *client) {
	fs_packet_t  *packet;
	int          size;
	char         ipbuf[16];

	if (!client) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	if (client->flags & (FS_CF_DISCONN | FS_CF_IO_WR)) return ERR_WRONG_STATE;
	strIpAddr(client->ipAddr,ipbuf);
	if (!client->inLen) {	// reading packet size
		client->flags |= FS_CF_IO_RD;
		client->iotime = fs_stime;
		size = safeReadNB(client->sock,client->tbuf+client->inPos,4-client->inPos);
		if (size < 0) {	// eof
			client->flags |= FS_CF_DISCONN;
			return ERR_EOF;
		}
		client->inPos += size;
		if (client->inPos < 4) return ERR_IO_ERROR;
		__check_policy_file_request(client);
		*(client->tbuf + 4) = 0;
		size = hexToInt(client->tbuf);
		if ((size <= 0) || (size > fs_ioMaxInPacketSize)) {
			WARN("Invalid incoming packet size (size: %d, max size: %d, data: %s, ip: %s)",size,fs_ioMaxInPacketSize,client->tbuf,ipbuf);
			client->flags |= FS_CF_DISCONN;
			return ERR_WRONG_DATA;
		}
		size++;	// Plus extra \0 at the end since we read C-strings 
		client->inBuf = malloc(size);
		client->inLen = size;
		client->inPos = 0;
	}
	if (client->inLen) {	// reading packet data
		size = safeReadNB(client->sock,client->inBuf+client->inPos,client->inLen-client->inPos);
		if (size < 0) {	// eof
			client->flags |= FS_CF_DISCONN;
			return ERR_EOF;
		}
		client->inPos += size;
		if (client->inPos < client->inLen) return ERR_IO_ERROR;
		size = client->inLen - 1;
		*(client->inBuf + size) = 0;
		if (!__check_policy_file_request(client)) {
			packet = fs_packetCreate();
			if ((strlen(client->inBuf) != size) || (fs_unpackParams(&(packet->params),client->inBuf,size) != OK)) {
				WARN("Can't unpack packet data (size: %d, data: %s, ip: %s)",size,client->inBuf,ipbuf);
				fs_packetDelete(packet,true);
				client->flags |= FS_CF_DISCONN;
				return ERR_WRONG_DATA;
			}
			packet->size = size;
			v_push(client->inPacketVec,packet);
		}
		free(client->inBuf);
		client->inBuf = NULL;
		client->inLen = client->inPos = 0;
		client->flags &= ~FS_CF_IO_RD;
	}
	return OK;
}

errno_t fs_clientWrite(fs_client_t *client) {
	fs_packet_t  *packet;
	int          size;
	char         tbuf[10];

	if (!client) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	if (client->flags & (FS_CF_DISCONN | FS_CF_IO_RD)) return ERR_WRONG_STATE;
	while (1) {
		if (client->outLen) {
			client->flags |= FS_CF_IO_WR;
			client->iotime = fs_stime;
			size = safeWriteNB(client->sock,client->outBuf+client->outPos,client->outLen-client->outPos);
			if (size < 0) {	// eof
				client->flags |= FS_CF_DISCONN;
				return ERR_EOF;
			}
			client->outPos += size;
			if (client->outPos < client->outLen) return ERR_IO_ERROR;
			free(client->outBuf);
			client->outBuf = NULL;
			client->outLen = client->outPos = 0;
			client->flags &= ~FS_CF_IO_WR;
		}
		packet = v_pop_back(client->outPacketVec,0);
		if (!packet) break;
		size = fs_getParamBufSize(&(packet->params));
		if ((size <= 0) || (size > fs_ioMaxOutPacketSize)) {
			WARN("Invalid outgoing packet size (size: %d, max size: %d)",size,fs_ioMaxOutPacketSize);
		} else {
			client->outBuf = malloc(size + 5);
			if (fs_packParams(&(packet->params),client->outBuf+4,size) != size) {
				WARN("Packet size mismatch");
				free(client->outBuf);
				client->outBuf = NULL;
			} else {
				sprintf(tbuf,"%04x",size);
				memcpy(client->outBuf,tbuf,4);
				size += 4;
				*(client->outBuf + size) = 0;
				client->outLen = size + 1;
			}
		}
		fs_packetDelete(packet,true);
	}
	return OK;
}

bool __check_policy_file_request(fs_client_t *client) {
	int     size = 23;	// "<policy-file-request/>\0"
	char    prefix[4] = { '<', 'p', 'o', 'l' };
	char*   answer = "<?xml version=\"1.0\"?>\r\n<cross-domain-policy>\r\n<allow-access-from domain=\"*\" to-ports=\"*\" />\r\n</cross-domain-policy>\r\n";

	if (client->flags & FS_CF_IO_RD) {
		if (!client->inLen) {
			if (!memcmp(client->tbuf, prefix, 4)) {
				DEBUG("Policy file requested (sock: %d)", client->sock);
				client->__policy_file_request = true;
				sprintf(client->tbuf, "%04x", size - 5);
			} else {
				client->__policy_file_request = false;
			}
		}
		if (client->inLen) {
			if (client->__policy_file_request) {
				DEBUG("Policy file given (sock: %d)", client->sock);
				client->outLen = strlen(answer) + 1;
				client->outBuf = malloc(client->outLen);
				memcpy(client->outBuf, answer, client->outLen);
				return true;
			}
		}
	}
	return false;
}

// ========================================= worker ========================================= //

fs_worker_t *fs_workerCreate(void) {
	fs_worker_t       *worker;
	pthread_attr_t    ta;
	int               socks[2];

	if (socketpair(AF_UNIX,SOCK_STREAM,0,socks) != 0) {
		WARN("socketpair() failed: %s",strerror(errno));
		return NULL;
	}
	worker = malloc(sizeof(fs_worker_t));
	if (!worker) {
		WARN("malloc() failed");
		return NULL;
	}
	memset(worker,0,sizeof(fs_worker_t));
	worker->mtime = fs_stime;
	worker->intPipeSock = socks[0];
	worker->extPipeSock = socks[1];
	worker->clientVec = v_init(NULL);
	pthread_mutex_init(&(worker->mutex),NULL);

	// starting the thread
	pthread_attr_init(&ta);
	pthread_attr_setdetachstate(&ta,PTHREAD_CREATE_JOINABLE);
	if (pthread_create(&(worker->th),&ta, (void * (*)(void *))fs_workerThreadRoutine, (void *)worker) != 0) {
		WARN("pthread_create() failed: %s",strerror(errno));
		return NULL;
	}
	return worker;
}

errno_t fs_workerDelete(fs_worker_t *worker) {
	fs_client_t  *client;

	if (!worker) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	// stopping the thread
	fs_workerSignal(worker,FS_WS_EXIT);
	if (pthread_join(worker->th,NULL) != 0) {
		WARN("pthread_join() failed: %s",strerror(errno));
		return ERR_SYS_ERROR;
	}
	// destroying
	while ((client = v_pop(worker->clientVec))) fs_clientDelete(client);
	v_free(worker->clientVec);
	close(worker->intPipeSock);
	close(worker->extPipeSock);
	pthread_mutex_destroy(&(worker->mutex));
	free(worker);
	return OK;
}

errno_t fs_workerLockMutex(fs_worker_t *worker) {
	if (!worker) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	if (pthread_mutex_lock(&(worker->mutex)) != 0) {
		WARN("pthread_mutex_lock() failed: %s",strerror(errno));
		return ERR_SYS_ERROR;
	}
	return OK;
}

errno_t fs_workerUnlockMutex(fs_worker_t *worker) {
	if (!worker) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	if (pthread_mutex_unlock(&(worker->mutex)) != 0) {
		WARN("pthread_mutex_unlock() failed: %s",strerror(errno));
		return ERR_SYS_ERROR;
	}
	return OK;
}

errno_t fs_workerThreadRoutine(fs_worker_t *worker) {
	fs_client_t       *client;
	fs_fight_t        *fight;
	fs_pers_t         *pers;
	fs_workerSignal_t sig = FS_WS_NONE;
	bool              workerLoop = true;
	struct pollfd     ufds[MAX_POLL_SIZE];
	int               nfds, ecnt;

	if (!worker) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	DEBUG("Worker ready");
	while (1) {	// main worker loop
		ufds[0].fd = worker->intPipeSock;
		ufds[0].events = POLLIN | POLLPRI;
		ufds[0].revents = 0;
		nfds = 1;
		fs_workerLockMutex(worker);
		v_reset(worker->clientVec,0);
		while ((client = v_each(worker->clientVec,0))) {
			fs_clientWrite(client);
			if ((client->flags & FS_CF_DISCONN) || (client->iotime <= (fs_stime - CLIENT_TTL))) {
 				v_remove(worker->clientVec,client);
 				fs_clientDelete(client);
 				continue;
 			}
			if (client->fight && client->pers) {	// switching the client to the fight thread here
				v_remove(worker->clientVec,client);
				fight = client->fight;
				pers = client->pers;
				pthread_mutex_lock(&(fight->mutex_cl));
				if (pers->client && (pers->client != client)) {	// client re-assignment
					pers->client->flags |= FS_CF_DISCONN;
					pers->client->pers = NULL;
					pers->client = NULL;
				}
				pers->client = client;
				client->pidx = -1;
				v_push(fight->clientVec,client);
				pthread_mutex_unlock(&(fight->mutex_cl));
				fs_fightSignal(fight,FS_FSIG_NONE);	// just wake up
				fs_fightLockMutex(fight);
				if (pers->status == FS_PS_CREATED) pers->status = FS_PS_FIGHTING; //FS_PS_PENDING
				fs_fightUnlockMutex(fight);
				continue;
			}
			if (nfds >= MAX_POLL_SIZE) {
				WARN("Maximum poll size reached");
				break;
			}
			ufds[nfds].fd = client->sock;
			ufds[nfds].events = POLLIN | POLLPRI | (client->flags & FS_CF_IO_WR ? POLLOUT : 0);
			ufds[nfds].revents = 0;
			client->pidx = nfds;
			nfds++;
		}
		fs_workerUnlockMutex(worker);
		if (!workerLoop) break;

		ecnt = poll(ufds,nfds,100);

		if ((ecnt == -1) && (errno != EINTR)) WARN("poll() failed: %s",strerror(errno));
		if (ecnt > 0) {
			worker->mtime = fs_stime;
			if (ufds[0].revents & (POLLIN | POLLPRI)) {	// signal pending
				memset(&sig,0,sizeof(sig));
				safeRead(worker->intPipeSock,(char *)(&sig),1);
				if (sig == FS_WS_EXIT) workerLoop = false;
			}
			fs_workerLockMutex(worker);
			v_reset(worker->clientVec,0);
			while ((client = v_each(worker->clientVec,0))) {
				if ((client->pidx < 0) || (client->pidx >= MAX_POLL_SIZE)) continue;
				if (ufds[client->pidx].revents & (POLLERR | POLLHUP | POLLNVAL)) client->flags |= FS_CF_DISCONN;
				if (!(ufds[client->pidx].revents & (POLLIN | POLLPRI))) continue;
				fs_clientRead(client);
				fs_processClient(client);
			}
			fs_workerUnlockMutex(worker);
		}
	}
	DEBUG("Worker done");
	return OK;
}

errno_t fs_workerSignal(fs_worker_t *worker, fs_workerSignal_t sig) {
	if (!worker) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	return safeWrite(worker->extPipeSock,(char *)(&sig),1);
}


// ================================================================================== //

errno_t fs_srvInit(char *host, int ctrlPort, int clPort) {
	struct hostent     *he;
	struct sockaddr_in sa1, sa2;
	int                optval;
	
	if (!ctrlPort || !clPort || (ctrlPort == clPort)) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	if (sigHandlerInstall() != OK) return ERR_SYS_ERROR;
	fs_ctrlSock = socket(AF_INET,SOCK_STREAM,0);
	fs_clSock = socket(AF_INET,SOCK_STREAM,0);
	if ((fs_ctrlSock == -1) || (fs_clSock == -1)) {
		WARN("socket() failed: %s",strerror(errno));
		return ERR_SYS_ERROR;
	}
	if (fcntl(fs_ctrlSock,F_SETFL,(fcntl(fs_ctrlSock,F_GETFL) | O_NONBLOCK)) || fcntl(fs_clSock,F_SETFL,(fcntl(fs_clSock,F_GETFL) | O_NONBLOCK))) {
		WARN("fcntl() failed: %s",strerror(errno));
		return ERR_SYS_ERROR;
	}
	optval = 1;
	if (setsockopt(fs_ctrlSock,SOL_TCP,TCP_NODELAY,&optval,sizeof(optval)) || setsockopt(fs_clSock,SOL_TCP,TCP_NODELAY,&optval,sizeof(optval))) {
		WARN("setsockopt() failed: %s",strerror(errno));
		return ERR_SYS_ERROR;
	}
	if (setsockopt(fs_ctrlSock,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval)) || setsockopt(fs_clSock,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval))) {
		WARN("setsockopt() failed: %s",strerror(errno));
		return ERR_SYS_ERROR;
	}
	memset(&sa1,0,sizeof(sa1));
	sa1.sin_family = AF_INET;
	if (host) {
		he = gethostbyname(host);
		if (!he) {
			WARN("gethostbyname('%s') failed: %s",host,strerror(h_errno));
			return ERR_SYS_ERROR;
		}
		memcpy(&(sa1.sin_addr.s_addr),he->h_addr_list[0],sizeof(sa1.sin_addr.s_addr));
		fs_ipAddr = ntohl(sa1.sin_addr.s_addr);
	}
	memcpy(&sa2,&sa1,sizeof(sa2));
	sa1.sin_port = htons(ctrlPort);
	sa2.sin_port = htons(clPort);
	if (bind(fs_ctrlSock,(struct sockaddr *)&sa1,sizeof(sa1)) || bind(fs_clSock,(struct sockaddr *)&sa2,sizeof(sa2))) {
		WARN("bind() failed: %s",strerror(errno));
		return ERR_SYS_ERROR;
	}
	if (listen(fs_ctrlSock,SOMAXCONN) || listen(fs_clSock,SOMAXCONN)) {
		WARN("listen() failed: %s",strerror(errno));
		return ERR_SYS_ERROR;
	}
	fs_ctime = fs_stime = time(NULL);
	srand(fs_ctime);
	fs_workerVec = v_init(NULL);
	fs_fightVec = v_init(NULL);
	fs_persVec = v_init(NULL);
	fs_fightInfoVec = v_init(NULL);
	fs_fightInfoVec1 = v_init(NULL);
	
	pthread_mutex_init(&fs_mutex,NULL);
	return OK;
}

errno_t fs_srvRun(void) {
	struct pollfd     ufds[2];
	int               nfds, ecnt;

	MSG("ready");
	while (1) {	// main server loop
		fs_stime = time(NULL);
		ufds[0].fd = fs_ctrlSock;
		ufds[0].events = POLLIN | POLLPRI;
		ufds[0].revents = 0;
		ufds[1].fd = fs_clSock;
		ufds[1].events = POLLIN | POLLPRI;
		ufds[1].revents = 0;
		nfds = 2;
		if (!fs_srvLoop) break;
		ecnt = poll(ufds,nfds,25);	// 25 msec
		if ((ecnt == -1) && (errno != EINTR)) WARN("poll() failed: %s",strerror(errno));
		if (ecnt > 0) {
			if (ufds[0].revents & (POLLIN | POLLPRI)) fs_srvAccept(fs_ctrlSock);
			if (ufds[1].revents & (POLLIN | POLLPRI)) fs_srvAccept(fs_clSock);
		}
		if (signals & CSF_SIGINT) {
			blockSignal(SIGINT);
			fs_srvLoop = false;
			signals &= ~CSF_SIGINT;
			unblockSignal(SIGINT);
		} else if (signals & CSF_SIGTERM) {
			blockSignal(SIGTERM);
			fs_srvLoop = false;
			signals &= ~CSF_SIGTERM;
			unblockSignal(SIGTERM);
		}
		fs_srvCleanup();
	}
	return OK;
}

errno_t fs_srvAccept(int sock) {
	struct sockaddr_in  sa;
	socklen_t           sl;
	int                 s, optval;
	fs_client_t         *client, *c_client;
	fs_worker_t         *worker;

	sl = sizeof(sa);
	
	if((s = accept(sock,(struct sockaddr *)(&sa),&sl)) <= 0){
		WARN("accept() failed: %s",strerror(errno));
		return ERR_SYS_ERROR;
	}
	
	/*do {
		s = accept(sock,(struct sockaddr *)(&sa),&sl));
	} while ((s == -1) && (errno == EINTR));
	if (s == -1) {
		WARN("accept() failed: %s",strerror(errno));
		return ERR_SYS_ERROR;
	}*/
	if (fcntl(s,F_SETFL,(fcntl(s,F_GETFL) | O_NONBLOCK))) {
		WARN("fcntl() failed: %s",strerror(errno));
		close(s);
		return ERR_SYS_ERROR;
	}
	optval = 1;
	if (setsockopt(s,SOL_TCP,TCP_NODELAY,&optval,sizeof(optval))) {
		WARN("setsockopt() failed: %s",strerror(errno));
		close(s);
		return ERR_SYS_ERROR;
	}
	client = fs_clientCreate(s);
	if (!client) {
		WARN("Can't create client");
		close(s);
		return ERR_NO_MEM;
	}
	client->ipAddr = ntohl(sa.sin_addr.s_addr);
	if (sock == fs_ctrlSock) client->flags |= FS_CF_CTRL;

	//check uebok
	/*int uebok_cnt = 0;
	v_reset(fs_workerVec,0);
	while ((worker = v_each(fs_workerVec,0))) {
		v_reset(worker->clientVec,0);
		while ((c_client = v_pop(worker->clientVec))){
			if(c_client->ipAddr == client->ipAddr) uebok_cnt++;
			if(uebok_cnt >= 10) break;
		}
	}
	
	if(uebok_cnt >= 10){
		DEBUG("UEBOK %s", client->ipAddr);
		fs_clientDelete(client);
		return ERR_INIT;
	}*/
	
	// moving the client to a worker's client pool
	if (v_size(fs_workerVec) < MAX_WORKER_CNT) {
		worker = fs_workerCreate();
		if (!worker) {
			WARN("Can't create worker");
			fs_clientDelete(client);
			return ERR_NO_MEM;
		}
		fs_srvLockMutex();
		v_push(fs_workerVec,worker);
		fs_srvUnlockMutex();
	} else {
		fs_srvLockMutex();
		worker = v_elem(fs_workerVec,randInt(0,v_size(fs_workerVec)-1,NULL));	// random worker
		fs_srvUnlockMutex();
		if (!worker) {
			WARN("Can't find worker");
			fs_clientDelete(client);
			return ERR_INIT;
		}
	}
	fs_workerLockMutex(worker);
	v_push(worker->clientVec,client);
	fs_workerUnlockMutex(worker);
	fs_workerSignal(worker,FS_WS_NONE);	// just wake up
	return OK;
}

errno_t fs_srvCleanup(void) {
	fs_worker_t       *worker;
	fs_fight_t        *fight;
	fs_pers_t         *pers;
	fs_fightInfo_t    *info;
	static int        cleanupTime = 0;

	if (cleanupTime > (fs_stime - CLEANUP_INTERVAL)) return OK;
	cleanupTime = fs_stime;
	fs_srvLockMutex();
	v_reset(fs_workerVec,0);
	while ((worker = v_each(fs_workerVec,0))) {
		if (worker->mtime <= (fs_stime - INACTIVE_WORKER_TTL)) {
			v_remove(fs_workerVec,worker);
			fs_workerDelete(worker);
		}
	}
	v_reset(fs_fightVec,0);
	while ((fight = v_each(fs_fightVec,0))) {
		if ((fight->mtime <= (fs_stime - INACTIVE_TIMEOUT)) || (fight->status == FS_FS_FINISHED)) {
			if ((fight->status == FS_FS_RUNNING) || (fight->status == FS_FS_OVER)) fs_fightSignal(fight,FS_FSIG_STOP);	// delaying thread stop
			else {
				INFO("FIGHT CLEANUP [id: %d, inactive %d sec]",fight->id,(fs_stime-fight->mtime));
				v_remove(fs_fightVec,fight);
				fs_fightDelete(fight);
			}
		}
	}
	v_reset(fs_persVec,0);
	while ((pers = v_each(fs_persVec,0))) {
		if (pers->ctime <= (fs_stime - INACTIVE_TIMEOUT)) {
			v_remove(fs_persVec,pers);
			fs_persDelete(pers);
		}
	}
	v_reset(fs_fightInfoVec,0);
	while ((info = v_each(fs_fightInfoVec,0))) {
		if (info->ctime <= (fs_stime - INACTIVE_TIMEOUT)) {
			INFO("FIGHTINFO CLEANUP [id: %d, inactive %d sec]",info->id,(fs_stime-info->ctime));
			v_remove(fs_fightInfoVec,info);
			fs_fightInfoDelete(info);
		}
	}
	v_reset(fs_fightInfoVec1,0);
	while ((info = v_each(fs_fightInfoVec1,0))) {
		if (info->ctime <= (fs_stime - INACTIVE_TIMEOUT)) {
			INFO("DÖVÜŞ VEKTOR SİLİNİYOR [id: %d, Etkisiz %d saniye]",info->id,(fs_stime-info->ctime));
			v_remove(fs_fightInfoVec1,info);
			fs_fightInfoDelete(info);
		}
	} 
	fs_srvUnlockMutex();
	return OK;
}

errno_t fs_srvDone(void) {
	fs_worker_t       *worker;
	fs_fight_t        *fight;
	fs_pers_t         *pers;
	fs_fightInfo_t    *info;

	MSG("shutting down...");
	shutdown(fs_ctrlSock,SHUT_RDWR);
	shutdown(fs_clSock,SHUT_RDWR);
	close(fs_ctrlSock);
	close(fs_clSock);

	fs_srvLockMutex();
	while ((worker = v_pop(fs_workerVec))) fs_workerDelete(worker);
	fs_srvUnlockMutex();

	v_reset(fs_fightVec,0);
	while ((fight = v_each(fs_fightVec,0))) fs_fightSignal(fight,FS_FSIG_STOP);

	while ((fight = v_pop(fs_fightVec))) fs_fightDelete(fight);
	while ((pers = v_pop(fs_persVec))) fs_persDelete(pers);
	while ((info = v_pop(fs_fightInfoVec))) fs_fightInfoDelete(info);
	while ((info = v_pop(fs_fightInfoVec1))) fs_fightInfoDelete(info);
	v_free(fs_workerVec);
	v_free(fs_fightVec);
	v_free(fs_persVec);
	v_free(fs_fightInfoVec);
	v_free(fs_fightInfoVec1);

	pthread_mutex_destroy(&fs_mutex);
	return OK;
}

errno_t fs_processClient(fs_client_t *client) {
	fs_packet_t            *inPacket, *outPacket;
	fs_srvCommand_t        cmd;
	fs_srvStatus_t         status;
	fs_param_t             *param, *param_cmd, *param_status;
	fs_srvCmdHandler_t     *cmdHandler;

	if (!client) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	if (client->flags & FS_CF_DISCONN) return ERR_WRONG_STATE;
	while ((inPacket = v_pop_back(client->inPacketVec,0))) {
		fs_debugParams(&(inPacket->params),"REQ: IN",true);
		outPacket = fs_packetCreate();
		v_push(client->outPacketVec,outPacket);
		PARAM_NEW(param_cmd);
		PARAM_PUSH(outPacket,param_cmd);
		PARAM_NEW(param_status);
		PARAM_PUSH(outPacket,param_status);
		PARAM_SETINT(param_cmd,FS_SC_UNDEFINED);
		PARAM_SETINT(param_status,FS_SS_WRONG_ARGS);
		if (PARAM_COUNT(inPacket) > 0) {
			param = PARAM_RESET(inPacket);
			if (PARAM_TYPE(param) == PT_INT) {
				cmd = PARAM_INT(param);
				cmdHandler = (client->flags & FS_CF_CTRL) ? fs_ctrlCmdHanderTable: fs_clCmdHanderTable;
				PARAM_SETINT(param_cmd,cmd);
				PARAM_SETINT(param_status,FS_SS_WRONG_CMD);
				while (cmdHandler->cmd && cmdHandler->func) {
					if (cmdHandler->cmd == cmd) {
						DEBUG("REQ: <== (cmd: %d, sock: %d)",cmd,client->sock);
						if (cmdHandler->srvLock) fs_srvLockMutex();
						status = cmdHandler->func(client,inPacket,outPacket);
						if (cmdHandler->srvLock) fs_srvUnlockMutex();
						PARAM_SETINT(param_status,status);
						DEBUG("REQ: ==> (cmd: %d, status: %d)",cmd,status);
						break;
					}
					cmdHandler++;
				} 
			} else WARN("First parameter is not PT_INT");
		} else WARN("Packet has no parameters");
		fs_debugParams(&(outPacket->params),"REQ: OUT",true);
		fs_packetDelete(inPacket,true);
	}
	return OK;
}


// ========================================= feedback ========================================= //

errno_t fs_feedbackInit(char *url) {
	struct hostent    *he;
	char              *s;
	int               i, port = 0;

	if (!url) {
		WARN("No feedback URL provided");
		return OK;
	}
	s = strstr(url,"://");
	if (s) url = s + 3;	// cutting 'http://'
	s = strchr(url,'/');
	if (s) {
		fs_feedbackHttpPath = strdup(s);
		*s = 0;
	} else fs_feedbackHttpPath = strdup("/");
	s = strchr(url,'@');
	if (s) {
		*s = 0;
		i = MIN(strlen(url),100);	// we're not gonna take too much as auth info
		fs_feedbackHttpAuth = malloc(i*4/3+4);
		memset(fs_feedbackHttpAuth,0,i*4/3+4);
		b64encode(fs_feedbackHttpAuth,url,i);
		url = s + 1;
	}
	s = strchr(url,':');
	if (s) {
		port = strtol(s+1,(char **)NULL,10);
		*s = 0;
	}
	fs_feedbackHttpHost = strdup(url);
	if (!port) port = 80;

	memset(&fs_feedback_sa,0,sizeof(fs_feedback_sa));
	fs_feedback_sa.sin_family = AF_INET;
	he = gethostbyname(url);
	if (!he) {
		WARN("gethostbyname('%s') failed: %s",url,strerror(h_errno));
		return ERR_SYS_ERROR;
	}
	memcpy(&(fs_feedback_sa.sin_addr.s_addr),he->h_addr_list[0],sizeof(fs_feedback_sa.sin_addr.s_addr));
	fs_feedback_sa.sin_port = htons(port);
	return OK;
}

errno_t fs_feedbackDone(void) {
	free(fs_feedbackHttpHost);
	free(fs_feedbackHttpPath);
	free(fs_feedbackHttpAuth);
	return OK;
}

errno_t fs_feedbackData(char *fmt, ...) {
	errno_t           status = OK;
	pthread_t         th;
	pthread_attr_t    ta;
	va_list           ap;
	fs_packet_t       *packet;
	int               size;
	char              *data = NULL;

	if (!fmt) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	if (!fs_feedbackHttpHost) return ERR_WRONG_STATE;
	packet = fs_packetCreate();
	va_start(ap,fmt);
	fs_addParamsVA(&(packet->params),fmt,ap);
	va_end(ap);
	size = fs_getParamBufSize(&(packet->params));
	if ((size <= 0) || (size > fs_ioMaxOutPacketSize)) {
		WARN("Invalid packet size (size: %d, max size: %d)",size,fs_ioMaxOutPacketSize);
		status = ERR_WRONG_DATA;
	} else {
		data = malloc(size + 1);
		if (fs_packParams(&(packet->params),data,size) != size) {
			WARN("Packet size mismatch");
			status = ERR_WRONG_DATA;
		} else {
			*(data + size) = 0;
		}
	}
	fs_packetDelete(packet,true);
	if (status == OK) {
		pthread_attr_init(&ta);
		pthread_attr_setdetachstate(&ta,PTHREAD_CREATE_DETACHED);
		if (pthread_create(&th,&ta, (void * (*)(void *))fs_feedbackThreadRoutine, (void *)data) != 0) {
			WARN("pthread_create() failed: %s",strerror(errno));
			status = ERR_SYS_ERROR;
		}
	}
	if (status != OK) free(data);
	return status;
}

errno_t fs_feedbackThreadRoutine(char *data) {
	errno_t           status = OK;
	int               sock, stime, i;
	struct timeval    tv;
	char              *buf;
	int               len, contentLength;

	if (!data) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	if (!fs_feedbackHttpHost) return ERR_WRONG_STATE;
	DEBUG("Feedback thread started (data: %s)",data);
	tv.tv_sec = 60;
	tv.tv_usec = 0;
	sock = socket(AF_INET,SOCK_STREAM,0);
	if (sock == -1) {
		WARN("socket() failed: %s",strerror(errno));
		status = ERR_SYS_ERROR;
	} else if (setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv)) || setsockopt(sock,SOL_SOCKET,SO_SNDTIMEO,&tv,sizeof(tv))) {
		WARN("setsockopt(sock, SO_(SND|RCV)TIMEO)) failed: %s",strerror(errno));    
		close(sock);
		status = ERR_SYS_ERROR;
	} else {
		do {
			i = connect(sock,(struct sockaddr *)&fs_feedback_sa,sizeof(fs_feedback_sa));
		} while ((i == -1) && (errno == EINTR));
		if (i == -1) {
			WARN("connect() failed: %s",strerror(errno));
			close(sock);
			status = ERR_SYS_ERROR;
		} else {
			contentLength = strlen(data) + 5;
			buf = malloc(1024 + contentLength);
			len = 0;
			len += sprintf(buf + len, "POST %s HTTP/1.1\r\nAccept: */*\r\nContent-Type: application/x-www-form-urlencoded\r\nUser-Agent: Fight Server\r\nHost: %s\r\nContent-Length: %d\r\nConnection: close\r\nCache-Control: no-cache\r\n",fs_feedbackHttpPath,fs_feedbackHttpHost,contentLength);
			if (fs_feedbackHttpAuth) len += sprintf(buf + len, "Authorization: Basic %s\r\n",fs_feedbackHttpAuth);
			len += sprintf(buf + len, "\r\n");
			len += sprintf(buf + len, "data=%s",data);
			// sending/receiving data
			if (safeWriteNB(sock,buf,len) != len) WARN("Error sending POST request (data: %s)",data);
			else {
				stime = fs_stime;
				len = 0;
				while ((i = safeReadNB(sock,buf+len,1024-len-1)) >= 0) {
					len += i;
					if (stime <= (fs_stime - INACTIVE_TIMEOUT)) break;
				}
				*(buf + len) = 0;
				if (!strstr(buf,"200 OK")) WARN("Bad status while sending POST request (data: %s)",data);
			}
			shutdown(sock,SHUT_RDWR);
			close(sock);
			free(buf);
		}
	}
	free(data);
	DEBUG("Feedback thread done");
	return status;
}
