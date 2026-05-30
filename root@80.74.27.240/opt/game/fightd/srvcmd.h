/* 
 * modifed: igorpauk 2017-18
 */

#ifndef __SRVCMD_H__
#define __SRVCMD_H__

#include "typedefs.h"
#include "srv.h"
#include "io.h"


struct fs_srvCmdHandler_s {
	fs_srvCommand_t   cmd;
	fs_srvCmdFunc_t   func;
	int               srvLock;
};


extern fs_srvCmdHandler_t  fs_ctrlCmdHanderTable[], fs_clCmdHanderTable[];


errno_t fs_cmdCheckParams(fs_packet_t *packet, fs_paramType_t paramTypes[], int cnt, bool notLess, bool notStrict);

fs_srvStatus_t fs_cmd_SC_SYNC_TIME(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
// CTRL COMMANDS
fs_srvStatus_t fs_cmd_SCCT_SRV_INFO(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCT_DEBUG_LEVEL(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCT_GET_SRV_OPTS(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCT_SET_SRV_OPTS(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCT_GET_FIGHTS(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCT_CREATE_FIGHT(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCT_SET_FIGHT_PARAMS(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCT_START_FIGHT(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCT_STOP_FIGHT(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCT_DELETE_FIGHT(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCT_CREATE_PERS(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCT_SET_SKILLS(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCT_SET_BOTDMG_SKILLS(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCT_SET_PARTS(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCT_ADD_EFFECT(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCT_ADD_COMBO(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCT_BIND_PERS(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCT_DELETE_PERS(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCT_GET_FIGHTSTATE(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCT_GET_FIGHTLOG(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCT_GET_FIGHTINFO(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCT_DELETE_FIGHTINFO(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCT_GET_LOG_AND_EFFECTS(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCL_EFFECT_SWAP_SUBSLOT(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCL_DROP_EFFECT(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);


fs_srvStatus_t fs_cmd_SCCT_SET_PARAMS_LUA(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);


// CLIENT COMMANDS
fs_srvStatus_t fs_cmd_SCCL_INIT(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCL_STATE(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCL_PERS_INFO(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCL_PERS_PARTS(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCL_ATTACK(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCL_FIGHT_STATE(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCL_EFFECTS(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCL_USE_EFFECT(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCL_CHANGE_MODE(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCL_PERS_ACT_EFFECTS(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCL_SEND_MSG(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);

//NEW
fs_srvStatus_t fs_cmd_SCCL_PERS_SUBSCRIBE(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCL_WATCH_FIGHT(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCL_SKIP_TURN(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCL_EFFUPDATE(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCL_MYFIGHTRETURN(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
//fs_srvStatus_t fs_cmd_SCCL_DEADCNT(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCL_NEWPERS(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCL_RESETCOMBO(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCL_EFFSWAP(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCL_ARROWCONSUM(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCL_ENERGYREGEN(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);
fs_srvStatus_t fs_cmd_SCCL_ENERGYCONSUM(fs_client_t *client, fs_packet_t *inPacket, fs_packet_t *outPacket);

#endif
