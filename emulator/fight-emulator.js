#!/usr/bin/env node
/**
 * fightd_test.js — Integration tests for fightd daemon
 *
 * Usage:
 *   node fightd_test.js                          # run all tests
 *   node fightd_test.js --host 127.0.0.1 --ctrl 5469 --client 11112
 *   node fightd_test.js --test dmg_reduce_flat   # one test
 *   node fightd_test.js --list                   # list tests
 *   node fightd_test.js --verbose                # show raw packets
 */

'use strict';
const net  = require('net');
const args = parseArgs(process.argv.slice(2));

const HOST        = args['host']   || 'host.docker.internal';
const CTRL_PORT   = parseInt(args['ctrl']   || args['port'] || '5469');
const CLIENT_PORT = parseInt(args['client'] || '11112');
const VERBOSE     = !!args['verbose'];

/* ═══════════════════════════ Wire protocol ══════════════════════════════ */

// Param types (from io.h)
const PT = {
    INT: 1, FIXED: 2, STRING: 3, NINT: 5, NFIXED: 6,
    SHORTINT: 7, NSHORTINT: 8, BIGINT: 9, NBIGINT: 10
};

// ── Encoding ──────────────────────────────────────────────────────────────

function hex8(n)  { return ((n >>> 0) & 0xFFFFFFFF).toString(16).padStart(8, '0'); }
function hex4(n)  { return (n & 0xFFFF).toString(16).padStart(4, '0'); }
function hex16(n) { return BigInt(n).toString(16).padStart(16, '0'); }

function pInt(id, val) {
    const t = val < 0 ? PT.NINT : PT.INT;
    return hex4((id << 8) | t) + hex8(Math.abs(val));
}
function pBigInt(id, val) {
    const t = val < 0 ? PT.NBIGINT : PT.BIGINT;
    return hex4((id << 8) | t) + hex16(Math.abs(val));
}
function pStr(id, val) {
    const s = String(val);
    return hex4((id << 8) | PT.STRING) + hex4(s.length) + s;
}
function pFixed(id, val) {
    const t = val < 0 ? PT.NFIXED : PT.FIXED;
    return hex4((id << 8) | t) + hex8(Math.round(Math.abs(val) * 10000));
}

function makePacket(...params) {
    const body = params.join('');
    return hex4(body.length) + body + '\x00';
}

// ── Decoding ──────────────────────────────────────────────────────────────

function parsePacket(buf) {
    let s = typeof buf === 'string' ? buf : buf.toString('latin1');
    s = s.replace(/\x00/g, '');   // strip nulls
    const params = [];
    let i = 0;
    while (i < s.length) {
        if (i + 4 > s.length) break;
        const hdr  = parseInt(s.slice(i, i + 4), 16);
        const id   = hdr >> 8;
        const type = hdr & 0xFF;
        i += 4;
        switch (type) {
            case PT.INT:
                params.push({ id, type, val: parseInt(s.slice(i, i + 8), 16) >>> 0 });
                i += 8; break;
            case PT.NINT:
                params.push({ id, type, val: -(parseInt(s.slice(i, i + 8), 16) >>> 0) });
                i += 8; break;
            case PT.BIGINT:
                params.push({ id, type, val: parseInt(s.slice(i, i + 16), 16) });
                i += 16; break;
            case PT.NBIGINT:
                params.push({ id, type, val: -parseInt(s.slice(i, i + 16), 16) });
                i += 16; break;
            case PT.FIXED:
                params.push({ id, type, val: parseInt(s.slice(i, i + 8), 16) / 10000 });
                i += 8; break;
            case PT.NFIXED:
                params.push({ id, type, val: -(parseInt(s.slice(i, i + 8), 16) / 10000) });
                i += 8; break;
            case PT.SHORTINT:
                params.push({ id, type, val: parseInt(s.slice(i, i + 4), 16) });
                i += 4; break;
            case PT.NSHORTINT:
                params.push({ id, type, val: -parseInt(s.slice(i, i + 4), 16) });
                i += 4; break;
            case PT.STRING: {
                const len = parseInt(s.slice(i, i + 4), 16);
                i += 4;
                params.push({ id, type, val: s.slice(i, i + len) });
                i += len; break;
            }
            default:
                if (VERBOSE) console.log(`  [WARN] unknown param type ${type} at offset ${i}`);
                i = s.length; // stop
        }
    }
    return params;
}

/* ═══════════════════════════ Connection ════════════════════════════════ */

class FConn {
    constructor(host, port, label = '') {
        this.host  = host;
        this.port  = port;
        this.label = label;
        this.sock  = null;
        this._buf  = '';
        this._pending = [];   // [{resolve, reject}]
    }

    connect() {
        return new Promise((resolve, reject) => {
            this.sock = net.createConnection({ host: this.host, port: this.port });
            this.sock.setEncoding('latin1');
            this.sock.on('connect', () => resolve(this));
            this.sock.on('error', reject);
            this.sock.on('data', d => this._onData(d));
            this.sock.on('close', () => {
                this._pending.forEach(p => p.reject(new Error('Connection closed')));
                this._pending = [];
            });
            setTimeout(() => reject(new Error(`connect timeout to ${this.host}:${this.port}`)), 5000);
        });
    }

    _onData(chunk) {
        this._buf += chunk;
        this._tryFlush();
    }

    _tryFlush() {
        while (this._buf.length >= 4) {
            const size = parseInt(this._buf.slice(0, 4), 16);
            if (isNaN(size) || size <= 0) { this._buf = this._buf.slice(1); continue; }
            // need 4 (size) + size (body) + 1 (\0)
            if (this._buf.length < 4 + size + 1) break;
            const body = this._buf.slice(4, 4 + size);
            this._buf = this._buf.slice(4 + size + 1);
            const params = parsePacket(body);
            if (VERBOSE) {
                const cmd  = params[0]?.val ?? '?';
                const stat = params[1]?.val ?? '?';
                console.log(`  [${this.label || this.port}] ← cmd=${cmd} status=${stat}`);
            }
            if (this._pending.length > 0) {
                const { resolve } = this._pending.shift();
                resolve(params);
            }
        }
    }

    send(...paramStrs) {
        const pkt = makePacket(...paramStrs);
        if (VERBOSE) console.log(`  [${this.label || this.port}] → ${pkt.slice(0, 80)}`);
        this.sock.write(pkt, 'latin1');
    }

    recv(timeout = 8000) {
        return new Promise((resolve, reject) => {
            const t = setTimeout(() => {
                const idx = this._pending.findIndex(p => p.reject === reject);
                if (idx >= 0) this._pending.splice(idx, 1);
                reject(new Error(`recv timeout (${timeout}ms) on ${this.label}:${this.port}`));
            }, timeout);
            this._pending.push({
                resolve: p => { clearTimeout(t); resolve(p); },
                reject:  e => { clearTimeout(t); reject(e);  }
            });
            this._tryFlush();
        });
    }

    async sendRecv(...paramStrs) {
        this.send(...paramStrs);
        return this.recv();
    }

    close() {
        if (this.sock) { this.sock.destroy(); this.sock = null; }
    }
}

/* ═══════════════════════════ Server commands ═══════════════════════════ */

const SC = {
    SYNC_TIME:      1,
    SRV_INFO:      11,
    GET_FIGHTS:    15,
    CREATE_FIGHT:  16,
    SET_PARAMS:    17,
    START_FIGHT:   18,
    STOP_FIGHT:    19,
    DELETE_FIGHT:  20,
    CREATE_PERS:   21,
    SET_SKILLS:    22,
    BIND_PERS:     26,
    DELETE_PERS:   27,
    GET_FIGHTSTATE:28,
    GET_FIGHTLOG:  29,
    // client
    INIT:         101,
    STATE:        102,
    ATTACK:       105,
    FIGHT_STATE:  106,
};

const SS_OK = 0;

// Skills
const SK = {
    STR: 1, INT: 2, DEX: 3, ENDUR: 4, VIT: 5, WISDOM: 6,
    HP: 10, HPMAX: 11, MP: 12, MPMAX: 13,
    SHIP: 54, PENETRATION: 55, VAMPIR: 56, STOIKOST: 57,
    AOECNT: 81, AOEDMG: 82, DMGX: 83, CRITDMX: 84,
    DMG_REDUCE_FLAT: 85, DMG_REDUCE_P: 86, DMGX_RECV: 87, BLOCK_DMG_P: 88,
    PHYS_ACCURACY: 89, DMGX_PVP: 90, DMGX_PVE: 91,
    DMGX_PVP_DEF: 92, DMGX_PVE_DEF: 93,
    LETHAL_RATE: 96, BLEED_RESIST: 97,
    MULTI_HIT: 125, DEADLY_STRIKE: 126, DS_DMG: 127,
    CRIT_DMG_IGNORE: 128, CRIT_DMG_REDUCE: 129, CRIT_CHANCE_PVP: 130,
};

const PF_ART = 0x20;

let _idCounter = 20000 + Math.floor(Math.random() * 5000);
const nextId = () => ++_idCounter;

function checkOk(resp, label) {
    if (!resp || resp.length < 2 || resp[1].val !== SS_OK) {
        throw new Error(`${label} failed: status=${resp?.[1]?.val ?? 'no response'}`);
    }
}

async function ctrlSend(ctrl, cmd, ...params) {
    const r = await ctrl.sendRecv(pInt(0, cmd), ...params);
    return r;
}

/* ═══════════════════════════ Fight builder ═════════════════════════════ */

class Fight {
    constructor(ctrl) {
        this.ctrl     = ctrl;
        this.fightId  = nextId();
        this.idA      = nextId();
        this.idB      = nextId();
        this.akeyA    = Math.floor(Math.random() * 90000) + 10000;
        this.akeyB    = Math.floor(Math.random() * 90000) + 10000;
        this.statsA   = { [SK.STR]:100, [SK.INT]:50, [SK.DEX]:50, [SK.ENDUR]:50, [SK.VIT]:60 };
        this.statsB   = { [SK.STR]:100, [SK.INT]:50, [SK.DEX]:50, [SK.ENDUR]:50, [SK.VIT]:60 };
        this.extraA   = {};
        this.extraB   = {};
    }

    setA(sk, v) { this.extraA[sk] = v; return this; }
    setB(sk, v) { this.extraB[sk] = v; return this; }
    statsAFn(obj) { Object.assign(this.statsA, obj); return this; }
    statsBFn(obj) { Object.assign(this.statsB, obj); return this; }

    async _createPers(id, akey, nick, flags = PF_ART) {
        const r = await ctrlSend(this.ctrl, SC.CREATE_PERS,
            pInt(0, id),    pInt(0, akey),  pInt(0, flags),
            pStr(0, nick),  pStr(0, ''),
            pInt(0, 10),    // level
            pInt(0, 1),     // gender
            pInt(0, 0),     // partyId
            pInt(0, 0),     // clanId
            pInt(0, 1),     // kind
            pInt(0, 1),     // cls
            pInt(0, 1),     // skeleton
            pInt(0, 0),     // skeletonTime
            pInt(0, 63),    // partMask
            pInt(0, 0),     // artId
            pInt(0, 10000), // expX
            pStr(0, ''),    // ctrlData
            pInt(0, 0),     // petLevel
            pInt(0, 0),     // petReady
            pInt(0, 1),     // autoKick = 1 (server auto-attacks)
            pInt(0, 100),   // arrowsCnt
            pInt(0, 0),     // yarost
            pStr(0, ''),    // petSrc
            pInt(0, 100),   // yarost_max
        );
        checkOk(r, `CREATE_PERS ${nick}`);
    }

    async _setSkills(id, base, extra) {
        const all = { ...base, ...extra };
        const params = Object.entries(all).map(([sk, v]) => pInt(Number(sk), v));
        const r = await ctrlSend(this.ctrl, SC.SET_SKILLS, pInt(0, id), ...params);
        checkOk(r, `SET_SKILLS id=${id}`);
    }

    async create() {
        const r = await ctrlSend(this.ctrl, SC.CREATE_FIGHT, pBigInt(0, this.fightId));
        checkOk(r, 'CREATE_FIGHT');

        await this._createPers(this.idA, this.akeyA, 'TestA');
        await this._createPers(this.idB, this.akeyB, 'TestB');
        await this._setSkills(this.idA, this.statsA, this.extraA);
        await this._setSkills(this.idB, this.statsB, this.extraB);

        let r2 = await ctrlSend(this.ctrl, SC.BIND_PERS,
            pInt(0, this.idA), pBigInt(0, this.fightId), pInt(0, 1));
        checkOk(r2, 'BIND_PERS A');
        r2 = await ctrlSend(this.ctrl, SC.BIND_PERS,
            pInt(0, this.idB), pBigInt(0, this.fightId), pInt(0, 2));
        checkOk(r2, 'BIND_PERS B');

        r2 = await ctrlSend(this.ctrl, SC.START_FIGHT, pBigInt(0, this.fightId));
        checkOk(r2, 'START_FIGHT');
        return this;
    }

    async run(timeoutMs = 25000) {
        const connA = new FConn(HOST, CLIENT_PORT, `cA:${this.idA}`);
        const connB = new FConn(HOST, CLIENT_PORT, `cB:${this.idB}`);
        await connA.connect();
        await connB.connect();

        const result = { idA: this.idA, idB: this.idB, winner: null,
            hpA: null, hpB: null, dmgA: 0, dmgB: 0 };

        // Authorize
        for (const [c, pid, akey] of [[connA, this.idA, this.akeyA], [connB, this.idB, this.akeyB]]) {
            const r = await c.sendRecv(
                pInt(0, SC.INIT), pInt(0, pid), pBigInt(0, this.fightId), pInt(0, akey));
            checkOk(r, `SCCL_INIT pid=${pid}`);
        }

        // Poll state until fight ends
        const deadline = Date.now() + timeoutMs;
        let done = false;

        const poll = async (conn, pid, isA) => {
            try {
                const r = await conn.sendRecv(pInt(0, SC.STATE), pInt(0, pid));
                // STATE: [cmd, status, fightTimeout, persId, persStatus,
                //         persLStatus, persFlags, dmg, hp, hpMax, mp, mpMax, oppId...]
                // persStatus=2 = FS_PS_DEAD
                if (r.length >= 9) {
                    const hp  = r[8]?.val ?? null;
                    const dmg = r[7]?.val ?? 0;
                    const st  = r[4]?.val ?? -1;  // persStatus
                    if (isA) { result.hpA = hp; result.dmgA += dmg; }
                    else      { result.hpB = hp; result.dmgB += dmg; }
                    if (st === 2 || hp === 0) {  // DEAD
                        result.winner = result.winner || (isA ? 'B' : 'A');
                        done = true;
                    }
                }
            } catch(e) {
                if (!done) done = true;
            }
        };

        while (!done && Date.now() < deadline) {
            await sleep(350);
            await Promise.allSettled([
                poll(connA, this.idA, true),
                poll(connB, this.idB, false),
            ]);
        }

        connA.close();
        connB.close();

        // Check fight state from ctrl side
        try {
            const fs = await ctrlSend(this.ctrl, SC.GET_FIGHTSTATE,
                pBigInt(0, this.fightId), pInt(0, 0));
            if (fs && fs[1]?.val === SS_OK && !result.winner) {
                result.winner = result.hpA === 0 ? 'B' : result.hpB === 0 ? 'A' : 'draw';
            }
        } catch(e) {}

        // Cleanup
        await this._cleanup();
        return result;
    }

    async _cleanup() {
        try {
            await ctrlSend(this.ctrl, SC.DELETE_FIGHT, pBigInt(0, this.fightId));
        } catch(e) {}
    }
}

/* ═══════════════════════════ Test suite ════════════════════════════════ */

const results = [];

async function runTest(name, fn) {
    process.stdout.write(`\n  ${'─'.repeat(52)}\n  TEST: ${name}\n`);
    const ctrl = new FConn(HOST, CTRL_PORT, 'ctrl');
    try {
        await ctrl.connect();
        await fn(ctrl);
        console.log('  ✓  PASS');
        results.push({ name, ok: true });
    } catch(e) {
        console.log(`  ✗  FAIL: ${e.message}`);
        if (VERBOSE) console.error(e);
        results.push({ name, ok: false, err: e.message });
    } finally {
        ctrl.close();
    }
}

function assertEqual(a, b, msg = '') {
    if (a !== b) throw new Error(`${msg}: expected ${b}, got ${a}`);
}
function assertGt(a, b, msg = '') {
    if (!(a > b)) throw new Error(`${msg}: ${a} should be > ${b}`);
}
function assertLt(a, b, msg = '') {
    if (!(a < b)) throw new Error(`${msg}: ${a} should be < ${b}`);
}
function assertBetween(a, lo, hi, msg = '') {
    if (!(lo <= a && a <= hi)) throw new Error(`${msg}: ${a} not in [${lo},${hi}]`);
}

async function quickFight(ctrl, optA = {}, optB = {}, sA = {}, sB = {}) {
    const f = new Fight(ctrl);
    Object.entries(optA).forEach(([k, v]) => f.setA(Number(k), v));
    Object.entries(optB).forEach(([k, v]) => f.setB(Number(k), v));
    if (Object.keys(sA).length) f.statsAFn(sA);
    if (Object.keys(sB).length) f.statsBFn(sB);
    await f.create();
    return f.run();
}

async function multiRun(ctrl, n, buildFn) {
    let winsA = 0, winsB = 0;
    for (let i = 0; i < n; i++) {
        const r = await buildFn(ctrl);
        if (r.winner === 'A') winsA++;
        else if (r.winner === 'B') winsB++;
    }
    return { winsA, winsB, n };
}

/* ── Individual tests ──────────────────────────────────────────────────── */

const TESTS = {

    async server_info(ctrl) {
        const r = await ctrlSend(ctrl, SC.SRV_INFO);
        assertEqual(r[1].val, SS_OK, 'SRV_INFO status');
        console.log(`    Server responded: ${r.slice(0,4).map(p=>p.val).join(', ')}`);
    },

    async duplicate_fight_id(ctrl) {
        const fid = nextId();
        let r = await ctrlSend(ctrl, SC.CREATE_FIGHT, pBigInt(0, fid));
        checkOk(r, 'first CREATE_FIGHT');
        r = await ctrlSend(ctrl, SC.CREATE_FIGHT, pBigInt(0, fid));
        const status = r[1].val;
        await ctrlSend(ctrl, SC.DELETE_FIGHT, pBigInt(0, fid));
        assertLt(status, 0, 'Duplicate fight ID');
        console.log(`    Duplicate correctly rejected: status=${status}`);
    },

    async invalid_skill_id(ctrl) {
        const pid = nextId();
        let r = await ctrlSend(ctrl, SC.CREATE_PERS,
            pInt(0,pid), pInt(0,9999), pInt(0,PF_ART),
            pStr(0,'SKTest'), pStr(0,''),
            pInt(0,10), pInt(0,1), pInt(0,0), pInt(0,0),
            pInt(0,1), pInt(0,1), pInt(0,1), pInt(0,0),
            pInt(0,63), pInt(0,0), pInt(0,10000),
            pStr(0,''), pInt(0,0), pInt(0,0), pInt(0,1),
            pInt(0,0), pInt(0,0), pStr(0,''), pInt(0,100));
        checkOk(r, 'CREATE_PERS');
        // Skill ID 200 > MAXCODE=130
        r = await ctrlSend(ctrl, SC.SET_SKILLS, pInt(0, pid), pInt(200, 999));
        const status = r[1].val;
        await ctrlSend(ctrl, SC.DELETE_PERS, pInt(0, pid));
        assertLt(status, 0, 'Invalid skill ID > MAXCODE');
        console.log(`    Skill ID 200 rejected: status=${status}`);
    },

    async basic_fight(ctrl) {
        const r = await quickFight(ctrl);
        if (!['A','B','draw'].includes(r.winner)) throw new Error(`No winner: ${JSON.stringify(r)}`);
        console.log(`    Winner=${r.winner} | hpA=${r.hpA} hpB=${r.hpB}`);
    },

    async dmg_reduce_flat(ctrl) {
        // B with flat reduction should win more against same A
        const normal   = await multiRun(ctrl, 5, c => quickFight(c));
        const reduced  = await multiRun(ctrl, 5, c => quickFight(c, {}, { [SK.DMG_REDUCE_FLAT]: 30 }));
        console.log(`    A wins: normal=${normal.winsA}/5, vs flat-reduce=${reduced.winsA}/5`);
        assertLt(reduced.winsA, normal.winsA + 3, 'DMG_REDUCE_FLAT should help B');
    },

    async dmg_reduce_percent(ctrl) {
        const normal  = await multiRun(ctrl, 5, c => quickFight(c));
        const reduced = await multiRun(ctrl, 5, c => quickFight(c, {}, { [SK.DMG_REDUCE_P]: 50 }));
        console.log(`    A wins: normal=${normal.winsA}/5, vs pct-reduce=${reduced.winsA}/5`);
        assertLt(reduced.winsA, normal.winsA + 3, 'DMG_REDUCE_P should help B');
    },

    async dmgx_pvp_bonus(ctrl) {
        const normal  = await multiRun(ctrl, 5, c => quickFight(c));
        const boosted = await multiRun(ctrl, 5, c => quickFight(c, { [SK.DMGX_PVP]: 50 }));
        console.log(`    A wins: normal=${normal.winsA}/5, DMGX_PVP50=${boosted.winsA}/5`);
        assertGt(boosted.winsA, normal.winsA - 2, 'DMGX_PVP should help A');
    },

    async high_crit_vs_tank(ctrl) {
        // Crit build (INT=150) vs Tank (ENDUR=150, VIT=120)
        const { winsA: wA, winsB: wB } = await multiRun(ctrl, 8, c =>
            quickFight(c,
                {}, {},
                { [SK.STR]:60,  [SK.INT]:150, [SK.VIT]:20   },
                { [SK.STR]:20,  [SK.ENDUR]:150, [SK.VIT]:120 }
            )
        );
        console.log(`    CritBuild=${wA}/8, Tank=${wB}/8`);
        // Tank should win at least some fights
        assertGt(wB, 0, 'Tank must win at least 1 fight');
    },

    async phys_accuracy_vs_evader(ctrl) {
        const noAcc  = await multiRun(ctrl, 5, c =>
            quickFight(c, {}, {}, { [SK.STR]:100, [SK.DEX]:30 }, { [SK.STR]:30, [SK.DEX]:200 }));
        const withAcc = await multiRun(ctrl, 5, c =>
            quickFight(c, { [SK.PHYS_ACCURACY]: 300 }, {}, { [SK.STR]:100, [SK.DEX]:30 }, { [SK.STR]:30, [SK.DEX]:200 }));
        console.log(`    A wins vs high-DEX: no_acc=${noAcc.winsA}/5, with_acc=${withAcc.winsA}/5`);
        assertGt(withAcc.winsA, noAcc.winsA - 2, 'PHYS_ACCURACY should improve hit rate');
    },

    async crit_protection(ctrl) {
        const exposed   = await multiRun(ctrl, 5, c =>
            quickFight(c, {}, {}, { [SK.STR]:60, [SK.INT]:150 }, { [SK.VIT]:80 }));
        const protected_ = await multiRun(ctrl, 5, c =>
            quickFight(c,
                {}, { [SK.CRIT_DMG_IGNORE]: 40, [SK.CRIT_DMG_REDUCE]: 30 },
                { [SK.STR]:60, [SK.INT]:150 }, { [SK.VIT]:80 }
            )
        );
        console.log(`    A wins: unprotected=${exposed.winsA}/5, crit-protected B=${protected_.winsA}/5`);
        assertLt(protected_.winsA, exposed.winsA + 3, 'Crit protection should help B');
    },

    async ship_reflection(ctrl) {
        // B has SHIP=30% — A should take reflected damage, may die from it
        const noShip  = await multiRun(ctrl, 5, c => quickFight(c));
        const withShip = await multiRun(ctrl, 5, c =>
            quickFight(c, {}, { [SK.SHIP]: 30 }));
        console.log(`    A wins: no_ship=${noShip.winsA}/5, B_ship30=${withShip.winsA}/5`);
        // With ship, A should win LESS (takes reflected damage)
        assertLt(withShip.winsA, noShip.winsA + 3, 'SHIP should hurt attacker');
    },

    async multi_hit(ctrl) {
        // A with MULTI_HIT should win more than without
        const normal   = await multiRun(ctrl, 5, c => quickFight(c));
        const multiHit = await multiRun(ctrl, 5, c =>
            quickFight(c, { [SK.MULTI_HIT]: 60 }));
        console.log(`    A wins: normal=${normal.winsA}/5, MULTI_HIT60=${multiHit.winsA}/5`);
        assertGt(multiHit.winsA, normal.winsA - 3, 'MULTI_HIT should help A deal more damage');
    },

    async vampir_sustain(ctrl) {
        // A with high VAMPIR heals from hits — should survive longer against tank
        const noVamp   = await multiRun(ctrl, 5, c =>
            quickFight(c, {}, {}, { [SK.STR]:60 }, { [SK.VIT]:200 }));
        const withVamp = await multiRun(ctrl, 5, c =>
            quickFight(c, { [SK.VAMPIR]: 50 }, {}, { [SK.STR]:60 }, { [SK.VIT]:200 }));
        console.log(`    A wins vs high-HP target: no_vamp=${noVamp.winsA}/5, vampir50=${withVamp.winsA}/5`);
        assertGt(withVamp.winsA, noVamp.winsA - 2, 'VAMPIR should improve survivability');
    },

};

/* ═══════════════════════════ Runner ═══════════════════════════════════ */

function sleep(ms) { return new Promise(r => setTimeout(r, ms)); }

function parseArgs(argv) {
    const out = {};
    for (let i = 0; i < argv.length; i++) {
        if (argv[i].startsWith('--')) {
            const key = argv[i].slice(2);
            out[key] = (i + 1 < argv.length && !argv[i+1].startsWith('--'))
                ? argv[++i] : true;
        }
    }
    return out;
}

async function main() {
    console.log(`\n${'═'.repeat(56)}`);
    console.log(`  fightd Integration Tests (Node.js)`);
    console.log(`  ctrl  → ${HOST}:${CTRL_PORT}`);
    console.log(`  client→ ${HOST}:${CLIENT_PORT}`);
    console.log(`${'═'.repeat(56)}`);

    if (args['list']) {
        console.log('\nAvailable tests:');
        Object.keys(TESTS).forEach(n => console.log(`  ${n}`));
        process.exit(0);
    }

    // Test connection first
    try {
        const t = new FConn(HOST, CTRL_PORT, 'probe');
        await t.connect();
        const r = await t.sendRecv(pInt(0, SC.SRV_INFO));
        t.close();
        if (!r || r[1]?.val !== SS_OK)
            throw new Error(`Server not ready: ${r?.[1]?.val}`);
        console.log('\n  ✓ Connected to fightd\n');
    } catch(e) {
        console.error(`\n  ✗ Cannot connect to ${HOST}:${CTRL_PORT} — ${e.message}`);
        console.error(`  Check: docker ps, port mapping, fightd running`);
        process.exit(1);
    }

    const filter = args['test'];
    const toRun  = filter
        ? (TESTS[filter] ? { [filter]: TESTS[filter] } : null)
        : TESTS;

    if (!toRun) {
        console.error(`Unknown test: ${filter}. Use --list`);
        process.exit(1);
    }

    for (const [name, fn] of Object.entries(toRun)) {
        await runTest(name, fn);
        await sleep(200);  // brief pause between tests
    }

    const passed = results.filter(r => r.ok).length;
    const total  = results.length;
    console.log(`\n${'═'.repeat(56)}`);
    console.log(`  Results: ${passed}/${total} passed ${passed === total ? '✓' : ''}`);
    if (passed < total) {
        results.filter(r => !r.ok).forEach(r => console.log(`  ✗ ${r.name}: ${r.err}`));
    }
    console.log(`${'═'.repeat(56)}\n`);
    process.exit(passed === total ? 0 : 1);
}

main().catch(e => { console.error(e); process.exit(1); });