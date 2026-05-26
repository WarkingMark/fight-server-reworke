#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "../vec.h"


#define LOG(s) printf(">>> %s\n",(s));

int luaif_traceback(lua_State *L) {
	lua_getfield(L,LUA_GLOBALSINDEX,"debug");
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

int luaif_assert(lua_State *L, int status) {
	const char *msg;
	if (status && !lua_isnil(L,-1)) {
		msg = lua_tostring(L,-1);
    if (!msg) msg = "(error object is not a string)";
    LOG(msg);
    lua_pop(L,1);
  }
  return status;
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
	return luaif_assert(L,(luaL_loadfile(L,name) || luaif_docall(L,0)));
}

void luaif_registerGlobals(lua_State *L, const luaL_Reg *l) {
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

// -----------------------------------------------------------

#define LUAIF_WRAPPER_STRUCT(NAME, STRUCTTYPE) \
	int luaif_##NAME(lua_State *L) { \
		STRUCTTYPE *data; \
		if (!lua_isuserdata(L,1)) return luaL_error(L,"Invalid arguments"); \
		data = lua_touserdata(L,1); \
		if (!data) return luaL_error(L,"Wrong data"); \
		lua_settop(L,0); \
		lua_newtable(L);
#define LUAIF_WRAPPER_STRUCT_END \
		return 1; \
	}

#define LUAIF_WRAPPER_VECTOR(NAME, DATATYPE) \
	int luaif_##NAME(lua_State *L) { \
		vector_t   *vec; \
		DATATYPE   *data; \
		int        idx = 0; \
		if (!lua_isuserdata(L,1)) return luaL_error(L,"Invalid arguments"); \
		vec = lua_touserdata(L,1); \
		if (!vec) return luaL_error(L,"Wrong data"); \
		lua_settop(L,0); \
		lua_newtable(L); \
		v_reset(vec,&vi); \
		while ((data = v_each(vec,&vi))) {
#define LUAIF_WRAPPER_VECTOR_END \
		} \
		return 1; \
	}

#define LUAIF_WRAPPER_STRUCTVECTOR(NAME, STRUCTTYPE, FIELD, DATATYPE) \
	int luaif_##NAME(lua_State *L) { \
		STRUCTTYPE *rec; \
		DATATYPE   *data; \
		int        idx = 0; \
		if (!lua_isuserdata(L,1)) return luaL_error(L,"Invalid arguments"); \
		rec = lua_touserdata(L,1); \
		if (!rec) return luaL_error(L,"Wrong data"); \
		lua_settop(L,0); \
		lua_newtable(L); \
		v_reset(rec->FIELD,&vi); \
		while ((data = v_each(rec->FIELD,&vi))) {
#define LUAIF_WRAPPER_STRUCTVECTOR_END \
		} \
		return 1; \
	}

#define LUAIF_WRAPPER_ADDFIELD(FIELD, PUSHTYPE) \
		lua_push##PUSHTYPE(L,data->FIELD); \
		lua_setfield(L,1,#FIELD);

#define LUAIF_WRAPPER_ADDDATA \
		lua_pushinteger(L,++idx); \
		lua_pushlightuserdata(L,data); \
		lua_settable(L,1); \

#define LUAIF_REGISTER_GLOBAL_WRAPPER(NAME, PTR) \
	lua_pushlightuserdata(L,PTR); \
	lua_pushcclosure(L,luaif_##NAME,1); \
	lua_setglobal(L,NAME);



typedef struct test_struct {
	int x;
	int y;
} test_struct_t;

int test1(lua_State *L);
int test2(lua_State *L);
int test3(lua_State *L);

LUAIF_WRAPPER_STRUCT(TEST, test_struct_t);
	LUAIF_WRAPPER_ADDFIELD(x, integer);
	LUAIF_WRAPPER_ADDFIELD(y, integer);
	//LUAIF_WRAPPER_ADDDATA;
LUAIF_WRAPPER_STRUCT_END

luaL_Reg func_list[] = {
	{"test1", test1},
	{"test2", test2},
	{"TEST", luaif_TEST},
//	{"test3", test3},
	{NULL, NULL}
};

int test1(lua_State *L) {
	int i1, i2;
	i1 = luaL_checkint(L,1);
	i2 = luaL_checkint(L,2);
	lua_pushnumber(L,i1+i2);
	return 1;
}

int test2(lua_State *L) {
	test_struct_t *ud;
	if (!lua_islightuserdata(L,1)) return luaL_error(L,"Invalid arguments");
	ud = lua_touserdata(L, 1);
	if (!ud) return luaL_error(L,"Wrong data");
	lua_pushnumber(L, ud->x + ud->y);
	return 1;
}

int test3(lua_State *L) {
	int i1;
	test_struct_t *ud;
	i1 = lua_upvalueindex(1);
//	i1 = 1;
	if (!lua_islightuserdata(L,i1)) return luaL_error(L,"Invalid arguments");
	ud = lua_touserdata(L,i1);
	if (!ud) return luaL_error(L,"Wrong data");
	lua_pushnumber(L, ud->x + ud->y);
	return 1;
}

// -----------------------------------------------------------------


int main() {
	lua_State *L;

	test_struct_t test_struct = { 2, 3 }, test_struct2 = { 20, 30 };
	int ddd[3] = { 11, 22, 33 };
	vector_t  v;
	v_init(&v);
	v_push(&v,ddd);
	v_push(&v,ddd+1);
	v_push(&v,ddd+2);

	LOG("*** START ***");
	L = luaL_newstate();
	luaL_openlibs(L);
	luaif_registerGlobals(L,func_list);

// C closure
	lua_pushlightuserdata(L,&test_struct2);
	lua_pushcclosure(L,test3,1);
	lua_setglobal(L,"test3");
// ---------

	luaif_dofile(L,"luatest.lua");
	lua_getglobal(L,"main");
	lua_pushlightuserdata(L,&test_struct);
	lua_pushlightuserdata(L,&v);
	printf("call=%d\n",luaif_docall(L,2));
	luaif_dumpStack(L);
	printf("stack=%d\n",lua_gettop(L));
	lua_close(L);
	LOG("*** DONE ***");
	v_zero(&v);
	return 0;
}
