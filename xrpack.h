/*
**  LICENSE: BSD
**  Author: CandyMi[https://github.com/candymi]
*/

#define LUA_LIB

// #include <xrio.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#ifndef xrio_malloc
  #define xrio_malloc malloc
#endif

#ifndef xrio_free
  #define xrio_free free
#endif

/* 基础类型 */
enum xrio_pack_t {
  /* pack and unpack */
  XRTYPE_NIL        =  11,
  XRTYPE_BOOLEAN    =  12,
  XRTYPE_INTEGER    =  13,
  XRTYPE_NUMBER     =  14,
  XRTYPE_STRING     =  15,
  XRTYPE_USERDATA   =  16,
  XRTYPE_TABLE      =  17,

  /* encode and decode */
  XRTYPE_INT8    =   97,
  XRTYPE_INT16   =   98,
  XRTYPE_INT32   =   99,
  XRTYPE_INT64   =  100,

  XRTYPE_SSTRING =  101,
  XRTYPE_LSTRING =  102,
};

/* 消息结构 */
struct xrio_msg {
  uint32_t type;
  uint32_t len;
  union {
    lua_Integer i;
    lua_Number n;
    void* data;
  };
  struct xrio_msg *next;
};

int packer(lua_State *L);
int unpacker(lua_State *L);

int encoder(lua_State *L);
int decoder(lua_State *L);