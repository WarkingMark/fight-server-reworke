/* 
 * modifed: igorpauk 2017-18
 */

#include "common.h"
#include "srv.h"
#include "pers.h"
#include "io.h"
#include "luaif.h"
#include "fight.h"


fs_fight_t *fs_fightCreate(int id) {
	fs_fight_t   *fight;
	static int   fightId = 0;
	int          socks[2];

	if (socketpair(AF_UNIX,SOCK_STREAM,0,socks) != 0) {
		WARN("socketpair() failed: %s",strerror(errno));
		return NULL;
	}
	fight = malloc(sizeof(fs_fight_t));
	if (!fight) {
		WARN("malloc() failed");
		return NULL;
	}
	if (id <= 0) id = ++fightId;
	fightId = MAX(fightId,id);
	memset(fight,0,sizeof(fs_fight_t));
	fight->id = id;
	fight->ctime = fight->mtime = fs_stime;
	fight->otime = 0;
	fight->intPipeSock = socks[0];
	fight->extPipeSock = socks[1];
	fight->clientVec = v_init(NULL);
	fight->persVec = v_init(NULL);
	fight->log = fs_fightLogCreate();
	
	fight->LuaParams = v_init(NULL);
	fight->persLuaParams = v_init(NULL);

	luaif_init(fight);
	pthread_mutex_init(&(fight->mutex),NULL);
	pthread_mutex_init(&(fight->mutex_cl),NULL);
	pthread_mutex_init(&(fight->mutex_lua),NULL);
	return fight;
}

errno_t fs_fightDelete(fs_fight_t *fight) {
	fs_client_t  *client;
	fs_pers_t    *pers;
	fs_luaParam_t *luaParam;

	if (!fight) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	fs_fightThreadStop(fight);
	while ((client = v_pop(fight->clientVec))) fs_clientDelete(client);
	while ((pers = v_pop(fight->persVec))) fs_persDelete(pers);
	v_free(fight->clientVec);
	v_free(fight->persVec);
	while ((luaParam = v_pop(fight->LuaParams))) fs_luaParamDelete(luaParam);
	while ((luaParam = v_pop(fight->persLuaParams))) fs_luaParamDelete(luaParam);
	v_free(fight->LuaParams);
	v_free(fight->persLuaParams);
	if (fight->log) fs_fightLogDelete(fight->log);
	close(fight->intPipeSock);
	close(fight->extPipeSock);

	luaif_done(fight);
	pthread_mutex_destroy(&(fight->mutex));
	pthread_mutex_destroy(&(fight->mutex_cl));
	pthread_mutex_destroy(&(fight->mutex_lua));
	free(fight);
	return OK;
}

fs_fight_t *fs_fightGetById(int id) {
	fs_fight_t   *fight;
	viter_t      vi;

	v_reset(fs_fightVec,&vi);
	while ((fight = v_each(fs_fightVec,&vi))) {
		if (fight->id == id) break;
	}
	return fight;
}

fs_pers_t *fs_fightPersGetById(fs_fight_t *fight, int id) {
	fs_pers_t    *pers;
	viter_t      vi;

	if (!fight) {
		WARN("Invalid arguments");
		return NULL;
	}
	v_reset(fight->persVec,&vi);
	while ((pers = v_each(fight->persVec,&vi))) {
		if (pers->id == id) break;
	}
	return pers;
}

/*new*/
fs_fightInfo_t *fs_fightInfoGetByIdEff(int id) {
	fs_fightInfo_t *info;
	viter_t        vi;
	fs_fight_t   *fight;
	v_freeData(fs_fightInfoVec1);	
	fight = fs_fightGetById(id);
	fs_fightInfoSync(fight);	 
	if (fight->status == FS_FS_RUNNING) {			
		v_reset(fs_fightInfoVec1,&vi);
		while ((info = v_each(fs_fightInfoVec1,&vi))) {	
			if (info->id == id) break;
		}	 
	}		
	return info;
}

fs_fightLog_t *fs_fightLogCreate(void) {
	fs_fightLog_t          *log;

	log = malloc(sizeof(fs_fightLog_t));
	if (!log) {
		WARN("malloc() failed");
		return NULL;
	}
	memset(log,0,sizeof(fs_fightLog_t));
	v_init(&(log->dataVec));

	pthread_mutex_init(&(log->mutex),NULL);
	return log;
}

errno_t fs_fightLogDelete(fs_fightLog_t *log) {
	fs_fightLogData_t *data;

	if (!log) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	v_reset(&(log->dataVec),0);
	while ((data = v_each(&(log->dataVec),0))) {
		free(data->s1);
	}
	v_freeData(&(log->dataVec));

	pthread_mutex_destroy(&(log->mutex));
	free(log);
	return OK;
}

fs_fightInfo_t *fs_fightInfoCreate(int id) {
	fs_fightInfo_t         *info;

	info = malloc(sizeof(fs_fightInfo_t));
	if (!info) {
		WARN("malloc() failed");
		return NULL;
	}
	memset(info,0,sizeof(fs_fightInfo_t));
	info->id = id;
	info->ctime = fs_stime;
	info->persDataVec = v_init(NULL);
	info->effDataVec = v_init(NULL);
	info->cmbDataVec = v_init(NULL);
	return info;
}

errno_t fs_fightInfoDelete(fs_fightInfo_t *info) {
	if (!info) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	v_freeData(info->persDataVec);
	v_freeData(info->effDataVec);
	v_freeData(info->cmbDataVec);
	v_free(info->persDataVec);
	v_free(info->effDataVec);
	v_free(info->cmbDataVec);
	if (info->log) fs_fightLogDelete(info->log);
	free(info);
	return OK;
}

fs_fightInfo_t *fs_fightInfoGetById(int id) {
	fs_fightInfo_t *info;
	viter_t        vi;

	v_reset(fs_fightInfoVec,&vi);
	while ((info = v_each(fs_fightInfoVec,&vi))) {	
		if (info->id == id) break;
	}
	return info;
}

errno_t fs_fightLockMutex(fs_fight_t *fight) {
	if (!fight) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	if (pthread_mutex_lock(&(fight->mutex)) != 0) {
		WARN("pthread_mutex_lock() failed: %s",strerror(errno));
		return ERR_SYS_ERROR;
	}
	return OK;
}

errno_t fs_fightUnlockMutex(fs_fight_t *fight) {
	if (!fight) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	if (pthread_mutex_unlock(&(fight->mutex)) != 0) {
		WARN("pthread_mutex_unlock() failed: %s",strerror(errno));
		return ERR_SYS_ERROR;
	}
	return OK;
}

errno_t fs_fightLogLockMutex(fs_fightLog_t *log) {
	if (!log) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	if (pthread_mutex_lock(&(log->mutex)) != 0) {
		WARN("pthread_mutex_lock() failed: %s",strerror(errno));
		return ERR_SYS_ERROR;
	}
	return OK;
}

errno_t fs_fightLogUnlockMutex(fs_fightLog_t *log) {
	if (!log) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	if (pthread_mutex_unlock(&(log->mutex)) != 0) {
		WARN("pthread_mutex_unlock() failed: %s",strerror(errno));
		return ERR_SYS_ERROR;
	}
	return OK;
}


// ========================================= fight thread ========================================= //

errno_t fs_fightThreadStart(fs_fight_t *fight) {
	pthread_attr_t    ta;
	
	if (!fight) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	if (fight->flags & FS_FF_TH_STARTED) return ERR_WRONG_STATE;
	pthread_attr_init(&ta);
	pthread_attr_setdetachstate(&ta,PTHREAD_CREATE_JOINABLE);
	if (pthread_create(&(fight->th),&ta, (void * (*)(void *))fs_fightThreadRoutine, (void *)fight) != 0) {
		WARN("pthread_create() failed: %s",strerror(errno));
		return ERR_SYS_ERROR;
	}
	fight->flags |= FS_FF_TH_STARTED;
	return OK;
}

errno_t fs_fightThreadRoutine(fs_fight_t *fight) {
	fs_client_t       *client;
	fs_pers_t         *pers;
	int               stime, ptime;
	int               pairMade, persActive, persDead, persPending;
	fs_fightSignal_t  sig = FS_FSIG_NONE;
	bool              fightLoop = true;
	struct pollfd     ufds[MAX_POLL_SIZE];
	int               nfds, ecnt;

	if (!fight) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}

	INFO("FIGHT RUNNING [id: %d]",fight->id);
	fight->status = FS_FS_RUNNING;
	while (1) {	// main fight loop
		stime = fs_stime;
		ufds[0].fd = fight->intPipeSock;
		ufds[0].events = POLLIN | POLLPRI;
		ufds[0].revents = 0;
		nfds = 1;
		pthread_mutex_lock(&(fight->mutex_cl));
		v_reset(fight->clientVec,0);
		while ((client = v_each(fight->clientVec,0))) {		
			fs_clientWrite(client);
 			if (client->flags & FS_CF_DISCONN) {
 				v_remove(fight->clientVec,client);
 				fs_clientDelete(client);
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
		pthread_mutex_unlock(&(fight->mutex_cl));
		if (!fightLoop) break;
		ecnt = poll(ufds,nfds,100); // Обработка раньше 1000 mc сейчас 100
		if ((ecnt == -1) && (errno != EINTR)) WARN("poll() failed: %s",strerror(errno));
		if (ecnt > 0) {
			fight->mtime = stime;
			if (ufds[0].revents & (POLLIN | POLLPRI)) {	// signal pending
				memset(&sig,0,sizeof(sig));
				safeRead(fight->intPipeSock,(char *)(&sig),1);
			}
			pthread_mutex_lock(&(fight->mutex_cl));
			v_reset(fight->clientVec,0);
			while ((client = v_each(fight->clientVec,0))) {			
				if ((client->pidx < 0) || (client->pidx >= MAX_POLL_SIZE)) continue;
				if (ufds[client->pidx].revents & (POLLERR | POLLHUP | POLLNVAL)) client->flags |= FS_CF_DISCONN;
				if (!(ufds[client->pidx].revents & (POLLIN | POLLPRI))) continue;
				fs_clientRead(client);
				fs_processClient(client);
			}
			pthread_mutex_unlock(&(fight->mutex_cl));
		}
		if (fight->status == FS_FS_RUNNING) {
			fs_fightUpdateState(fight,&pairMade,&persActive,&persDead,&persPending);
			fs_fightBroadcastStateChange(fight);
			if (!pairMade && !persActive && persDead) {	// normal fight finish
				DEBUG("fight over");
				
				/*fs_fightLockMutex(fight);
				v_reset(fight->persVec,0);
				while ((pers = v_each(fight->persVec,0))) {		
					if(pers->new_pers_id){
						fs_persSetEvent(pers,FS_PE_MYFIGHTRETURN,0);
						fs_persSetEvent(pers,FS_PE_OPPWAIT,0);
						pers->new_pers_id = 0;
					}
				}
				fs_fightUnlockMutex(fight);*/
				
				fs_fightGenerateInfo(fight);
				fight->status = FS_FS_OVER;
				fs_feedbackData("si","FIGHT_FINISH",fight->id);
			}
			ptime = stime;
		} else if (!v_size(fight->clientVec) || (ptime <= (stime - FIGHT_OVER_TTL))) fightLoop = false;
		if (sig == FS_FSIG_STOP) {	// got the stop signal
			DEBUG("got FS_FSIG_STOP");
			fs_fightLockMutex(fight);
			v_reset(fight->persVec,0);
			while ((pers = v_each(fight->persVec,0))) {			
				fs_persSetEvent(pers,FS_PE_SRVSHUTDOWN,0);
			}
			fs_fightUnlockMutex(fight);
			fightLoop = false;
			fs_feedbackData("si","FIGHT_ABORT",fight->id);
		}
		//usleep(3);
	}

	fight->status = FS_FS_FINISHED;
	INFO("FIGHT FINISHED [id: %d]",fight->id);
	return OK;
}

errno_t fs_fightThreadStop(fs_fight_t *fight) {
	if (!fight) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	if (!(fight->flags & FS_FF_TH_STARTED) || (fight->flags & FS_FF_TH_STOPPED)) return ERR_WRONG_STATE;
	fs_fightSignal(fight,FS_FSIG_STOP);
	if (pthread_join(fight->th,NULL) != 0) {
		WARN("pthread_join() failed: %s",strerror(errno));
		return ERR_SYS_ERROR;
	}
	fight->flags |= FS_FF_TH_STOPPED;
	return OK;
}

errno_t fs_fightSignal(fs_fight_t *fight, fs_fightSignal_t sig) {
	if (!fight) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	return safeWrite(fight->extPipeSock,(char *)(&sig),1);
}

/*new*/
errno_t fs_fightPersStatus(fs_pers_t *pers) { 	
	if(!pers) return OK;
	switch (pers->status) {		 
		case FS_PS_ACTIVE: 
			pers->statusl = FS_PLS_ACTIVE;
			//return FS_PLS_ACTIVE;
			break;
		case FS_PS_PASSIVE: 
			pers->statusl = FS_PLS_PASSIVE;
			//return FS_PLS_PASSIVE;
			break;		
		case FS_PS_FIGHTING: 
			pers->statusl = FS_PLS_PENDING;
			return FS_PLS_PENDING;
			break;
		case FS_PS_PENDING: 
			//return 1;
			break;
		case FS_PS_CREATED: 
			//return FS_PS_CREATED;
			break;
		case FS_PS_DEAD: 
			//return FS_PS_DEAD;
			break;
	} 
	
	return OK;
}

//after lock change dead cnt
errno_t fs_fightDeadCnt(fs_fight_t *fight, int team, int add) {
	if (!fight) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	
	if (fight->status != FS_FS_RUNNING) {
		return FS_SS_OK;
	}
	
	if(team == 1){
		fight->Team_1_DeadCnt += add;
	}else{
		fight->Team_2_DeadCnt += add;
	}
	fs_fightDeadCntSendAll(fight);
	return OK;
}

//for all users
errno_t fs_fightDeadCntSendAll(fs_fight_t *fight) {
	fs_pers_t *pers;
	if (!fight) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	
	if (fight->status != FS_FS_RUNNING) {
		return FS_SS_OK;
	}
	
	v_reset(fight->persVec,0);
	while ((pers = v_each(fight->persVec,0))) {
		if(pers->flags & FS_PF_BOT) continue;
		if(pers->_dieEventCnt >= 1) continue;
		fs_persSetEvent(pers,FS_PE_DEADCNT,"ii",fight->Team_1_DeadCnt,fight->Team_2_DeadCnt);
		pers->_dieEventCnt++;
	}
	return OK;
}

//for subscrible users
errno_t fs_fightDeadCntSend(fs_fight_t *fight, fs_pers_t *pers) {
	if (!fight) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	
	if (fight->status != FS_FS_RUNNING) {
		return FS_SS_OK;
	}
	
	fs_persSetEvent(pers,FS_PE_DEADCNT,"ii",fight->Team_1_DeadCnt,fight->Team_2_DeadCnt);
	return OK;
}

// ================================================================================== //

errno_t fs_fightUpdateState(fs_fight_t *fight, int *pairMade, int *persActive, int *persDead, int *persPending) {
	fs_pers_t    *pers1, *pers2, *pers3;
	int          lvlDiff, stime, idx;
	vector_t     persVec1, persVec2; /*, persVec3*/

	if (!fight || !pairMade || !persActive || !persDead) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	
	if(fight->otime > 0){
		if(fight->ctime + fight->otime > fight->mtime){
			return OK;
		}
	}
	
	
	stime = fs_stime;
	*pairMade = *persActive = *persDead = *persPending = 0;
	v_init(&persVec1);
	v_init(&persVec2);
	//v_init(&persVec3);

	// updating personage states
	fs_fightLockMutex(fight);
	v_reset(fight->persVec,0);
	while ((pers1 = v_each(fight->persVec,0))) {
		if (fight->utime < stime) fs_persRecalcEffects(pers1);
		if ((pers1->status == FS_PS_CREATED) && (pers1->ctime <= (stime - PERS_STUCK_TTL))) {	// personage stuck timeout
			pers1->flags |= FS_PF_TIMEOUTKILL;
			fs_persDie(pers1,NULL);
			continue;
		}
		if ((pers1->status == FS_PS_ACTIVE) && (pers1->mtime <= (stime - fight->timeout))) {	// personage attack timeout
			/*if(pers1->id == 1) {
				fs_persAttack(pers1,1,0);
				continue;
			}*/
			pers1->timeoutCnt++;
			fs_persSetEvent(pers1,FS_PE_ATTACKTIMEOUT,"i",pers1->id);
			fs_persSetEvent(pers1->opponent,FS_PE_ATTACKTIMEOUT,"i",pers1->id);
			if (pers1->timeoutCnt >= FIGHT_MAX_TIMEOUT_CNT) {
				if(pers1->autoKick == 1){
					fs_persAttack(pers1,randInt(1,3,NULL),0);
					continue;
				}else{
					pers1->flags |= FS_PF_TIMEOUTKILL;
					fs_persDie(pers1,pers1->opponent);
				}
			} else {
				fs_persDischarge(pers1,FS_PDT_PHYSICAL); //Фикс чардж, если пропустили ход эффекты от чардж должны отняться
				fs_persGetTurn(pers1->opponent);
			}
			continue;
		}
		if(pers1->status == FS_PS_ACTIVE && pers_is_player(pers1)) {
			fs_persPersIntelligence(pers1);
			fs_fightAutoActiveEffect(pers1); //Авто-активация эффекетов
		}
		if (pers_is_bot(pers1)) v_push(&persVec1,pers1);
		/*if (pers_is_bot(pers1)){
			v_push(&persVec1,pers1);
		}else{
			v_push(&persVec3,pers1);
		}*/
	}
	fs_fightUnlockMutex(fight);

	// running bots
	v_reset(&persVec1,0);
	while ((pers1 = v_each(&persVec1,0))) fs_persBotIntelligence(pers1);
	v_zero(&persVec1);

	// running bots
	//v_reset(&persVec3,0);
	//while ((pers3 = v_each(&persVec3,0))) fs_persPersIntelligence(pers3);
	//v_zero(&persVec3);

	// forming new pairs
	fs_fightLockMutex(fight);
	v_reset(fight->persVec,0);
	while ((pers1 = v_each(fight->persVec,0))) {
		if (pers1->status != FS_PS_FIGHTING) {
			if (pers1->status == FS_PS_ACTIVE) (*persActive)++;
			else if (pers1->status == FS_PS_DEAD) (*persDead)++;
			continue;
		}
		v_push(&persVec1,pers1);
	}
	while (v_size(&persVec1)) {
		idx = randInt(0,v_size(&persVec1)-1,NULL);
		pers1 = v_elem(&persVec1,idx);
		v_remove_at(&persVec1,idx,0);
		v_reset(&persVec1,0);
		while ((pers2 = v_each(&persVec1,0))) {
			if ((pers2 == pers1) || (pers2->teamNum == pers1->teamNum) || ((pers2->flags & FS_PF_STUNNED) && (pers1->flags & FS_PF_STUNNED))) continue;
			v_push(&persVec2,pers2);
		}
		if (!v_size(&persVec2)) {	// no opponent found
			pers1->opponent = NULL;
			if (!(pers1->flags & FS_PF_OPPWAIT)) {
				pers1->flags |= FS_PF_OPPWAIT;
				fs_persSetEvent(pers1,FS_PE_OPPWAIT,0);
			}
			(*persPending)++;
			continue;
		}
		idx = randInt(0,v_size(&persVec2)-1,NULL);
		pers2 = v_elem(&persVec2,idx);
		v_remove(&persVec1,pers2);
		v_zero(&persVec2);
		pers1->flags &= ~FS_PF_OPPWAIT;
		pers2->flags &= ~FS_PF_OPPWAIT;
		if (pers1->opponent != pers2) fs_persSetEvent(pers1,FS_PE_OPPNEW,"i",pers2->id);
		if (pers2->opponent != pers1) fs_persSetEvent(pers2,FS_PE_OPPNEW,"i",pers1->id);
		pers1->opponent = pers2;
		pers2->opponent = pers1;

		//fs_persGetTurn((pers1->canAttack ? pers1 : (pers2->canAttack ? pers2 : (pers1->mtime <= pers2->mtime ? pers1: pers2))), true);
		if(!pers1->_inicHod && !pers2->_inicHod) {
			do{
				pers1->_inicHod = true;
				pers2->_inicHod = true;

				// INITIATIVE + DEX contribution (formula from formulas.h)
				double pers1_inic = calc_initiative(PERS_SKILL(pers1, FS_SK_INITIATIVE), PERS_SKILL(pers1, FS_SK_DEX));
				double pers2_inic = calc_initiative(PERS_SKILL(pers2, FS_SK_INITIATIVE), PERS_SKILL(pers2, FS_SK_DEX));

				if(pers_is_player(pers1) && pers_is_bot(pers2)){
					lvlDiff = MIN(MAX(PERS_LEVEL_SAFE(pers1) - PERS_LEVEL_SAFE(pers2), 0), 10);
					if(pers1_inic > (pow(1.4,lvlDiff))*50) {
						fs_persGetTurn(pers1);
						break;
					}
				}
				if(pers_is_player(pers2) && pers_is_bot(pers1)){
					lvlDiff = MIN(MAX(PERS_LEVEL_SAFE(pers1) - PERS_LEVEL_SAFE(pers2), 0), 10);
					if(pers2_inic > (pow(1.4,lvlDiff))*50) {
						fs_persGetTurn(pers2);
						break;
					}
				}

				if(pers1_inic == pers2_inic){
					fs_persGetTurn(pers1->mtime <= pers2->mtime ? pers1: pers2);
				}else{
					fs_persGetTurn(pers1_inic >= pers2_inic ? pers1: pers2);
				}
			}while(0);
		}else{
			fs_persGetTurn(pers1->mtime <= pers2->mtime ? pers1: pers2);
		}
        
        //DEBUGXXX("PERSMTIME %d %d (pers1->mtime: %d, pers1->mtime: %d)",(int)pers1->canAttack, (int)pers2->canAttack, pers1->mtime,pers2->mtime);
		//fs_persGetTurn(pers1->mtime <= pers2->mtime ? pers1: pers2);
		(*pairMade)++;
	}
	fight->utime = stime;
	//fs_fightDeadCntSendAll(fight); //send Info For all
	fs_fightUnlockMutex(fight);
	return OK;
}

errno_t fs_fightBroadcastStateChange(fs_fight_t *fight) {
	viter_t      vi1, vi2;
	fs_pers_t    *pers1, *pers2;
	fs_client_t  *client;
	bool         fStatus, fStatusAll, fHp;

	if (!fight) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	
	if(fight->otime > 0){
		if(fight->ctime + fight->otime > fight->mtime){
			return OK;
		}
	}
	
	fs_fightLockMutex(fight);
	v_reset(fight->persVec,&vi1);
	while ((pers1 = v_each(fight->persVec,&vi1))) {
		pers1->_dmgEventCnt = 0;
		pers1->_dieEventCnt = 0;
		if (pers1->status == FS_PS_CREATED) continue;
		fStatus = (pers1->status != pers1->statusOld) || (PF_CLMASK(pers1->flags) != PF_CLMASK(pers1->flagsOld));
		fStatusAll = fStatus && ((pers1->statusOld == FS_PS_CREATED) || (pers1->statusOld == FS_PS_DEAD) || (pers1->status == FS_PS_DEAD));
		fHp = (PERS_HP(pers1) != pers1->hpOld) || (PERS_HPMAX(pers1) != pers1->hpMaxOld) || (PERS_MP(pers1) != pers1->mpOld) || (PERS_MPMAX(pers1) != pers1->mpMaxOld);
		if (!fStatus && !fHp) continue;
		v_reset(fight->persVec,&vi2);
		while ((pers2 = v_each(fight->persVec,&vi2))) {		
			client = pers2->client;
			if (!client) continue;
			if ((pers1 != pers2) && (!PERS_OPP_ID(pers1) || (pers1->opponent != pers2))) {	// limiting state change traffic
				if (!fStatusAll && !fHp) continue;
				if (!fStatusAll && (v_size(client->outPacketVec) > 5)) continue;	// don't say about hp if there are some packets in the queue
			}
			fs_fightPersStatus(pers1); /*new*/
			fs_persSetEvent(pers2,FS_PE_FIGHTSTATE,"iiiiiiii",pers1->id,pers1->status,pers1->statusl,PF_CLMASK(pers1->flags),PERS_HP(pers1),PERS_HPMAX(pers1),PERS_MP(pers1),PERS_MPMAX(pers1));
		}
		pers1->statusOld = pers1->status;
		pers1->flagsOld = pers1->flags;
		pers1->hpOld = PERS_HP(pers1);
		pers1->hpMaxOld = PERS_HPMAX(pers1);
		pers1->mpOld = PERS_MP(pers1);
		pers1->mpMaxOld = PERS_MPMAX(pers1);
	}
	fs_fightUnlockMutex(fight);
	return OK;
}

/*new*/
errno_t fs_fightAutoActiveEffect(fs_pers_t *pers) {
	fs_persEff_t      *eff;  
	fs_pers_t         *target;
	int usageStatus = -5000;
	
	if (!pers) return ERR_WRONG_ARGS;
	
	//DEBUGX("[aetime: %d] [fstime: %d]",pers->aetime, fs_stime);
	
	//if(pers->status != FS_PS_ACTIVE) return OK;
	//if((pers->atime > (stime - 1))) return OK;
	
	if (pers->aetime + 15 > fs_stime) return OK;
	if (pers->fight->status != FS_FS_RUNNING) return FS_SS_WRONG_STATE;	 	
 
 	v_reset(pers->effVec,0);
	while ((eff = v_each(pers->effVec,0))) {
		if (eff->cnt <= 0) continue;
		if (eff->flags & FS_PEF_ACTIVE) continue; //Эффект уже активен!
		if (!eff->probAuto) continue;
		if (!(eff->flags & FS_PEF_TARGETSELF) && !(eff->flags & FS_PEF_TARGETOPP)) continue; //Пропускаем такие "божественные"
		if (!randRoll(eff->probAuto,NULL)) continue; //В этот раз божественный не кастанул
		if (eff->flags & FS_PEF_TARGETSELF){
			target = pers;
		}else if(eff->flags & FS_PEF_TARGETOPP){
			if(pers->opponent) target = pers->opponent;
		}
		if (target) {
			fs___persActivateEffectCheck(pers, eff, target);
		}
	}
	
	pers->aetime = fs_stime;
	
	return OK;
}

errno_t fs_fightSaveLog(fs_pers_t *pers, fs_fightLogCode_t code, int i1, int i2, int i3, char *s1, bool silent) {
	fs_fightLog_t     *log;
	fs_fightLogData_t *data;

	if (!pers || !pers->fight) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	log = pers->fight->log;
	if (!log) return ERR_WRONG_STATE;
	data = malloc(sizeof(fs_fightLogData_t));
	if (!data) {
		WARN("malloc() failed");
		return ERR_NO_MEM;
	}
	memset(data,0,sizeof(fs_fightLogData_t));
	data->ctime = fs_stime;
	data->persId = pers->id;
	data->persHp = PERS_HP(pers);
	data->persHpMax = PERS_HPMAX(pers);
	if (pers->opponent) {
		data->oppId = pers->opponent->id;
		data->oppHp = PERS_HP(pers->opponent);
		data->oppHpMax = PERS_HPMAX(pers->opponent);
	}
	data->code = code;
	data->i1 = i1;
	data->i2 = i2;
	data->i3 = i3;
	if (s1) data->s1 = strdup(s1);

	fs_fightLogLockMutex(log);
	v_push(&(log->dataVec),data);
	fs_fightLogUnlockMutex(log);
	if (!silent) {
		fs_persSetEvent(pers,FS_PE_FIGHTLOG,"iiiiiiis",data->ctime,data->persId,data->oppId,data->code,data->i1,data->i2,data->i3,data->s1);
		if (pers->opponent) fs_persSetEvent(pers->opponent,FS_PE_FIGHTLOG,"iiiiiiis",data->ctime,data->persId,data->oppId,data->code,data->i1,data->i2,data->i3,data->s1);
	}
	return OK;
}

errno_t fs_fightGenerateInfo(fs_fight_t *fight) {
	fs_fightInfo_t         *info;
	fs_pers_t              *pers;
	fs_persEff_t           *eff;
	fs_persCmb_t           *cmb;
	fs_fightInfoPersData_t *persData;
	fs_fightInfoEffData_t  *effData;
	fs_fightInfoCmbData_t  *cmbData;
	
	int                    winnerTeam = 0;

	if (!fight) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	fs_fightLockMutex(fight);
	
	v_reset(fight->persVec,0);
	while ((pers = v_each(fight->persVec,0))) {
		if (pers->status != FS_PS_FIGHTING) continue;
		winnerTeam = pers->teamNum;
		break;
	}
	
	info = fs_fightInfoCreate(fight->id);
	info->fctime = fight->ctime;
	info->frtime = fight->mtime - fight->ctime;
	info->winnerTeam = winnerTeam;
	v_reset(fight->persVec,0);
	while ((pers = v_each(fight->persVec,0))) {
		persData = malloc(sizeof(fs_fightInfoPersData_t));
		memset(persData,0,sizeof(fs_fightInfoPersData_t));
		persData->id = pers->id;
		persData->status = pers->status;
		persData->flags = pers->flags;
		persData->teamNum = pers->teamNum;
		persData->dmg = pers->dmg;
		persData->heal = pers->heal;
		persData->exp = round(pers->exp);
		persData->honor = round(pers->honor);
		persData->hp = PERS_HP(pers);
		persData->mp = PERS_MP(pers);
		persData->killCnt = pers->killCnt;
		persData->enemyKillCnt = pers->enemyKillCnt;
		persData->arrowsCnt = pers->arrowsCnt;
		v_push(info->persDataVec,persData);
		v_reset(pers->effVec,0);
		while ((eff = v_each(pers->effVec,0))) {
			if (eff->flags & (FS_PEF_ACTIVE | FS_PEF_AUX | FS_PEF_SPELL)) continue;
			effData = malloc(sizeof(fs_fightInfoEffData_t));
			memset(effData,0,sizeof(fs_fightInfoEffData_t));
			effData->id = eff->id;
			effData->persId = pers->id;
			effData->cnt = eff->cnt;
			effData->cdrtime = eff->cdrtime;
			effData->slotNum = eff->slotNum;
			effData->subSlot = eff->subSlot;
			v_push(info->effDataVec,effData);
		}
		v_reset(pers->cmbVec,0);
		while ((cmb = v_each(pers->cmbVec,0))) {
			cmbData = malloc(sizeof(fs_fightInfoCmbData_t));
			memset(cmbData,0,sizeof(fs_fightInfoCmbData_t));
			cmbData->id = cmb->id;
			cmbData->persId = pers->id;
			cmbData->useCnt = cmb->useCnt;
			v_push(info->cmbDataVec,cmbData);
		}
		
		v_freeData(pers->followPers);
		//v_free(pers->followPers);
		
		if(pers->new_pers_id){
			pers->new_pers_id = pers->id;
			fs_persSetEvent(pers,FS_PE_MYFIGHTRETURN,0);
		}
		
		fs_persSetEvent(pers,FS_PE_FIGHTOVER,"i",winnerTeam);
	}
	info->log = fight->log;
	fight->log = NULL;
	fs_fightUnlockMutex(fight);

	fs_srvLockMutex();
	v_push(fs_fightInfoVec,info);
	fs_srvUnlockMutex();
	INFO("FIGHTINFO GENERATED [id: %d]",info->id);
	return OK;
}

/*
fs_fightInfo_t *fs_fightInfoGetByIdEff(int id) {
	fs_fightInfo_t *info;
	viter_t        vi;
	fs_fight_t   *fight;
	v_freeData(fs_fightInfoVec1);	
	fight = fs_fightGetById(id);
	fs_fightInfoSync(fight);	 
	if (fight->status == FS_FS_RUNNING) {			
		v_reset(fs_fightInfoVec1,&vi);
		while ((info = v_each(fs_fightInfoVec1,&vi))) {	
			if (info->id == id) break;
		}	 
	}		
	return info;
}*/

fs_fightInfo_t *fs_fightInfoCreate1(int id) {
	fs_fightInfo_t         *info1;
	 
	info1 = malloc(sizeof(fs_fightInfo_t));
	if (!info1) {
		WARN("malloc() failed");
		return NULL;
	}
	memset(info1,0,sizeof(fs_fightInfo_t));
	info1->id = id;
	info1->ctime = fs_stime;
    info1->persDataVec = v_init(NULL);
	info1->effDataVec = v_init(NULL);
	info1->cmbDataVec = v_init(NULL);
	return info1;
}
errno_t fs_fightInfoSync(fs_fight_t *fight) {
	fs_fightInfo_t         *dovusinfo;
	fs_pers_t              *pers;
	fs_persEff_t           *eff;
	fs_persCmb_t           *cmb;
	fs_fightInfoPersData_t *persData;
	fs_fightInfoEffData_t  *effData;
	fs_fightInfoCmbData_t  *cmbData;
	if (!fight) {
		WARN("Geçersiz Argument");
		return ERR_WRONG_ARGS;
	}	 
	fs_fightLockMutex(fight);
	dovusinfo = fs_fightInfoCreate1(fight->id);	
	v_freeData(dovusinfo->effDataVec);
	v_freeData(dovusinfo->cmbDataVec);
	v_reset(fight->persVec,0); 
	while ((pers = v_each(fight->persVec,0))) {	
		//if (pers->status != FS_PS_DEAD) continue;			 
		v_reset(pers->effVec,0);	 				
		while ((eff = v_each(pers->effVec,0))) {
			if (eff->flags & (FS_PEF_ACTIVE | FS_PEF_AUX | FS_PEF_SPELL)) continue;
			effData = malloc(sizeof(fs_fightInfoEffData_t));
			memset(effData,0,sizeof(fs_fightInfoEffData_t));
			effData->id = eff->id;
			effData->persId = pers->id;
			effData->cnt = eff->cnt;			
			effData->cdrtime = eff->cdrtime;
			effData->slotNum = eff->slotNum;
			effData->subSlot = eff->subSlot;
			v_push(dovusinfo->effDataVec,effData);
		}
		v_reset(pers->cmbVec,0);
		while ((cmb = v_each(pers->cmbVec,0))) {
			cmbData = malloc(sizeof(fs_fightInfoCmbData_t));
			memset(cmbData,0,sizeof(fs_fightInfoCmbData_t));
			cmbData->id = cmb->id;
			cmbData->persId = pers->id;
			cmbData->useCnt = cmb->useCnt;
			v_push(dovusinfo->cmbDataVec,cmbData);
		}		 
	}	 
		fs_fightUnlockMutex(fight); 
		v_push(fs_fightInfoVec1,dovusinfo);	
	 
	return OK;
}


//вызов только под pthread_mutex_lock
errno_t fs_fightLuaInitParams(fs_fight_t *fight) {
	fs_pers_t *pers;
	lua_State    *L;
	fs_luaParam_t *luaParam;
	if (!fight) {
		return 0;
	}
	
	if (fight->status != FS_FS_RUNNING) {
		return 0;
	}
	
	L = fight->L;
	v_reset(fight->persLuaParams,0);
	while ((luaParam = v_each(fight->persLuaParams,0))) {
		DEBUG("ULUA: %s", luaParam->param);
		//luaL_loadstring(L, luaParam->param);
		luaL_dostring(L, luaParam->param); //Подгружаем объекты игроков
	}
	v_reset(fight->LuaParams,0);
	while ((luaParam = v_each(fight->LuaParams,0))) {
		DEBUG("FLUA: %s", luaParam->param);
		//luaL_loadstring(L, luaParam->param);
		luaL_dostring(L, luaParam->param); //Подгружаем переменные боя
	}
	return OK;
}

fs_luaParam_t *fs_luaParamCreate(char *param) {
	fs_luaParam_t         *luaParam;

	luaParam = malloc(sizeof(fs_luaParam_t));
	if (!luaParam) {
		WARN("malloc() failed");
		return NULL;
	}
	memset(luaParam,0,sizeof(fs_luaParam_t));
	luaParam->param = strdup(param);
	return luaParam;
}

errno_t fs_luaParamDelete(fs_luaParam_t *luaParam) {
	if (!luaParam) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	free(luaParam->param);
	free(luaParam);
	return OK;
}