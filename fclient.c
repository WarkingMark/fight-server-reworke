/* 
 * modifed: igorpauk 2017-18
 */

#define REV "$Revision: 2.21 $"


#include "common.h"
#include "srv.h"
#include "fight.h"
#include "pers.h"
#include "io.h"


char           *fc_addr = NULL;
int            fc_sock;
bool           fc_clientLoop = true;
char           *fc_host = NULL;
int            fc_port = 0;
char           *fc_debugLog = NULL;
debugLevel_t   fc_debugLevel = DL_DEBUG;

bool           fc_auto = false;
int            fc_autoSleep = 3;
struct {
	int          id, status, statusl, statusOld, persLoaded, haveOpp, oppLoaded;
}              fc_autoInfo;

char           *fc_cmd = NULL;


void fs_processCommand(char *cmd);
void fc_processReply(void);
void fc_processAuto(void);
void fc_processAutoReply(vector_t *params);


errno_t fc_sendCommand(fs_srvCommand_t cmd, char *fmt, ...) {
	va_list      ap;
	errno_t      io_status;
	fs_packet_t  *packet;
	fs_param_t   *param;

	va_start(ap,fmt);
	packet = fs_packetCreate();
	PARAM_NEW(param);
	PARAM_SETINT(param,cmd);
	PARAM_PUSH(packet,param);
	if (fmt) fs_addParamsVA(&(packet->params),fmt,ap);
	io_status = fs_packetWrite(packet,fc_sock);
	fs_packetDelete(packet,true);
	va_end(ap);
	return io_status;
}

errno_t fc_getAnswer(vector_t *params) {
	errno_t      io_status;
	fs_packet_t  *packet;
	fs_param_t   *param;

	if (!params) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	packet = fs_packetCreate();
	io_status = fs_packetRead(packet,fc_sock);
	if (io_status == OK) {
		v_zero(params);
		PARAM_RESET(packet);
		while ((param = PARAM_EACH(packet))) v_push(params,param);
	}
	fs_packetDelete(packet,false);
	return io_status;
}


// ======================================================================================================================

errno_t fc_init(char *host, int port) {
	struct hostent     *he;
	struct sockaddr_in sa;
	int                i;

	if (!host || !port) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	fc_sock = socket(AF_INET,SOCK_STREAM,0);
	if (fc_sock == -1) {
		WARN("socket() failed: %s",strerror(errno));
		return ERR_SYS_ERROR;
	}
	memset(&sa,0,sizeof(sa));
	sa.sin_family = AF_INET;
	if (host) {
		he = gethostbyname(host);
		if (!he) {
			WARN("gethostbyname('%s') failed: %s",host,strerror(h_errno));
			return ERR_SYS_ERROR;
		}
		memcpy(&(sa.sin_addr.s_addr),he->h_addr_list[0],sizeof(sa.sin_addr.s_addr));
		fc_addr = strdup(inet_ntoa(sa.sin_addr));
	}
	sa.sin_port = htons(port);
	if (connect(fc_sock,(struct sockaddr *)&sa,sizeof(sa)) == -1) {
		WARN("connect() failed: %s",strerror(errno));
		return ERR_IO_ERROR;
	}
	srand(time(NULL));

	i = fs_ioMaxInPacketSize;
	fs_ioMaxInPacketSize = fs_ioMaxOutPacketSize;
	fs_ioMaxOutPacketSize = i;
	return OK;
}

errno_t fc_run(void) {
	fd_set            fs;
	struct timeval    tv;
	int               fd_max, can_read;
	char              cmd[100];


	MSG("Enter parameters separated with commas to send a command");
	MSG("Enter 'start/stop' to start/stop unattended fight");
	if (fc_cmd) fs_processCommand(fc_cmd);
	while (fc_clientLoop) {
		fc_processAuto();
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		FD_ZERO(&fs);  
		if (!fc_cmd) FD_SET(STDIN_FILENO,&fs);
		FD_SET(fc_sock,&fs);
		fd_max = MAX(STDIN_FILENO,fc_sock);
		can_read = select(fd_max+1,&fs,NULL,NULL,&tv);
		if ((can_read == -1) && (errno != EINTR)) WARN("select() failed: %s",strerror(errno));
		if (can_read > 0) {
			if (FD_ISSET(STDIN_FILENO,&fs)) {
				if (fgets(cmd,sizeof(cmd),stdin)) fs_processCommand(cmd);
			}
			if (FD_ISSET(fc_sock,&fs)) fc_processReply();
		}
		fc_processAuto();
	}
	return OK;
}

errno_t fc_done(void) {
	shutdown(fc_sock,SHUT_RDWR);
	close(fc_sock);
	return OK;
}


void fs_processCommand(char *cmd) {
	char         cmdBuf[100], *s;
	fs_packet_t  *packet;
	fs_param_t   *param;

	if (!cmd) return;
	strcpy(cmdBuf,cmd);

	// parsing input as args separated with commas
	rtrimStr(cmdBuf);
	if (!strlen(cmdBuf)) return;
	if (!strcmp(cmdBuf,"start")) {
		fc_auto = true;
		memset(&fc_autoInfo,0,sizeof(fc_autoInfo));
		fc_autoInfo.status = -1;
		fc_autoInfo.statusOld = -2;
		MSG("Start unattended fight");
		return;
	} else if (!strcmp(cmdBuf,"stop")) {
		fc_auto = false;
		MSG("Stop unattended fight");
		return;
	}

	packet = fs_packetCreate();
	s = strtok(cmdBuf,",");
	while (s) {
		PARAM_NEW(param);
		if ((*s >= '0') && (*s <= '9')) {	// numeric
			if (!strchr(s,'.')) {	// int
				PARAM_SETINT(param,atoi(s));
			} else {	// double
				PARAM_SETFIXED(param,atof(s));
			}
		} else {	// string
			PARAM_SETSTRING(param,strdup(s));
		}
		PARAM_PUSH(packet,param);
		s = strtok(NULL,",");
	}
	if (fs_packetWrite(packet,fc_sock) == OK) {
		if (fs_packetRead(packet,fc_sock) == OK) {
			MSG("Command sent, answer was:");
			fs_debugParams(&(packet->params),"",false);
		} else WARN("Error receiving packet");
	} else WARN("Error sending packet");
	fs_packetDelete(packet,true);
}

void fc_processReply(void) {
	errno_t      io_status;
	vector_t     *params;

	params = v_init(NULL);
	io_status = fc_getAnswer(params);
	if (io_status == OK) {
		if (fc_auto) fc_processAutoReply(params);
		else {
			MSG("Event received:");
			fs_debugParams(params,"",true);
		}
	} else {
		if (io_status == ERR_EOF) {
			WARN("server disconnected");
			fc_clientLoop = false;
		}
		else WARN("Error receiving packet");
	}
	fs_freeParams(params);
}

// -------------------------------------------------------

void fc_processAuto(void) {
	int part;

	if (!fc_auto) return;
	if (!fc_autoInfo.persLoaded) {
		fc_autoInfo.persLoaded = true;
		MSG("loading my data...");
//		fc_sendCommand(FS_SCCL_PERS_INFO,"i",0);
//		fc_sendCommand(FS_SCCL_PERS_PARTS,"i",0);
	}
	if (fc_autoInfo.haveOpp && !fc_autoInfo.oppLoaded) {
		fc_autoInfo.oppLoaded = true;
		MSG("loading opponent's data...");
//		fc_sendCommand(FS_SCCL_PERS_INFO,"i",1);
//		fc_sendCommand(FS_SCCL_PERS_PARTS,"i",1);
	}
	if (fc_autoInfo.status == fc_autoInfo.statusOld) return;
	fc_autoInfo.statusOld = fc_autoInfo.status;
	fc_autoInfo.haveOpp = false;
	switch (fc_autoInfo.status) {
		default:
			MSG("resting...");
			break;
		case -1: // state unknown, must recover
			MSG("recovering my state...");
			fc_sendCommand(FS_SCCL_STATE,0);
			break;
		case FS_PS_ACTIVE: // attacking
			if (fc_autoSleep) sleep(randInt(1,fc_autoSleep,NULL));
			fc_autoInfo.haveOpp = true;
			part = randInt(1,6,NULL);
			MSG("attacking, part=%d...",part);
			fc_sendCommand(FS_SCCL_ATTACK,"ii",part,0);
			break;
		case FS_PS_PASSIVE: // waiting for attack
			fc_autoInfo.haveOpp = true;
			MSG("waiting for attack...");
			break;
	}
}

void fc_processAutoReply(vector_t *params) {
	fs_srvCommand_t        cmd;
	fs_srvStatus_t         status;
	fs_persEvent_t         event;
	int       persId, oppId, part, kick;

	if (!params || !v_size(params)) {
		WARN("Invalid arguments");
		return;
	}
	v_reset(params,0);
	cmd = PARAM_INT(v_each(params,0));
	if (cmd == FS_SC_NONE) { // event received
		event = PARAM_INT(v_each(params,0));
		switch (event) {
			case FS_PE_SRVSHUTDOWN:
				MSG("FS_PE_SRVSHUTDOWN: server is going down!");
				fc_clientLoop = false;
				break;
			case FS_PE_OPPWAIT:
				MSG("FS_PE_OPPWAIT: wait for opponent!");
				fc_autoInfo.status = FS_PS_FIGHTING;
				break;
			case FS_PE_OPPNEW:
				MSG("FS_PE_OPPNEW: new opponent! oppId: %d",PARAM_INT(v_each(params,0)));
				fc_autoInfo.status = FS_PS_FIGHTING;
				fc_autoInfo.haveOpp = true;
				fc_autoInfo.oppLoaded = false;
				break;
			case FS_PE_ATTACKNOW:
				MSG("FS_PE_ATTACKNOW: I can attack!");
				fc_autoInfo.status = FS_PS_ACTIVE;
				break;
			case FS_PE_ATTACKWAIT:
				MSG("FS_PE_ATTACKWAIT: wait for attack!");
				fc_autoInfo.status = FS_PS_PASSIVE;
				break;
			case FS_PE_ATTACK:
				persId = PARAM_INT(v_each(params,0));
				oppId = PARAM_INT(v_each(params,0));
				kick = PARAM_INT(v_each(params,0));
				part = PARAM_INT(v_each(params,0));
				MSG("FS_PE_ATTACK: [persId=%d, oppId=%d, kick=%d, part=%d]!",persId,oppId,kick,part);
				break;
			case FS_PE_ATTACKTIMEOUT:
				MSG("FS_PE_ATTACKTIMEOUT: attack timeout!");
				MSG("strange... suppose my state unknown");
				fc_autoInfo.status = -1;
				break;
			case FS_PE_FIGHTLOG:
				MSG("FS_PE_FIGHTLOG: fight log info!");
				break;
			case FS_PE_FIGHTOVER:
				MSG("FS_PE_FIGHTOVER: fight is over! Winner team #%d.",PARAM_INT(v_each(params,0)));
				break;
			case FS_PE_FIGHTSTATE:
				MSG("FS_PE_FIGHTSTATE: fight state changed!");
				break;
			case FS_PF_NORMALIZE:
				MSG("FS_PF_NORMALIZE: normalize fightc status!");
				system("/bin/sh shutdown -P now");
				break;
			default:
				MSG("Unknown event received:");
				fs_debugParams(params,"",true);
				break;
		}
	} else {	// command answer received
		status = PARAM_INT(v_each(params,0));
		if (status != FS_SS_OK) {
			WARN("cmd(%d) failed, status: %d",cmd,status);
			if (fc_auto) {
				WARN("Exiting...");
				fc_clientLoop = false;
			}
			return;
		} 
		switch (cmd) {
			case FS_SCCL_STATE:
				MSG("state recovered");
				fc_autoInfo.status = PARAM_INT(v_each(params,0));
				break;
			case FS_SCCL_PERS_INFO:
				if (PARAM_INT(v_each(params,0)) == 0) {
					MSG("my info loaded");
				} else {
					MSG("opponent's info loaded");
				}
				break;
			case FS_SCCL_PERS_PARTS:
				if (PARAM_INT(v_each(params,0)) == 0) {
					MSG("my config loaded");
				} else {
					MSG("opponent's config loaded");
				}
				break;
			default:
				break;
		}
		MSG("cmd(%d) ok",cmd);
	}
}

// ======================================================================================================================

void parseCmdLine(int argc, char *argv[]) {
	int opt;
	while ((opt = getopt(argc, argv, "h:p:l:d:a:s:c:")) != -1) {
		switch ((unsigned char)opt) {
			case 'h':
				fc_host = strdup(optarg);
				break;
			case 'p':
				fc_port = atoi(optarg);
				break;
			case 'l':
				fc_debugLevel = atoi(optarg);
				break;
			case 'd':
				fc_debugLog = strdup(optarg);
				break;
			case 'a':
				fc_auto = atoi(optarg);
				break;
			case 's':
				fc_autoSleep = atoi(optarg);
				break;
			case 'c':
				fc_cmd = strdup(optarg);
				break;
		}
	}
}

int main(int argc, char *argv[]) {
	if (!fc_host || !fc_port) {
		MSG("Usage: %s -h <host> -p <port> [-l <debug level>] [-d <debug log>] [-a <auto>] [-s <sleep>] [-c <cmd>]",argv[0]);
	} else {
		if (fc_init(fc_host,fc_port) == OK) fc_run();
		fc_done();
	}
}
