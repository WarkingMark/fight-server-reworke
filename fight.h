/*
 * modifed: igorpauk 2017-18
 */

#ifndef __FIGHT_H__
#define __FIGHT_H__

#include "typedefs.h"


#define FIGHT_OVER_TTL             60
#define FIGHT_DEFAULT_TIMEOUT      30
#define FIGHT_MIN_SPELL_COOLDOWN   3
#define FIGHT_MAX_TIMEOUT_CNT      4


enum fs_fightStatus_e {
	FS_FS_CREATED     = 0,
	FS_FS_RUNNING     = 1,
	FS_FS_OVER        = 2,
	FS_FS_FINISHED    = 3
};

enum fs_fightFlags_e {
	FS_FF_TH_STARTED  = 0x01,
	FS_FF_TH_STOPPED  = 0x02,
	FS_FF_NO_BLOCK    = 0x04,
};

enum fs_fightSignal_e {
	FS_FSIG_NONE   = 0,
	FS_FSIG_STOP   = 1,
};

enum fs_fightLogCode_e {
	FS_FLC_KICK       =  1, // i1-kick, i2-part, i3-dmg
	FS_FLC_DEATH      =  2,	// i1=(1 - FS_PF_TIMEOUTKILL is set), i2-killerId
	FS_FLC_INTRUSION  =  3,
	FS_FLC_EFFECTUSE  =  4, // i1-effId, i2-(direct dmg or -1) s1-effTitle
	FS_FLC_SHIP  =  5,  // shipi
	FS_FLC_PROB  =  6,  // probivanie
	FS_FLC_VAMP  =  7,  // vampiric
	FS_FLC_COMMON  =  8,  // common
	FS_FLC_FIGHTOVER = 9, //fight_finish
};


struct fs_fight_s {
	int              id;
	int               ctime, mtime, utime, intPipeSock, extPipeSock;
	fs_fightStatus_t  status;
	fs_fightFlags_t   flags;
	
	int				  Team_1_DeadCnt, Team_2_DeadCnt;

	int				  otime; //Отложенный запуск тайм
	
	vector_t          *clientVec;
	vector_t          *persVec;
	/*new 3*/
	vector_t          *persDataVec;	// 'fs_fightInfoPersData_t' vector
	vector_t          *effDataVec;	// 'fs_fightInfoEffData_t' vector
	vector_t          *cmbDataVec;	// 'fs_fightInfoCmbData_t' vector
	
	int               timeout;
	fs_fightLog_t     *log;

	lua_State         *L;
	vector_t          *LuaParams; // 'fs_luaParam_t' vector
	vector_t          *persLuaParams; // 'fs_luaParam_t' vector
	pthread_t         th;
	pthread_mutex_t   mutex, mutex_cl, mutex_lua;
};

struct fs_luaParam_s {
	char *param;
};

struct fs_fightLog_s {
	vector_t          dataVec;	// 'fs_fightLogData_t' vector
	pthread_mutex_t   mutex;
};

struct fs_fightLogData_s {
	int               ctime;
	int               persId, persHp, persHpMax;
	int               oppId, oppHp, oppHpMax;
	fs_fightLogCode_t code;
	int               i1, i2, i3;
	char              *s1;
};

struct fs_fightInfo_s {
	int              id;
	int               ctime;
	int               fctime, frtime, winnerTeam;
	vector_t          *persDataVec;	// 'fs_fightInfoPersData_t' vector
	vector_t          *effDataVec;	// 'fs_fightInfoEffData_t' vector
	vector_t          *cmbDataVec;	// 'fs_fightInfoCmbData_t' vector
	fs_fightLog_t     *log;
};

struct fs_fightInfoPersData_s {
	int               id, status, flags, teamNum;
	int               dmg, heal, exp, honor, hp, mp, killCnt, enemyKillCnt, arrowsCnt;
};

struct fs_fightInfoEffData_s {
	int               id, persId, cnt;
	int				  cdrtime, slotNum, subSlot;
};

struct fs_fightInfoCmbData_s {
	int               id, persId, useCnt;
};


fs_fight_t *fs_fightCreate(int id);
errno_t fs_fightDelete(fs_fight_t *fight);
fs_fight_t *fs_fightGetById(int id);
fs_pers_t *fs_fightPersGetById(fs_fight_t *fight, int id);
fs_fightLog_t *fs_fightLogCreate(void);
errno_t fs_fightLogDelete(fs_fightLog_t *fightLog);
fs_fightInfo_t *fs_fightInfoCreate(int id);
errno_t fs_fightInfoDelete(fs_fightInfo_t *fightInfo);
fs_fightInfo_t *fs_fightInfoGetById(int id);
fs_fightInfo_t *fs_fightInfoGetByIdEff(int id);
errno_t fs_fightLockMutex(fs_fight_t *fight);
errno_t fs_fightUnlockMutex(fs_fight_t *fight);
errno_t fs_fightLogLockMutex(fs_fightLog_t *fightLog);
errno_t fs_fightLogUnlockMutex(fs_fightLog_t *fightLog);

errno_t fs_fightThreadStart(fs_fight_t *fight);
errno_t fs_fightThreadRoutine(fs_fight_t *fight);
errno_t fs_fightThreadStop(fs_fight_t *fight);
errno_t fs_fightSignal(fs_fight_t *fight, fs_fightSignal_t sig);

errno_t fs_fightUpdateState(fs_fight_t *fight, int *pairMade, int *persActive, int *persDead, int *persPending);
errno_t fs_fightBroadcastStateChange(fs_fight_t *fight);
errno_t fs_fightSaveLog(fs_pers_t *pers, fs_fightLogCode_t code, int i1, int i2, int i3, char *s1, bool silent);
errno_t fs_fightGenerateInfo(fs_fight_t *fight);

errno_t fs_fightDeadCnt(fs_fight_t *fight, int team, int add);
errno_t fs_fightDeadCntSend(fs_fight_t *fight, fs_pers_t *pers);
errno_t fs_fightDeadCntSendAll(fs_fight_t *fight);
errno_t fs_fightPersStatus(fs_pers_t *pers);
errno_t fs_fightAutoActiveEffect(fs_pers_t *pers);

errno_t fs_fightLuaInitParams(fs_fight_t *fight);

fs_luaParam_t *fs_luaParamCreate(char *param);
errno_t fs_luaParamDelete(fs_luaParam_t *luaParam);

#endif
