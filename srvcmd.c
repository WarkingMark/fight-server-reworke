/* 
 * modifed: igorpauk 2017-18
 */

#include "common.h"
#include "io.h"
#include "srv.h"
#include "pers.h"
#include "fight.h"
#include "luaif.h"
#include "srvcmd.h"

// ctrl command handler table
fs_srvCmdHandler_t    fs_ctrlCmdHanderTable[] = {
	{ FS_SC_SYNC_TIME,          fs_cmd_SC_SYNC_TIME,          0 },
	{ FS_SCCT_SRV_INFO,         fs_cmd_SCCT_SRV_INFO,         1 },
	{ FS_SCCT_DEBUG_LEVEL,      fs_cmd_SCCT_DEBUG_LEVEL,      0 },
	{ FS_SCCT_GET_SRV_OPTS,     fs_cmd_SCCT_GET_SRV_OPTS,     0 },
	{ FS_SCCT_SET_SRV_OPTS,     fs_cmd_SCCT_SET_SRV_OPTS,     0 },
	{ FS_SCCT_GET_FIGHTS,       fs_cmd_SCCT_GET_FIGHTS,       1 },
	{ FS_SCCT_CREATE_FIGHT,     fs_cmd_SCCT_CREATE_FIGHT,     1 },
	{ FS_SCCT_SET_FIGHT_PARAMS, fs_cmd_SCCT_SET_FIGHT_PARAMS, 1 },
	{ FS_SCCT_START_FIGHT,      fs_cmd_SCCT_START_FIGHT,      1 },
	{ FS_SCCT_STOP_FIGHT,       fs_cmd_SCCT_STOP_FIGHT,       1 },
	{ FS_SCCT_DELETE_FIGHT,     fs_cmd_SCCT_DELETE_FIGHT,     1 },
	{ FS_SCCT_CREATE_PERS,      fs_cmd_SCCT_CREATE_PERS,      1 },
	{ FS_SCCT_SET_SKILLS,       fs_cmd_SCCT_SET_SKILLS,       1 },
	{ FS_SCCT_BOTDMG_SKILLS,    fs_cmd_SCCT_SET_BOTDMG_SKILLS,1 },
	{ FS_SCCT_SET_PARAMS_LUA,   fs_cmd_SCCT_SET_PARAMS_LUA,   1 },
	{ FS_SCCT_SET_PARTS,        fs_cmd_SCCT_SET_PARTS,        1 },
	{ FS_SCCT_ADD_EFFECT,       fs_cmd_SCCT_ADD_EFFECT,       1 },
	{ FS_SCCT_ADD_COMBO,        fs_cmd_SCCT_ADD_COMBO,        1 },
	{ FS_SCCT_BIND_PERS,        fs_cmd_SCCT_BIND_PERS,        1 },
	{ FS_SCCT_DELETE_PERS,      fs_cmd_SCCT_DELETE_PERS,      1 },
	{ FS_SCCT_GET_FIGHTSTATE,   fs_cmd_SCCT_GET_FIGHTSTATE,   1 },
	{ FS_SCCT_GET_FIGHTLOG,     fs_cmd_SCCT_GET_FIGHTLOG,     1 },
	{ FS_SCCT_GET_FIGHTINFO,    fs_cmd_SCCT_GET_FIGHTINFO,    1 },
	{ FS_SCCT_DELETE_FIGHTINFO, fs_cmd_SCCT_DELETE_FIGHTINFO, 1 },
	{ FS_SCCT_GET_LOG_AND_EFFECTS,   fs_cmd_SCCT_GET_LOG_AND_EFFECTS,   1 },
	{ 0, NULL, 0 }
};

// client command handler table
fs_srvCmdHandler_t    fs_clCmdHanderTable[] = {
	{ FS_SC_SYNC_TIME,          fs_cmd_SC_SYNC_TIME,          0 },
	{ FS_SCCL_INIT,             fs_cmd_SCCL_INIT,             1 },
	{ FS_SCCL_STATE,            fs_cmd_SCCL_STATE,            0 },
	{ FS_SCCL_PERS_INFO,        fs_cmd_SCCL_PERS_INFO,        0 },
	{ FS_SCCL_PERS_SUBSCRIBE,   fs_cmd_SCCL_PERS_SUBSCRIBE,   0 },
	{ FS_SCCL_WATCH_FIGHT,   	fs_cmd_SCCL_WATCH_FIGHT,  	  0 },
	{ FS_SCCL_PERS_PARTS,       fs_cmd_SCCL_PERS_PARTS,       0 },
	{ FS_SCCL_ATTACK,           fs_cmd_SCCL_ATTACK,           0 },
	{ FS_SCCL_FIGHT_STATE,      fs_cmd_SCCL_FIGHT_STATE,      0 },
	{ FS_SCCL_EFFECTS,          fs_cmd_SCCL_EFFECTS,          0 },
	{ FS_SCCL_USE_EFFECT,       fs_cmd_SCCL_USE_EFFECT,       0 },
	{ FS_SCCL_CHANGE_MODE,      fs_cmd_SCCL_CHANGE_MODE,      0 },
	{ FS_SCCL_PERS_ACT_EFFECTS, fs_cmd_SCCL_PERS_ACT_EFFECTS, 0 },
	{ FS_SCCL_MYFIGHTRETURN,    fs_cmd_SCCL_MYFIGHTRETURN,    0 },
	{ FS_SCCL_EFFUPDATE,        fs_cmd_SCCL_EFFUPDATE,        0 },
	{ FS_SCCL_SKIP_TURN,        fs_cmd_SCCL_SKIP_TURN,        0 },
	{ FS_SCCL_EFFECT_SWAP_SUBSLOT,fs_cmd_SCCL_EFFECT_SWAP_SUBSLOT,0},
	{ FS_SCCL_DROP_EFFECT,		fs_cmd_SCCL_DROP_EFFECT,	  0 },
	
	//{ FS_SCCL_ENERGYCONSUM,     fs_cmd_SCCL_ENERGYCONSUM,     0 },
	//{ FS_SCCL_ENERGYREGEN,      fs_cmd_SCCL_ENERGYREGEN,      0 },
	//{ FS_SCCL_ARROWCONSUM,      fs_cmd_SCCL_ARROWCONSUM,      0 },
	{ FS_SCCL_EFFSWAP,          fs_cmd_SCCL_EFFSWAP,          0 },
	{ FS_SCCL_RESETCOMBO,       fs_cmd_SCCL_RESETCOMBO,       0 },
	{ FS_SCCL_NEWPERS,          fs_cmd_SCCL_NEWPERS,          0 },
	//{ FS_SCCL_DEADCNT,          fs_cmd_SCCL_DEADCNT,          0 },
	{ FS_SCCL_SEND_MSG,         fs_cmd_SCCL_SEND_MSG,         0 },
	{ 0, NULL, 0 }
};


errno_t fs_cmdCheckParams(fs_packet_t *packet, fs_paramType_t paramTypes[], int cnt, bool notLess, bool notStrict) {
	fs_param_t             *param;
	int                    cond;

	cond = notLess ? PARAM_COUNT(packet) < cnt: PARAM_COUNT(packet) != cnt;
	if (cond) {
		WARN((notLess ? "Too few parameters (given %d, need at least %d)": "Invalid number of parameters (given %d, need %d)"),PARAM_COUNT(packet),cnt);
		return ERR_WRONG_ARGS;
	}
	for (PARAM_RESET(packet); (param = PARAM_CURRENT(packet)); PARAM_NEXT(packet)) {
		if (notLess && (PARAM_IDX(packet) >= cnt)) break;
		if (PARAM_TYPE(param) != paramTypes[PARAM_IDX(packet)]) {
			if (notStrict) {
				if ((PARAM_TYPE(param) == PT_INT) && (paramTypes[PARAM_IDX(packet)] == PT_NINT)) continue;
				if ((PARAM_TYPE(param) == PT_NINT) && (paramTypes[PARAM_IDX(packet)] == PT_INT)) continue;
				if ((PARAM_TYPE(param) == PT_SHORTINT) && (paramTypes[PARAM_IDX(packet)] == PT_NSHORTINT)) continue;
				if ((PARAM_TYPE(param) == PT_NSHORTINT) && (paramTypes[PARAM_IDX(packet)] == PT_SHORTINT)) continue;
				if ((PARAM_TYPE(param) == PT_BIGINT) && (paramTypes[PARAM_IDX(packet)] == PT_NBIGINT)) continue;
				if ((PARAM_TYPE(param) == PT_NBIGINT) && (paramTypes[PARAM_IDX(packet)] == PT_BIGINT)) continue;
				if ((PARAM_TYPE(param) == PT_FIXED) && (paramTypes[PARAM_IDX(packet)] == PT_NFIXED)) continue;
				if ((PARAM_TYPE(param) == PT_NFIXED) && (paramTypes[PARAM_IDX(packet)] == PT_FIXED)) continue;
			}
			WARN("Invalid type for parameter #%d (must be %d, got %d)",PARAM_IDX(packet),paramTypes[PARAM_IDX(packet)],PARAM_TYPE(param));
			return ERR_WRONG_ARGS;
		}
	}
	PARAM_RESET(packet);
	return OK;
}


// ========================================= command handlers ========================================= //

// Parameters:
// #0  cmd          (type INT, val=FS_SC_SYNC_TIME)
// Answer:
// #0  cmd          (type INT, val=FS_SC_SYNC_TIME)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
// #2  timestamp    (type INT)
//
// Description: time sync
// Thread: worker, fight
fs_srvStatus_t fs_cmd_SC_SYNC_TIME(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT };

	if (fs_cmdCheckParams(inPacket,paramTypes,1,false,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	PARAM_ADD(outPacket,"i",fs_stime);
	return FS_SS_OK;
}


// ---------------------------------------------------- CTRL COMMANDS ----------------------------------------------------

// Parameters:
// #0  cmd          (type INT, val=FS_SCCT_SRV_INFO)
// Answer:
// #0  cmd          (type INT, val=FS_SCCT_SRV_INFO)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
// #2  ctime        (type INT)
// #3  rtime        (type INT)
// #4  clientCnt    (type INT)
// #5  fightCnt     (type INT)
// #6  persCnt      (type INT)
// #7  fightPersCnt (type INT)
// #8  fightInfoCnt (type INT)
//
// Description: server statistics
// Thread: worker
fs_srvStatus_t fs_cmd_SCCT_SRV_INFO(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT };
	fs_worker_t       *worker;
	fs_fight_t        *fight;
	int               clientCnt, fightCnt, persCnt, fightPersCnt, fightInfoCnt;

	if (fs_cmdCheckParams(inPacket,paramTypes,1,false,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	clientCnt = 0;
	v_reset(fs_workerVec,0);
	while ((worker = v_each(fs_workerVec,0))) {
		clientCnt += v_size(worker->clientVec);
	}
	fightCnt = v_size(fs_fightVec);
	persCnt = v_size(fs_persVec);
	fightPersCnt = 0;
	v_reset(fs_fightVec,0);
	while ((fight = v_each(fs_fightVec,0))) {
		clientCnt += v_size(fight->clientVec);
		fightPersCnt += v_size(fight->persVec);
	}
	fightInfoCnt = v_size(fs_fightInfoVec);
	PARAM_ADD(outPacket,"iiiiiii",fs_ctime,(fs_stime-fs_ctime),clientCnt,fightCnt,persCnt,fightPersCnt,fightInfoCnt);
	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCT_DEBUG_LEVEL)
// #1  level        (type INT)
// Answer:
// #0  cmd          (type INT, val=FS_SCCT_DEBUG_LEVEL)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
// #2  levelOld     (type INT)
//
// Description: create a fight instance
// Thread: worker
fs_srvStatus_t fs_cmd_SCCT_DEBUG_LEVEL(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT };
	fs_param_t        *param;
	int               level, levelOld;

	if (fs_cmdCheckParams(inPacket,paramTypes,1,true,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	levelOld = debugLevel;
	param = PARAM_NEXT(inPacket);
	if (param) {
		if (PARAM_TYPE(param) != PT_INT) {
			WARN("Wrong 'level' type: %d",PARAM_TYPE(param));
			return FS_SS_WRONG_ARGS;
		}
		level = PARAM_INT(param);
		if ((level < -1) || (level > DL_DEBUG)) return FS_SS_WRONG_DEBUG_LEVEL;
		debugLevel = level;
	}
	PARAM_ADD(outPacket,"i",levelOld);
	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCT_GET_SRV_OPTS)
// Answer:
// #0  cmd          (type INT, val=FS_SCCT_GET_SRV_OPTS)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
// #2  value        (type INT, id=fs_srvOption_t)
// #n  ...
//
// Description: get common fight options
// Thread: worker
fs_srvStatus_t fs_cmd_SCCT_GET_SRV_OPTS(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT };
	fs_param_t        *param;
	fs_srvOption_t    opt;

	if (fs_cmdCheckParams(inPacket,paramTypes,1,false,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	for (opt=0; opt<=FS_SO_MAXCODE; opt++) {
		PARAM_NEW(param);
		PARAM_ID(param) = opt;
		PARAM_SETINT(param,SRV_OPT(opt));
		PARAM_PUSH(outPacket,param);
	}
	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCT_SET_SRV_OPTS)
// #1  value        (type FIXED, id IN fs_srvOption_t)
// #n  ...
// Answer:
// #0  cmd          (type INT, val=FS_SCCT_SET_SRV_OPTS)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
//
// Description: set common fight options
// Thread: worker
fs_srvStatus_t fs_cmd_SCCT_SET_SRV_OPTS(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	return FS_SS_WRONG_ARGS;
	fs_paramType_t    paramTypes[] = { PT_INT };
	fs_param_t        *param;
	fs_srvOption_t    opt;

	if (fs_cmdCheckParams(inPacket,paramTypes,1,true,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	while ((param = PARAM_NEXT(inPacket))) {
		if (PARAM_TYPE(param) != PT_INT) {
			WARN("Wrong 'value' type: %d",PARAM_TYPE(param));
			return FS_SS_WRONG_ARGS;
		}
		opt = PARAM_ID(param);
		if ((opt < 0) || (opt > FS_SO_MAXCODE)) {
			WARN("Wrong option: %d",opt);
			return FS_SS_WRONG_DATA2;
		}
		SRV_OPT(opt) = PARAM_INT(param);
	}
	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCT_GET_FIGHTS)
// #1  fightId      (type INT, val=(0 - all fights, !=0 - defined fight))
// #2  startIdx     (type INT)
// Answer:
// #0  cmd          (type INT, val=FS_SCCT_GET_FIGHTS)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
// #2  listSize     (type INT)
// ...
// #n  fightId      (type INT)
// #+1 ctime        (type INT)
// #+2 mtime        (type INT)
// #+3 status       (type INT)
// #+4 flags        (type INT)
// #+5 persCnt      (type INT)
// ...
//
// Description: get the fight list (no more than 100 chunks at once)
// Thread: worker
fs_srvStatus_t fs_cmd_SCCT_GET_FIGHTS(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT, PT_INT, PT_INT };
	fs_fight_t        *fight;
	int               fightId, startIdx, i;
	viter_t           vi;

	if (fs_cmdCheckParams(inPacket,paramTypes,3,false,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	fightId = PARAM_INT(PARAM_NEXT(inPacket));
	startIdx = PARAM_INT(PARAM_NEXT(inPacket));
	PARAM_ADD(outPacket,"i",(fightId ? 0 : v_size(fs_fightVec)));
	if (v_jump(fs_fightVec,startIdx,&vi)) {
		for (i=0; (fight = v_each(fs_fightVec,&vi)) && (i < 100); ) {
			if (fightId && (fight->id != fightId)) continue;
			PARAM_ADD(outPacket,"iiiiii",fight->id,fight->ctime,fight->mtime,fight->status,fight->flags,v_size(fight->persVec));
			i++;
		}
	}
	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCT_CREATE_FIGHT)
// #1  fightId      (type INT)
// Answer:
// #0  cmd          (type INT, val=FS_SCCT_CREATE_FIGHT)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
//
// Description: create a fight instance
// Thread: worker
fs_srvStatus_t fs_cmd_SCCT_CREATE_FIGHT(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT, PT_INT };
	fs_fight_t        *fight;
	int               fightId;
	
	if (fs_cmdCheckParams(inPacket,paramTypes,2,false,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	fightId = PARAM_BIGINT(PARAM_NEXT(inPacket));
	
	DEBUG("FIGHT_CREATE_REQ %d",fightId);
	
	if ((fightId <= 0) || fs_fightGetById(fightId)) return FS_SS_WRONG_FIGHT;
	if (v_size(fs_fightVec) >= SRV_OPT(FS_SO_MAX_FIGHT_CNT)) return FS_SS_WRONG_STATE;
	DEBUG("FIGHT_NEW %d",fightId);
	fight = fs_fightCreate(fightId);
	if (!fight) return FS_SS_INT_ERROR;
	fight->timeout = FIGHT_DEFAULT_TIMEOUT;
	v_push(fs_fightVec,fight);
	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCT_SET_FIGHT_PARAMS)
// #1  fightId      (type INT)
// #2  timeout      (type INT, val=(0 - default)
// Answer:
// #0  cmd          (type INT, val=FS_SCCT_SET_FIGHT_PARAMS)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
//
// Description: set the fight parameters
// Thread: worker
fs_srvStatus_t fs_cmd_SCCT_SET_FIGHT_PARAMS(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT, PT_INT, PT_INT, PT_INT  };
	fs_fight_t        *fight;

	if (fs_cmdCheckParams(inPacket,paramTypes,4,false,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	fight = fs_fightGetById(PARAM_BIGINT(PARAM_NEXT(inPacket)));
	if (!fight) return FS_SS_WRONG_FIGHT;
	fight->timeout = PARAM_INT(PARAM_NEXT(inPacket));
	if (fight->timeout <= 0) fight->timeout = FIGHT_DEFAULT_TIMEOUT;
	fight->flags |= PARAM_INT(PARAM_NEXT(inPacket));
	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCT_START_FIGHT)
// #1  fightId      (type INT)
// Answer:
// #0  cmd          (type INT, val=FS_SCCT_START_FIGHT)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
//
// Description: start the fight thread
// Thread: worker
fs_srvStatus_t fs_cmd_SCCT_START_FIGHT(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT, PT_INT };
	fs_fight_t        *fight;

	if (fs_cmdCheckParams(inPacket,paramTypes,2,false,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	fight = fs_fightGetById(PARAM_BIGINT(PARAM_NEXT(inPacket)));
	if (!fight) return FS_SS_WRONG_FIGHT;
	if (fs_fightThreadStart(fight) != OK) return FS_SS_WRONG_STATE;
	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCT_STOP_FIGHT)
// #1  fightId      (type INT)
// Answer:
// #0  cmd          (type INT, val=FS_SCCT_STOP_FIGHT)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
//
// Description: stop the fight thread
// Thread: worker
fs_srvStatus_t fs_cmd_SCCT_STOP_FIGHT(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT, PT_INT };
	fs_fight_t        *fight;

	if (fs_cmdCheckParams(inPacket,paramTypes,2,false,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	fight = fs_fightGetById(PARAM_BIGINT(PARAM_NEXT(inPacket)));
	if (!fight) return FS_SS_WRONG_FIGHT;
/*	if (fs_fightThreadStop(fight) != OK) return FS_SS_WRONG_STATE;*/
	fs_fightSignal(fight,FS_FSIG_STOP);
	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCT_DELETE_FIGHT)
// #1  fightId      (type INT)
// Answer:
// #0  cmd          (type INT, val=FS_SCCT_DELETE_FIGHT)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
//
// Description: delete the fight instance
// Thread: worker
fs_srvStatus_t fs_cmd_SCCT_DELETE_FIGHT(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT, PT_INT };
	fs_fight_t        *fight;

	if (fs_cmdCheckParams(inPacket,paramTypes,2,false,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	fight = fs_fightGetById(PARAM_BIGINT(PARAM_NEXT(inPacket)));
	if (!fight) return FS_SS_WRONG_FIGHT;
/*	v_remove(fs_fightVec,fight);
	fs_fightDelete(fight);*/
	fs_fightSignal(fight,FS_FSIG_STOP);
	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCT_CREATE_PERS)
// #1  persId       (type INT)
// #2  akey         (type INT)
// #3  persFlags    (type INT)
// #4  nick         (type STRING)
// #5  nickData     (type STRING)
// #6  level        (type INT)
// #7  gender       (type INT)
// #8  kind         (type INT)
// #9  cls          (type INT)
// #10 skeleton     (type INT)
// #11 skeletonTime (type INT)
// #12 partMask     (type INT)
// #13 artId        (type INT)
// #14 expX         (type FIXED)
// #15 ctrlData     (type STRING, val="[filename:][funcname]")
// #16 petLevel     (type INT)
// #17 petReady     (type INT)
// #18 autoKick     (type INT)
// #19 arrowsCnt     (type INT)
// #20 yarost     (type INT)
// #21 petSrc     (type STRING)
// Answer:
// #0  cmd          (type INT, val=FS_SCCT_CREATE_PERS)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
//
// Description: create a personage record
// Thread: worker
fs_srvStatus_t fs_cmd_SCCT_CREATE_PERS(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = {
		PT_INT, 
		PT_INT, 
		PT_INT, 
		PT_INT, 
		PT_STRING, 
		PT_STRING, 
		PT_INT, 
		PT_INT, //gender
		PT_INT, //party_id
		PT_INT, //clan_id
		PT_INT, 
		PT_INT, 
		PT_INT, 
		PT_INT, 
		PT_INT, 
		PT_INT, 
		PT_FIXED,
		PT_STRING, 
		PT_INT, 
		PT_INT, 
		PT_INT, 
		PT_INT, 
		PT_INT,
		PT_STRING,		
		PT_INT,
	};
	fs_pers_t         *pers;
	int               persId;
	char              *ctrlData, *ctrlFunc;

	if (fs_cmdCheckParams(inPacket,paramTypes,25,false,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	persId = PARAM_INT(PARAM_NEXT(inPacket));
	if (persId <= 0) return FS_SS_WRONG_PERS;
	pers = fs_persGetById(persId);
	if (pers) {
		if (pers->fight) return FS_SS_WRONG_STATE;
		v_remove(fs_persVec,pers);
		fs_persDelete(pers);
	}
	pers = fs_persCreate(persId);
	if (!pers) return FS_SS_INT_ERROR;
	pers->akey = PARAM_INT(PARAM_NEXT(inPacket));
	pers->flags = PARAM_INT(PARAM_NEXT(inPacket));
	pers->nick = strdup(PARAM_STRING(PARAM_NEXT(inPacket)));
	pers->nickData = strdup(PARAM_STRING(PARAM_NEXT(inPacket)));
	pers->level = PARAM_INT(PARAM_NEXT(inPacket));
	pers->gender = PARAM_INT(PARAM_NEXT(inPacket));
	pers->partyId = PARAM_INT(PARAM_NEXT(inPacket));
	pers->clanId = PARAM_INT(PARAM_NEXT(inPacket));
	pers->kind = PARAM_INT(PARAM_NEXT(inPacket));
	pers->cls = PARAM_INT(PARAM_NEXT(inPacket));
	pers->skeleton = PARAM_INT(PARAM_NEXT(inPacket));
	pers->skeletonTime = PARAM_INT(PARAM_NEXT(inPacket));
	pers->partMask = PARAM_INT(PARAM_NEXT(inPacket));
	pers->artId = PARAM_INT(PARAM_NEXT(inPacket));
	pers->expX = PARAM_FIXED(PARAM_NEXT(inPacket));
	ctrlData = PARAM_STRING(PARAM_NEXT(inPacket));
	if (pers->flags & FS_PF_BOT) {
		ctrlFunc = strchr(ctrlData,':');
		if (ctrlFunc) *(ctrlFunc++) = 0;
		else ctrlFunc = ctrlData;
		if((ctrlFunc > ctrlData) && (strlen(ctrlData) > 0)){
			pers->ctrlFile = strdup(ctrlData);
		}
		if(strlen(ctrlFunc) > 0){
			pers->ctrlFunc = strdup(ctrlFunc);
		}
		//pers->ctrlFile = strdup((ctrlFunc > ctrlData) && (strlen(ctrlData) > 0) ? ctrlData : "lua/default.lua");
		//pers->ctrlFunc = strdup(strlen(ctrlFunc) > 0 ? ctrlFunc : "default");

		pers->botTypeId = PARAM_INT(PARAM_NEXT(inPacket));
	}else{
		ctrlFunc = strchr(ctrlData,':');
		if (ctrlFunc) *(ctrlFunc++) = 0;
		else ctrlFunc = ctrlData;
		pers->ctrlFile = strdup((ctrlFunc > ctrlData) && (strlen(ctrlData) > 0) ? ctrlData : "");
		pers->ctrlFunc = strdup(strlen(ctrlFunc) > 0 ? ctrlFunc : "");
		
		pers->petLevel = PARAM_INT(PARAM_NEXT(inPacket));
		pers->petReady = PARAM_INT(PARAM_NEXT(inPacket));
		pers->autoKick = PARAM_INT(PARAM_NEXT(inPacket));
		pers->arrowsCnt = PARAM_INT(PARAM_NEXT(inPacket)); //luki
		pers->yarost = PARAM_INT(PARAM_NEXT(inPacket)); //yarost;
		pers->PetSrc = strdup(PARAM_STRING(PARAM_NEXT(inPacket))); // pet
		
		pers->yarost_max = PARAM_INT(PARAM_NEXT(inPacket)); //yarost_max;
	}
	v_push(fs_persVec,pers);
	if (!pers->partMask) pers->partMask = 63;
	pers->art = (pers->flags & FS_PF_ART) > 0;
	pers->pettime = time(NULL);
	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCT_SET_SKILLS)
// #1  persId       (type INT)
// ...
// #n  value        (type INT, id=fs_skill_t)
// ...
// Answer:
// #0  cmd          (type INT, val=FS_SCCT_SET_SKILLS)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
//
// Description: set personage skills
// Thread: worker
fs_srvStatus_t fs_cmd_SCCT_SET_SKILLS(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT, PT_INT };
	fs_param_t        *param;
	fs_pers_t         *pers;
	fs_skill_t        skill;

	if (fs_cmdCheckParams(inPacket,paramTypes,2,true,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	pers = fs_persGetById(PARAM_INT(PARAM_NEXT(inPacket)));
	if (!pers) return FS_SS_WRONG_PERS;
	while ((param = PARAM_NEXT(inPacket))) {
		if (PARAM_TYPE(param) != PT_INT) {
			WARN("Wrong 'value' type: %d",PARAM_TYPE(param));
			return FS_SS_WRONG_ARGS;
		}
		skill = PARAM_ID(param);
		if ((skill < 0) || (skill > FS_SK_MAXCODE)) {
			WARN("Wrong skill: %d",skill);
			return FS_SS_WRONG_SKILL;
		}
		PERS_INTSKILL(pers,skill) = PARAM_INT(param);
		PERS_EXTSKILL(pers,skill) = PARAM_INT(param);
	}
	return FS_SS_OK;
}

fs_srvStatus_t fs_cmd_SCCT_SET_BOTDMG_SKILLS(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT, PT_INT };
	fs_param_t        *param;
	fs_pers_t         *pers;
	fs_skill_t        skill;

	if (fs_cmdCheckParams(inPacket,paramTypes,2,true,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	pers = fs_persGetById(PARAM_INT(PARAM_NEXT(inPacket)));
	if (!pers) return FS_SS_WRONG_PERS;
	while ((param = PARAM_NEXT(inPacket))) {
		if (PARAM_TYPE(param) != PT_INT) {
			WARN("Wrong 'value' type: %d",PARAM_TYPE(param));
			return FS_SS_WRONG_ARGS;
		}
		skill = PARAM_ID(param);
		PERS_BOTDMGSKILL(pers, skill) = PARAM_INT(param);
	}
	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCT_SET_PARTS)
// #1  persId       (type INT)
// #2  layer        (type INT)
// ...
// #n  pack         (type INT, id=fs_persPart_t)
// ...
// Answer:
// #0  cmd          (type INT, val=FS_SCCT_SET_PARTS)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
//
// Description: set personage parts
// Thread: worker
fs_srvStatus_t fs_cmd_SCCT_SET_PARTS(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT, PT_INT, PT_STRING };
	fs_param_t        *param;
	fs_pers_t         *pers;
	fs_persPart_t     part;
	int               layer;
	char 			  *parts;

	if (fs_cmdCheckParams(inPacket,paramTypes,3,true,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	
	
	pers = fs_persGetById(PARAM_INT(PARAM_NEXT(inPacket)));
	
	pers->nParts = strdup(PARAM_STRING(PARAM_NEXT(inPacket)));
	
	DEBUG("PUK %s",pers->nParts);
	
	return FS_SS_OK;
	
	layer = PARAM_INT(PARAM_NEXT(inPacket));
	if (!pers) return FS_SS_WRONG_PERS;
	if ((layer < 0) || (layer >= FS_PPT_LAYERCNT)) {
		WARN("Wrong layer: %d",layer);
		return FS_SS_WRONG_LAYER;
	}
	while ((param = PARAM_NEXT(inPacket))) {
		if (PARAM_TYPE(param) != PT_INT) {
			WARN("Wrong 'pack' type: %d",PARAM_TYPE(param));
			return FS_SS_WRONG_ARGS;
		}
		part = PARAM_ID(param);
		if ((part < 0) || (part > FS_PPT_MAXCODE)) {
			WARN("Wrong part: %d",part);
			return FS_SS_WRONG_PART2;
		}
		PERS_PART(pers,part,layer) = PARAM_INT(param);
	}
	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCT_ADD_EFFECT)
// #1  persId       (type INT)
// #2  effId        (type INT)
// #3  cnt          (type INT)
// #4  code         (type INT, val IN fs_persEffCode_t)
// #5  f1           (type FIXED/NFIXED)
// #6  f2           (type FIXED/NFIXED)
// #7  f3           (type FIXED/NFIXED)
// #8  i1           (type INT/NINT)
// #9  i2           (type INT/NINT)
// #10 i3           (type INT/NINT)
// #11 flags        (type INT, val IN fs_persEffFlags_t)
// #12 artId        (type INT)
// #13 grpId        (type INT)
// #14 dmg          (type INT)
// #15 dmgType      (type INT)
// #16 actTime      (type INT)
// #17 actMoveCnt   (type INT)
// #18 actPeriod    (type INT)
// #19 cdTime       (type INT)
// #20 cdGrpId      (type INT)
// #21 mp           (type INT)
// #22 aoeCnt       (type INT)
// #23 prob         (type FIXED)
// #24 title        (type STRING)
// #25 picture      (type STRING)
// #26 slotNum      (type INT)
// #27 animData     (type STRING)
// #28 e_yarost     (type INT)
// #29 slot_id     (type PT_INT)
// #30 sub_slot     (type PT_INT)
// #31 e_cdrtime     (type PT_INT)
// #32 autoprob     (type FIXED)
// ...
// #n  value        (type INT, id=fs_skill_t)
// ...
// Answer:
// #0  cmd          (type INT, val=FS_SCCT_ADD_EFFECT)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
//
// Description: load an effect
// Thread: worker
fs_srvStatus_t fs_cmd_SCCT_ADD_EFFECT(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { 
		PT_INT, //0
		PT_INT, //1
		PT_INT, //2
		PT_INT, //3
		PT_INT, //4
		PT_FIXED, //5
		PT_FIXED, //6
		PT_FIXED, //7
		PT_INT, //8
		PT_INT, //9
		PT_INT, //10
		PT_INT, //11
		PT_INT, //12
		PT_INT, //13
		PT_INT, //14
		PT_INT, //15
		PT_INT, //16
		PT_INT, //17
		PT_INT, //18
		PT_INT, //19
		PT_INT, //20
		PT_INT, //21
		PT_INT, //22
		PT_FIXED, //23
		PT_STRING, //24
		PT_STRING, //25
		PT_INT, //26
		PT_STRING, //27 
		PT_INT, //28
		PT_STRING, //29
		PT_INT, //30
		PT_INT, //31
		PT_FIXED, //32
		PT_INT //33
	};
	fs_param_t        *param;
	fs_pers_t         *pers;
	fs_persEff_t      *eff;
	fs_skill_t        skill;

	if (fs_cmdCheckParams(inPacket,paramTypes,33,true,true) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	pers = fs_persGetById(PARAM_INT(PARAM_NEXT(inPacket)));
	if (!pers) return FS_SS_WRONG_PERS;
	eff = fs_persEffCreate(PARAM_INT(PARAM_NEXT(inPacket)));
	if (!eff) return FS_SS_INT_ERROR;
	eff->cnt = PARAM_INT(PARAM_NEXT(inPacket));
	eff->code = PARAM_INT(PARAM_NEXT(inPacket));
	eff->f1 = PARAM_FIXED(PARAM_NEXT(inPacket));
	eff->f2 = PARAM_FIXED(PARAM_NEXT(inPacket));
	eff->f3 = PARAM_FIXED(PARAM_NEXT(inPacket));
	eff->i1 = PARAM_INT(PARAM_NEXT(inPacket));
	eff->i2 = PARAM_INT(PARAM_NEXT(inPacket));
	eff->i3 = PARAM_INT(PARAM_NEXT(inPacket));
	eff->flags = PARAM_INT(PARAM_NEXT(inPacket));
	eff->artId = PARAM_INT(PARAM_NEXT(inPacket));
	eff->grpId = PARAM_INT(PARAM_NEXT(inPacket));
	eff->dmg = PARAM_INT(PARAM_NEXT(inPacket));
	eff->dmgType = PARAM_INT(PARAM_NEXT(inPacket));
	eff->actTime = PARAM_INT(PARAM_NEXT(inPacket));
	eff->actMoveCnt = PARAM_INT(PARAM_NEXT(inPacket));
	eff->actPeriod = PARAM_INT(PARAM_NEXT(inPacket));
	eff->cdTime = PARAM_INT(PARAM_NEXT(inPacket));
	eff->cdGrpId = PARAM_INT(PARAM_NEXT(inPacket));
	eff->mp = PARAM_INT(PARAM_NEXT(inPacket));
	eff->aoeCnt = PARAM_INT(PARAM_NEXT(inPacket));
	eff->prob = PARAM_FIXED(PARAM_NEXT(inPacket));
	eff->title = strdup(PARAM_STRING(PARAM_NEXT(inPacket)));
	eff->picture = strdup(PARAM_STRING(PARAM_NEXT(inPacket)));
	eff->slotNum = PARAM_INT(PARAM_NEXT(inPacket));
	eff->animData = strdup(PARAM_STRING(PARAM_NEXT(inPacket)));
	eff->e_yarost = PARAM_INT(PARAM_NEXT(inPacket));
	
	//*NEW*//
	eff->slotId = strdup(PARAM_STRING(PARAM_NEXT(inPacket)));
	eff->subSlot = PARAM_INT(PARAM_NEXT(inPacket));
	eff->cdrtime = PARAM_INT(PARAM_NEXT(inPacket));
	eff->probAuto = PARAM_FIXED(PARAM_NEXT(inPacket)); //2012
	
	eff->cdType = PARAM_INT(PARAM_NEXT(inPacket));
	eff->detStack = PARAM_INT(PARAM_NEXT(inPacket));

	v_push(pers->effVec,eff);
	while ((param = PARAM_NEXT(inPacket))) {
		if ((PARAM_TYPE(param) != PT_INT) && (PARAM_TYPE(param) != PT_NINT)) {
			WARN("Wrong 'value' type: %d",PARAM_TYPE(param));
			return FS_SS_WRONG_ARGS;
		}
		skill = PARAM_ID(param);
		if ((skill < 0) || (skill > FS_SK_MAXCODE)) {
			WARN("Wrong skill: %d",skill);
			return FS_SS_WRONG_SKILL;
		}
		eff->skills[skill] = PARAM_INT(param);
	}
	if ((eff->cdTime > 0) && (eff->flags & FS_PEF_CDSTART)) eff->cdrtime = fs_stime + eff->cdTime;	// starting cooldown
	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCT_ADD_COMBO)
// #1  persId       (type INT)
// #2  cmbId        (type INT)
// #3  level        (type INT)
// #4  auxEffId     (type INT)
// ...
// #n  seqItem      (type INT)
// ...
// Answer:
// #0  cmd          (type INT, val=FS_SCCT_ADD_COMBO)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
//
// Description: load a combo
// Thread: worker
fs_srvStatus_t fs_cmd_SCCT_ADD_COMBO(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT, PT_INT, PT_INT, PT_INT, PT_INT };
	fs_param_t        *param;
	fs_pers_t         *pers;
	fs_persCmb_t      *cmb;
	int               cmbId, level, auxEffId, i;

	if (fs_cmdCheckParams(inPacket,paramTypes,5,true,true) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	pers = fs_persGetById(PARAM_INT(PARAM_NEXT(inPacket)));
	cmbId = PARAM_INT(PARAM_NEXT(inPacket));
	level = PARAM_INT(PARAM_NEXT(inPacket));
	auxEffId = PARAM_INT(PARAM_NEXT(inPacket));
	if (!pers || (cmbId <= 0) || (auxEffId <= 0) || (level < 0)) return FS_SS_WRONG_PERS;
	cmb = malloc(sizeof(fs_persCmb_t));
	if (!cmb) {
		WARN("malloc() failed");
		return FS_SS_INT_ERROR;
	}
	memset(cmb,0,sizeof(fs_persCmb_t));
	cmb->id = cmbId;
	cmb->level = level;
	cmb->auxEffId = auxEffId;
	v_push(pers->cmbVec,cmb);
	for (i=0; (param = PARAM_NEXT(inPacket)) && (i < MAX_COMBO_SIZE); i++) {
		if (PARAM_TYPE(param) != PT_INT) {
			WARN("Wrong 'seqItem' type: %d",PARAM_TYPE(param));
			return FS_SS_WRONG_ARGS;
		}
		cmb->seq[i] = PARAM_INT(param);
	}
	cmb->size = i;
	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCT_BIND_PERS)
// #1  persId       (type INT)
// #2  fightId      (type INT)
// #3  teamNum      (type INT)
// Answer:
// #0  cmd          (type INT, val=FS_SCCT_BIND_PERS)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
//
// Description: bind the personage to the fight
// Thread: worker
fs_srvStatus_t fs_cmd_SCCT_BIND_PERS(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT, PT_INT, PT_INT, PT_INT };
	fs_pers_t         *pers;
	fs_fight_t        *fight;
	fs_persEff_t      *eff;
	int               teamNum, i1, i2;

	if (fs_cmdCheckParams(inPacket,paramTypes,4,false,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	pers = fs_persGetById(PARAM_INT(PARAM_NEXT(inPacket)));
	fight = fs_fightGetById(PARAM_BIGINT(PARAM_NEXT(inPacket)));
	teamNum = PARAM_INT(PARAM_NEXT(inPacket));
	if (!pers || !fight || (teamNum < 1) || (teamNum > 2)) return FS_SS_WRONG_PERS;
	pers->fight = fight;
	pers->client = NULL;
	pers->teamNum = teamNum;

	fs_persRecalcEffects(pers);
	if (PERS_SKILL(pers,FS_SK_MPMAX) > 0) {
		i1 = PERS_SKILL(pers,FS_SK_MREG);
		i1 = MAX(i1, 6);
		i2 = equalDoze(&i1);
		eff = fs_persEffCreate(0);
		eff->code = FS_PEC_ADDSKILLS;
		eff->cnt = 1;
		eff->actTime = 1000000;
		eff->actPeriod = i2;
		eff->skills[FS_SK_MP] = i1;
		fs_persUseEffect(pers,eff,pers,NULL);
		fs_persEffDelete(eff);
	}
	pthread_mutex_lock(&(fight->mutex_lua));
	if (pers->ctrlFile && (luaif_dofile(fight->L,pers->ctrlFile) != 0)) WARN("Error loading script file: file = %s",pers->ctrlFile);
	pthread_mutex_unlock(&(fight->mutex_lua));
	v_remove(fs_persVec,pers);
	fs_fightLockMutex(fight);
	v_push(fight->persVec,pers);
	if (fight->status == FS_FS_RUNNING) fs_fightSaveLog(pers,FS_FLC_INTRUSION,0,0,0,NULL,false);
	fs_fightUnlockMutex(fight);
	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCT_DELETE_PERS)
// #1  persId       (type INT)
// Answer:
// #0  cmd          (type INT, val=FS_SCCT_DELETE_PERS)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
//
// Description: delete the personage record
// Thread: worker
fs_srvStatus_t fs_cmd_SCCT_DELETE_PERS(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT, PT_INT };
	fs_pers_t         *pers;

	if (fs_cmdCheckParams(inPacket,paramTypes,2,false,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	pers = fs_persGetById(PARAM_INT(PARAM_NEXT(inPacket)));
	if (!pers) return FS_SS_WRONG_PERS;
	v_remove(fs_persVec,pers);
	fs_persDelete(pers);
	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCT_GET_FIGHTSTATE)
// #1  fightId      (type INT)
// #2  startIdx     (type INT)
// Answer:
// #0  cmd          (type INT, val=FS_SCCT_GET_FIGHTSTATE)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
// #2  listSize     (type INT)
// ...
// #n   persId      (type INT)
// #+1  persStatus  (type INT, val IN fs_persStatus_t)
// #+2  persFlags   (type INT, val IN fs_persFlags_t)
// #+3  teamNum     (type INT)
// #+4  hp          (type INT)
// #+5  hpMax       (type INT)
// #+6  mp          (type INT)
// #+7  mpMax       (type INT)
// ...
//
// Description: get the fight overall status (no more than 100 chunks at once)
// Thread: worker
fs_srvStatus_t fs_cmd_SCCT_GET_FIGHTSTATE(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT, PT_INT, PT_INT };
	fs_fight_t        *fight;
	fs_pers_t         *pers;
	int               startIdx, i;
	viter_t           vi;

	if (fs_cmdCheckParams(inPacket,paramTypes,3,false,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	fight = fs_fightGetById(PARAM_BIGINT(PARAM_NEXT(inPacket)));
	startIdx = PARAM_INT(PARAM_NEXT(inPacket));
	if (!fight) return FS_SS_WRONG_FIGHT;
	
	if(fight->status != FS_FS_RUNNING){
		return FS_SS_OK;
	}
	
	fs_fightLockMutex(fight);
	PARAM_ADD(outPacket,"i",v_size(fight->persVec));
	if (v_jump(fight->persVec,startIdx,&vi)) {
		for (i=0; (pers = v_each(fight->persVec,&vi)) && (i < 100); i++) {
			//fs_fightPersStatus(pers);
			PARAM_ADD(outPacket,"iiiiiiiiiii",pers->id,pers->status,pers->statusl,pers->flags,pers->teamNum,PERS_HP(pers),PERS_HPMAX(pers),PERS_MP(pers),PERS_MPMAX(pers), pers->yarost, pers->arrowsCnt);
		}
	}
	fs_fightUnlockMutex(fight);
	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCT_GET_FIGHTLOG)
// #1  fightId      (type INT)
// #2  mode         (type INT, val=(0 - fight, 1 - fightInfo))
// #3  startIdx     (type INT)
// Answer:
// #0  cmd          (type INT, val=FS_SCCT_GET_FIGHTLOG)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
// #2  listSize     (type INT)
// ...
// #n   ctime       (type INT)
// #+1  persId      (type INT)
// #+2  persHp      (type INT)
// #+3  persHpMax   (type INT)
// #+4  oppId       (type INT)
// #+5  oppHp       (type INT)
// #+6  oppHpMax    (type INT)
// #+7  code        (type INT)
// #+8  i1          (type INT)
// #+9  i2          (type INT)
// #+10 i3          (type INT)
// #+11 s1          (type STRING)
// ...
//
// Description: get the fight log data (no more than 100 chunks at once)
// Thread: worker
fs_srvStatus_t fs_cmd_SCCT_GET_FIGHTLOG(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT, PT_INT, PT_INT, PT_INT };
	fs_fight_t        *fight;
	fs_fightInfo_t    *info;
	fs_fightLog_t     *log;
	fs_fightLogData_t *data;
	int               fightId, mode, startIdx, i;
	viter_t           vi;

	if (fs_cmdCheckParams(inPacket,paramTypes,4,false,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	fightId = PARAM_BIGINT(PARAM_NEXT(inPacket));
	mode = PARAM_INT(PARAM_NEXT(inPacket));
	startIdx = PARAM_INT(PARAM_NEXT(inPacket));
	if (!mode) {	// take the realtime fight log
		fight = fs_fightGetById(fightId);
		if (!fight) return FS_SS_WRONG_FIGHT;
		log = fight->log;
	} else {	// take the fight saved log
		info = fs_fightInfoGetById(fightId);
		if (!info) return FS_SS_WRONG_INFO;
		log = info->log;
	}
	if (!log) return FS_SS_WRONG_STATE;
	fs_fightLogLockMutex(log);
	PARAM_ADD(outPacket,"i",v_size(&(log->dataVec)));
	if (v_jump(&(log->dataVec),startIdx,&vi)) {
		for (i=0; (data = v_each(&(log->dataVec),&vi)) && (i < 100); i++) {
			PARAM_ADD(outPacket,"iiiiiiiiiiis",data->ctime,data->persId,data->persHp,data->persHpMax,data->oppId,data->oppHp,data->oppHpMax,data->code,data->i1,data->i2,data->i3,data->s1);
		}
	}
	fs_fightLogUnlockMutex(log);
	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCT_GET_FIGHTINFO)
// #1  fightId      (type INT)
// #2  mode         (type INT, val=(0-overall, 1-persData, 2-effData, 3-cmbData))
// #3  startIdx     (type INT)
// Answer:
// #0  cmd          (type INT, val=FS_SCCT_GET_FIGHTINFO)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
// ------------ mode==0 ------------
// #2  ctime        (type INT)
// #3  rtime        (type INT)
// #4  winnerTeam   (type INT)
// ---------------------------------
// ------------ mode==1 ------------
// #2  listSize     (type INT)
// ...
// #n   persId       (type INT)
// #+1  persStatus   (type INT)
// #+2  persFlags    (type INT, val IN fs_persFlags_t)
// #+3  teamNum      (type INT)
// #+4  dmg          (type INT)
// #+5  heal         (type INT)
// #+6  exp          (type INT)
// #+7  honor        (type INT)
// #+8  hp           (type INT)
// #+9  mp           (type INT)
// #+10 killCnt      (type INT)
// #+11 enemyKillCnt (type INT)
// #+12 arrowsCnt    (type INT)
// ...
// ---------------------------------
// ------------ mode==2 ------------
// #2  listSize     (type INT)
// ...
// #n   effId        (type INT)
// #+1  persId       (type INT)
// #+2  cnt          (type INT)
// ...
// ---------------------------------
// ------------ mode==3 ------------
// #2  listSize     (type INT)
// ...
// #n   cmbId        (type INT)
// #+1  persId       (type INT)
// #+2  useCnt       (type INT)
// ...
// ---------------------------------
//
// Description: get the fight info (no more than 100 chunks at once in the lists)
// Thread: worker
fs_srvStatus_t fs_cmd_SCCT_GET_FIGHTINFO(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t         paramTypes[] = { PT_INT, PT_INT, PT_INT, PT_INT };
	fs_fightInfo_t         *info;
	fs_fightInfoPersData_t *persData;
	fs_fightInfoEffData_t  *effData;
	fs_fightInfoCmbData_t  *cmbData;
	int                    fightId, mode, startIdx, i;
	viter_t                vi;

	if (fs_cmdCheckParams(inPacket,paramTypes,4,false,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	fightId = PARAM_BIGINT(PARAM_NEXT(inPacket));
	mode = PARAM_INT(PARAM_NEXT(inPacket));
	startIdx = PARAM_INT(PARAM_NEXT(inPacket));
	info = fs_fightInfoGetById(fightId);
	INFO("FIGHTINFO REQ [id: %d, mode: %d]",fightId,mode);
	if (!info) return FS_SS_WRONG_INFO;
	switch (mode) {
		case 0:	// overall
			PARAM_ADD(outPacket,"iii",info->fctime,info->frtime,info->winnerTeam);
			break;
		case 1:	// persData
			PARAM_ADD(outPacket,"i",v_size(info->persDataVec));
			if (v_jump(info->persDataVec,startIdx,&vi)) {
				for (i=0; (persData = v_each(info->persDataVec,&vi)) && (i < 100); i++) {
					PARAM_ADD(outPacket,"iiiiiiiiiiiii",persData->id,persData->status,persData->flags,persData->teamNum,persData->dmg,persData->heal,persData->exp,persData->honor,persData->hp,persData->mp,persData->killCnt,persData->enemyKillCnt,persData->arrowsCnt);
				}
			}
			break;
		case 2:	// effData
		//fs_cmd_SCCT_GET_FIGHTINFO
			PARAM_ADD(outPacket,"i",v_size(info->effDataVec));
			if (v_jump(info->effDataVec,startIdx,&vi)) {
				for (i=0; (effData = v_each(info->effDataVec,&vi)) && (i < 100); i++) {
					PARAM_ADD(outPacket,"iiiiii",effData->id, effData->persId, effData->cnt, effData->cdrtime, effData->slotNum, effData->subSlot);
				}
			}
			break;
		case 3:	// cmbData
			PARAM_ADD(outPacket,"i",v_size(info->cmbDataVec));
			if (v_jump(info->cmbDataVec,startIdx,&vi)) {
				for (i=0; (cmbData = v_each(info->cmbDataVec,&vi)) && (i < 100); i++) {
					PARAM_ADD(outPacket,"iii",cmbData->id,cmbData->persId,cmbData->useCnt);
				}
			}
			break;
		default:
			return FS_SS_WRONG_DATA2;
	}
	INFO("FIGHTINFO GIVEN [id: %d, mode: %d]",info->id,mode);
	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCT_DELETE_FIGHTINFO)
// #1  fightId      (type INT)
// Answer:
// #0  cmd          (type INT, val=FS_SCCT_DELETE_FIGHTINFO)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
//
// Description: delete the fight info data
// Thread: worker
fs_srvStatus_t fs_cmd_SCCT_DELETE_FIGHTINFO(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT, PT_INT };
	fs_fightInfo_t    *info, *info1;

	if (fs_cmdCheckParams(inPacket,paramTypes,2,false,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	int fID = PARAM_BIGINT(PARAM_NEXT(inPacket));
	info = fs_fightInfoGetById(fID);
	if (!info) return FS_SS_WRONG_INFO;
	INFO("FIGHTINFO DELETED [id: %d]",info->id);
	v_remove(fs_fightInfoVec,info);
	v_remove(fs_fightInfoVec1,info); 
	fs_fightInfoDelete(info);
	return FS_SS_OK;
}


// ---------------------------------------------------- CLIENT COMMANDS ----------------------------------------------------

// Parameters:
// #0  cmd          (type INT, val=FS_SCCL_INIT)
// #1  persId       (type INT)
// #2  fightId      (type INT)
// #3  akey         (type INT)
// Answer:
// #0  cmd          (type INT, val=FS_SCCL_INIT)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
//
// Description: client's authorization
// Thread: worker
fs_srvStatus_t fs_cmd_SCCL_INIT(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT, PT_INT, PT_BIGINT, PT_INT };
	fs_fight_t        *fight;
	fs_pers_t         *pers;
	int			  	  fightId;
	int               persId, akey;
	char              ipbuf[16];

	if (fs_cmdCheckParams(inPacket,paramTypes,4,false,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	if (client->fight) return FS_SS_OK;	// strange... seems like client is already on a fight thread's listener, do nothing
	persId = PARAM_INT(PARAM_NEXT(inPacket));
	fightId = PARAM_BIGINT(PARAM_NEXT(inPacket));
	
	//fightId -= 8240304;
	
	DEBUG("FIGHT_INIT FIGHT %d",fightId);
	
	akey = PARAM_INT(PARAM_NEXT(inPacket));
	fight = fs_fightGetById(fightId);
	if (!fight) return FS_SS_WRONG_FIGHT;
	if (fight->status != FS_FS_RUNNING) return FS_SS_WRONG_STATE;
	fs_fightLockMutex(fight);
	pers = fs_fightPersGetById(fight,persId);
	fs_fightUnlockMutex(fight);
	if (!pers) return FS_SS_WRONG_PERS;
	strIpAddr(client->ipAddr,ipbuf);
	if ((pers->akey != akey) || (pers->flags & FS_PF_BOT)) {
		WARN("Personage access violation (persId: %d, fightId: %d, akey: %d, akey given: %d, ip: %s)",persId,fightId,pers->akey,akey,ipbuf);
		client->flags |= FS_CF_DISCONN;
		return FS_SS_WRONG_ACCESS;
	}
	client->pers = pers;
	client->fight = fight;
	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCL_STATE)
// Answer:
// #0  cmd          (type INT, val=FS_SCCL_STATE)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
// #2  fightTimeout (type INT)
// #3  persId       (type INT)
// #4  persStatus   (type INT, val IN fs_persStatus_t)
// #5  persLStatus   (type INT, val IN fs_persLStatus_t)
// #6  persFlags    (type INT, val IN fs_persFlags_t)
// #7  dmg          (type INT)
// #8  hp           (type INT)
// #9  hpMax        (type INT)
// #10  mp           (type INT)
// #11 mpMax        (type INT)
// #12 oppId        (type INT)
// #13 minCooldown  (type INT)
// #14 timestamp    (type INT)
// #15 new_pers_id 	(type INT)
// #16 arrowsCnt 	(type INT)
// #17 yarost 	(type INT)
// #18 max_yarost (not use for me) 	(type INT)
//
// Description: personage's state
// Thread: fight
fs_srvStatus_t fs_cmd_SCCL_STATE(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT, PT_INT };
	fs_pers_t         *pers;	
	int persId;
	fs_persEff_t      *auraEff;
	
	if (fs_cmdCheckParams(inPacket,paramTypes,2,false,false) != OK) {			 
		pers = client->pers;
		if (!pers) return FS_SS_NO_AUTH;
	}else{
		persId = PARAM_INT(PARAM_NEXT(inPacket));
		fs_fightLockMutex(client->fight);
		pers = fs_fightPersGetById(client->fight, persId);
		fs_fightUnlockMutex(client->fight);
		if (!pers) return FS_SS_NO_AUTH;
	}
	fs_fightLockMutex(client->fight);
	fs_fightPersStatus(pers);	 
	fs_fightUnlockMutex(client->fight);
	int running = (client->fight->status == FS_FS_RUNNING ? 0 : 1);
	
	if(pers->fight->otime > 0){
		if(pers->fight->ctime + pers->fight->otime > pers->fight->mtime){
			running = 1;
		}
	}else{
		running = 0;
	}
	
	v_reset(pers->effVec,0);
    while ((auraEff = v_each(pers->effVec,0))) {
        if (!(auraEff->flags & FS_PEF_ACTIVE) || !(auraEff->flags & FS_PEF_AURA)) continue;
        if (auraEff->flags & FS_PEF_AURA) break;
    }
	
	int aura_id = 0; //FIX
	if(auraEff){
		if((auraEff->flags & FS_PEF_AURA) && (auraEff->flags & FS_PEF_ACTIVE)){
			aura_id = auraEff->slotId;
		}			
	}
	DEBUG("AURA_INFO %d", aura_id);
	
 	PARAM_ADD(outPacket,"iiiiiiiiiiiiiiiiiii",running,pers->fight->timeout,pers->id,pers->status,pers->statusl,PF_CLMASK(pers->flags),pers->dmg,PERS_HP(pers),PERS_HPMAX(pers),PERS_MP(pers),PERS_MPMAX(pers),PERS_OPP_ID(pers),FIGHT_MIN_SPELL_COOLDOWN,fs_stime,pers->new_pers_id,pers->arrowsCnt,pers->yarost,100,aura_id); //last aura?
	/*
	2	running
	3	pers->fight->timeout,
	4	pers->id,
	5	pers->status,
	6	pers->statusl,
	7	PF_CLMASK(pers->flags),
	8	pers->dmg,
	9	PERS_HP(pers),
	10	PERS_HPMAX(pers),
	11	PERS_MP(pers),
	12	PERS_MPMAX(pers),
	13	PERS_OPP_ID(pers),
	14	FIGHT_MIN_SPELL_COOLDOWN,
	15	fs_stime,
	16	pers->new_pers_id,
	17	pers->arrowsCnt,
	18	pers->yarost,
	19	100
	20 	""
	*/
	
	return FS_SS_OK;
}

/*fs_srvStatus_t fs_cmd_SCCL_STATE(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT };
	fs_pers_t         *pers;

	if (fs_cmdCheckParams(inPacket,paramTypes,1,false,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	if (!client->pers) return FS_SS_NO_AUTH;
	pers = client->pers;
	fs_fightPersStatus(pers);	 
	PARAM_ADD(outPacket,"iiiiiiiiiiiiiiiii",pers->fight->timeout,pers->id,pers->status,pers->statusl,PF_CLMASK(pers->flags),pers->dmg,PERS_HP(pers),PERS_HPMAX(pers),PERS_MP(pers),PERS_MPMAX(pers),PERS_OPP_ID(pers),FIGHT_MIN_SPELL_COOLDOWN,fs_stime,pers->new_pers_id,pers->arrowsCnt,pers->yarost,100);
	return FS_SS_OK;
}*/

/*fs_srvStatus_t fs_cmd_SCCL_STATE(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_pers_t         *pers;
	fs_param_t		  *param;
	int personageID;
	
	DEBUG("personageID SSCL_STATE");
	
	if (!client->pers) return FS_SS_OK;
	param = PARAM_NEXT(inPacket);
	if(param) personageID = PARAM_INT(param);
	DEBUG("personageID SSCL_STATE: %d", personageID);
	param = PARAM_NEXT(inPacket);
	if(param) personageID = PARAM_INT(param);
	DEBUG("personageID SSCL_STATE: %d", personageID);
	pers = (personageID ? fs_fightPersGetById(client->fight, personageID) : client->pers);
	if(!pers) return FS_SS_OK;
	fs_fightPersStatus(pers);	 
	PARAM_ADD(outPacket,"iiiiiiiiiiiiiiiii",pers->fight->timeout,pers->id,pers->status,pers->statusl,PF_CLMASK(pers->flags),pers->dmg,PERS_HP(pers),PERS_HPMAX(pers),PERS_MP(pers),PERS_MPMAX(pers),PERS_OPP_ID(pers),FIGHT_MIN_SPELL_COOLDOWN,fs_stime,pers->new_pers_id,pers->arrowsCnt,pers->yarost,100);
	return FS_SS_OK;
}*/

//
// Açıklama: personage's state
// Thread: fight
//fs_srvStatus_t fs_cmd_SCCL_PERS_SUBSCRIBE(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
//	fs_pers_t         *pers, *opp;
//	fs_fight_t        *fight;
//	fs_param_t        *param;
//	int persId, persVisible;
//	
//	if (!client->fight) return FS_SS_NO_AUTH;
//	fight = client->fight;
//	if (!client->pers) return FS_SS_NO_AUTH;
//	if(!persVisible) return FS_SS_OK;
//	
//	persVisible = PARAM_INT(PARAM_NEXT(inPacket)); // ?????
//	
//	//DEBUG("persVisible: %d", persVisible);
//	
//	if(!persVisible) return FS_SS_OK; // no see i
//	
//	while ((param = PARAM_NEXT(inPacket))) { // who see i subscrible?
//		if (PARAM_TYPE(param) != PT_INT) {
//			break;
//		}
//		persId = PARAM_INT(param);
//		//DEBUG("persId: %d", persId);
//		if(persId <= 0) continue; //continue BOT
//		//what to send to the player?
//	}
//	
//	pers = client->pers;
//	fs_fightPersStatus(pers);
//	PARAM_ADD(outPacket,"iiiiiiiiiiiiiiiii",pers->fight->timeout,pers->id,pers->status,pers->statusl,PF_CLMASK(pers->flags),pers->dmg,PERS_HP(pers),PERS_HPMAX(pers),PERS_MP(pers),PERS_MPMAX(pers),PERS_OPP_ID(pers),FIGHT_MIN_SPELL_COOLDOWN,fs_stime,pers->new_pers_id,pers->arrowsCnt,pers->yarost,100);
//	
//	
//	/*pers = client->pers;	
//	if ((pers->status == FS_PS_ACTIVE)) {
//		opp = pers->opponent;
//		fs_persSetEvent(pers,FS_PE_ATTACKNOW,"i",PF_CLMASK(opp->flags));
//		fs_persSetEvent(opp,FS_PE_ATTACKWAIT,0); 
//		fight = client->fight;		 
//		fs_fightLockMutex(fight);
//		fs_fightDeadCntSend(fight, pers); //for only subscrible pers)
//		fs_fightUnlockMutex(fight);
//	}*/	
//	return FS_SS_OK;
//}

//v2
fs_srvStatus_t fs_cmd_SCCL_PERS_SUBSCRIBE(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT };
	fs_pers_t         *pers, *opp;
	fs_fight_t        *fight;
	int time ;
	time = fs_stime;
	return FS_SS_OK;
	if(!client) return FS_SS_NO_AUTH;
	if(!client->pers) return FS_SS_NO_AUTH;
	pers = client->pers;	
	if(!pers) return FS_SS_OK;

	fs_fightPersStatus(pers);		
	fight = pers->fight;
	if(!fight) return FS_SS_OK;
	
	if(fight->status != FS_FS_RUNNING){
		pers->new_pers_id = 0;
		fs_persSetEvent(pers, FS_PE_MYFIGHTRETURN,0); 
	}
	
	//fs_persSetEvent(pers,FS_PE_FIGHTSTATE,"iiiiiiii",pers->id,pers->status,pers->statusl,PF_CLMASK(pers->flags),PERS_HP(pers),PERS_HPMAX(pers),PERS_MP(pers),PERS_MPMAX(pers));	
	return FS_SS_OK;
	
	
	if ((pers->status == FS_PS_ACTIVE)) {
		opp = pers->opponent;
		//fs_persSetEvent(pers,FS_PE_ATTACKNOW,"i",PF_CLMASK(opp->flags));
		//fs_persSetEvent(opp,FS_PE_ATTACKWAIT,0); 
		fs_fightPersStatus(pers);			 
		fs_persSetEvent(opp,FS_PE_FIGHTSTATE,"iiiiiiii",pers->id,pers->status,pers->statusl,PF_CLMASK(pers->flags),PERS_HP(pers),PERS_HPMAX(pers),PERS_MP(pers),PERS_MPMAX(pers));	
		fight = client->fight;
	} 	
	if (pers->status == FS_PS_DEAD){
		fight = client->fight;		 
 
		pers->status = FS_PS_DEAD;
		pers->flags &= ~(FS_PF_GETTURN | FS_PF_TIMEOUTKILL | FS_PF_LIFELESS);
		pers->opponent = NULL;		
		PERS_INTSKILL(pers,FS_SK_HP) = PERS_EXTSKILL(pers,FS_SK_HP) = 0;	
		fs_persSetEvent(pers,FS_PE_DEATH,"i",pers->id);
	}
	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCL_WATCH_FIGHT)
// Answer:
// #0  cmd          (type INT, val=FS_SCCL_WATCH_FIGHT)
// #1  watch_pers_id       (type INT)
// 2-9 ?????
// #10 opp watch_pers_id    (type INT)
//
// Description: personage's state
// Thread: fight
fs_srvStatus_t fs_cmd_SCCL_WATCH_FIGHT(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT };
	fs_pers_t         *pers, *watch_pers, *watch_pers_old;
	fs_fight_t        *fight;
	fs_followPers_t   *followPersData;
	bool old_remove = false;
	if (!client->pers) return FS_SS_NO_AUTH;
	pers = client->pers;

	if(!pers) return FS_SS_OK;
	if(!client->fight) return FS_SS_OK;
	
	if (pers->fight->status != FS_FS_RUNNING) {
		return FS_SS_OK;
	}
	
	//DEBUG("WP0 IDI NAHUI!");
	fs_fightLockMutex(pers->fight);
	do{
		if(pers->status != FS_PS_DEAD) break;
		int pers_id = PARAM_INT(PARAM_NEXT(inPacket));
		if(pers_id <= 0) break;
		if(!pers_id) break;
		
		if(pers->id == pers_id){
			//fs_persSetEvent(pers,FS_PE_MYFIGHTRETURN,0);
			pers->new_pers_id = 0;
			break; //DONT!
		}
		
		watch_pers = fs_fightPersGetById(client->fight, pers_id);
		if(!watch_pers) break;
		if(watch_pers->flags & FS_PF_BOT){
			break;
		}
		
		//DEBUGXXX("SAD: ==> pers->id=%d, pers_id=%d, watch_pers->id=%d",pers->id,pers_id,watch_pers->id);
		
		//Задача: Удалить предыдущий follow pers
		if(pers->new_pers_id != 0 && pers->id != pers_id && pers->new_pers_id != pers_id){
			if(pers->new_pers_id > 0) watch_pers_old = fs_fightPersGetById(client->fight, pers->new_pers_id);
			if(watch_pers_old && watch_pers_old->followPers){
				v_reset(watch_pers_old->followPers,0);
				while ((followPersData = v_each(watch_pers_old->followPers,0))) {
					if(!followPersData->pers) continue;
					if (followPersData->pers->id == pers->id){
						old_remove = true;
						break;
					}
				}
				if(old_remove && followPersData){
					//DEBUGXXX("REMOVER: ==> followPersData->pers->id=%d, watch_pers_old->id=%d",followPersData->pers->id,watch_pers_old->id);
					//v_remove(watch_pers_old->followPers, followPersData);
					//free(followPersData);
				}
			}
		}

		
		
		pers->new_pers_id = watch_pers->id; //chanje new_pers_id for watching
		if(!pers->new_pers_id) break;
		//opp = pers->opponent;
		/*outa parameters (maybe fight state)
			1 = _persIdFromWFCmd = parser.params[2].val;
			9 = _oppIdFromWFCmd = parser.params[10].val;
		*/
		
		//ADD FOLLOWER
		//HMM IF 1000 FOLLOWERS SEE ONE PLAYER, WHO REPAIR PLAYER BRAIN?
		
		bool i_see = false;
		v_reset(watch_pers->followPers,0);
		while ((followPersData = v_each(watch_pers->followPers,0))) {
			if (followPersData->pers->id == pers->id){
				i_see = true;
				break;
			}
		}
		if (!i_see) {
		
			//DEBUGXXX("NEWFOLLOWER_SET");
			
			followPersData = malloc(sizeof(fs_followPers_t));
			if(!followPersData){
				return FS_SS_OK;
			}
			memset(followPersData,0,sizeof(fs_followPers_t));
			followPersData->pers = pers;
			v_push(watch_pers->followPers,followPersData);
			//DEBUGXXX("NEWFOLLOWER: ==> persId=%d, new_pers_id=%d",pers->id,watch_pers->id);
		}
		
		
	}while(0);
	fs_fightUnlockMutex(pers->fight);
	
	PARAM_ADD(outPacket,"iiiiiiiiii",pers->new_pers_id,0,0,0,0,0,0,0,(pers->new_pers_id ? (watch_pers ? PERS_OPP_ID(watch_pers) : 0) : 0));
	
	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCL_PERS_INFO)
// #1  persId       (type INT)
// Answer:
// #0  cmd          (type INT, val=FS_SCCL_PERS_INFO)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
// #2  persId       (type INT)
// #3  persStatus   (type INT, val IN fs_persStatus_t)
// #4  persStatusl   (type INT, val IN fs_perslStatus_t)
// #5  persFlags    (type INT, val IN fs_persFlags_t)
// #6  nick         (type STRING)
// #7  nickData     (type STRING)
// #8  level        (type INT)
// #9  gender       (type INT)
// #10  kind         (type INT)
// #11 cls          (type INT)
// #12 skeleton     (type INT)
// #13 skeletonTime (type INT)
// #14 partMask     (type INT)
// #15 teamNum      (type INT)
// #16 hp           (type INT)
// #17 hpMax        (type INT)
// #18 mp           (type INT)
// #19 mpMax        (type INT)
// #20 PetSrc       (type STRING)
// #21 arrowsCnt       (type INT)
// #22 yarost       (type INT)
// #23 max_yarost (not use for me)       (type INT)
//
// Description: personage's info
// Thread: fight
fs_srvStatus_t fs_cmd_SCCL_PERS_INFO(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT, PT_INT };
	fs_fight_t        *fight;
	fs_pers_t         *pers;
	int               persId;

	if (fs_cmdCheckParams(inPacket,paramTypes,2,false,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	if (!client->fight) return FS_SS_NO_AUTH;
	fight = client->fight;
	persId = PARAM_INT(PARAM_NEXT(inPacket));
	fs_fightLockMutex(fight);
	pers = fs_fightPersGetById(fight,persId);
	fs_fightUnlockMutex(fight);
	if (!pers) return FS_SS_WRONG_PERS;
	fs_fightPersStatus(pers);	 
	PARAM_ADD(outPacket,"iiiiissiiiiiiiiiiiisiii",pers->id,pers->ibotArtikulId,pers->status,pers->statusl,PF_CLMASK(pers->flags),pers->nick,pers->nickData,pers->level,pers->gender,pers->kind,pers->cls,pers->skeleton,pers->skeletonTime,pers->partMask,pers->teamNum,PERS_HP(pers),PERS_HPMAX(pers),PERS_MP(pers),PERS_MPMAX(pers),pers->PetSrc,pers->arrowsCnt,pers->yarost,pers->yarost_max);
	/*
	pers->id,
	pers->ibotArtikulId, //new
	pers->status,
	pers->statusl,
	PF_CLMASK(pers->flags),
	pers->nick,
	pers->nickData,
	pers->level,
	pers->gender,
	pers->kind,
	pers->cls,
	pers->skeleton,
	pers->skeletonTime,
	pers->partMask,
	pers->teamNum,
	PERS_HP(pers),
	PERS_HPMAX(pers),
	PERS_MP(pers),
	PERS_MPMAX(pers),
	pers->PetSrc,
	pers->arrowsCnt,
	pers->yarost,
	100
	*/
	
	
	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCL_PERS_PARTS)
// #1  persId       (type INT)
// Answer:
// #0  cmd          (type INT, val=FS_SCCL_PERS_PARTS)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
// #2  persId       (type INT)
// ...
// #n  pack         (type INT, id=fs_persPart_t)
// ...
//
// Description: personage's configuration
// Thread: fight
fs_srvStatus_t fs_cmd_SCCL_PERS_PARTS(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT, PT_INT };
	fs_param_t        *param;
	fs_fight_t        *fight;
	fs_pers_t         *pers;
	int               persId, part, layer;
	
	
	if (fs_cmdCheckParams(inPacket,paramTypes,2,false,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	if (!client->fight) return FS_SS_NO_AUTH;
	fight = client->fight;
	persId = PARAM_INT(PARAM_NEXT(inPacket));
	fs_fightLockMutex(fight);
	pers = fs_fightPersGetById(fight,persId);
	fs_fightUnlockMutex(fight);
	if (!pers) return FS_SS_WRONG_PERS;
	PARAM_ADD(outPacket,"is",pers->id,pers->nParts);
	
	
	//PARAM_NEW(param);
	//PARAM_ID(param) = part;
	//DEBUG('PARTS_LOADED %s', n_part);
	//PARAM_SETSTRING(param,n_part);
	
	//for (part=0; part<=FS_PPT_MAXCODE; part++) {
	//	for (layer=0; layer<FS_PPT_LAYERCNT; layer++) {
	//		
	//		
	//		strcat(str,PERS_PART(pers,part,layer)); //PERS_PART(pers,part,layer) + 
	//		strcat(str,";;;,");
	//		//n_part = addCh(n_part,";;;,");
	//	}
	//	//n_part = n_part + ",";
	//}
	//
	//char* m;
	//m = &str;
	
	DEBUG("JOPA %s",pers->nParts);
	
	//PARAM_SETSTRING(param,pers->nParts);
	//PARAM_PUSH(outPacket,param);
	
	//PARAM_ADD(outPacket,"is",pers->id,n_part);
	/*PARAM_NEW(param);
	PARAM_ID(param) = part;
	PARAM_SETSTRING(param,c_part);
	PARAM_PUSH(outPacket,param);*/
	
	
	
	return FS_SS_OK;
}

/*char[] addCh(char[] o, char[] t){
	char[] f;
	f = malloc(strlen(o)+strlen(t)+1); 
	strcpy(f, o);
	strcat(f, t);
	return f;
}*/

// Parameters:
// #0  cmd          (type INT, val=FS_SCCL_ATTACK)
// #1  part         (type INT, val=(1..6))
// #2  wpnEff       (type INT, val=(0 - off, 1 - on))
// Answer:
// #0  cmd          (type INT, val=FS_SCCL_ATTACK)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
//
// Description: personage attack
// Thread: fight
fs_srvStatus_t fs_cmd_SCCL_ATTACK(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT, PT_INT, PT_INT };
	fs_pers_t         *pers;
	int               part, wpnEff, stime;

	if (fs_cmdCheckParams(inPacket,paramTypes,3,false,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	if (!client->pers) return FS_SS_NO_AUTH;
	
	pers = client->pers;
	part = PARAM_INT(PARAM_NEXT(inPacket));
	wpnEff = PARAM_INT(PARAM_NEXT(inPacket));
	stime = fs_stime;
	
	if ((pers->fight->status != FS_FS_RUNNING) || (pers->status != FS_PS_ACTIVE) || (pers->atime > (stime - 2))) return FS_SS_WRONG_STATE;	// personage can't attack
	if ((part < 1) || (part > 6)) return FS_SS_WRONG_PART;
	if (!((1 << (part-1)) & pers->opponent->partMask)) return FS_SS_WRONG_PARTMASK;
	fs_fightLockMutex(pers->fight);
	fs_persAttack(pers,part,wpnEff);
	fs_fightUnlockMutex(pers->fight);
	pers->atime = stime;
	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCL_FIGHT_STATE)
// #1  persId       (type INT, val=(0 - all personages, !=0 - defined personage))
// #2  startIdx     (type INT)
// Answer:
// #0  cmd          (type INT, val=FS_SCCL_FIGHT_STATE)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
// #2  listSize     (type INT)
// ...
// #n   persId      (type INT)
// #+1  persStatus  (type INT, val IN fs_persStatus_t)
// #+2  persLStatus  (type INT, val IN fs_persLStatus_t)
// #+3  persFlags   (type INT, val IN fs_persFlags_t)
// #+4  nick        (type STRING)
// #+5  nickData    (type STRING)
// #+6  level       (type INT)
// #+7  teamNum     (type INT)
// #+8  hp          (type INT)
// #+9  hpMax       (type INT)
// #+10 mp          (type INT)
// #+11 mpMax       (type INT)
// ...
//
// Description: overall fight status (no more than 100 chunks at once)
// Thread: fight
fs_srvStatus_t fs_cmd_SCCL_FIGHT_STATE(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT, PT_INT, PT_INT };
	fs_pers_t         *pers;
	fs_fight_t        *fight;
	int               persId, startIdx, i;
	viter_t           vi;

	if (fs_cmdCheckParams(inPacket,paramTypes,3,false,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	if (!client->fight) return FS_SS_NO_AUTH;
	fight = client->fight;
	persId = PARAM_INT(PARAM_NEXT(inPacket));
	startIdx = PARAM_INT(PARAM_NEXT(inPacket));
	fs_fightLockMutex(fight);
	PARAM_ADD(outPacket,"i",(persId ? 0: v_size(fight->persVec)));
	if (v_jump(fight->persVec,startIdx,&vi)) {
		for (i=0; (pers = v_each(fight->persVec,&vi)) && (i < 100); ) {
			if (persId && (pers->id != persId)) continue;
			fs_fightPersStatus(pers);
			PARAM_ADD(outPacket,"iiiissiiiiii",pers->id,pers->status,pers->statusl,PF_CLMASK(pers->flags),pers->nick,pers->nickData,pers->level,pers->teamNum,PERS_HP(pers),PERS_HPMAX(pers),PERS_MP(pers),PERS_MPMAX(pers));
			i++;
		}
	}
	fs_fightUnlockMutex(fight);
	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCL_EFFECTS)
// Answer:
// #0  cmd          (type INT, val=FS_SCCL_EFFECTS)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
// ...
// #n   effId       (type INT)
// #+1  cnt         (type INT)
// #+2  effFlags    (type INT, val IN fs_persEffFlags_t)
// #+3  artId       (type INT)
// #+4  cdTime      (type INT)
// #+5  cdGrpId     (type INT)
// #+6  title       (type STRING)
// #+7  picture     (type STRING)
// #+8  SlotName    (type STRING)
// #+9  slotNum     (type INT)
// #+10  mp          (type INT)
// #+11 cdTimeLeft  (type INT)
// #+12 energy  (type INT)
// #+13 curEffSubSlot  (type INT)
// ...
//
// Description: get the effect list
// Thread: fight
fs_srvStatus_t fs_cmd_SCCL_EFFECTS(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT };
	fs_pers_t         *pers;
	fs_persEff_t      *eff;
	int cnts = 0;

	if (fs_cmdCheckParams(inPacket,paramTypes,1,false,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	if (!client->pers) return FS_SS_NO_AUTH;
	pers = client->pers;
	v_reset(pers->effVec,0);
	while ((eff = v_each(pers->effVec,0))) {	
		if (eff->flags & (FS_PEF_ACTIVE | FS_PEF_AUX | FS_PEF_PET_ASSIST)) continue;
		if ((eff->cnt <= 0) && !(eff->flags & FS_PEF_SPELL)) continue;
		if(!eff->id || eff->id == 0) continue;
		//return effect
		//info
		//2   	=> id						//_loc18_
		//2+1 	=> cnt						//_loc18_+1
		//2+2 	=> flags					//_loc18_+2
		//2+3 	=> artId					//_loc18_+3	
		//2+4 	=> cdTime					//_loc18_+4
		//2+5 	=> cdType					//_loc18_+5
		//2+6 	=> cdGrpId					//_loc18_+6
		//2+7 	=> title					//_loc18_+7
		//2+8 	=> picture					//_loc18_+8
		//2+9 	=> slotName					//_loc18_+9
		//2+10 	=> slotNum					//_loc18_+10
		//2+11 	=> mp						//_loc18_+11
		//2+12 	=> cdTimeLeft				//_loc18_+12
		//2+13 	=> energy					//_loc18_+13
		//2+14 	=> curEffSubSlot			//_loc18_+14
		PARAM_ADD(outPacket,"iiiiiiisssiiiii",eff->id,eff->cnt,PEF_CLMASK(eff->flags),eff->artId,eff->cdTime,eff->cdType,eff->cdGrpId,eff->title,eff->picture,eff->slotId,eff->slotNum,eff->mp,MAX(eff->cdrtime-fs_stime,0),eff->e_yarost,eff->subSlot);
	}
	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCL_USE_EFFECT)
// #1  effId        (type INT)
// #2  targetId     (type INT, val=(0 - self, >0 - personage in the fight))
// Answer:
// #0  cmd          (type INT, val=FS_SCCL_USE_EFFECT)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
// #2  effId        (type INT)
// #3  targetId     (type INT)
// #4  usageStatus  (type INT, optional)
//
// Description: use an effect
// Thread: fight
fs_srvStatus_t fs_cmd_SCCL_USE_EFFECT(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT, PT_BIGINT, PT_INT };
	fs_pers_t         *pers, *target;
	fs_fight_t        *fight;
	fs_persEff_t      *eff;
	int               effId, targetId, stime, usageStatus;
	errno_t           status;

	if (fs_cmdCheckParams(inPacket,paramTypes,3,false,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	if (!client->pers) return FS_SS_NO_AUTH;
	pers = client->pers;
	effId = PARAM_INT(PARAM_NEXT(inPacket));
	targetId = PARAM_INT(PARAM_NEXT(inPacket));
	if (pers->fight->status != FS_FS_RUNNING) return FS_SS_WRONG_STATE;	// personage can't use effects
	v_reset(pers->effVec,0);
	while ((eff = v_each(pers->effVec,0))) {
		if (eff->flags & (FS_PEF_ACTIVE | FS_PEF_AUX)) continue;
		if (eff->id == effId) break;
	}
	if (targetId > 0) {
		fight = pers->fight;
		fs_fightLockMutex(fight);
		target = fs_fightPersGetById(fight,targetId);
		fs_fightUnlockMutex(fight);
	} else target = pers;
	PARAM_ADD(outPacket,"ii",effId,targetId);
	if (!eff || !target) return FS_SS_WRONG_DATA2;
	stime = fs_stime;
	if (pers->fight->status != FS_FS_RUNNING) return FS_SS_WRONG_STATE;	// personage can't use effects
	if ((eff->flags & FS_PEF_SPELL) && (pers->eutime > (stime - FIGHT_MIN_SPELL_COOLDOWN))) return FS_SS_WRONG_STATE;	// checking spell cooldown
	usageStatus = 0;
	status = fs_persUseEffect(pers,eff,target,&usageStatus);
	PARAM_ADD(outPacket,"i",usageStatus);
	if (status != OK) return (status == ERR_GENERAL) ? FS_SS_WRONG_STATE : FS_SS_INT_ERROR;
	if (eff->flags & FS_PEF_SPELL) pers->eutime = stime;	// updating spell cooldown
	
	pers->lastEffectUpdateIndex++;
	
	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCL_CHANGE_MODE)
// #1  mode         (type INT, val=(0 - change FS_PF_DEFENDED flag, 1 - change FS_PF_MAGIC flag))
// #2  modeSwitch   (type INT, val=(0 - off, 1 - on))
// Answer:
// #0  cmd          (type INT, val=FS_SCCL_CHANGE_MODE)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
// #2  mode         (type INT)
//
// Description: change personage mode
// Thread: fight
fs_srvStatus_t fs_cmd_SCCL_CHANGE_MODE(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT, PT_INT, PT_INT };
	fs_pers_t         *pers;
	int               mode, modeSwitch;
	fs_persEff_t      *eff;

	if (fs_cmdCheckParams(inPacket,paramTypes,3,false,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	if (!client->pers) return FS_SS_NO_AUTH;
	pers = client->pers;
	if(!pers->fight) return FS_SS_NO_AUTH;
	if(pers->fight->status != FS_FS_RUNNING) return FS_SS_NO_AUTH;
	mode = PARAM_INT(PARAM_NEXT(inPacket));
	modeSwitch = PARAM_INT(PARAM_NEXT(inPacket));
	//PARAM_ADD(outPacket,"i",mode);
	switch (mode) {
		case 0:
			if (pers->status != FS_PS_ACTIVE) return FS_SS_WRONG_STATE;
			if (pers->fight->flags & FS_FF_NO_BLOCK) return FS_SS_WRONG_STATE;
			if (modeSwitch) {
				pers->flags |= FS_PF_DEFENDED;
				pers->flags &= ~FS_PF_MAGIC;
			} else pers->flags &= ~FS_PF_DEFENDED;
			break;
		case 1:
			if (pers->status != FS_PS_ACTIVE) return FS_SS_WRONG_STATE;
			if (modeSwitch) {
				pers->flags |= FS_PF_MAGIC;
				pers->flags &= ~FS_PF_DEFENDED;
			} else pers->flags &= ~FS_PF_MAGIC;
			break;
		case 2:
			 
			break;
		default:
			return FS_SS_WRONG_DATA2;
	}
	PARAM_ADD(outPacket,"i",PF_CLMASK(pers->flags));
	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCL_PERS_ACT_EFFECTS)
// #1  persId       (type INT)
// Answer:
// #0  cmd          (type INT, val=FS_SCCL_PERS_ACT_EFFECTS)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
// #2  persId       (type INT)
// ...
// #n   artId       (type INT)
// #+1  cnt         (type INT)
// #+2  title       (type STRING)
// #+3  picture     (type STRING)
// #+4  animData    (type STRING)
// #+5  eetimeMax   (type INT)
// ...
//
// Description: personage's active effects grouped by artId
// Thread: fight
fs_srvStatus_t fs_cmd_SCCL_PERS_ACT_EFFECTS(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT, PT_INT };
	fs_fight_t        *fight;
	fs_pers_t         *pers;
	fs_persEff_t      *eff, *temp;
	int               persId;
	vector_t          v;
	int cnts = 0;

	if (fs_cmdCheckParams(inPacket,paramTypes,2,false,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	if (!client->fight) return FS_SS_NO_AUTH;
	fight = client->fight;
	persId = PARAM_INT(PARAM_NEXT(inPacket));
	fs_fightLockMutex(fight);
	pers = fs_fightPersGetById(fight,persId);
	fs_fightUnlockMutex(fight);
	if (!pers) return FS_SS_WRONG_PERS;
	if(!fight) return FS_SS_NO_AUTH;
	if(fight->status != FS_FS_RUNNING) return FS_SS_NO_AUTH;
	PARAM_ADD(outPacket,"i",pers->id);
	v_init(&v);
	v_reset(pers->effVec,0);
	while ((eff = v_each(pers->effVec,0))) {
		if (!(eff->flags & FS_PEF_ACTIVE)) continue;
		if (!eff->title || !eff->picture) continue;
		v_reset(&v,0);
		while ((temp = v_each(&v,0))) {		
//			if (!strcmp(temp->picture,eff->picture)) break;	// grouping by picture
			if (temp->artId == eff->artId) break;	// grouping by artId
		}
		if (temp) {
			temp->cnt++;
			temp->eetime = MAX(temp->eetime,eff->eetime);
		} else {
			temp = fs_persEffCopy(eff);
			temp->cnt = 1;
			v_push(&v,temp);
		}
	}
	v_reset(&v,0);
	while ((temp = v_each(&v,0))) {
		cnts = (temp->actMoveCnt > 0 ? temp->actMoveCnt : (temp->slotId == "GOD_EFFECT" ? 1 : temp->cnt));
		PARAM_ADD(outPacket,"iisssii",temp->artId,cnts,temp->title,temp->picture,temp->animData,temp->eetime,temp->turnsLeft);
		fs_persEffDelete(temp);
	}
	v_zero(&v);
	
	PARAM_ADD(outPacket,"i", (pers->lastEffectUpdateIndex)); //persLastEffectsUpdateIndex
	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCL_SEND_MSG)
// #1  persId       (type INT)
// #2  code         (type INT)
// #3  msg          (type STRING)
// Answer:
// #0  cmd          (type INT, val=FS_SCCL_SEND_MSG)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
//
// Açıklama: send a message
// Thread: fight
fs_srvStatus_t fs_cmd_SCCL_SKIP_TURN(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT };
	fs_pers_t         *pers;
	
	if (fs_cmdCheckParams(inPacket,paramTypes,1,false,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}

	if(!client){
		return FS_SS_OK;
	}
	pers = client->pers;
	
	if(!pers){
		return FS_SS_OK;
	}
	if(!pers->fight) return FS_SS_WRONG_STATE;	

	if (pers->fight->status != FS_FS_RUNNING) return FS_SS_WRONG_STATE;	// personage can't use effects
	if(pers->status != FS_PS_ACTIVE){
		return FS_SS_OK;
	}
	
	if(!pers->opponent){
		return FS_SS_OK;
	}
	
	if(pers->opponent){
		fs_fightLockMutex(pers->fight);
		//fs_persGetCharge(pers,FS_PDT_PHYSICAL,&charge, true);
		fs_persDischarge(pers,FS_PDT_PHYSICAL); //Фикс чардж, если пропустили ход эффекты от чардж должны отняться
		fs_persGetTurn(pers->opponent);
		fs_fightUnlockMutex(pers->fight);
	}

	return FS_SS_OK;
}


fs_srvStatus_t fs_cmd_SCCL_EFFUPDATE(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT, PT_INT, PT_INT, PT_STRING };
	fs_pers_t         *pers;
	fs_fight_t        *fight;
	int               code;
	char              *msg;


	return FS_SS_OK;
}
// Parameters:
// #0  cmd          (type INT, val=FS_SCCL_SEND_MSG)
// #1  persId       (type INT)
// #2  code         (type INT)
// #3  msg          (type STRING)
// Answer:
// #0  cmd          (type INT, val=FS_SCCL_SEND_MSG)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
//
// Açıklama: send a message
// Thread: fight
fs_srvStatus_t fs_cmd_SCCL_MYFIGHTRETURN(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT, PT_INT, PT_INT, PT_STRING };
	fs_pers_t         *pers;
	fs_fight_t        *fight;
	int               code;
	char              *msg;


	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCL_SEND_MSG)
// #1  persId       (type INT)
// #2  code         (type INT)
// #3  msg          (type STRING)
// Answer:
// #0  cmd          (type INT, val=FS_SCCL_SEND_MSG)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
//
// Açıklama: send a message
// Thread: fight
fs_srvStatus_t fs_cmd_SCCL_NEWPERS(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT, PT_INT, PT_INT, PT_STRING };
	fs_pers_t         *pers;
	fs_fight_t        *fight;
	int               code;
	char              *msg;


	return FS_SS_OK;
}
// Parameters:
// #0  cmd          (type INT, val=FS_SCCL_SEND_MSG)
// #1  persId       (type INT)
// #2  code         (type INT)
// #3  msg          (type STRING)
// Answer:
// #0  cmd          (type INT, val=FS_SCCL_SEND_MSG)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
//
// Açıklama: send a message
// Thread: fight
fs_srvStatus_t fs_cmd_SCCL_RESETCOMBO(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT, PT_INT, PT_INT, PT_STRING };
	fs_pers_t         *pers;
	fs_fight_t        *fight;
	int               code;
	char              *msg;


	return FS_SS_OK;
}
// Parameters:
// #0  cmd          (type INT, val=FS_SCCL_SEND_MSG)
// #1  persId       (type INT)
// #2  code         (type INT)
// #3  msg          (type STRING)
// Answer:
// #0  cmd          (type INT, val=FS_SCCL_SEND_MSG)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
//
// Açıklama: send a message
// Thread: fight
fs_srvStatus_t fs_cmd_SCCL_EFFSWAP(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT, PT_INT, PT_INT, PT_STRING };
	fs_pers_t         *pers;
	fs_fight_t        *fight;
	int               code;
	char              *msg;

	//NO ACTION, ONLY EVENT NO SCCL!!!

	return FS_SS_OK;
}

// Parameters:
// #0  cmd          (type INT, val=FS_SCCL_SEND_MSG)
// #1  persId       (type INT)
// #2  code         (type INT)
// #3  msg          (type STRING)
// Answer:
// #0  cmd          (type INT, val=FS_SCCL_SEND_MSG)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
//
// Description: send a message
// Thread: fight
fs_srvStatus_t fs_cmd_SCCL_SEND_MSG(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT, PT_INT, PT_INT, PT_STRING };
	fs_pers_t         *pers;
	fs_fight_t        *fight;
	int               code;
	char              *msg;

	if (fs_cmdCheckParams(inPacket,paramTypes,4,false,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	if (!client->fight || !client->pers) return FS_SS_NO_AUTH;
	fight = client->fight;
	if(!fight) return FS_SS_NO_AUTH;
	if(fight->status != FS_FS_RUNNING) return FS_SS_NO_AUTH;
	fs_fightLockMutex(fight);
	pers = fs_fightPersGetById(fight,PARAM_INT(PARAM_NEXT(inPacket)));
	fs_fightUnlockMutex(fight);
	if (!pers) return FS_SS_WRONG_PERS;
	code = PARAM_INT(PARAM_NEXT(inPacket));
	msg = PARAM_STRING(PARAM_NEXT(inPacket));
	fs_persSetEvent(pers,FS_PE_MSG,"iis",client->pers->id,code,msg);
	return FS_SS_OK;
}


// ---------------------------------------------------- CTRL COMMANDS ----------------------------------------------------

// Parameters:
// #0  cmd          (type INT, val=fs_cmd_SCCT_GET_LOG_AND_EFFECTS)
// #1  fightId      (type INT)
// #2  mode         (type INT, val=(0-overall, 1-persData, 2-effData, 3-cmbData))
// #3  startIdx     (type INT)
// Answer:
// #0  cmd          (type INT, val=fs_cmd_SCCT_GET_LOG_AND_EFFECTS)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
// ------------ mode==0 ------------
// #2  ctime        (type INT)
// #3  rtime        (type INT)
// #4  winnerTeam   (type INT)
// ---------------------------------
// ------------ mode==1 ------------
// #2  listSize     (type INT)
// ...
// #n   persId       (type INT)
// #+1  persStatus   (type INT)
// #+2  persFlags    (type INT, val IN fs_persFlags_t)
// #+3  teamNum      (type INT)
// #+4  dmg          (type INT)
// #+5  heal         (type INT)
// #+6  exp          (type INT)
// #+7  honor        (type INT)
// #+8  hp           (type INT)
// #+9  mp           (type INT)
// #+10 killCnt      (type INT)
// #+11 enemyKillCnt (type INT)
// ...
// ---------------------------------
// ------------ mode==2 ------------
// #2  listSize     (type INT)
// ...
// #n   effId        (type INT)
// #+1  persId       (type INT)
// #+2  cnt          (type INT)
// ...
// ---------------------------------
// ------------ mode==3 ------------
// #2  listSize     (type INT)
// ...
// #n   cmbId        (type INT)
// #+1  persId       (type INT)
// #+2  useCnt       (type INT)
// ...
// ---------------------------------
//
// Açıklama: get the fight info (no more than 100 chunks at once in the lists)
// Thread: worker
fs_srvStatus_t fs_cmd_SCCT_GET_LOG_AND_EFFECTS(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t         paramTypes[] = { PT_INT, PT_INT, PT_INT, PT_INT };
	fs_fightInfo_t         *info;
	fs_fightInfoPersData_t *persData;
	fs_fightInfoEffData_t  *effData;
	fs_fightInfoCmbData_t  *cmbData;
	int                    fightId, mode, startIdx, i;
	viter_t                vi;

	
	if (fs_cmdCheckParams(inPacket,paramTypes,4,false,false) != OK) {
		WARN("Geçersiz Argument");
		return FS_SS_WRONG_ARGS;
	}
	fightId = PARAM_BIGINT(PARAM_NEXT(inPacket));
	mode = PARAM_INT(PARAM_NEXT(inPacket));
	startIdx = PARAM_INT(PARAM_NEXT(inPacket));
	info = fs_fightInfoGetByIdEff(fightId);
	INFO("FIGHTINFOLOGREG REQ [id: %d, mode: %d]",fightId,mode);
	if (!info) return FS_SS_WRONG_INFO;
	switch (mode) {
		case 0:	// overall
			PARAM_ADD(outPacket,"iii",info->fctime,info->frtime,info->winnerTeam);
			break;
		case 2:	// effData
			PARAM_ADD(outPacket,"i",v_size(info->effDataVec));
			if (v_jump(info->effDataVec,startIdx,&vi)) {
				for (i=0; (effData = v_each(info->effDataVec,&vi)) && (i < 100); i++) {
					PARAM_ADD(outPacket,"iii",effData->id,effData->persId,effData->cnt);
					//PARAM_ADD(outPacket,"iiii",effData->id,effData->persId,effData->cnt,effData->subSlot);
				}
			}
			break;
		case 3:	// cmbData
			PARAM_ADD(outPacket,"i",v_size(info->cmbDataVec));
			if (v_jump(info->cmbDataVec,startIdx,&vi)) {
				for (i=0; (cmbData = v_each(info->cmbDataVec,&vi)) && (i < 100); i++) {
					PARAM_ADD(outPacket,"iii",cmbData->id,cmbData->persId,cmbData->useCnt);
				}
			}
			break;
		default:
			return FS_SS_WRONG_DATA2;
	}
	INFO("FIGHTINFOLOGREG GIVEN [id: %d, mode: %d]",info->id,mode);
	return FS_SS_OK;
}

// Parameters: 
// #0  cmd          (type INT, val=FS_SCCT_SET_PARAMS_LUA)
// #1  persId       (type INT)
// ...
// #n  value        (type INT, id=fs_skill_lua_t)
// ...
// Answer:
// #0  cmd          (type INT, val=FS_SCCT_SET_PARAMS_LUA)
// #1  status       (type INT/NINT, val IN fs_srvStatus_t)
//
// Thread: worker
fs_srvStatus_t fs_cmd_SCCT_SET_PARAMS_LUA(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket) {
	fs_paramType_t    paramTypes[] = { PT_INT, PT_INT, PT_STRING, PT_STRING };
	fs_param_t        *param;
	fs_pers_t         *pers; 
	fs_fight_t        *fight;
	fs_luaParam_t	  *luaParam;
	int 	id, i;
	char	*to;
	char 	*lua;
	char	*persStr;
	char	*fightStr;
	if (fs_cmdCheckParams(inPacket,paramTypes,4,true,false) != OK) {
		WARN("Invalid Argument");
		return FS_SS_WRONG_ARGS; 
	}
	
	fight = fs_fightGetById(PARAM_BIGINT(PARAM_NEXT(inPacket)));
	if(!fight) return FS_SS_OK;
	if(fight->status == FS_FS_FINISHED) return FS_SS_OK;
	to = PARAM_STRING(PARAM_NEXT(inPacket));
	lua = PARAM_STRING(PARAM_NEXT(inPacket));
	DEBUG("Insert TO : %s LUA : %s",to,lua);
	
	if(!fight){
		DEBUG("HEI, GDE BOY");
		return FS_SS_OK;
	}
	
	luaParam = fs_luaParamCreate(lua);
	if(!luaParam){
		DEBUG("LUAPARAM NOT CREATED!");
		return FS_SS_OK;
	}
	
	persStr = "Pers";
	fightStr = "Fight";
	
  fs_fightLockMutex(fight);
	if(strcmp(to, persStr) == 0){
		v_push(fight->persLuaParams, luaParam);
	}else if(strcmp(to, fightStr) == 0){
		v_push(fight->LuaParams, luaParam);
	}
	fs_fightUnlockMutex(fight);
	return FS_SS_OK;
}


fs_srvStatus_t fs_cmd_SCCL_DROP_EFFECT(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket){
	fs_paramType_t    paramTypes[] = { PT_INT, PT_BIGINT };
	fs_pers_t         *pers, *target;
	fs_fight_t        *fight;
	fs_persEff_t      *eff;
	int               effId;
	errno_t           status;

	if (fs_cmdCheckParams(inPacket,paramTypes,2,false,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	if (!client->pers) return FS_SS_NO_AUTH;
	pers = client->pers;
	if(!pers) return FS_SS_WRONG_STATE;
	if(!pers->fight) return FS_SS_WRONG_STATE;
	if(pers->fight->status != FS_FS_RUNNING) return FS_SS_WRONG_STATE;
	fight = pers->fight;
	effId = PARAM_BIGINT(PARAM_NEXT(inPacket));
	DEBUG("EFFECT_ID [%d]",effId);
	v_reset(pers->effVec,0);
    while ((eff = v_each(pers->effVec,0))) {
        if (!(eff->flags & FS_PEF_ACTIVE) || !(eff->flags & FS_PEF_AURA)) continue;
        if (eff->flags & FS_PEF_AURA) break;
    }
	if(!eff){ return FS_SS_OK; } //FIX
	if(!(eff->flags & FS_PEF_AURA)){ return FS_SS_OK; } //FIX
	if(eff->id != effId){ return FS_SS_OK; } //FIX
    if (eff) {
		DEBUG("DROP_EFFECT! [%d]",effId);
        eff->flags |= FS_PEF_DROP;
    }
	
	return FS_SS_OK;
}

fs_srvStatus_t fs_cmd_SCCL_EFFECT_SWAP_SUBSLOT(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket){
	fs_paramType_t    paramTypes[] = { PT_INT, PT_BIGINT };
	fs_pers_t         *pers, *target;
	fs_fight_t        *fight;
	fs_persEff_t      *eff;
	fs_persEff_t      *effS;
	int               effId, targetId, stime, usageStatus;
	int				  n_subSlot;
	errno_t           status;

	if (fs_cmdCheckParams(inPacket,paramTypes,2,false,false) != OK) {
		WARN("Invalid parameters");
		return FS_SS_WRONG_ARGS;
	}
	if (!client->pers) return FS_SS_NO_AUTH;
	pers = client->pers;
	if(!pers){
		return FS_SS_OK;
	}
	if(pers->status == FS_PS_DEAD){
		//Ты не пройдешь подлый трус!
		return FS_SS_OK;
	}
	if(!pers->fight) return FS_SS_WRONG_STATE;
	if(pers->fight->status != FS_FS_RUNNING) return FS_SS_WRONG_STATE;
	
	effId = PARAM_BIGINT(PARAM_NEXT(inPacket));
	v_reset(pers->effVec,0);
	while ((eff = v_each(pers->effVec,0))) {
		if(!eff || eff->id <= 0) continue;
		if (eff->flags & FS_PEF_ACTIVE || eff->flags & FS_PEF_AUX || eff->flags & FS_PEF_SPELL) {
			//DEBUGXXX("CHECK EFFECT [%d], TITLE: %s, SUBSLOT: %d",eff->id,eff->title,eff->subSlot);
			//if(eff->flags & FS_PEF_ACTIVE) { DEBUGXXX("FS_PEF_ACTIVE"); }
			//if(eff->flags & FS_PEF_AUX) { DEBUGXXX("FS_PEF_AUX"); }
			//if(eff->flags & FS_PEF_SPELL) { DEBUGXXX("FS_PEF_SPELL"); }
			continue;
		}
		if (eff->id == effId) break;
	}
	
	if(!eff){
		return FS_SS_OK; //fix crash fightd on effswap sub slots
	}
	
	n_subSlot = 2;
	v_reset(pers->effVec,0);
	fs_fightLockMutex(pers->fight); //Лочик
	//Свапаем онли слотовые субслоты
	while ((effS = v_each(pers->effVec,0))) {
		
		if(!effS || effS->id <= 0) continue;
		if (effS->flags & FS_PEF_ACTIVE || effS->flags & FS_PEF_AUX || effS->flags & FS_PEF_SPELL) continue;
		
		if(effS->slotNum == eff->slotNum && effS->subSlot > 0) {
			if(eff->id == effS->id){
				effS->subSlot = 1; //ENGLISH MF
			}else{
				effS->subSlot = (n_subSlot % 2 ? 2 : 3);
				n_subSlot++;
			}
		}
	}
	fs_persSetEvent(pers, FS_PE_EFFSWAP,"iiii", eff->id, eff->subSlot, 0, 0); // WHO IS 1?
	fs_fightUnlockMutex(pers->fight); //Анлочик
	return FS_SS_OK;
}