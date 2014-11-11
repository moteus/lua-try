/*=========================================================================*\
* Simple exception support
* LuaSocket toolkit
\*=========================================================================*/
#include <stdio.h>

#include "lua.h"
#include "lauxlib.h"

/*export*/
#ifdef _WIN32
#  define EXPORT_API __declspec(dllexport)
#else
#  define EXPORT_API LUALIB_API
#endif

static const char *TRY_ERROR_NIL = "NIL";

/*=========================================================================*\
* Internal function prototypes.
\*=========================================================================*/
static int try_new_assert(lua_State *L);
static int try_new_protect(lua_State *L);

/* except functions */
static luaL_Reg func[] = {
    {"new",       try_new_assert},
    {"protect",   try_new_protect},

    /* compat with LuaSocket */
    {"newtry",    try_new_assert},
    {NULL,        NULL}
};

#if LUA_VERSION_NUM < 502

#define lua_absindex(L, i) (((i)>0)?(i):((i)<=LUA_REGISTRYINDEX?(i):(lua_gettop(L)+(i)+1)))

void lua_rawgetp(lua_State *L, int index, const void *p){
  index = lua_absindex(L, index);
  lua_pushlightuserdata(L, (void *)p);
  lua_rawget(L, index);
}

void lua_rawsetp (lua_State *L, int index, const void *p){
  index = lua_absindex(L, index);
  lua_pushlightuserdata(L, (void *)p);
  lua_insert(L, -2);
  lua_rawset(L, index);
}

#endif

/*-------------------------------------------------------------------------*\
* Error wrapper
\*-------------------------------------------------------------------------*/
static int try_check_error(lua_State *L) {
  if(lua_istable(L, -1)){
    lua_rawgetp(L, -1, TRY_ERROR_NIL);

    if(lua_isnil(L, -1)){ /*not try error*/
      lua_pop(L, 1);
      return 0;
    }

    if(lua_touserdata(L, -1) == TRY_ERROR_NIL){ /*no value*/
      lua_pop(L, 1);
      lua_pushnil(L);
    }

    lua_pushnil(L);
    lua_insert(L, -2);
    return 1;
  }

  return 0;
}

static int try_error(lua_State *L){
  lua_newtable(L);
  if(lua_isnil(L, -2)){
    lua_pushlightuserdata(L, (void*)TRY_ERROR_NIL);
  }
  else{
    lua_pushvalue(L, -2);
  }
  lua_rawsetp(L, -2, TRY_ERROR_NIL);
  return lua_error(L);
}

/*-------------------------------------------------------------------------*\
* Try factory
\*-------------------------------------------------------------------------*/

static int try_assert(lua_State *L) {
  if(!lua_toboolean(L, 1)){
    /* call finalizer */
    if(!lua_isnil(L, lua_upvalueindex(1))){
      lua_pushvalue(L, lua_upvalueindex(1));
      lua_pcall(L, 0, 0, 0);
    }
    lua_settop(L, 2);

    return try_error(L);
  }
  return lua_gettop(L);
}

static int try_new_assert(lua_State *L) {
  lua_settop(L, 1);
  lua_pushcclosure(L, try_assert, 1);
  return 1;
}

/*-------------------------------------------------------------------------*\
* Protect factory
\*-------------------------------------------------------------------------*/

#if LUA_VERSION_NUM < 503

typedef int lua_KContext;

#endif

#if LUA_VERSION_NUM <= 501
#ifndef LUA_OK
#define LUA_OK 0
#endif
#endif

#if LUA_VERSION_NUM <= 501
# define PCALLK(L, N, M, H, C, K) lua_pcall(L, N, M, H)
#else
# define PCALLK(L, N, M, H, C, K) lua_pcallk(L, N, M, H, C, K)
#endif

#if LUA_VERSION_NUM <= 501
# define try_protected_cont 0
#elif LUA_VERSION_NUM == 502
# define try_protected_cont try_protected
#else
# define try_protected_cont try_protected_k
#endif

static int try_protected(lua_State *L);

static int try_protected_k(lua_State *L, int status, lua_KContext ctx){
  if(status == LUA_OK){
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_insert(L, 1);
    status = PCALLK(L, lua_gettop(L) - 1, LUA_MULTRET, 0, 0, try_protected_cont);
    if(status == LUA_OK) status = LUA_YIELD;
  }

  if(status == LUA_YIELD){
    return lua_gettop(L);
  }

  if(status == LUA_ERRRUN){
    if(try_check_error(L)) return 2;
  }

  return lua_error(L);
}

static int try_protected(lua_State *L){
  int status;
  lua_KContext ctx;

#if LUA_VERSION_NUM == 502
  status = lua_getctx(L, &ctx);
#else
  status = LUA_OK;
  ctx = 0;
#endif

  return try_protected_k(L, status, ctx);
}

static int try_new_protect(lua_State *L){
  lua_pushcclosure(L, try_protected, 1);
  return 1;
}

/*-------------------------------------------------------------------------*\
* Init module
\*-------------------------------------------------------------------------*/
EXPORT_API int luaopen_try(lua_State *L) {
  lua_newtable(L);
#if LUA_VERSION_NUM > 501 && !defined(LUA_COMPAT_MODULE)
  luaL_setfuncs(L, func, 0);
#else
  luaL_openlib(L, NULL, func, 0);
#endif

  lua_pushnil(L);
  lua_pushcclosure(L, try_assert, 1);
  lua_setfield(L, -2, "assert");

  lua_pushlightuserdata(L, (void*)TRY_ERROR_NIL);
  lua_setfield(L, -2, "NIL");

  return 1;
}
