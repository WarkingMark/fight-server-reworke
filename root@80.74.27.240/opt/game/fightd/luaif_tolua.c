/*
** Lua binding: luaif_tolua
** Generated automatically by tolua++-1.0.92 on Thu Sep  6 14:33:14 2018.
*/

#ifndef __cplusplus
#include "stdlib.h"
#endif
#include "string.h"

#include "tolua++.h"

/* Exported function */
TOLUA_API int  tolua_luaif_tolua_open (lua_State* tolua_S);

#include "common.h"
#include "pers.h"

/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
}

/* Open function */
TOLUA_API int tolua_luaif_tolua_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 tolua_reg_types(tolua_S);
 tolua_module(tolua_S,NULL,0);
 tolua_beginmodule(tolua_S,NULL);
  tolua_constant(tolua_S,"FS_PS_CREATED",FS_PS_CREATED);
  tolua_constant(tolua_S,"FS_PS_FIGHTING",FS_PS_FIGHTING);
  tolua_constant(tolua_S,"FS_PS_DEAD",FS_PS_DEAD);
  tolua_constant(tolua_S,"FS_PS_PENDING",FS_PS_PENDING);
  tolua_constant(tolua_S,"FS_PS_ACTIVE",FS_PS_ACTIVE);
  tolua_constant(tolua_S,"FS_PS_PASSIVE",FS_PS_PASSIVE);
  tolua_constant(tolua_S,"FS_PLS_PENDING",FS_PLS_PENDING);
  tolua_constant(tolua_S,"FS_PLS_ACTIVE",FS_PLS_ACTIVE);
  tolua_constant(tolua_S,"FS_PLS_PASSIVE",FS_PLS_PASSIVE);
 tolua_endmodule(tolua_S);
 return 1;
}


#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501
 TOLUA_API int luaopen_luaif_tolua (lua_State* tolua_S) {
 return tolua_luaif_tolua_open(tolua_S);
};
#endif

