
#ifndef __TYPEDEFS_H__
#define __TYPEDEFS_H__


// io.c
typedef enum fs_paramType_e fs_paramType_t;
typedef struct fs_packet_s fs_packet_t;
typedef struct fs_param_s fs_param_t;

// srv.c
typedef enum fs_srvCommand_e fs_srvCommand_t;
typedef enum fs_srvStatus_e fs_srvStatus_t;
typedef enum fs_srvOption_e fs_srvOption_t;
typedef enum fs_clientFlags_e fs_clientFlags_t;
typedef enum fs_workerSignal_e fs_workerSignal_t;
typedef struct fs_client_s fs_client_t;
typedef struct fs_worker_s fs_worker_t;
typedef int fs_srvOptionVal_t;
typedef fs_srvStatus_t (* fs_srvCmdFunc_t)(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);

// srvcmd.c
typedef struct fs_srvCmdHandler_s fs_srvCmdHandler_t;

// pers.c
typedef enum fs_persStatus_e fs_persStatus_t;
typedef enum fs_persLStatus_e fs_persLStatus_t;
typedef enum fs_persFlags_e fs_persFlags_t;
typedef enum fs_skill_e fs_skill_t;
typedef enum fs_persPart_e fs_persPart_t;
typedef enum fs_persEvent_e fs_persEvent_t;
typedef enum fs_persEffCode_e fs_persEffCode_t;
typedef enum fs_persEffFlags_e fs_persEffFlags_t;
typedef struct fs_pers_s fs_pers_t;
typedef struct fs_persEff_s fs_persEff_t;
typedef struct fs_persCmb_s fs_persCmb_t;
typedef struct fs_persCharge_s fs_persCharge_t;
typedef struct fs_persHonorData_s fs_persHonorData_t;
typedef struct fs_followPers_s fs_followPers_t;
typedef int fs_skillVal_t;

// fight.c
typedef enum fs_fightStatus_e fs_fightStatus_t;
typedef enum fs_fightFlags_e fs_fightFlags_t;
typedef enum fs_fightSignal_e fs_fightSignal_t;
typedef enum fs_fightLogCode_e fs_fightLogCode_t;
typedef struct fs_fight_s fs_fight_t;
typedef struct fs_fightLog_s fs_fightLog_t;
typedef struct fs_fightLogData_s fs_fightLogData_t;
typedef struct fs_fightInfo_s fs_fightInfo_t;
typedef struct fs_fightInfoPersData_s fs_fightInfoPersData_t;
typedef struct fs_fightInfoEffData_s fs_fightInfoEffData_t;
typedef struct fs_fightInfoCmbData_s fs_fightInfoCmbData_t;
typedef struct fs_luaParam_s fs_luaParam_t;

#endif
