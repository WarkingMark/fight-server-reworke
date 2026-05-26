/* 
 * modifed: igorpauk 2017-18
 */

#include "common.h"
#include "pers.h"
#include "persmagic.h"


double    fs_persMagicHitProb[LEVEL_DIFF_MAX+1][2] = { // hit probability
	// PvE   PvP
	{ 0.99, 0.99 },
	{ 0.97, 0.97 },
	{ 0.95, 0.95 },
	{ 0.90, 0.90  },
	{ 0.85, 0.85 },
	{ 0.80, 0.80 },
	{ 0.70, 0.70 },
	{ 0.60, 0.60 },
	{ 0.60, 0.60 },
	{ 0.60, 0.60 },
	{ 0.60, 0.60 },
	{ 0.60, 0.60 },
	{ 0.60, 0.60 },
	{ 0.60, 0.60 },
	{ 0.60, 0.60 },
	{ 0.60, 0.60 },
	{ 0.60, 0.60 },
	{ 0.60, 0.60 },
	{ 0.60, 0.60 },
	{ 0.60, 0.60 },
	{ 0.60, 0.60 },
	{ 0.60, 0.60 },
	{ 0.60, 0.60 }
};

double    fs_persPartResistData[4][4] = {
	{   2.2,   0.7, 0.12, 0.01 },
	{ -1.44,  1.54, 0.64, 0.11 },
	{ -0.64, -1.54, 1.44, 0.85 },
	{ -0.12,  -0.7, -2.2, 3.03 }
};


inline fs_skill_t fs_resistSkill(int dmgType) {
	return 
		(dmgType & FS_PDT_PHYSICAL    ? FS_SK_RSTPHYSIC:
		(dmgType & FS_PDT_KIDMAGIC    ? FS_SK_RSTKIDMAG:
		(dmgType & FS_PDT_AIRFIRE     ? FS_SK_RSTAIRFIRE:
		(dmgType & FS_PDT_WATERGROUND ? FS_SK_RSTWTRGRND:
		(dmgType & FS_PDT_LIGHTSHADOW ? FS_SK_RSTLGHSHAD:
		FS_SK_RSTPHYSIC)))));
}

int fs_antiResistSkill(int dmgType, fs_pers_t *pers) {
	int skill_id =  (dmgType & FS_PDT_AIRFIRE     ? FS_SK_FW_PENETRATION:
					(dmgType & FS_PDT_WATERGROUND ? FS_SK_WE_PENETRATION:
					(dmgType & FS_PDT_LIGHTSHADOW ? FS_SK_LD_PENETRATION:
					0)));
	if(skill_id == 0) return 0;	
	return PERS_SKILL(pers,skill_id);
}
double fs_persPartResist(double p) {	// (c) gunsky, not me :)
	double  part[4], r;
	int     i;

	p = MAX(MIN(p,0.75),0);
	/*
	for (i=0; i<4; i++) {
		part[i] = 
			fs_persPartResistData[0][i] * MIN(p, 0.25) +
			fs_persPartResistData[1][i] * MAX(0,MIN(p-0.25, 0.25)) +
			fs_persPartResistData[2][i] * MAX(0,MIN(p-0.5, 0.25)) +
			fs_persPartResistData[3][i] * MAX(0,MIN(p-0.75, 0.25));
	}
	r = randDouble(0,1,NULL);
	for (i=0; i<4; i++) {
		r -= part[i];
		if (r <= 0) return 0.25*(i+1);
	}
	*/

	if(p >= 0.75) {
		if(randRoll(0.50,NULL)) p -= 0.25;
		if(randRoll(0.10,NULL)) p -= 0.25;
		if(randRoll(0.05,NULL)) p -= 0.25;
	}
	if(p >= 0.50) {
		if(randRoll(0.50,NULL)) p -= 0.25;
		if(randRoll(0.05,NULL)) p -= 0.25;
	}
	if(p >= 0.25) {
		if(randRoll(0.25,NULL)) p -= 0.25;
	}
	if(p > 0) return p;
    
	return 0;
}

double fs_persHitProb(fs_pers_t *pers, fs_pers_t *opp, int dmgType) {
	int lvlDiff, pvp;
	if ((pers == opp) || !dmgType || (dmgType & OLD_DMGTYPE)) return 1.0;
	lvlDiff = MIN(MAX(PERS_LEVEL_SAFE(opp) - PERS_LEVEL_SAFE(pers), 0), LEVEL_DIFF_MAX);
	pvp = opp->flags & FS_PF_BOT ? 0 : 1;
	double prbMag, prbRepr, prAll;
	prbMag = (MIN(PERS_SKILL(pers,FS_SK_MAGHIT),1000)/1000.0) / pow(1.01, PERS_LEVEL_SAFE(pers));
	prbRepr = (pvp == 1 ? (MIN(PERS_SKILL(opp,FS_SK_WILL_REPRESSION),1000)/1000.0) / pow(1.01, PERS_LEVEL_SAFE(opp)) : 0.0);
	prAll = prbMag - prbRepr;

	//DEBUGX("LPERS=%d LOPP=%d PRMAG=%.2f PRREPR=%.2f PRALL=%.2f AND=%.2f", PERS_LEVEL_SAFE(pers), PERS_LEVEL_SAFE(opp), prbMag, prbRepr, prAll, (fs_persMagicHitProb[lvlDiff][pvp] + prAll));

	return fs_persMagicHitProb[lvlDiff][pvp] + prAll;
}

double fs_persResistProb(fs_pers_t *pers, fs_pers_t *opp, int dmgType) {
	if (pers == opp) return 0;
	return (MAX(PERS_SKILL(opp,fs_resistSkill(dmgType))-fs_antiResistSkill(dmgType,pers),0) * 0.75 / (30 * MAX(PERS_LEVEL(pers) - 10, 1)));
}

// mode: 0 - deviation, 1 - average
double fs_persRecalcDmg(fs_pers_t *pers, fs_pers_t *opp, int dmgType, double dmg, int mode, double n) {
	double dmgD, dmgMin, dmgMax;

	if ((pers == opp) || !dmgType || (dmgType & OLD_DMGTYPE)) return dmg;
	n = MAX(n, 1);
	dmg += PERS_SKILL(pers,FS_SK_WISDOM) * _Xs / n;

	// ELEMENT_ATK: elemental attack bonus based on damage type
	int elementAtk = PERS_SKILL(pers, FS_SK_ELEMENT_ATK);
	if (elementAtk > 0 && dmgType != FS_PDT_PHYSICAL) {
		dmg += dmg * (elementAtk / 100.0);
	}

	// ELEMENT_DEF: elemental defense bonus based on incoming damage type
	int elementDef = PERS_SKILL(opp, FS_SK_ELEMENT_DEF);
	if (elementDef > 0 && dmgType != FS_PDT_PHYSICAL) {
		dmg -= dmg * (elementDef / 100.0);
	}

	if (mode == 0) {
		dmgD = dmg < 5 ? dmg / 5.0 : log(dmg/5.0) / log(2.681792830507);
		dmgMin = dmg + PERS_SKILL(pers,FS_SK_MAGPWRMIN)/10.0 / n - dmgD;
		dmgMax = dmg + PERS_SKILL(pers,FS_SK_MAGPWRMAX)/10.0 / n + dmgD;
		dmg = randDouble(0,dmgMax-dmgMin+1,NULL)/2 + randDouble(0,dmgMax-dmgMin+1,NULL)/2 + dmgMin;
	} else if (mode == 1) {
		dmg += (PERS_SKILL(pers,FS_SK_MAGPWRMIN)/10.0 /n + PERS_SKILL(pers,FS_SK_MAGPWRMAX)/10.0 / n) / 2;
	}
	return MAX(dmg, 0.1);
}

double fs_persRecalcDmgAura(fs_pers_t *pers, fs_pers_t *opp, fs_persEff_t *auraEff) {
	double dmgD, dmgMin, dmgMax, pResist;
	bool crit = false;
	double dmg = 0;
	if (pers == opp) return dmg;
	//Магический урон от боевой ауры использует 90% мудрости игрока.
	//FS_SK_USE_WISDOM = 90 (90/100=0.90) то что нужно
	dmg += PERS_SKILL(pers,FS_SK_WISDOM) * (auraEff->skills[FS_SK_USE_WISDOM] / 100.0) * _Xs;
	
	DEBUG("dmgcalc=%.2f", dmg);
	DEBUG("FS_SK_WISDOM=%d", PERS_SKILL(pers,FS_SK_WISDOM));		
	DEBUG("FS_SK_USE_WISDOM=%d", auraEff->skills[FS_SK_USE_WISDOM]);
	//Добавляем бонусный магический урон
	dmg += (PERS_INTSKILL(pers,FS_SK_MAGPWRMIN)/10.0 + PERS_INTSKILL(pers,FS_SK_MAGPWRMAX)/10.0) / 2;

	DEBUG("dmgcalc=%.2f", dmg);
	return dmg;
}