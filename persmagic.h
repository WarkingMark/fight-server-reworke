/* 
 * modifed: igorpauk 2017-18
 */

#ifndef __PERSMAGIC_H__
#define __PERSMAGIC_H__

#include "typedefs.h"


#define OLD_DMGTYPE      (FS_PDT_PHYSICAL | FS_PDT_KIDMAGIC)
#define LEVEL_DIFF_MAX   10


inline fs_skill_t fs_resistSkill(int dmgType);
double fs_persPartResist(double p);
double fs_persHitProb(fs_pers_t *pers, fs_pers_t *opp, int dmgType);
double fs_persResistProb(fs_pers_t *pers, fs_pers_t *opp, int dmgType);
double fs_persRecalcDmg(fs_pers_t *pers, fs_pers_t *opp, int dmgType, double dmg, int mode, double n);
double fs_persRecalcDmgAura(fs_pers_t *pers, fs_pers_t *opp, fs_persEff_t *auraEff);
int fs_antiResistSkill(int dmgType, fs_pers_t *pers);

#endif
