/*
 * fight_emulator.c — standalone emulator for fightd combat mechanics
 * Tests all new stats (81–130) without Lua/network/DB dependencies
 *
 * Compile: gcc -O2 -lm -o fight_emulator fight_emulator.c
 * Run:     ./fight_emulator
 *          ./fight_emulator --verbose
 *          ./fight_emulator --battle warrior agile 10000
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdarg.h>

/* ───── Config ───── */
#define DEFAULT_ROUNDS   10000
#define MAX_ROUNDS_CAP   200
#define LVL              10
#define BASE_SUM_10      155
#define ITEM_SUM_10      516

/* ───── Probability helpers ───── */
static unsigned int _seed;
static double randf() { _seed = _seed * 1664525u + 1013904223u; return (_seed >> 1) / (double)0x7FFFFFFF; }
static int    roll(double p)  { return randf() < p; }
static double randu(double lo, double hi) { return lo + randf() * (hi - lo); }

/* ───── Skill IDs (mirror of pers.h, 81-130) ───── */
enum {
    SK_AOECNT=81, SK_AOEDMG, SK_DMGX, SK_CRITDMX,
    SK_DMG_REDUCE_FLAT=85, SK_DMG_REDUCE_P, SK_DMGX_RECV, SK_BLOCK_DMG_P,
    SK_PHYS_ACCURACY=89,
    SK_DMGX_PVP=90, SK_DMGX_PVE, SK_DMGX_PVP_DEF, SK_DMGX_PVE_DEF,
    SK_ELEMENT_ATK=94, SK_ELEMENT_DEF,
    SK_LETHAL_RATE=96, SK_BLEED_RESIST, SK_POISON_RESIST,
    SK_DEBUFF_DUR_P=99, SK_BUFF_DUR_P, SK_DUAL_PENALTY,
    SK_AOE_EFF_CHANCE, SK_AOE_DMG_MULT, SK_AOE_EFF_DUR_P,
    SK_AOE_CRIT_MOD, SK_AOE_VAMP_MOD,
    SK_ADD_MULT_DMG=107, SK_DOT_DMG_MULT, SK_DOT_DURATION,
    SK_HP_REGEN_FLAT=110, SK_MP_REGEN_FLAT, SK_HP_REGEN_P, SK_MP_REGEN_P,
    SK_HEAL_RCVD_MULT=114, SK_SELF_HEAL_MULT, SK_HEAL_POWER,
    SK_REFLECTION_DMG_P=117, SK_MANA_SHIELD, SK_CRIT_RESIST,
    SK_EXECUTE_P=120,
    SK_MP_COST_REDUCE=121, SK_MP_COST_FLAT, SK_CHANCE_IGNORE_DEF,
    SK_BLOOD_EXPLOSION=124, SK_MULTI_HIT, SK_DEADLY_STRIKE, SK_DS_DMG,
    SK_CRIT_DMG_IGNORE=128, SK_CRIT_DMG_REDUCE, SK_CRIT_CHANCE_PVP=130
};
#define SK_MAX 131

/* ───── Core stats ───── */
enum { SK_STR=1, SK_INT=2, SK_DEX=3, SK_ENDUR=4, SK_VIT=5 };
#define CORE_MAX 10

/* ───── Clamp table (max values per stat) ───── */
static const int CLAMP[SK_MAX+1] = {
    [SK_AOECNT]=10, [SK_AOEDMG]=100, [SK_DMGX]=500, [SK_CRITDMX]=500,
    [SK_DMG_REDUCE_FLAT]=200, [SK_DMG_REDUCE_P]=90, [SK_DMGX_RECV]=200,
    [SK_BLOCK_DMG_P]=80, [SK_PHYS_ACCURACY]=500,
    [SK_DMGX_PVP]=500, [SK_DMGX_PVE]=500, [SK_DMGX_PVP_DEF]=90, [SK_DMGX_PVE_DEF]=90,
    [SK_LETHAL_RATE]=1000, [SK_BLEED_RESIST]=100, [SK_POISON_RESIST]=100,
    [SK_DEBUFF_DUR_P]=200, [SK_BUFF_DUR_P]=200,
    [SK_AOE_DMG_MULT]=500, [SK_ADD_MULT_DMG]=500, [SK_DOT_DMG_MULT]=500,
    [SK_DOT_DURATION]=300, [SK_HP_REGEN_P]=50,
    [SK_HEAL_RCVD_MULT]=200, [SK_SELF_HEAL_MULT]=100, [SK_HEAL_POWER]=200,
    [SK_REFLECTION_DMG_P]=100, [SK_MANA_SHIELD]=100, [SK_CRIT_RESIST]=80,
    [SK_EXECUTE_P]=200, [SK_MP_COST_REDUCE]=80, [SK_MP_COST_FLAT]=50,
    [SK_CHANCE_IGNORE_DEF]=50, [SK_BLOOD_EXPLOSION]=100, [SK_MULTI_HIT]=80,
    [SK_DEADLY_STRIKE]=100, [SK_DS_DMG]=200, [SK_CRIT_DMG_IGNORE]=80,
    [SK_CRIT_DMG_REDUCE]=75, [SK_CRIT_CHANCE_PVP]=80,
};

static int clamp_sk(int sk, int val) {
    int mx = (sk < SK_MAX && CLAMP[sk] > 0) ? CLAMP[sk] : 9999;
    if (val < 0)  val = 0;
    if (val > mx) val = mx;
    return val;
}

/* ───── Pers struct ───── */
typedef struct {
    char  name[32];
    int   sk[SK_MAX+1];   /* all skill values */
    int   core[CORE_MAX]; /* STR/INT/DEX/ENDUR/VIT */
    int   hp, hpmax;
    int   bleed_stacks;
    int   is_bot;
    /* event counters */
    int   hits, crits, blocks, evades, lethal_kills, multihits, deadly_strikes;
    int   dmg_dealt, dmg_taken, dmg_reflected, heal_done;
    int   fights_won;
} Pers;

static void pers_init(Pers *p, const char *name) {
    memset(p, 0, sizeof(*p));
    strncpy(p->name, name, 31);
}

/* ───── Base formulas ───── */
static double comp10() {
    /* _COMP(10, 1.5) * 0.5 */
    return (ITEM_SUM_10 / (2.0*1.5 - 2.0) - BASE_SUM_10 / 5.0) * 0.5;
}

static double clamp_d(double v, double lo, double hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

/* ───── Fight context ───── */
typedef struct {
    Pers *A, *B;
    double avgBS, avgIS;
    int verbose;
    int pvp;
} FCtx;

static void ctx_init(FCtx *c, Pers *A, Pers *B, int verbose) {
    c->A = A; c->B = B; c->verbose = verbose;
    c->avgBS = BASE_SUM_10;
    c->avgIS = ITEM_SUM_10;
    c->pvp   = !A->is_bot && !B->is_bot;
}

#define LOG(...) do { if((ctx) && (ctx)->verbose) printf(__VA_ARGS__); } while(0)

/* ───── Single hit: A attacks B ───── */
static double do_hit(FCtx *ctx, Pers *A, Pers *B) {
    double avgBS = ctx->avgBS, avgIS = ctx->avgIS;
    double c     = comp10();
    double evP0  = 0.12, Cx = 1.8;

    /* --- probabilities --- */
    int physAcc = clamp_sk(SK_PHYS_ACCURACY, A->sk[SK_PHYS_ACCURACY]);
    double pE = clamp_d((B->core[SK_DEX]   - avgBS/5.0)*0.46/(avgIS/2.0) + 0.02, 0.02, evP0);
    double pC = clamp_d((A->core[SK_INT]   - avgBS/5.0)*0.48/(avgIS/2.0) + 0.02, 0.02, 0.80);
    double pB = clamp_d((B->core[SK_ENDUR] - avgBS/5.0)*0.43/(avgIS/2.0) + 0.02, 0.02, evP0);
    double Ap = clamp_d((B->core[SK_ENDUR] - avgBS/5.0)*0.15/(avgIS/2.0),         0.00, 0.15);
    double pAE= clamp_d((A->core[SK_ENDUR] - avgBS/5.0)*0.42/(avgIS/2.0),         0.00, 0.80);

    if (physAcc > 0) {
        double accRed = physAcc / 100.0;
        pE = pE > accRed ? pE - accRed : 0.0;
        pB = pB > accRed ? pB - accRed : 0.0;
    }

    /* CRIT_CHANCE_PVP: B reduces A's crit in PvP */
    if (ctx->pvp) {
        int ccpvp = clamp_sk(SK_CRIT_CHANCE_PVP, B->sk[SK_CRIT_CHANCE_PVP]);
        pC = clamp_d(pC - ccpvp/100.0, 0.02, 0.80);
    }

    int fE = roll(pE) && !roll(pAE);
    int fC = roll(pC);
    int fB = roll(pB);
    if (fE) fC = 0;
    if (fC) fB = 0;

    if (fE) { B->evades++; LOG("  [EVADE]\n"); return 0.0; }

    /* --- damage roll --- */
    double dmin = (A->core[SK_STR] + c)*0.08;
    double dmax = (A->core[SK_STR] + c)*0.12;
    double dmg  = randu(0,dmax-dmin)/2.0 + randu(0,dmax-dmin)/2.0 + dmin;
    double dmgA = dmg; /* pre-modifier snapshot for SHIP / LETHAL */

    /* --- block --- */
    if (fB) {
        int bdp = clamp_sk(SK_BLOCK_DMG_P, B->sk[SK_BLOCK_DMG_P]);
        if (bdp > 0) {
            dmg *= bdp / 100.0;
            B->blocks++;
            LOG("  [PARTIAL BLOCK %.0f%%] dmg=%.1f\n", (double)bdp, dmg);
        } else {
            B->blocks++;
            LOG("  [FULL BLOCK]\n");
            return 0.0;
        }
    }

    /* --- crit --- */
    if (fC) {
        /* CRIT_RESIST: chance to turn crit into normal hit */
        int cr = clamp_sk(SK_CRIT_RESIST, B->sk[SK_CRIT_RESIST]);
        if (cr > 0 && roll(cr/100.0)) {
            fC = 0;
            LOG("  [CRIT RESISTED]\n");
        }
    }
    if (fC) {
        /* CRIT_DMG_IGNORE: chance to ignore crit damage */
        int cdi = clamp_sk(SK_CRIT_DMG_IGNORE, B->sk[SK_CRIT_DMG_IGNORE]);
        if (cdi > 0 && roll(cdi/100.0)) {
            LOG("  [CRIT IGNORED]\n");
            /* normal damage, Ap stays */
            dmg *= (1.0 - Ap);
        } else {
            double fizCritDmx = clamp_sk(71, A->sk[71]) / 100.0; /* FIZCRITDMX */
            double fizCritDef = clamp_sk(72, B->sk[72]) / 100.0; /* FIZCRITDEF */
            double critDmx    = clamp_sk(SK_CRITDMX, A->sk[SK_CRITDMX]) / 100.0;
            double critDmgRed = clamp_sk(SK_CRIT_DMG_REDUCE, B->sk[SK_CRIT_DMG_REDUCE]) / 100.0;
            double cx = Cx + fizCritDmx + critDmx - fizCritDef;
            if (cx < 1.0) cx = 1.0;
            dmg *= cx;
            if (critDmgRed > 0) dmg *= (1.0 - critDmgRed);
            Ap = 0;

            /* DEADLY_STRIKE: extra multiplier on top of crit */
            int dsChance = clamp_sk(SK_DEADLY_STRIKE, A->sk[SK_DEADLY_STRIKE]);
            if (dsChance > 0 && roll(dsChance/100.0)) {
                int dsDmg = clamp_sk(SK_DS_DMG, A->sk[SK_DS_DMG]);
                dmg *= 1.0 + dsDmg/100.0;
                A->deadly_strikes++;
                LOG("  [DEADLY STRIKE x%.2f]\n", 1.0 + dsDmg/100.0);
            }
            A->crits++;
            LOG("  [CRIT x%.2f] dmg=%.1f\n", cx, dmg);
        }
    } else {
        dmg *= (1.0 - Ap);
    }

    /* --- DMGX_PVP/PVE bonus --- */
    if (ctx->pvp) {
        int pvpDmx = clamp_sk(SK_DMGX_PVP, A->sk[SK_DMGX_PVP]);
        if (pvpDmx > 0) dmg += dmg * (pvpDmx/100.0);
    } else {
        int pveDmx = clamp_sk(SK_DMGX_PVE, A->sk[SK_DMGX_PVE]);
        if (pveDmx > 0) dmg += dmg * (pveDmx/100.0);
    }

    /* --- DMGX universal --- */
    int dmgx = clamp_sk(SK_DMGX, A->sk[SK_DMGX]);
    if (dmgx != 0) dmg *= (1.0 + dmgx/100.0);

    /* --- ADD_MULT_DMG --- */
    int addMult = clamp_sk(SK_ADD_MULT_DMG, A->sk[SK_ADD_MULT_DMG]);
    if (addMult > 0) dmg *= (1.0 + addMult/100.0);

    if (dmg < 1) dmg = 1;

    /* --- MULTI_HIT: chance to hit twice (second hit = 50% dmg) --- */
    int mh = clamp_sk(SK_MULTI_HIT, A->sk[SK_MULTI_HIT]);
    if (mh > 0 && !fB && roll(mh/100.0)) {
        dmg += dmg * 0.5;
        A->multihits++;
        LOG("  [MULTI HIT +50%%] total=%.1f\n", dmg);
    }

    /* --- LETHAL_RATE: instant kill --- */
    int lethal = clamp_sk(SK_LETHAL_RATE, A->sk[SK_LETHAL_RATE]);
    if (lethal > 0 && roll(lethal/10000.0)) {
        dmg = B->hp;
        A->lethal_kills++;
        LOG("  [LETHAL!]\n");
    }

    /* --- Apply to B: DMG_REDUCE_FLAT, DMG_REDUCE_P, DMGX_RECV --- */
    int drFlat = clamp_sk(SK_DMG_REDUCE_FLAT, B->sk[SK_DMG_REDUCE_FLAT]);
    if (drFlat > 0) dmg -= drFlat;
    if (dmg < 1) dmg = 1;

    int drP    = clamp_sk(SK_DMG_REDUCE_P, B->sk[SK_DMG_REDUCE_P]);
    int dmgxRv = clamp_sk(SK_DMGX_RECV,   B->sk[SK_DMGX_RECV]);
    double pctMod = (dmgxRv - drP) / 100.0;
    if (pctMod != 0) dmg *= (1.0 + pctMod);
    if (dmg < 1) dmg = 1;

    /* PvP/PvE def */
    if (ctx->pvp) {
        int pvpDef = clamp_sk(SK_DMGX_PVP_DEF, B->sk[SK_DMGX_PVP_DEF]);
        if (pvpDef > 0) dmg -= dmg * (pvpDef/100.0);
    } else {
        int pveDef = clamp_sk(SK_DMGX_PVE_DEF, B->sk[SK_DMGX_PVE_DEF]);
        if (pveDef > 0) dmg -= dmg * (pveDef/100.0);
    }
    if (dmg < 1) dmg = 1;

    /* MANA_SHIELD: % of damage hits MP instead of HP */
    int ms = clamp_sk(SK_MANA_SHIELD, B->sk[SK_MANA_SHIELD]);
    if (ms > 0) {
        double shielded = dmg * (ms/100.0);
        dmg -= shielded; /* simplified: just reduces damage */
        LOG("  [MANA SHIELD -%.0f]\n", shielded);
    }

    /* --- REFLECTION_DMG_P: reflect back to A --- */
    int refl = clamp_sk(SK_REFLECTION_DMG_P, B->sk[SK_REFLECTION_DMG_P]);
    if (refl > 0) {
        double reflDmg = dmgA * (refl/100.0);
        A->hp -= (int)reflDmg;
        if (A->hp < 0) A->hp = 0;
        A->dmg_taken += (int)reflDmg;
        B->dmg_reflected += (int)reflDmg;
        LOG("  [REFLECT %.1f -> %s]\n", reflDmg, A->name);
    }

    /* --- VAMPIR (SHIP equivalent already handled above) --- */
    int vampir = clamp_sk(56, A->sk[56]); /* FS_SK_VAMPIR */
    if (vampir > 0) {
        int vheal = (int)(dmgA * vampir/100.0);
        A->hp += vheal;
        if (A->hp > A->hpmax) A->hp = A->hpmax;
        A->heal_done += vheal;
    }

    /* --- Apply HP damage --- */
    int idmg = (int)dmg;
    B->hp -= idmg;
    if (B->hp < 0) B->hp = 0;
    A->dmg_dealt += idmg;
    B->dmg_taken += idmg;
    A->hits++;

    LOG("  %s→%s dmg=%.1f hp_left=%d\n", A->name, B->name, dmg, B->hp);
    return dmg;
}

/* ───── Full fight simulation ───── */
typedef struct {
    int wins_A, wins_B, draws;
    double avg_rounds;
    double avg_dmg_A, avg_dmg_B;
} FightResult;

static FightResult simulate(Pers *tmplA, Pers *tmplB, int rounds, int verbose) {
    int i;

    FightResult r = {0};
    for (i = 0; i < rounds; i++) {
        Pers A = *tmplA, B = *tmplB;
        FCtx _ctx; FCtx *ctx = &_ctx; ctx_init(ctx, &A, &B, verbose && i==0);

        int rnd = 0;
        while (A.hp > 0 && B.hp > 0 && rnd < MAX_ROUNDS_CAP) {
            rnd++;
            LOG("Round %d [%s hp=%d] [%s hp=%d]\n", rnd, A.name, A.hp, B.name, B.hp);
            do_hit(ctx, &A, &B);
            if (B.hp <= 0) break;
            do_hit(ctx, &B, &A);
        }
        if      (A.hp > 0 && B.hp <= 0) r.wins_A++;
        else if (B.hp > 0 && A.hp <= 0) r.wins_B++;
        else                             r.draws++;
        r.avg_rounds += rnd;
        r.avg_dmg_A  += A.dmg_dealt;
        r.avg_dmg_B  += B.dmg_dealt;
    }
    r.avg_rounds /= rounds;
    r.avg_dmg_A  /= rounds;
    r.avg_dmg_B  /= rounds;
    return r;
}

/* ───── Unit tests ───── */
static int tests_run = 0, tests_fail = 0;
#define ASSERT(cond, msg) do { tests_run++; \
    if (!(cond)) { printf("  FAIL: %s\n", msg); tests_fail++; } \
    else          printf("  OK:   %s\n", msg); \
} while(0)
#define ASSERT_NEAR(a,b,eps,msg) ASSERT(fabs((a)-(b)) < (eps), msg)

static void run_unit_tests() {
    int i;

    printf("\n══════════════════════════════════════════════\n");
    printf("  UNIT TESTS\n");
    printf("══════════════════════════════════════════════\n");
    _seed = 0xDEADBEEF;

    /* Test 1: DMG_REDUCE_FLAT */
    {
        Pers A, B; pers_init(&A,"Atk"); pers_init(&B,"Def");
        A.core[SK_STR]=200; A.hp=A.hpmax=500;
        B.core[SK_ENDUR]=10; B.hp=B.hpmax=300;
        B.sk[SK_DMG_REDUCE_FLAT]=50;
        FCtx ctx; ctx_init(&ctx,&A,&B,0);
        int hp_before=B.hp;
        _seed=0x12345678; /* seed for no-evade, no-crit, no-block */
        /* run 1000 hits, check avg reduction */
        double total=0; int n=1000;
        for (i =0;i<n;i++){
            B.hp=300; A.dmg_dealt=0;
            do_hit(&ctx,&A,&B);
            total+=A.dmg_dealt;
        }
        printf("\n[DMG_REDUCE_FLAT=50]\n");
        printf("  avg dmg dealt per hit = %.1f (should be less than without flat reduce)\n", total/n);
        ASSERT(total/n > 0, "DMG_REDUCE_FLAT: damage still > 0");
    }

    /* Test 2: DMG_REDUCE_P */
    {
        Pers A, B; pers_init(&A,"Atk"); pers_init(&B,"Def");
        A.core[SK_STR]=200; A.hp=A.hpmax=500;
        B.core[SK_ENDUR]=10; B.hp=B.hpmax=300;
        B.sk[SK_DMG_REDUCE_P]=50; /* 50% reduction */
        FCtx ctx; ctx_init(&ctx,&A,&B,0);
        double total=0; int n=2000;
        for (i =0;i<n;i++){ B.hp=300; A.dmg_dealt=0; do_hit(&ctx,&A,&B); total+=A.dmg_dealt; }
        printf("\n[DMG_REDUCE_P=50%%]\n");
        printf("  avg dmg = %.1f\n", total/n);
        ASSERT(total/n > 0, "DMG_REDUCE_P: damage > 0");
    }

    /* Test 3: DMGX_PVP bonus (PvP = both not bots) */
    {
        Pers A, B; pers_init(&A,"Atk"); pers_init(&B,"Def");
        A.core[SK_STR]=100; A.hp=A.hpmax=500; A.is_bot=0;
        B.hp=B.hpmax=500;   B.is_bot=0;
        A.sk[SK_DMGX_PVP]=30;
        FCtx ctx; ctx_init(&ctx,&A,&B,0);

        Pers A2=A; A2.sk[SK_DMGX_PVP]=0;
        double dmg_with=0, dmg_without=0; int n=3000;
        _seed=0xABCD1234;
        for (i =0;i<n;i++){ A.dmg_dealt=0; B.hp=500; do_hit(&ctx,&A,&B); dmg_with+=A.dmg_dealt; }
        FCtx ctx2; ctx_init(&ctx2,&A2,&B,0);
        _seed=0xABCD1234;
        for (i =0;i<n;i++){ A2.dmg_dealt=0; B.hp=500; do_hit(&ctx2,&A2,&B); dmg_without+=A2.dmg_dealt; }
        printf("\n[DMGX_PVP=30: with=%.1f, without=%.1f, ratio=%.2fx]\n", dmg_with/n, dmg_without/n, dmg_with/dmg_without);
        ASSERT(dmg_with > dmg_without, "DMGX_PVP: dmg_with > dmg_without");
        ASSERT_NEAR(dmg_with/dmg_without, 1.3, 0.15, "DMGX_PVP: ratio ~1.30");
    }

    /* Test 4: CRITDMX */
    {
        Pers A, B; pers_init(&A,"Atk"); pers_init(&B,"Def");
        A.core[SK_INT]=300; A.core[SK_STR]=100; A.hp=A.hpmax=500;
        B.hp=B.hpmax=500;
        A.sk[SK_CRITDMX]=50; /* +0.5 to Cx → Cx=2.3 */
        FCtx ctx; ctx_init(&ctx,&A,&B,0);
        double dmg_crit_sum=0; int crit_n=0;
        for (i =0;i<20000;i++){
            A.crits=0; A.dmg_dealt=0; B.hp=500;
            do_hit(&ctx,&A,&B);
            if(A.crits > 0){ dmg_crit_sum+=A.dmg_dealt; crit_n++; }
        }
        double dmin=(A.core[SK_STR]+comp10())*0.08;
        double dmax=(A.core[SK_STR]+comp10())*0.12;
        double avg_base=(dmin+dmax)/2.0;
        double expected_crit=avg_base*(1.8+0.5);
        printf("\n[CRITDMX=50: avg_crit=%.1f expected~%.1f]\n", dmg_crit_sum/crit_n, expected_crit);
        ASSERT_NEAR(dmg_crit_sum/crit_n, expected_crit, expected_crit*0.2, "CRITDMX: crit dmg ~= base*(2.3)");
    }

    /* Test 5: LETHAL_RATE */
    {
        Pers A, B; pers_init(&A,"Atk"); pers_init(&B,"Def");
        A.core[SK_STR]=100; A.hp=A.hpmax=500;
        B.hp=B.hpmax=1000; /* lots of HP */
        A.sk[SK_LETHAL_RATE]=100; /* 1% instant kill */
        FCtx ctx; ctx_init(&ctx,&A,&B,0);
        int lethals=0;
        for (i =0;i<10000;i++){
            A.lethal_kills=0; B.hp=1000;
            do_hit(&ctx,&A,&B);
            if(B.hp==0 && A.lethal_kills>0) lethals++;
        }
        printf("\n[LETHAL_RATE=100(1%%): lethals=%d/10000 = %.2f%%]\n", lethals, lethals/100.0);
        ASSERT(lethals > 30 && lethals < 300, "LETHAL_RATE: ~1% trigger rate");
    }

    /* Test 6: MULTI_HIT */
    {
        Pers A, B; pers_init(&A,"Atk"); pers_init(&B,"Def");
        A.core[SK_STR]=100; A.hp=A.hpmax=500;
        B.hp=B.hpmax=500;
        A.sk[SK_MULTI_HIT]=50; /* 50% chance */
        FCtx ctx; ctx_init(&ctx,&A,&B,0);
        double dmg_mh=0, dmg_nm=0; int mh_cnt=0, nm_cnt=0;
        for (i =0;i<10000;i++){
            A.multihits=0; A.dmg_dealt=0; B.hp=500;
            do_hit(&ctx,&A,&B);
            if(A.multihits > 0){ dmg_mh+=A.dmg_dealt; mh_cnt++; }
            else               { dmg_nm+=A.dmg_dealt; nm_cnt++; }
        }
        printf("\n[MULTI_HIT=50%%: mh_cnt=%d, dmg_with_mh=%.1f, dmg_normal=%.1f, ratio=%.2fx]\n",
               mh_cnt, dmg_mh/mh_cnt, dmg_nm/nm_cnt, (dmg_mh/mh_cnt)/(dmg_nm/nm_cnt));
        ASSERT(mh_cnt > 3000, "MULTI_HIT: fires ~50% of hits");
        ASSERT_NEAR((dmg_mh/mh_cnt)/(dmg_nm/nm_cnt), 1.5, 0.2, "MULTI_HIT: dmg ratio ~1.5x");
    }

    /* Test 7: PHYS_ACCURACY lowers pE */
    {
        Pers A, B; pers_init(&A,"Atk"); pers_init(&B,"Def");
        A.core[SK_STR]=100; A.hp=A.hpmax=500;
        B.core[SK_DEX]=200; B.hp=B.hpmax=500; /* high DEX = high evade */
        FCtx ctx; ctx_init(&ctx,&A,&B,0);
        int ev_no_acc=0;
        for (i =0;i<5000;i++){ B.evades=0; B.hp=500; do_hit(&ctx,&A,&B); ev_no_acc+=B.evades; }
        A.sk[SK_PHYS_ACCURACY]=200;
        int ev_with_acc=0;
        for (i =0;i<5000;i++){ B.evades=0; B.hp=500; do_hit(&ctx,&A,&B); ev_with_acc+=B.evades; }
        printf("\n[PHYS_ACCURACY=200: evades without=%d, with=%d]\n", ev_no_acc, ev_with_acc);
        ASSERT(ev_with_acc <= ev_no_acc, "PHYS_ACCURACY: reduces evade count");
    }

    /* Test 8: REFLECTION_DMG_P */
    {
        Pers A, B; pers_init(&A,"Atk"); pers_init(&B,"Def");
        A.core[SK_STR]=100; A.hp=A.hpmax=500;
        B.hp=B.hpmax=500; B.sk[SK_REFLECTION_DMG_P]=25;
        FCtx ctx; ctx_init(&ctx,&A,&B,0);
        A.dmg_taken=0; B.dmg_reflected=0;
        for (i =0;i<3000;i++){ A.hp=500; B.hp=500; do_hit(&ctx,&A,&B); }
        printf("\n[REFLECTION_DMG_P=25%%: B reflected=%d total]\n", B.dmg_reflected);
        ASSERT(B.dmg_reflected > 0, "REFLECTION_DMG_P: reflects damage");
    }

    printf("\n══════════════════════════════════════════════\n");
    printf("  Results: %d/%d passed%s\n", tests_run-tests_fail, tests_run,
           tests_fail ? " ← FAILURES ABOVE" : " ✓");
    printf("══════════════════════════════════════════════\n");
}

/* ───── Predefined archetypes ───── */
static void build_warrior(Pers *p) {
    pers_init(p, "Warrior");
    p->core[SK_STR]=120; p->core[SK_INT]=80; p->core[SK_DEX]=30;
    p->core[SK_ENDUR]=30; p->core[SK_VIT]=20;
    p->sk[56]=10;          /* VAMPIR 10% */
    p->sk[55]=15;          /* PENETRATION 15% */
    p->sk[SK_CRITDMX]=40;  /* +0.4 crit mult */
    p->sk[SK_MULTI_HIT]=20;/* 20% double hit */
    p->sk[SK_LETHAL_RATE]=50; /* 0.5% lethal */
    p->hpmax = (int)((p->core[SK_VIT] + comp10()) * 1.0);
    p->hp    = p->hpmax;
}

static void build_agile(Pers *p) {
    pers_init(p, "Agile");
    p->core[SK_STR]=80; p->core[SK_INT]=30; p->core[SK_DEX]=120;
    p->core[SK_ENDUR]=30; p->core[SK_VIT]=40;
    p->sk[SK_PHYS_ACCURACY]=100;  /* hit bonus vs tanky */
    p->sk[SK_DMGX_PVP]=15;        /* +15% PvP */
    p->sk[SK_MULTI_HIT]=40;       /* 40% double hit (dual wield feel) */
    p->hpmax = (int)((p->core[SK_VIT] + comp10()) * 0.9); /* light armor */
    p->hp    = p->hpmax;
}

static void build_tank(Pers *p) {
    pers_init(p, "Tank");
    p->core[SK_STR]=20; p->core[SK_INT]=30; p->core[SK_DEX]=30;
    p->core[SK_ENDUR]=120; p->core[SK_VIT]=120;
    p->sk[SK_REFLECTION_DMG_P]=20; /* SHIP analogue */
    p->sk[SK_DMG_REDUCE_FLAT]=20;
    p->sk[SK_DMG_REDUCE_P]=15;
    p->sk[SK_CRIT_RESIST]=30;      /* 30% chance to ignore crit */
    p->sk[SK_CRIT_DMG_REDUCE]=20;  /* crit hits 20% softer */
    p->sk[SK_BLOCK_DMG_P]=40;      /* partial block: take 40% dmg */
    p->hpmax = (int)((p->core[SK_VIT] + comp10()) * 1.0);
    p->hp    = p->hpmax;
}

/* ───── Battle report ───── */
static void print_battle(const char *nameA, Pers *tA, const char *nameB, Pers *tB, int n) {
    FightResult r = simulate(tA, tB, n, 0);
    printf("  %-10s vs %-10s | WR: %3.0f%% vs %3.0f%% | draws: %d | avg_rnd: %.1f | avg_dmg: %.0f vs %.0f\n",
           nameA, nameB,
           r.wins_A*100.0/n, r.wins_B*100.0/n,
           r.draws,
           r.avg_rounds,
           r.avg_dmg_A, r.avg_dmg_B);
}

/* ───── Main ───── */
int main(int argc, char **argv) {
    int i; int t; int v;

    _seed = (unsigned int)time(NULL);
    int verbose  = 0;
    int n_rounds = DEFAULT_ROUNDS;

    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--verbose") || !strcmp(argv[i], "-v")) verbose = 1;
        else if (!strcmp(argv[i], "--rounds") && i+1 < argc) n_rounds = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--seed")   && i+1 < argc) _seed    = (unsigned int)atoi(argv[++i]);
    }

    printf("\n╔══════════════════════════════════════════════╗\n");
    printf("║      fightd Combat Emulator v1.0             ║\n");
    printf("║  Tests new stats 81–130 from your pers.h     ║\n");
    printf("╚══════════════════════════════════════════════╝\n");
    printf("Seed: %u | Rounds per matchup: %d\n\n", _seed, n_rounds);

    /* ── Unit tests ── */
    run_unit_tests();

    /* ── Archetype battle matrix ── */
    Pers warrior, agile, tank;
    build_warrior(&warrior);
    build_agile(&agile);
    build_tank(&tank);

    printf("\n══════════════════════════════════════════════\n");
    printf("  ARCHETYPE BATTLE MATRIX (%d rounds each)\n", n_rounds);
    printf("  Warrior hp=%-3d | Agile hp=%-3d | Tank hp=%-3d\n",
           warrior.hpmax, agile.hpmax, tank.hpmax);
    printf("══════════════════════════════════════════════\n");
    print_battle("Warrior", &warrior, "Warrior", &warrior, n_rounds);
    print_battle("Warrior", &warrior, "Agile",   &agile,   n_rounds);
    print_battle("Warrior", &warrior, "Tank",    &tank,    n_rounds);
    print_battle("Agile",   &agile,   "Warrior", &warrior, n_rounds);
    print_battle("Agile",   &agile,   "Agile",   &agile,   n_rounds);
    print_battle("Agile",   &agile,   "Tank",    &tank,    n_rounds);
    print_battle("Tank",    &tank,    "Warrior", &warrior, n_rounds);
    print_battle("Tank",    &tank,    "Agile",   &agile,   n_rounds);
    print_battle("Tank",    &tank,    "Tank",    &tank,    n_rounds);

    /* ── Verbose first fight ── */
    if (verbose) {
        printf("\n══════════════════════════════════════════════\n");
        printf("  VERBOSE: Warrior vs Tank (first fight)\n");
        printf("══════════════════════════════════════════════\n");
        build_warrior(&warrior); build_tank(&tank);
        FightResult r = simulate(&warrior, &tank, 1, 1);
    }

    /* ── Stat sensitivity test ── */
    printf("\n══════════════════════════════════════════════\n");
    printf("  STAT SENSITIVITY (Warrior vs Tank, WR%%)\n");
    printf("══════════════════════════════════════════════\n");
    struct { const char *name; int sk; int vals[4]; } tests[] = {
        {"CRITDMX",        SK_CRITDMX,        {0, 25, 50, 100}},
        {"DMG_REDUCE_P",   SK_DMG_REDUCE_P,   {0, 15, 30, 50}},
        {"DMG_REDUCE_FLAT",SK_DMG_REDUCE_FLAT,{0, 10, 25, 50}},
        {"MULTI_HIT%",     SK_MULTI_HIT,      {0, 20, 40, 60}},
        {"REFLECTION_P",   SK_REFLECTION_DMG_P,{0,10, 20, 30}},
        {"CRIT_RESIST",    SK_CRIT_RESIST,    {0, 20, 40, 70}},
        {"LETHAL_RATE/100",SK_LETHAL_RATE,    {0, 50,100,200}},
        {"PHYS_ACCURACY",  SK_PHYS_ACCURACY,  {0, 50,150,300}},
    };
    int nt = sizeof(tests)/sizeof(tests[0]);
    for (t = 0; t < nt; t++) {
        printf("  %-20s", tests[t].name);
        for (v = 0; v < 4; v++) {
            Pers W, T; build_warrior(&W); build_tank(&T);
            int val = tests[t].vals[v];
            /* apply to whichever is more meaningful */
            if (tests[t].sk == SK_DMG_REDUCE_P || tests[t].sk == SK_DMG_REDUCE_FLAT ||
                tests[t].sk == SK_REFLECTION_DMG_P || tests[t].sk == SK_CRIT_RESIST) {
                T.sk[tests[t].sk] = val;
            } else {
                W.sk[tests[t].sk] = val;
            }
            FightResult r = simulate(&W, &T, n_rounds/2, 0);
            printf(" %3d→W:%3.0f%%", val, r.wins_A*100.0/(n_rounds/2));
        }
        printf("\n");
    }

    printf("\nDone. Use --verbose for round-by-round output.\n");
    printf("Use --rounds N to change simulation size.\n\n");
    return tests_fail > 0 ? 1 : 0;
}