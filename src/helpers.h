#ifndef LUASHARED_QUEUE_HELPERS_H
#define LUASHARED_QUEUE_HELPERS_H

#include "lauxlib.h"

#include<string.h>
#include <stdlib.h>
#include <stdarg.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define MAX_ERR_LENGTH 200

int luaL_optboolean(lua_State *L, int narg, int def);

LUALIB_API void luaL_setfuncs (lua_State *L, const luaL_Reg *l, int nup);
LUALIB_API void setmeta(lua_State *L, const char *name);
LUALIB_API int createmeta(lua_State *L, const char *name, const luaL_Reg *methods);
LUALIB_API void die(lua_State *L, const char *fmt, ...);
LUALIB_API void throw_error(lua_State *L, const char* error_msg); 

#endif
