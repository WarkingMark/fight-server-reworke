/* 
 * modifed: igorpauk 2017-18
 */

#ifndef __LUAIF_H__
#define __LUAIF_H__

#include "typedefs.h"


#define LUAIF_TABLE_ADDFIELD(L, FIELD, VALUE, PUSHTYPE) \
	lua_push##PUSHTYPE(L,VALUE); \
	lua_setfield(L,-2,FIELD);


extern int tolua_luaif_tolua_open(lua_State*);

void *luaif_checkptr(lua_State *L, int index, const char *tname);
void luaif_pushptr(lua_State *L, void *ptr, const char *tname);

int luaif_assert(lua_State *L, int status);
int luaif_traceback(lua_State *L);
int luaif_docall(lua_State *L, int nargs);
int luaif_dofile(lua_State *L, const char *name);
void luaif_registerFunctions(lua_State *L, const luaL_Reg *l);

errno_t luaif_init(fs_fight_t *fight);
errno_t luaif_done(fs_fight_t *fight);


#endif
