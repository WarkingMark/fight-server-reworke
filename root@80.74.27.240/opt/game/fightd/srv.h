/* 
 * modifed: igorpauk 2017-18
 */

#ifndef __SRV_H__
#define __SRV_H__

#include "typedefs.h"


#define MAX_WORKER_CNT        16
#define MAX_POLL_SIZE         4096
#define CLIENT_TTL            1800
#define CLEANUP_INTERVAL      1
#define INACTIVE_WORKER_TTL   1800
#define INACTIVE_TIMEOUT      1800

#define SRV_OPT(A)            (fs_srvOptions[(A)])


enum fs_srvCommand_e {
	FS_SC_UNDEFINED             =  -1,
	FS_SC_NONE                  =   0,

	// common
	FS_SC_SYNC_TIME             =   1,

	// ctrl
	FS_SCCT_SRV_INFO            =  11,
	FS_SCCT_DEBUG_LEVEL         =  12,
	FS_SCCT_GET_SRV_OPTS        =  13,
	FS_SCCT_SET_SRV_OPTS        =  14,
	FS_SCCT_GET_FIGHTS          =  15,
	FS_SCCT_CREATE_FIGHT        =  16,
	FS_SCCT_SET_FIGHT_PARAMS    =  17,
	FS_SCCT_START_FIGHT         =  18,
	FS_SCCT_STOP_FIGHT          =  19,
	FS_SCCT_DELETE_FIGHT        =  20,
	FS_SCCT_CREATE_PERS         =  21,
	FS_SCCT_SET_SKILLS          =  22,
	FS_SCCT_SET_PARTS           =  23,
	FS_SCCT_ADD_EFFECT          =  24,
	FS_SCCT_ADD_COMBO           =  25,
	FS_SCCT_BIND_PERS           =  26,
	FS_SCCT_DELETE_PERS         =  27,
	FS_SCCT_GET_FIGHTSTATE      =  28,
	FS_SCCT_GET_FIGHTLOG        =  29,
	FS_SCCT_GET_FIGHTINFO       =  30,
	FS_SCCT_DELETE_FIGHTINFO    =  31,
	FS_SCCT_SET_PARAMS_LUA      =  32,
	FS_SCCT_GET_LOG_AND_EFFECTS =  33,

	FS_SCCT_BOTDMG_SKILLS = 99,

	// client
	FS_SCCL_INIT                = 101,
	FS_SCCL_STATE               = 102,
	FS_SCCL_PERS_INFO           = 103,
	FS_SCCL_PERS_PARTS          = 104,
	FS_SCCL_ATTACK              = 105,
	FS_SCCL_FIGHT_STATE         = 106,
	FS_SCCL_EFFECTS             = 107,
	FS_SCCL_USE_EFFECT          = 108,
	FS_SCCL_CHANGE_MODE         = 109,
	FS_SCCL_PERS_ACT_EFFECTS    = 110,
	FS_SCCL_WATCH_FIGHT   		= 111,
	FS_SCCL_EFFECT_SWAP_SUBSLOT = 112,
	FS_SCCL_DROP_EFFECT			= 113,
	FS_SCCL_SEND_MSG            = 201,
	FS_SCCL_SKIP_TURN		 = 114,
	FS_SCCL_MYFIGHTRETURN    = 115,	// INT persId, INT code, STRING msg - Mesaj
	FS_SCCL_EFFUPDATE        = 116,	// INT persId, INT code, STRING msg - Mesaj
	FS_SCCL_ENERGYCONSUM     = 117,	// INT persId, INT code, STRING msg - Mesaj
	FS_SCCL_ENERGYREGEN      = 118,	// INT persId, INT code, STRING msg - Mesaj
	FS_SCCL_ARROWCONSUM      = 119,	// INT persId, INT code, STRING msg - Mesaj
	FS_SCCL_EFFSWAP          = 120,	// INT persId, INT code, STRING msg - Mesaj
	FS_SCCL_RESETCOMBO       = 121,	// INT persId, INT code, STRING msg - Mesaj
	FS_SCCL_NEWPERS          = 122,	// INT persId, INT code, STRING msg - Mesaj
	FS_SCCL_DEADCNT          = 123,	// INT persId, INT code, STRING msg - Mesa	
	FS_SCCL_PERS_SUBSCRIBE      = 202,

};

enum fs_srvStatus_e {
	FS_SS_OK                  =   0,
	FS_SS_WRONG_CMD           =  -1,
	FS_SS_WRONG_ARGS          =  -2,
	FS_SS_WRONG_DATA          =  -3,
	FS_SS_WRONG_STATE         =  -4,
	FS_SS_NO_AUTH             =  -5,
	FS_SS_INT_ERROR           =  -6,
	FS_SS_WRONG_PART          =  -7,
	FS_SS_WRONG_PARTMASK      =  -8,
	FS_SS_WRONG_LAYER         =  -9,
	FS_SS_WRONG_DEBUG_LEVEL   =  -10,
	FS_SS_WRONG_FIGHT         =  -11,
	FS_SS_WRONG_PERS          =  -12,
	FS_SS_WRONG_SKILL         =  -13,
	FS_SS_WRONG_INFO          =  -14,
	FS_SS_WRONG_DATA2         =  -15,
	FS_SS_WRONG_PART2         =  -16,
	FS_SS_WRONG_ACCESS        =  -17
};

enum fs_srvOption_e {
	FS_SO_MAX_FIGHT_CNT    =  0,
};
#define FS_SO_MAXCODE    0

enum fs_clientFlags_e {
	FS_CF_CTRL             = 0x01,
	FS_CF_DISCONN          = 0x02,
	FS_CF_IO_RD            = 0x04,
	FS_CF_IO_WR            = 0x08,
};

enum fs_workerSignal_e {
	FS_WS_NONE   = 0,
	FS_WS_EXIT   = 1,
};


struct fs_client_s {
	int               sock, pidx, iotime;
	unsigned long     ipAddr;
	fs_clientFlags_t  flags;

	char              *inBuf, *outBuf, tbuf[10];
	int               inLen, outLen, inPos, outPos;
	vector_t          *inPacketVec, *outPacketVec;

	fs_pers_t         *pers;
	fs_fight_t        *fight;

	bool              __policy_file_request;
};

struct fs_worker_s {
	int               mtime, intPipeSock, extPipeSock;
	vector_t          *clientVec;

	pthread_t         th;
	pthread_mutex_t   mutex;
};


extern int               fs_ipAddr;
extern int               fs_ctrlSock, fs_clSock;
extern bool              fs_srvLoop;
extern int               fs_ctime, fs_stime;

extern vector_t          *fs_workerVec;
extern vector_t          *fs_fightVec;
extern vector_t          *fs_persVec;
extern vector_t          *fs_persLua;
extern vector_t          *fs_fightInfoVec;
extern vector_t          *fs_fightInfoVec1;
extern pthread_mutex_t   fs_mutex;

extern fs_srvOptionVal_t fs_srvOptions[];


errno_t fs_srvLockMutex(void);
errno_t fs_srvUnlockMutex(void);

fs_client_t *fs_clientCreate(int sock);
errno_t fs_clientDelete(fs_client_t *client);
errno_t fs_clientDisconnect(fs_client_t *client);
errno_t fs_clientRead(fs_client_t *client);
errno_t fs_clientWrite(fs_client_t *client);
bool __check_policy_file_request(fs_client_t *client);	// Flash had us all here...

fs_worker_t *fs_workerCreate(void);
errno_t fs_workerDelete(fs_worker_t *worker);
errno_t fs_workerLockMutex(fs_worker_t *worker);
errno_t fs_workerUnlockMutex(fs_worker_t *worker);
errno_t fs_workerThreadRoutine(fs_worker_t *worker);
errno_t fs_workerSignal(fs_worker_t *worker, fs_workerSignal_t sig);

errno_t fs_srvInit(char *host, int ctrlPort, int clPort);
errno_t fs_srvRun(void);
errno_t fs_srvAccept(int sock);
errno_t fs_srvCleanup(void);
errno_t fs_srvDone(void);
errno_t fs_processClient(fs_client_t *client);

errno_t fs_feedbackInit(char *url);
errno_t fs_feedbackDone(void);
errno_t fs_feedbackData(char *fmt, ...);
errno_t fs_feedbackThreadRoutine(char *data);

#endif
