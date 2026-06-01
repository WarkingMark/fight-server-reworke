/* 
 * modifed: igorpauk 2017-18
 */

#ifndef __PERS_H__
#define __PERS_H__

#include "typedefs.h"
#include "formulas.h"
#include "utils.h"


#define PERS_STUCK_TTL   300

// random seed codes
#define _RSC_EVD    0
#define _RSC_AEVD   1
#define _RSC_CRT    2
#define _RSC_ACRT   3
#define _RSC_BLK    4
#define _RSC_ABLK   5
#define _RSC_MAXCODE     5

#define MAX_COMBO_SIZE   10

#define PERS_INTSKILL(A, B) ((A)->intSkills[(B)])
#define PERS_EXTSKILL(A, B) ((A)->extSkills[(B)])
#define PERS_BOTDMGSKILL(A, B) ((A)->botTypeDmgSkills[(B)])
#define PERS_PART(A, B, C) ((A)->parts[(B)][(C)])
#define PERS_OPP_ID(A) ((A)->opponent && (((A)->status == FS_PS_ACTIVE) || ((A)->status == FS_PS_PASSIVE)) ? (A)->opponent->id: 0)

#define PERS_LEVEL(A) ((int)PERS_EXTSKILL(A,FS_SK_LEVEL))
#define PERS_LEVEL_SAFE(A) ((int)A->level)
#define PERS_HP(A) ((int)PERS_EXTSKILL(A,FS_SK_HP))
#define PERS_HPMAX(A) ((int)PERS_EXTSKILL(A,FS_SK_HPMAX))
#define PERS_MP(A) ((int)PERS_EXTSKILL(A,FS_SK_MP))
#define PERS_MPMAX(A) ((int)PERS_EXTSKILL(A,FS_SK_MPMAX))
#define PERS_SKILL  PERS_EXTSKILL

#define PF_CLMASK(A) ((A) & (FS_PF_BOT | FS_PF_STUNNED | FS_PF_PET | FS_PF_DEFENDED | FS_PF_MAGIC | FS_PF_SKGHOST | FS_PF_INVISIBLE | FS_PF_FLEE | FS_PF_NO_AURAS | FS_PF_NO_OPP_AUTO | FS_PF_SHADOW))
#define PEF_CLMASK(A) ((A) & (FS_PEF_WEAPONEFFECT | FS_PEF_SPELL | FS_PEF_PASSTURN | FS_PEF_NEEDTURN | FS_PEF_TARGETSELF | FS_PEF_TARGETOPP | FS_PEF_TEAMSELF | FS_PEF_TEAMOPP | FS_PEF_CONFIRM | FS_PEF_DISABLED | FS_PEF_BOW | FS_PEF_AURA | FS_PEF_USEDEAD))

// Level tables (arrays defined in pers.c)
#define MAX_LEVEL   20
#define BASE_EXP(LVL) (((LVL) >= 0) && ((LVL) <= MAX_LEVEL) ? fs_persBaseExp[(LVL)]: 0)
#define BASE_SUM(LVL) (((LVL) >= 0) && ((LVL) <= MAX_LEVEL) ? fs_persSkillSum[(LVL)][0]: 0)
#define ITEM_SUM(LVL) (((LVL) >= 0) && ((LVL) <= MAX_LEVEL) ? fs_persSkillSum[(LVL)][1]: 0)
#define BASE_HONOR(LVL) (((LVL) >= 0) && ((LVL) <= MAX_LEVEL) ? fs_persBaseHonor[(LVL)]: 0)
#define _COMP(LVL, RATIO) (ITEM_SUM(LVL)/(2.0*(RATIO)-2) - BASE_SUM(LVL)/5.0)

// Все боевые формулы и константы перенесены в formulas.h

// ---------- fight events & codes ----------



// tolua_begin
enum fs_persStatus_e {
	FS_PS_CREATED     = 0,
	FS_PS_FIGHTING    = 1,
	FS_PS_DEAD        = 2,
	FS_PS_PENDING     = 3,
	FS_PS_ACTIVE      = 4,	
    FS_PS_PASSIVE     = 5,
};
enum fs_persLStatus_e {	
	FS_PLS_PENDING    = 1,
	FS_PLS_ACTIVE     = 2,
	FS_PLS_PASSIVE    = 3,
};
// tolua_end

enum fs_persFlags_e {
	FS_PF_FIGHTTEST   = 0x0001,
	FS_PF_IMMORTAL    = 0x0002,
	FS_PF_GETTURN     = 0x0004,
	FS_PF_OPPWAIT     = 0x0008,
	FS_PF_TIMEOUTKILL = 0x0010,
	FS_PF_ART         = 0x0020,
	FS_PF_BOT         = 0x0040,
	FS_PF_STUNNED     = 0x0080,
	FS_PF_DEFENDED    = 0x0100,
	FS_PF_CRITSIMILAR = 0x0200,
	FS_PF_MAGIC       = 0x0400,
	FS_PF_SKGHOST     = 0x0800,
	FS_PF_LIFELESS    = 0x1000,
	FS_PF_PET   	  = 0x4000,	//PET Durumu
	FS_PF_INVISIBLE	  = 0x8000,  //32768
	FS_PF_DUALWIELD   = 0x08000000, //134217728
	FS_PF_FLEE		  = 0x10000, //65536
	FS_PF_NO_AURAS	  = 0x10000000, //268435456
	FS_PF_NO_OPP_AUTO = 0x20000000, //536870912
	FS_PF_SHADOW	  = 0x40000000, //1073741824
	
};

enum fs_skill_e {
	FS_SK_LEVEL       =  0,
	FS_SK_STR         =  1,
	FS_SK_INT         =  2,
	FS_SK_DEX         =  3,
	FS_SK_ENDUR       =  4,
	FS_SK_VIT         =  5,
	FS_SK_WISDOM      =  6,
	FS_SK_INTELL      =  7,
	FS_SK_PWRMIN      =  8,
	FS_SK_PWRMAX      =  9,
	FS_SK_HP          = 10,
	FS_SK_HPMAX       = 11,
	FS_SK_MP          = 12,
	FS_SK_MPMAX       = 13,
	FS_SK_XHPMAX      = 14,
	FS_SK_XMPMAX      = 15,
	FS_SK_MAGCRIT     = 16,
	FS_SK_MAGHIT      = 17,
	FS_SK_MAGPWRMIN   = 18,
	FS_SK_MAGPWRMAX   = 19,
	FS_SK_RSTPHYSIC   = 20,
	FS_SK_RSTKIDMAG   = 21,
	FS_SK_RSTAIRFIRE  = 22,
	FS_SK_RSTWTRGRND  = 23,
	FS_SK_RSTLGHSHAD  = 24,
	FS_SK_RSTALL      = 25,
	FS_SK_MREG        = 26,
	FS_SK_CHRGDMGX    = 27,
	FS_SK_CHRGVAMP    = 28,
	FS_SK_CHRGSTUNP   = 29,
	FS_SK_CHRGSTUNT   = 30,
	FS_SK_CHRGCRIT    = 31,
	FS_SK_WILL        = 32,
	FS_SK_FW_PENETR   = 33,
	FS_SK_WE_PENETR   = 34,
	FS_SK_LD_PENETR   = 35,
	FS_SK_W_REPRESS   = 36,
	FS_SK_PH_PCRESIST = 37,
	FS_SK_KM_PCRESIST = 38,
	FS_SK_FW_PCRESIST = 39,
	FS_SK_WE_PCRESIST = 40,
	FS_SK_LD_PCRESIST = 41,
	FS_SK_ENERGYMAX   = 42,
	FS_SK_ENERGY      = 43,
	FS_SK_BOWDMG      = 44,
	FS_SK_EVADEPENALTY = 45,
	FS_SK_BLOCKPENALTY = 46,
	FS_SK_CRITPENALTY  = 47,
	FS_SK_HEALONATTACK = 48,
	FS_SK_INJURYDEC   = 49,
	FS_SK_CRITAMP     = 50,
	FS_SK_MANAREGPROB = 51,
	FS_SK_DMGMAX      = 52,
	FS_SK_HEALPENALTY = 53,
	FS_SK_SHIP = 54,
	FS_SK_PENETRATION = 55,
	FS_SK_VAMPIR = 56,
	FS_SK_STOIKOST = 57,
	FS_SK_HPMOD = 58,
	FS_SK_INICIATIV = 59,
	FS_SK_UNHPMOD = 60,
	FS_SK_AURACHRGDMGX = 61,
	FS_SK_AURACHRGVAMP = 62,
	FS_SK_AURACHRGCRIT = 63,
	FS_SK_USE_WISDOM = 64,
	FS_SK_RSTPHYSICBLOCK = 65,
	FS_SK_PCRSTAIRBLOCK = 66,
	FS_SK_MAGCRITDEF = 67,
	FS_SK_MAGCRITDMX = 68,
	FS_SK_CHVAMPMINUS = 69,
	FS_SK_CHVAMPPLUS = 70,
	FS_SK_FIZCRITDEF = 71,
	FS_SK_FIZCRITDMX = 72,
	FS_SK_RAZPELENY = 73,

    FS_SK_WILL_REPRESSION = 74,
	FS_SK_FW_PENETRATION = 75,
	FS_SK_LD_PENETRATION = 76,
	FS_SK_WE_PENETRATION = 77,
	FS_SK_INITIATIVE = 78,
    FS_SK_ANTISTUN = 79,
	FS_SK_CHRGSTUNC   = 80, 

	FS_SK_AOECNT      = 81,  // bonus to AOE target count
	FS_SK_AOEDMG      = 82,  // % damage for splash targets
	FS_SK_DMGX        = 83,  // % bonus to ANY outgoing damage
	FS_SK_CRITDMX     = 84,  // % bonus to crit multiplier

	FS_SK_DMG_REDUCE_FLAT = 85,  // flat incoming damage reduction (50 = -50 dmg)
	FS_SK_DMG_REDUCE_P    = 86,  // % incoming damage reduction (10 = 10%)
	FS_SK_DMGX_RECV       = 87,  // % increase incoming damage taken (20 = +20% dmg taken)
	FS_SK_BLOCK_DMG_P     = 88,  // % damage taken on block (40 = take 40% on block)
	FS_SK_PHYS_ACCURACY   = 89,  // physical hit chance bonus
	FS_SK_DMGX_PVP        = 90,  // % damage bonus vs players
	FS_SK_DMGX_PVE        = 91,  // % damage bonus vs monsters
	FS_SK_DMGX_PVP_DEF    = 92,  // % damage reduction from players
	FS_SK_DMGX_PVE_DEF    = 93,  // % damage reduction from monsters
	FS_SK_ELEMENT_ATK     = 94,  // elemental attack bonus (all elements)
	FS_SK_ELEMENT_DEF     = 95,  // elemental defense bonus (all elements)
	FS_SK_LETHAL_RATE     = 96,  // % chance of instant kill (0.1-1%)
	FS_SK_BLEED_RESIST    = 97,  // bleed effect resistance
	FS_SK_POISON_RESIST   = 98,  // poison effect resistance
	FS_SK_DEBUFF_DUR_P    = 99,  // % debuff duration modifier (attacker)
	FS_SK_BUFF_DUR_P      = 100, // % buff duration modifier (self)
	FS_SK_DUAL_PENALTY    = 101, // % dual wield damage penalty

	/* AOE skills */
	FS_SK_AOE_EFF_CHANCE  = 102, // % chance to apply effect (bleed, stun) to AOE targets
	FS_SK_AOE_DMG_MULT    = 103, // % multiplier for AOE damage (100 = no change, 150 = +50%)
	FS_SK_AOE_EFF_DUR_P   = 104, // % duration modifier for effects applied to AOE targets
	FS_SK_AOE_CRIT_MOD    = 105, // additional crit chance for AOE targets (10 = +10% more likely)
	FS_SK_AOE_VAMP_MOD    = 106, // % of vampirism applied from AOE targets (50 = 50% of normal vamp)
	/* Universal damage multiplier — works on ALL damage types */
	FS_SK_ADD_MULT_DMG    = 107, // % additional damage multiplier (applies everywhere)

	/* DoT & Healing Enhancements */
	FS_SK_DOT_DMG_MULT    = 108, // % усиления DoT за каждый тик (накапливается)
	FS_SK_DOT_DURATION    = 109, // % длительности DoT эффектов

	/* Regeneration */
	FS_SK_HP_REGEN_FLAT   = 110, // Flat HP regen per turn
	FS_SK_MP_REGEN_FLAT   = 111, // Flat MP regen per turn
	FS_SK_HP_REGEN_P      = 112, // % Max HP regen per turn
	FS_SK_MP_REGEN_P      = 113, // % Max MP regen per turn

	/* Healing Modifiers */
	FS_SK_HEAL_RCVD_MULT  = 114, // % усиления входящего лечения
	FS_SK_SELF_HEAL_MULT  = 115, // % усиления самолечения
	FS_SK_HEAL_POWER      = 116, // % усиления исходящего лечения

	/* Defensive & Utility */
	FS_SK_REFLECTION_DMG_P= 117, // % возврата урона при получении удара (макс 100%)
	FS_SK_MANA_SHIELD     = 118, // % урона снимаемого с MP вместо HP
	FS_SK_CRIT_RESIST     = 119, // % шанс превратить крит в обычный удар

	/* Execute */
	FS_SK_EXECUTE_P       = 120, // % доп урона если у цели HP < 30%

	/* Utility */
	FS_SK_MP_COST_REDUCE  = 121, // % снижения затрат маны
	FS_SK_MP_COST_FLAT    = 122, // плоское снижение затрат маны
	FS_SK_CHANCE_IGNORE_DEF= 123,// % шанс игнорировать защиты цели

	/* Advanced Mechanics */
	FS_SK_BLOOD_EXPLOSION = 124, // % шанс взрыва при 3+ стеках кровотечения
	FS_SK_MULTI_HIT_PERCENT_DAMAGE = 125, // % урона от двойного удара
	FS_SK_DEADLY_STRIKE   = 126, // % шанс смертельного удара (поверх крита)
	FS_SK_DS_DMG          = 127, // % множитель урона при смерт. ударе (50 = x1.5)

	/* Crit Defense */
	// CRIT_RESIST - дубликат
	FS_SK_CRIT_DMG_IGNORE = 128, // % шанс полностью игнорировать крит (урон как обычный)
	FS_SK_CRIT_DMG_REDUCE = 129, // % снижение входящего крит урона
	FS_SK_CRIT_CHANCE_PVP = 130, // % снижение шанса крита
	FS_SK_BLOCK_CHANCE = 131,   // прямой % бонус к шансу блока
    FS_SK_EVADE_CHANCE = 132,   // прямой % бонус к шансу уклонения
	FS_SK_MULTI_HIT_CHANCE = 133, // % шанс двойного удара
	FS_SK_CRIT_CHANCE = 134,   // плоский % к шансу крита (независимо от INT)
	FS_SK_EVADE_CHANCE_IGNORE = 135, //  % снижение шанса уворота
	FS_SK_BLOCK_CHANCE_IGNORE = 136, // % снижение шанса блока
	FS_SK_BUFF_DUR_REDUCE     = 137, // % снижение уменьшения длительности положительных эффектов
	FS_SK_DEBUFF_DUR_REDUCE   = 138, // % снижение уменьшения длительности отрициальных эффектов
};

#define FS_SK_MAXCODE   138

enum fs_persPart_e {
	FS_PPT_HD1   =  0,
	FS_PPT_HD2   =  1,
	FS_PPT_BD1   =  2,
	FS_PPT_BD2   =  3,
	FS_PPT_RH1   =  4,
	FS_PPT_RH2   =  5,
	FS_PPT_RH3   =  6,
	FS_PPT_LH1   =  7,
	FS_PPT_LH2   =  8,
	FS_PPT_LH3   =  9,
	FS_PPT_RL1   = 10,
	FS_PPT_RL2   = 11,
	FS_PPT_RL3   = 12,
	FS_PPT_LL1   = 13,
	FS_PPT_LL2   = 14,
	FS_PPT_LL3   = 15,
	FS_PPT_RW    = 16,	// left weapon
	FS_PPT_LW    = 17,	// right weapon
	FS_PPT_MW    = 18,	// middle weapon
	FS_PPT_BN    = 19,
	FS_PPT_BOW   = 20,
};
#define FS_PPT_MAXCODE   20
#define FS_PPT_LAYERCNT   3

#define PET_TIME_MIN   40
#define PET_TIME_MAX   50
#define PET_PERIOD_TIME_MIN   50
#define PET_PERIOD_TIME_MAX   70
#define PET_USER_MIN_STUCK_CNT 5

enum fs_persEvent_e {
	FS_PE_SRVSHUTDOWN      =   1,	// fight interrupted
	FS_PE_OPPWAIT          = 101,	// wait for opponent
	FS_PE_OPPNEW           = 102,	// INT oppId - opponent changed, load info
	FS_PE_ATTACKNOW        = 103,	// attack now!
	FS_PE_ATTACKWAIT       = 104,	// wait for attack
	FS_PE_ATTACK           = 105,	// INT persId, INT oppId, INT kick, INT part, INT rnd=(0..0xFFFF), STRING animData - personage attack
	FS_PE_ATTACKTIMEOUT    = 106,	// INT persId - personage attack timeout
	FS_PE_FIGHTLOG         = 107,	// INT ctime, INT persId, INT oppId, INT code, INT i1, INT i2, INT i3, STRING s1 - fight log data
	FS_PE_FIGHTOVER        = 108,	// INT winnerTeam - fight over
	FS_PE_FIGHTSTATE       = 109,	// INT persId, INT persStatus, INT persFlags, INT hp, INT hpMax, INT mp, INT mpMax - personage status change
	FS_PE_EFFECTUSE        = 110,	// INT persId, INT targetId, STRING animData, INT usageSatus - effect usage
	FS_PE_EFFECTAPPLY      = 111,	// INT persId, INT artId, INT/NINT xcnt=(-1,+1), STRING title, STRING picture, STRING animData, INT eetime - active effect applied to "persId"
	FS_PE_DAMAGE           = 112,	// INT persId, INT dmg, INT dmgType, INT crit=(0,1), INT absorb, INT activatorId, INT resist - damage
	FS_PE_DEATH            = 113,	// INT persId - death
	FS_PE_MANNACONSUM      = 114,	// INT persId, INT manna - manna consumption
	FS_PE_MYFIGHTRETURN    = 115,	// INT persId, INT code, STRING msg - Mesaj
	FS_PE_EFFUPDATE        = 116,	// INT persId, INT code, STRING msg - Mesaj
	FS_PE_ENERGYCONSUM     = 117,	// INT persId, INT code, STRING msg - Mesaj
	FS_PE_ENERGYREGEN      = 118,	// INT persId, INT code, STRING msg - Mesaj
	FS_PE_ARROWCONSUM      = 119,	// INT persId, INT code, STRING msg - Mesaj
	FS_PE_EFFSWAP          = 120,	// INT persId, INT code, STRING msg - Mesaj
	FS_PE_RESETCOMBO       = 121,	// INT persId, INT code, STRING msg - Mesaj
	FS_PE_NEWPERS          = 122,	// INT persId, INT code, STRING msg - Mesaj
	FS_PE_DEADCNT          = 123,	// INT persId, INT code, STRING msg - Mesa	
	FS_PE_MSG              = 201,	// INT persId, INT code, STRING msg - message
	FS_PF_NORMALIZE		   = 202,	// INT persId, INT code, STRING msg - message
};

enum fs_persEffCode_e {
	FS_PEC_ADDSKILLS       = 1,	// eff->i1 = (0 - absolute, 1 - percent), eff->i2 = max value
	FS_PEC_ADDHP           = 2,	// FS_SK_HP += FS_SK_HPMAX*eff->f1, eff->i2 = max value
	FS_PEC_ADDMP           = 3,	// FS_SK_MP += FS_SK_MPMAX*eff->f1, eff->i2 = max value
	FS_PEC_STUN            = 4,	// eff->f1 = drop on tick (probability)
	FS_PEC_RESETCOMBO      = 5,
	FS_PEC_RESURRECT       = 6,	// status = FS_PS_PENDING, FS_SK_HP = FS_SK_HPMAX*eff->f1 
	FS_PEC_ABSORB          = 7,	// eff->dmgType - damage type, eff->i1 - absorb quantity
	FS_PEC_BOTHELP         = 8,	// eff->i1 - bot artikul ID
	FS_PEC_REFLECT         = 9,
	FS_PEC_SETFLAG         = 10,
	FS_PEC_MODHP           = 11,
};

enum fs_persEffFlags_e {
	FS_PEF_GRPDENY         = 0x0000001,
	FS_PEF_GRPDROP         = 0x0000002,
	FS_PEF_WEAPONEFFECT    = 0x0000004,
	FS_PEF_SPELL           = 0x0000008,
	FS_PEF_PASSTURN        = 0x0000010,
	FS_PEF_NEEDTURN        = 0x0000020,
	FS_PEF_TARGETSELF      = 0x0000040,
	FS_PEF_TARGETOPP       = 0x0000080,
	FS_PEF_TEAMSELF        = 0x0000100,
	FS_PEF_TEAMOPP         = 0x0000200,
	FS_PEF_TARGETNOTBOT    = 0x0000400,
	FS_PEF_USEDEAD         = 0x0000800,
	FS_PEF_AUX             = 0x0001000,
	FS_PEF_CONFIRM         = 0x0002000,
	FS_PEF_CDGRPDENY       = 0x0004000,
	FS_PEF_CDGRPDROP       = 0x0008000,
	FS_PEF_CDSTART         = 0x0010000,
	FS_PEF_NOFULLRESIST    = 0x0020000,
	FS_PEF_CHARGE          = 0x0040000,
	FS_PEF_ACTIVE          = 0x0080000,
	FS_PEF_DROP            = 0x0100000,
	FS_PEF_SILENT		   = 0x0200000,
	FS_PEF_PET_ASSIST      = 0x0400000,
	FS_PEF_PARTYSELF       = 0x0800000,
	FS_PEF_INFINITE        = 0x1000000, //16777216
	FS_PEF_RESETONDIE      = 0x2000000,
	FS_PEF_CLANSELF        = 0x4000000,
	FS_PEF_ANIMINVERT      = 0x8000000, 
	FS_PEF_DISABLED		   = 0x10000000, //268435456
	FS_PEF_BOW             = 0x20000000, //536870912
	FS_PEF_TARGETNOTUSER   = 0x40000000, //1073741824
	FS_PEF_AURA	  		   = 0x80000000, //2147483648
	//FS_PEF_DELETE_LOG	   = 0x100000000, //4294967296
};

enum fs_persDmgType_e {
	FS_PDT_PHYSICAL        = 0x0001,
	FS_PDT_KIDMAGIC        = 0x0002,
	FS_PDT_AIRFIRE         = 0x0004,
	FS_PDT_WATERGROUND     = 0x0008,
	FS_PDT_LIGHTSHADOW     = 0x0010,
};


typedef fs_skillVal_t fs_skillArr_t[FS_SK_MAXCODE+1];


struct fs_pers_s {
	int               id, akey;
	int               ctime, mtime, atime, eutime, aetime, pettime, petRest;
	fs_persStatus_t   status, statusOld;
	fs_persLStatus_t  statusl;
	fs_persFlags_t    flags, flagsOld;
	char              *nick, *PetSrc, *nickData;
	int               level, gender, kind, cls, skeleton, skeletonTime, partMask, artId, ImmunDmgType, clanId, partyId, new_pers_id, ibotArtikulId;
	bool              art;
	fs_skillArr_t     intSkills, extSkills;
	int               parts[FS_PPT_MAXCODE+1][FS_PPT_LAYERCNT];
	vector_t          *effVec;	// 'fs_persEff_t' vector
	vector_t          *cmbVec;	// 'fs_persCmb_t' vector
	vector_t          *honorDataVec;	// 'fs_pershonorData_t' vector
	fs_skillVal_t     hpOld, hpMaxOld, mpOld, mpMaxOld;
	
	fs_skillArr_t     botTypeDmgSkills;

	int				  lastEffectUpdateIndex;
	
	vector_t          *followPers;	// 'fs_followPers_t' vector
	
	fs_fight_t        *fight;
	fs_client_t       *client;

	int               teamNum, dmg, heal, killCnt, enemyKillCnt, timeoutCnt;
	double            exp, expX, honor;
	int               cmbSeq[MAX_COMBO_SIZE], cmbSize;
	int				  petLevel, petReady, autoKick;
	int				  userUdarCnt;
	int				  botTypeId;
	bool			  canAttack;
	
	int				  yarost, yarost_max, yarost_time, arrowsCnt;
	
	bool			  _inicHod;
	
	fs_pers_t         *opponent, *killer;

	int               botActionTime, stunTime, stunCnt, stunSafeCnt;
	char              *ctrlFile, *ctrlFunc;
	
	char			  *nParts;

	unsigned          _rs[_RSC_MAXCODE+1];
	int               _dmgEventCnt, _dieEventCnt;
};

struct fs_persEff_s {
	int               id, estime, eetime, eptime, cdrtime;
	fs_persEffCode_t  code;
	fs_persEffFlags_t flags;
	double            f1, f2, f3, prob, probAuto, dmgRecalc;
	int               i1, i2, i3, cnt, artId, grpId, dmg, dmgType, actTime, actMoveCnt, actPeriod, cdTime, cdType, cdGrpId, mp, aoeCnt, slotNum, subSlot, energyCost, turnsLeft;
	int				  e_yarost;
	fs_skillArr_t     skills;
	char              *title, *picture, *animData, *slotId;

	fs_pers_t         *activator;
};

struct fs_persCmb_s {
	int               id;
	int               seq[MAX_COMBO_SIZE], size;
	int               level, auxEffId, useCnt;
};

struct fs_persCharge_s {
	double            dmgX, vamp, stunProb, critProb;
	double			  aura_dmgX, aura_vamp, aura_critProb; 
	int               stunTime, stunCnt, cnt, razpeleny;
};

struct fs_persHonorData_s {
	fs_pers_t         *pers;
	double            honor;
};

struct fs_followPers_s {
	fs_pers_t     *pers;
};

fs_pers_t *fs_persCreate(int id);
errno_t fs_persDelete(fs_pers_t *pers);
fs_pers_t *fs_persGetById(int id);
fs_persEff_t *fs_persEffCreate(int id);
errno_t fs_persEffDelete(fs_persEff_t *eff);
fs_persEff_t *fs_persEffCopy(fs_persEff_t *eff);

errno_t fs_persSetEvent(fs_pers_t *pers, fs_persEvent_t event, char *fmt, ...);

errno_t fs_persDebugSkills(fs_pers_t *pers);
errno_t fs_persAttack(fs_pers_t *pers, int part, bool wpnEff);
errno_t fs_persAddYarost(fs_pers_t *pers, int cnt);
errno_t fs_persStun(fs_pers_t *pers, fs_pers_t *opp, int stunTime, int stunCnt);
errno_t fs_persGetTurn(fs_pers_t *pers);
errno_t fs_persDie(fs_pers_t *pers, fs_pers_t *killer);
errno_t fs_persBotIntelligence(fs_pers_t *pers);
errno_t fs_persPersIntelligence(fs_pers_t *pers);
double fs_persGetCost(fs_pers_t *pers);
double fs_persGetCost2(fs_pers_t *pers);
double fs_persGetExp(fs_pers_t *pers, fs_pers_t *opp, double dmg);
double fs_persGetHonor(fs_pers_t *pers, fs_pers_t *opp, double dmg);
double fs___persMaxHonor(fs_pers_t *pers, fs_pers_t *opp, double honor);
double fs_persDamage(fs_pers_t *pers, double dmg, int dmgType, bool crit, fs_pers_t *activator);
double fs_persDamageEx(fs_pers_t *pers, double dmg, int dmgType, bool crit, fs_pers_t *activator, bool noEnhance);
int fs_persConsumeManna(fs_pers_t *pers, int manna, bool silent);
errno_t fs_persUseEffect(fs_pers_t *pers, fs_persEff_t *eff, fs_pers_t *target, int *usageStatus);
errno_t fs___persEffectTargetCheck(fs_pers_t *pers, fs_persEff_t *eff, fs_pers_t *target, int *usageStatus);
void fs___persEffectTargetUpdate(fs_pers_t *pers, fs_persEff_t *eff, fs_pers_t *target);
void fs___persActivateEffect(fs_pers_t *pers, fs_persEff_t *eff, fs_pers_t *target);
errno_t fs___persActivateEffectCheck(fs_pers_t *pers, fs_persEff_t *eff, fs_pers_t *target);

errno_t fs_persRecalcEffects(fs_pers_t *pers);
errno_t fs_persDropEffects(fs_pers_t *pers);
errno_t fs_persGetCharge(fs_pers_t *pers, int dmgType, fs_persCharge_t *charge, bool moveCnt);
errno_t fs_persDischarge(fs_pers_t *pers, int dmgType);
errno_t fs_persAddComboItem(fs_pers_t *pers, int item);
fs_persCmb_t *fs_persCheckCombo(fs_pers_t *pers);
errno_t fs_followPersDelete(fs_followPers_t *follow);

#endif
