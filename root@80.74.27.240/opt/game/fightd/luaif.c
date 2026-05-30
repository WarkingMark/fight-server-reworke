/* 
 * modifed: igorpauk 2017-18
 */

#include "common.h"
#include "fight.h"
#include "pers.h"
#include "luaif.h"


// wrapper constructors -----------------------------------------

#define LUAIF_WRAPPER(NAME, DATATYPE) \
	int luaif_##NAME(lua_State *L) { \
		DATATYPE   *data = luaif_checkptr(L,1,#DATATYPE); \
		if (!data) return 0;
#define LUAIF_WRAPPER_END \
		return 1; \
	}

#define LUAIF_WRAPPER_STRUCT(NAME, STRUCTTYPE) \
	int luaif_##NAME(lua_State *L) { \
		STRUCTTYPE *data = luaif_checkptr(L,1,#STRUCTTYPE); \
		if (!data) return 0; \
		LUAIF_WRAPPER_NEWTABLE;
#define LUAIF_WRAPPER_STRUCT_END \
		return 1; \
	}

#define LUAIF_WRAPPER_VECTOR(NAME, DATATYPE) \
	int luaif_##NAME(lua_State *L) { \
		DATATYPE   *data; \
		vector_t   *vec; \
		viter_t    vi; \
		int        idx = 1; \
		vec = luaif_checkptr(L,1,"vector_t"); \
		if (!vec) return luaL_error(L,"Wrong data"); \
		LUAIF_WRAPPER_NEWTABLE; \
		v_reset(vec,&vi); \
		while ((data = v_each(vec,&vi))) {
#define LUAIF_WRAPPER_VECTOR_END \
			idx++; \
		} \
		return 1; \
	}

#define LUAIF_WRAPPER_STRUCTVECTOR(NAME, STRUCTTYPE, FIELD, DATATYPE) \
	int luaif_##NAME(lua_State *L) { \
		STRUCTTYPE *rec; \
		DATATYPE   *data; \
		viter_t    vi; \
		int        idx = 1; \
		rec = luaif_checkptr(L,1,#STRUCTTYPE); \
		if (!rec) return luaL_error(L,"Wrong data"); \
		LUAIF_WRAPPER_NEWTABLE; \
		v_reset(rec->FIELD,&vi); \
		while ((data = v_each(rec->FIELD,&vi))) {
#define LUAIF_WRAPPER_STRUCTVECTOR_END \
			idx++; \
		} \
		return 1; \
	}

// push new table
#define LUAIF_WRAPPER_NEWTABLE \
	lua_newtable(L);

// push data->FIELD
#define LUAIF_WRAPPER_PUSHFIELD(FIELD, PUSHTYPE) \
	lua_push##PUSHTYPE(L,data->FIELD);

// push data->FIELD[KEY]
#define LUAIF_WRAPPER_PUSHFIELDVALUE(FIELD, KEY, PUSHTYPE) \
	lua_push##PUSHTYPE(L,data->FIELD[KEY]);

// push EXPR
#define LUAIF_WRAPPER_PUSHEXPR(EXPR, PUSHTYPE) \
	lua_push##PUSHTYPE(L,EXPR);

// table.FIELD = data->FIELD
#define LUAIF_WRAPPER_ADDFIELD(FIELD, PUSHTYPE) \
	lua_push##PUSHTYPE(L,data->FIELD); \
	lua_setfield(L,-2,#FIELD);

// table.FIELD = *data
#define LUAIF_WRAPPER_ADDPTR(FIELD, DATATYPE) \
	luaif_pushptr(L,data,#DATATYPE); \
	lua_setfield(L,-2,#FIELD);

// table.FIELD = EXPR
#define LUAIF_WRAPPER_ADDEXPR(FIELD, EXPR, PUSHTYPE) \
	lua_push##PUSHTYPE(L,EXPR); \
	lua_setfield(L,-2,#FIELD);

// table[] = data->FIELD
#define LUAIF_WRAPPER_NEXTFIELD(FIELD, PUSHTYPE) \
	lua_push##PUSHTYPE(L,data->FIELD); \
	lua_rawseti(L,-2,idx);

// table[] = *data
#define LUAIF_WRAPPER_NEXTPTR(DATATYPE) \
	luaif_pushptr(L,data,#DATATYPE); \
	lua_rawseti(L,-2,idx);

// table[] = table
#define LUAIF_WRAPPER_NEXTTABLE \
	lua_rawseti(L,-2,idx);


// ----------------------------------------------------------------------------

void *luaif_checkptr(lua_State *L, int index, const char *tname) {
	void *ptr;
	if (!lua_istable(L,index)) {
		luaL_typerror(L,index,"data pointer");
		return NULL;
	}
	lua_rawgeti(L,index,1);
	lua_pushstring(L,tname);
	if (!lua_rawequal(L,-1,-2)) {
		luaL_error(L,"bad data pointer #%d (expected data type '%s')",index,tname);
		return NULL;
	}
	lua_pop(L,2);
	lua_rawgeti(L,index,2);
	ptr = lua_touserdata(L,-1);
	lua_pop(L,1);
	return ptr;
}

void luaif_pushptr(lua_State *L, void *ptr, const char *tname) {
	lua_newtable(L);
	lua_pushstring(L,tname);
	lua_rawseti(L,-2,1);
	lua_pushlightuserdata(L,ptr);
	lua_rawseti(L,-2,2);
}

int luaif_assert(lua_State *L, int status) {
	const char *msg;
	if (status && !lua_isnil(L,-1)) {
		msg = lua_tostring(L,-1);
		lua_pop(L,1);
		if (!msg) msg = "(error object is not a string)";
		WARN("%s",msg);
	}
	return status;
}

int luaif_traceback(lua_State *L) {
	lua_getglobal(L,"debug");
	if (!lua_istable(L,-1)) {
		lua_pop(L,1);
		return 1;
	}
	lua_getfield(L,-1,"traceback");
	if (!lua_isfunction(L,-1)) {
		lua_pop(L,2);
		return 1;
	}
	lua_insert(L,-3);
	lua_pop(L,1);
	lua_pushinteger(L,2);	// traceback level
	lua_call(L,2,1);	// call debug.traceback()
	return 1;
}

int luaif_docall(lua_State *L, int nargs) {
	int base, status;
	base = lua_gettop(L) - nargs;
	lua_pushcfunction(L,luaif_traceback);
	lua_insert(L,base);
	status = lua_pcall(L,nargs,LUA_MULTRET,base);
	lua_remove(L,base);
	if (status != 0) lua_gc(L,LUA_GCCOLLECT,0);
	return luaif_assert(L,status);
}

int luaif_dofile(lua_State *L, const char *name) {
	return luaif_assert(L,(luaL_loadfile(L,name))) || luaif_docall(L,0);
}

void luaif_registerFunctions(lua_State *L, const luaL_Reg *l) {
	for ( ; l->name && l->func; l++) lua_register(L,l->name,l->func);
}

void luaif_dumpStack(lua_State *L) {
	int i, t;
	int top = lua_gettop(L);
	printf(">>> ");
	for (i = 1; i <= top; i++) {  /* repeat for each level */
		t = lua_type(L, i);
		switch (t) {
			case LUA_TSTRING:  /* strings */
				printf("\"%s\"", lua_tostring(L, i));
				break;
			case LUA_TBOOLEAN:  /* booleans */
				printf(lua_toboolean(L, i) ? "true" : "false");
				break;
			case LUA_TNUMBER:  /* numbers */
				printf("%g", lua_tonumber(L, i));
				break;
			default:  /* other values */
				printf("%s", lua_typename(L, t));
				break;
		}
		printf("  ");  /* put a separator */
	}
	printf("<<<\n");  /* end the listing */
}


// ========================================= interface functions ========================================= //

// WARN(str)
int luaif_WARN(lua_State *L) {
	const char *str = luaL_checkstring(L,1);
	WARN("LUA: %s",str);
	return 1;
}

// DEBUG(str)
int luaif_DEBUG(lua_State *L) {
	const char *str = luaL_checkstring(L,1);
	DEBUG("LUA: %s",str);
	return 1;
}

// FIGHT(fightPtr)
LUAIF_WRAPPER_STRUCT(FIGHT, fs_fight_t);
LUAIF_WRAPPER_ADDFIELD(id,         integer); \
LUAIF_WRAPPER_ADDFIELD(timeout,    integer); \
LUAIF_WRAPPER_STRUCT_END;

// PERS_LIST([teamNum [, alive [, except]]])
int luaif_PERS_LIST(lua_State *L) {
	fs_fight_t   *fight;
	fs_pers_t    *data, *pers = 0;
	viter_t      vi;
	int          idx = 1;
	int          teamNum = !lua_isnoneornil(L,1) ? luaL_checkint(L,1) : -1;
	int          alive = !lua_isnoneornil(L,2) ? lua_toboolean(L,2) : -1;
	bool         except = lua_toboolean(L,3);

	lua_getfield(L,LUA_REGISTRYINDEX,"fight");
	fight = luaif_checkptr(L,-1,"fs_fight_t");
	if (!fight) return 0;
	if (except) {
		lua_getfield(L,LUA_REGISTRYINDEX,"pers");
		pers = luaif_checkptr(L,-1,"fs_pers_t");
		if (!pers) return 0;
	}	
	LUAIF_WRAPPER_NEWTABLE;
	fs_fightLockMutex(fight);
	v_reset(fight->persVec,&vi);
	while ((data = v_each(fight->persVec,&vi))) {
		if ((teamNum != -1) && (data->teamNum != teamNum)) continue;
		if ((alive != -1) && ((data->status != FS_PS_DEAD) != alive)) continue;
		if (except && (data == pers)) continue;
		LUAIF_WRAPPER_NEXTPTR(fs_pers_t);
		idx++;
	}
	fs_fightUnlockMutex(fight);
	return 1;
}

// PERS_COUNT([teamNum [, alive]])
int luaif_PERS_COUNT(lua_State *L) {
	fs_fight_t   *fight;
	fs_pers_t    *data;
	viter_t      vi;
	int          size = 0;
	int          teamNum = !lua_isnoneornil(L,1) ? luaL_checkint(L,1) : -1;
	int          alive = !lua_isnoneornil(L,2) ? lua_toboolean(L,2) : -1;

	lua_getfield(L,LUA_REGISTRYINDEX,"fight");
	fight = luaif_checkptr(L,-1,"fs_fight_t");
	if (!fight) return 0;
	if ((teamNum == -1) && (alive == -1)) {	// quick vector size
		size = v_size(fight->persVec);
	} else {
		fs_fightLockMutex(fight);
		v_reset(fight->persVec,&vi);
		while ((data = v_each(fight->persVec,&vi))) {
			if ((teamNum != -1) && (data->teamNum != teamNum)) continue;
			if ((alive != -1) && ((data->status != FS_PS_DEAD) != alive)) continue;
			size++;
		}
		fs_fightUnlockMutex(fight);
	}
	lua_pushinteger(L,size);
	return 1;
}

// PERS(persPtr)
LUAIF_WRAPPER_STRUCT(PERS, fs_pers_t);
LUAIF_WRAPPER_ADDFIELD(id,         integer);
LUAIF_WRAPPER_ADDFIELD(status,     integer);
LUAIF_WRAPPER_ADDFIELD(level,      integer);
LUAIF_WRAPPER_ADDFIELD(gender,     integer);
LUAIF_WRAPPER_ADDFIELD(kind,       integer);
LUAIF_WRAPPER_ADDFIELD(partMask,   integer);
LUAIF_WRAPPER_ADDFIELD(artId,      integer);
LUAIF_WRAPPER_ADDFIELD(art,        boolean);
LUAIF_WRAPPER_ADDFIELD(teamNum,    integer);
LUAIF_WRAPPER_ADDEXPR(hp, PERS_HP(data), integer);
LUAIF_WRAPPER_ADDEXPR(hpMax, PERS_HPMAX(data), integer);
LUAIF_WRAPPER_ADDEXPR(mp, PERS_MP(data), integer);
LUAIF_WRAPPER_ADDEXPR(mpMax, PERS_MPMAX(data), integer);
LUAIF_WRAPPER_ADDFIELD(mtime,      integer);
LUAIF_WRAPPER_ADDFIELD(atime,      integer);
LUAIF_WRAPPER_ADDFIELD(eutime,     integer);
LUAIF_WRAPPER_STRUCT_END;

// PERS_ID(persPtr)
LUAIF_WRAPPER(PERS_ID, fs_pers_t);
LUAIF_WRAPPER_PUSHFIELD(id, integer);
LUAIF_WRAPPER_END;

// PERS_STATUS(persPtr)
LUAIF_WRAPPER(PERS_STATUS, fs_pers_t);
LUAIF_WRAPPER_PUSHFIELD(status, integer);
LUAIF_WRAPPER_END;

// PERS_LEVEL(persPtr)
LUAIF_WRAPPER(PERS_LEVEL, fs_pers_t);
LUAIF_WRAPPER_PUSHFIELD(level, integer);
LUAIF_WRAPPER_END;

// PERS_HP(persPtr)
LUAIF_WRAPPER(PERS_HP, fs_pers_t);
LUAIF_WRAPPER_PUSHEXPR(PERS_HP(data), integer);
LUAIF_WRAPPER_END;

// PERS_HPMAX(persPtr)
LUAIF_WRAPPER(PERS_HPMAX, fs_pers_t);
LUAIF_WRAPPER_PUSHEXPR(PERS_HPMAX(data), integer);
LUAIF_WRAPPER_END;

// PERS_ISBOT(persPtr)
LUAIF_WRAPPER(PERS_ISBOT, fs_pers_t);
LUAIF_WRAPPER_PUSHEXPR((data->flags & FS_PF_BOT), boolean);
LUAIF_WRAPPER_END;

// PERS_ISDEFENDED(persPtr)
LUAIF_WRAPPER(PERS_ISDEFENDED, fs_pers_t);
LUAIF_WRAPPER_PUSHEXPR((data->flags & FS_PF_DEFENDED), boolean);
LUAIF_WRAPPER_END;

// PERS_ISSTUNNED(persPtr)
LUAIF_WRAPPER(PERS_ISSTUNNED, fs_pers_t);
LUAIF_WRAPPER_PUSHEXPR((data->flags & FS_PF_STUNNED), boolean);
LUAIF_WRAPPER_END;

// OPPONENT(persPtr)
int luaif_OPPONENT(lua_State *L) {
	fs_pers_t *pers = luaif_checkptr(L,1,"fs_pers_t");
	if (!pers) return 0;
	if (PERS_OPP_ID(pers)) luaif_pushptr(L,pers->opponent,"fs_pers_t");
	else lua_pushnil(L);
	return 1;
}

// EFF_LIST(persPtr)
LUAIF_WRAPPER_STRUCTVECTOR(EFF_LIST, fs_pers_t, effVec, fs_persEff_t);
if (!data->id) continue;	// skipping unnamed effects
LUAIF_WRAPPER_NEXTPTR(fs_persEff_t);
LUAIF_WRAPPER_STRUCTVECTOR_END;

// EFF(effPtr)
LUAIF_WRAPPER_STRUCT(EFF, fs_persEff_t);
LUAIF_WRAPPER_ADDFIELD(id,         integer);
LUAIF_WRAPPER_ADDFIELD(cnt,        integer);
LUAIF_WRAPPER_ADDFIELD(artId,      integer);
LUAIF_WRAPPER_ADDFIELD(grpId,      integer);
LUAIF_WRAPPER_ADDEXPR(active, (data->flags & FS_PEF_ACTIVE), boolean);
LUAIF_WRAPPER_ADDEXPR(activatorId, (data->activator ? data->activator->id : 0), integer);
LUAIF_WRAPPER_STRUCT_END;

// EFF_ISACTIVE(effPtr)
LUAIF_WRAPPER(EFF_ISACTIVE, fs_persEff_t);
LUAIF_WRAPPER_PUSHEXPR((data->flags & FS_PEF_ACTIVE), boolean);
LUAIF_WRAPPER_END;

// EFF_ISUSABLE(effPtr)
LUAIF_WRAPPER(EFF_ISUSABLE, fs_persEff_t);
LUAIF_WRAPPER_PUSHEXPR((!(data->flags & FS_PEF_ACTIVE) && (data->cnt > 0)), boolean);
LUAIF_WRAPPER_END;

// ATTACK(part)
int luaif_ATTACK(lua_State *L) {
	fs_pers_t    *pers;
	int          part = luaL_checkinteger(L,1);
	errno_t      status;

	lua_getfield(L,LUA_REGISTRYINDEX,"pers");
	pers = luaif_checkptr(L,-1,"fs_pers_t");
	if (!pers) return 0;
	status = fs_persAttack(pers,part,false);
	lua_pushinteger(L,status);
	return 1;
}

// USE_EFFECT(effPtr[, persPtr])
int luaif_USE_EFFECT(lua_State *L) {
	fs_pers_t    *pers;
	fs_persEff_t *eff = luaif_checkptr(L,1,"fs_persEff_t");
	fs_pers_t    *target = !lua_isnoneornil(L,2) ? luaif_checkptr(L,2,"fs_pers_t") : NULL;
	errno_t      status;
	int          usageStatus, i;

	lua_getfield(L,LUA_REGISTRYINDEX,"pers");
	pers = luaif_checkptr(L,-1,"fs_pers_t");
	if (!pers || !eff) return 0;
	if (!target) target = pers;
	usageStatus = 0;
	i = eff->cdTime; eff->cdTime = 0;	// assume zero cooldown
	status = fs_persUseEffect(pers,eff,target,&usageStatus);
	eff->cdTime = i;
	lua_pushinteger(L,status);
	lua_pushinteger(L,usageStatus);
	return 2;
}

// DROP_EFFECT(effPtr)
int luaif_DROP_EFFECT(lua_State *L) {
	fs_persEff_t *eff = luaif_checkptr(L,1,"fs_persEff_t");
	if (!eff || !(eff->flags & FS_PEF_ACTIVE)) return 0;
	eff->flags |= FS_PEF_DROP;
	return 0;
}

// TOGGLE_DEFENCE([toggle])
int luaif_TOGGLE_DEFENCE(lua_State *L) {
	fs_pers_t    *pers;
	int          toggle = !lua_isnoneornil(L,1) ? lua_toboolean(L,1) : -1;
	bool         status;

	lua_getfield(L,LUA_REGISTRYINDEX,"pers");
	pers = luaif_checkptr(L,-1,"fs_pers_t");
	if (!pers) return 0;
	status = (pers->flags & FS_PF_DEFENDED) > 0;
	if (toggle != -1) {
		if (toggle) pers->flags |= FS_PF_DEFENDED;
		else pers->flags &= ~FS_PF_DEFENDED;
	}
	lua_pushboolean(L,status);
	return 1;
}

luaL_Reg luaif_funcList[] = {
	{"WARN",               luaif_WARN},
	{"DEBUG",              luaif_DEBUG},
	{"FIGHT",              luaif_FIGHT},
	{"PERS_LIST",          luaif_PERS_LIST},
	{"PERS_COUNT",         luaif_PERS_COUNT},
	{"PERS",               luaif_PERS},
	{"PERS_ID",            luaif_PERS_ID},
	{"PERS_STATUS",        luaif_PERS_STATUS},
	{"PERS_LEVEL",         luaif_PERS_LEVEL},
	{"PERS_HP",            luaif_PERS_HP},
	{"PERS_HPMAX",         luaif_PERS_HPMAX},
	{"PERS_ISBOT",         luaif_PERS_ISBOT},
	{"PERS_ISDEFENDED",    luaif_PERS_ISDEFENDED},
	{"OPPONENT",           luaif_OPPONENT},
	{"EFF_LIST",           luaif_EFF_LIST},
	{"EFF",                luaif_EFF},
	{"EFF_ISACTIVE",       luaif_EFF_ISACTIVE},
	{"EFF_ISUSABLE",       luaif_EFF_ISUSABLE},
	{"ATTACK",             luaif_ATTACK},
	{"USE_EFFECT",         luaif_USE_EFFECT},
	{"DROP_EFFECT",        luaif_DROP_EFFECT},
	{"TOGGLE_DEFENCE",     luaif_TOGGLE_DEFENCE},
	{"PERS_ISSTUNNED",	   luaif_PERS_ISSTUNNED},
	{NULL, NULL}
};


// ========================================= initialization functions ========================================= //

errno_t luaif_init(fs_fight_t *fight) {
	char    *initFile = "lua/init.lua";

	if (!fight) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	if (fight->L) return ERR_WRONG_STATE;
	fight->L = luaL_newstate();
	if (!fight->L) {
		WARN("Can't create state object");
		return ERR_INIT;
	}
	luaL_openlibs(fight->L);
	luaif_registerFunctions(fight->L,luaif_funcList);
	tolua_luaif_tolua_open(fight->L);
	if (luaif_dofile(fight->L,initFile) != 0) {
		WARN("Error loading script file: %s",initFile);
		return ERR_INIT;
	}
	return OK;
}

errno_t luaif_done(fs_fight_t *fight) {
	if (!fight) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	if (!fight->L) return ERR_WRONG_STATE;
	lua_close(fight->L);
	return OK;
}
