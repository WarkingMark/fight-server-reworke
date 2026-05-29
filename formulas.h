/* formulas.h - Единый источник формул и лимитов */
#ifndef __FORMULAS_H__
#define __FORMULAS_H__

#include "common.h"

// =============================================================================
// Глобальные балансные лимиты
// =============================================================================
#define MAX_EVADE_RATE 0.75
#define MAX_BLOCK_RATE 0.85

// =============================================================================
// Базовые константы боевой системы
// =============================================================================
// HP/_damage ratios
#define _V0    1.5    // health ratio ("tank" and undressed)
#define _Vs    1      // health per 1 stat
#define _X0    1.5    // damage ratio ("crit" and undressed)
#define _Xs    0.1    // damage per 1 stat

// Event limits
#define _evPA  0.8    // dressed well
#define _evP0  0.15  // dressed bad

// Item compensation
#define _IComp 0.5
#define _decHP 0.8    // HP penalty for being dressed bad

// Probabilities (undressed)
#define _pE0   0.02
#define _pC0   0.02
#define _pB0   0.02

// Anti-probabilities (undressed)
#define _pAE0  0.00
#define _pAC0  0.00
#define _pAB0  0.00

// Probabilities (normal)
#define _pE    0.48
#define _pC    0.5
#define _pB    0.45

// Anti-probabilities (normal)
#define _pAE   0.42
#define _pAC   0.50
#define _pAB   0.60

// Damage absorb
#define _Ap0   0
#define _Ap    0.15

// Critical damage
#define _Cx    2.2   // crit damage increase multiplier

// Combo penalty
#define _CMBp  0.30  // combo probability decrease per level

// Defended flag
#define _dmgDx 0.5   // damage under FS_PF_DEFENDED flag

#define _evP0           0.15    // было 0.05 → DEX даёт max 15% (не 5%)
#define DEX_EVADE_CAP   0.15    // cap уклонения от DEX
#define DEX_BLOCK_CAP   0.15    // cap блока от ENDUR (при новом лимите)
#define EVADE_FLAT_CAP  0.80    // max с учётом плоских бонусов (арт)
#define EVADE_FLAT_CAP_NOART 0.30  // max без шмота

// kick codes (для FS_PE_ATTACK события):
// 1 = evade, 2 = crit, 3 = normal hit, 4 = block, 5 = deadly strike
#define KICK_EVADE          1
#define KICK_CRIT           2
#define KICK_NORMAL         3
#define KICK_BLOCK          4
#define KICK_DEADLY_STRIKE  5

// =============================================================================
// Каппинг вероятностей
// =============================================================================
static inline double clamp_prob(double val, double max) {
	return (val < 0) ? 0 : ((val > max) ? max : val);
}

static inline double clamp_pct(double val, double max) {
	return (val < 0) ? 0 : ((val > max) ? max : val);
}

static inline double calc_evasion_capped(double raw) { return clamp_prob(raw, MAX_EVADE_RATE); }
static inline double calc_block_capped(double raw)   { return clamp_prob(raw, MAX_BLOCK_RATE); }

// =============================================================================
// Инициатива
// =============================================================================
static inline double calc_initiative(int init, int dex) { return init + (dex / 2.0); }

// =============================================================================
// Регенерация
// =============================================================================
static inline double calc_regen_hp(int flat, int pct, int max_hp) {
	int safe = (pct > 50) ? 50 : pct;
	return flat + (max_hp * (safe / 100.0));
}
static inline double calc_regen_mp(int flat, int pct, int max_mp) {
	int safe = (pct > 50) ? 50 : pct;
	return flat + (max_mp * (safe / 100.0));
}

// =============================================================================
// AOE урон
// =============================================================================
static inline double calc_aoe_dmg(double base, int pct, int mult) {
	int p = (pct > 500) ? 500 : pct;
	int m = (mult > 500) ? 500 : mult;
	return base * (p / 100.0) * (1.0 + m / 100.0);
}

// =============================================================================
// Формула стоимости персонажа (fs_persGetCost)
// compX = _COMP(level, _X0), compV = _COMP(level, _V0)
// =============================================================================
static inline double calc_pers_cost(double avg_str, double avg_pwrmin, double avg_pwrmax,
                                     double avg_xhpmax, double avg_magma_pwrmin,
                                     double avg_magma_pwrmax, double compX, double compV) {
	return (avg_str + (avg_pwrmin * 0.1 + avg_pwrmax * 0.1) / (2 * _Xs)
		+ avg_xhpmax / _Vs
		+ (avg_magma_pwrmin * 0.1 + avg_magma_pwrmax * 0.1) / (2 * _Xs)
		- (compX + compV) * _IComp);
}

// =============================================================================
// Формулы вероятностей (evade, crit, block, anti-*)
// =============================================================================
// avgBS = (BASE_SUM(lvl1) + BASE_SUM(lvl2)) / 2
// avgIS = max((ITEM_SUM(lvl1) + ITEM_SUM(lvl2)) / 2, (cost1 + cost2) / 2 - avgBS * 0.6)

static inline double calc_evade_prob(double stat, double avg_bs, double avg_is,
                                      bool target_art, double limit) {
	double p0 = _pE0;
	double p  = _pE;
	double evP = target_art ? _evPA : _evP0;
	return MIN(MAX((stat - (avg_bs / 5)) * (p - p0) / (avg_is / 2), 0) + p0, evP);
}

static inline double calc_crit_prob(double stat, double avg_bs, double avg_is,
                                     bool attacker_art, double limit) {
	double p0 = _pC0;
	double p  = _pC;
	double evP = attacker_art ? _evPA : _evP0;
	return MIN(MAX((stat - (avg_bs / 5)) * (p - p0) / (avg_is / 2), 0) + p0, evP);
}

static inline double calc_block_prob(double stat, double avg_bs, double avg_is,
                                      bool target_art, double limit) {
	double p0 = _pB0;
	double p  = _pB;
	double evP = target_art ? _evPA : _evP0;
	return MIN(MAX((stat - (avg_bs / 5)) * (p - p0) / (avg_is / 2), 0) + p0, evP);
}

static inline double calc_anti_evade(double stat, double avg_bs, double avg_is,
                                      bool attacker_art, double limit) {
	double p0 = _pAE0;
	double p  = _pAE;
	double evP = attacker_art ? _evPA : _evP0;
	return MIN(MAX((stat - (avg_bs / 5)) * (p - p0) / (avg_is / 2), 0) + p0, evP);
}

static inline double calc_anti_crit(double stat, double avg_bs, double avg_is,
                                     bool target_art, double limit) {
	if (!target_art) return 0;
	double p0 = _pAC0;
	double p  = _pAC;
	double evP = _evPA;
	return MIN(MAX((stat - (avg_bs / 5)) * (p - p0) / (avg_is / 2), 0) + p0, evP);
}

static inline double calc_anti_block(double stat, double avg_bs, double avg_is,
                                      bool attacker_art, double limit) {
	if (!attacker_art) return 0;
	double p0 = _pAB0;
	double p  = _pAB;
	double evP = attacker_art ? _evPA : _evP0;
	return MIN(MAX((stat - (avg_bs / 5)) * (p - p0) / (avg_is / 2), 0) + p0, evP);
}

// =============================================================================
// Формулы урона (physical min/max)
// usage: comp = _COMP(MIN(level, 10), _X0)  // caller must pass pre-computed value
// =============================================================================
static inline double calc_phys_dmg(double pwr, double str, double comp, double factor) {
	return pwr * 0.1 + (str + comp * (1 - _IComp)) * _Xs * factor;
}

// =============================================================================
// Формула абсорба (damage absorb)
// =============================================================================
static inline double calc_absorb(double stat, double avg_bs, double avg_is,
                                  bool target_art) {
	double max_absorb = target_art ? 0.8 : 0.07;
	return MIN(MAX((stat - (avg_bs / 5)) * (_Ap - _Ap0) / (avg_is / 2), 0) + _Ap0, max_absorb);
}

// =============================================================================
// Формула combo probability
// =============================================================================
static inline double calc_combo_prob(int pers_level, int cmb_level) {
	return 1 - (pers_level - cmb_level) * _CMBp;
}

// =============================================================================
// Формула критического урона
// =============================================================================
static inline double calc_crit_dmg(double base, double attacker_crit_bonus,
                                    double defender_crit_reduce, double deadly_mult) {
	double cxMagAdd = attacker_crit_bonus - defender_crit_reduce;
	double dmg = base * (_Cx + cxMagAdd);
	if (deadly_mult > 0) dmg *= deadly_mult;
	return dmg;
}

// =============================================================================
// Формула HP при recalc (fs_persRecalcEffects)
// compV = _COMP(level, _V0) - caller must pass pre-computed value
// =============================================================================
static inline double calc_recalc_hp(int vit, double compV, bool art) {
	return round((vit + compV * (1 - _IComp)) * _Vs * (art ? 1 : _decHP));
}

// =============================================================================
// Формула magic damage (wisdom contribution)
// =============================================================================
static inline double calc_wisdom_dmg(double wisdom, int n) {
	return wisdom * _Xs / n;
}

// =============================================================================
// Объявление clamp-функции (реализация в pers.c)
// =============================================================================
int fs_clamp_skill(int skill, int val);

static inline double calc_block_damage(double dmg, int block_dmg_p) {
	if (block_dmg_p <= 0) return 0.0;
	double pct = (double)block_dmg_p / 100.0;
	if (pct > 1.0) pct = 1.0;
	return dmg * pct;
}

static inline double calc_block_phys_reduce(double dmg, int rst_phys_block) {
	if (rst_phys_block <= 0) return dmg;
	double reduce = 1.0 - ((double)rst_phys_block / 100.0);
	if (reduce < 0.0) reduce = 0.0;
	return dmg * reduce;
}

// =============================================================================
// EXECUTE_P: добивающий удар
// Активируется когда HP цели ≤ threshold
// =============================================================================
#define EXECUTE_HP_THRESHOLD 0.25   // 25% HP
static inline double calc_execute_bonus(double dmg, int execute_p,
										 int target_hp, int target_hpmax) {
	if (execute_p <= 0 || target_hpmax <= 0) return dmg;
	double hp_ratio = (double)target_hp / (double)target_hpmax;
	if (hp_ratio > EXECUTE_HP_THRESHOLD) return dmg;
	return dmg * (1.0 + execute_p / 100.0);
}

// =============================================================================
// CRIT_RATE: плоский бонус к шансу крита
// BLOCK_CHANCE: плоский бонус к шансу блока
// EVADE_CHANCE: плоский бонус к шансу уклонения
// =============================================================================

// Применять после формульного расчёта, до общего clamp
static inline double apply_flat_crit_bonus(double pC, int crit_rate_skill,
											bool art) {
	if (crit_rate_skill <= 0) return pC;
	double bonus = crit_rate_skill / 100.0;
	double cap   = art ? _evPA : 0.80;
	double result = pC + bonus;
	return result > cap ? cap : result;
}

static inline double apply_flat_block_bonus(double pB, int block_chance_skill,
											 bool art) {
	if (block_chance_skill <= 0) return pB;
	double bonus  = block_chance_skill / 100.0;
	double cap    = art ? _evPA : 0.80;
	double result = pB + bonus;
	return result > cap ? cap : result;
}

static inline double apply_flat_evade_bonus(double pE, int evade_chance_skill,
											 bool art) {
	if (evade_chance_skill <= 0) return pE;
	double bonus  = evade_chance_skill / 100.0;
	double cap    = art ? EVADE_FLAT_CAP : EVADE_FLAT_CAP_NOART;
	double result = pE + bonus;
	return result > cap ? cap : result;
}

// =============================================================================
// FS_SK_MULTI_HIT_PERCENT_DAMAGE / FS_SK_MULTI_HIT_CHANCE: доп. удар с шансом
// =============================================================================
// percent_damage: % от dmgA для доп. удара (FS_SK_PENETRATION)
// multi_hit_chance: % шанс нанести этот удар (FS_SK_MULTI_HIT_CHANCE)
//   если multi_hit_chance=0 → всегда срабатывает (обратная совместимость)
// Возвращает: урон доп. удара или 0 если не сработал

static inline double calc_penetration_dmg(double dmgA, int percent_damage,
											int multi_hit_chance_skill) {
	if (multi_hit_chance_skill <= 0) return 0.0;

	if (percent_damage <= 0) return 0.0;

	if (multi_hit_chance_skill > 0) {
		// Проверка шанса происходит снаружи (randRoll) — здесь только расчёт
		return dmgA * ((double)percent_damage / 100.0);
	}
	// Без шанса — всегда 100%
	return dmgA * ((double)percent_damage / 100.0);
}

// =============================================================================
// SELF_HEAL_MULT / HEAL_RCVD_MULT / HEAL_POWER
// =============================================================================

// Применяется в fs_persDamage когда dmg < 0 (лечение)
// is_self_heal = true если activator == pers или activator == NULL
// (зелья, CD-предметы без внешнего активатора)

static inline double calc_heal_with_mult(double heal,
										  int heal_rcvd_mult,    // у цели (%)
										  int heal_power,        // у хилера (%)
										  int self_heal_mult,    // у себя (%)
										  bool is_self_heal) {
	if (heal_rcvd_mult > 0)
		heal *= (1.0 + heal_rcvd_mult / 100.0);
	if (!is_self_heal && heal_power > 0)
		heal *= (1.0 + heal_power / 100.0);
	if (is_self_heal && self_heal_mult > 0)
		heal *= (1.0 + self_heal_mult / 100.0);
	return heal;
}

// =============================================================================
// DEADLY_STRIKE / DS_DMG
// =============================================================================
// Срабатывает только при физическом крите (fC=true)
// dsChance: шанс (%)
// dsDmg: бонус % к урону
// Возвращает итоговый урон и устанавливает *fired=true если сработал

static inline double calc_deadly_strike(double crit_dmg, int ds_chance,
										 int ds_dmg, bool *fired) {
	*fired = false;
	if (ds_chance <= 0) return crit_dmg;
	// Проверка randRoll снаружи
	if (ds_dmg <= 0) ds_dmg = 50;    // минимум +50% к крит-урону
	*fired = true;
	return crit_dmg * (1.0 + ds_dmg / 100.0);
}

// =============================================================================
// DMGX_RECV: увеличение получаемого урона (дебафф)
// DMG_REDUCE_P: снижение получаемого урона
// Применяются вместе: итоговый модификатор = dmgx_recv - dmg_reduce_p
// =============================================================================

static inline double calc_recv_dmg_modifiers(double dmg,
											  int dmgx_recv,
											  int dmg_reduce_p) {
	int net_pct = dmgx_recv - dmg_reduce_p;  // может быть отрицательным
	if (net_pct == 0) return dmg;
	return dmg * (1.0 + net_pct / 100.0);
}

static inline double calc_recv_dmg_flat(double dmg, int dmg_reduce_flat) {
	if (dmg_reduce_flat <= 0) return dmg;
	double result = dmg - (double)dmg_reduce_flat;
	return result < 1.0 ? 1.0 : result;
}

#endif
