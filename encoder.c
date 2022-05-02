#include "xrpack.h"

#define xrio_write_len(buffer, len) do { \
  buffer[0] = len & 0xFF;                \
  buffer[1] = len >> 8  & 0xFF;          \
  buffer[2] = len >> 16 & 0xFF;          \
  buffer[3] = len >> 24 & 0xFF;          \
} while (0)

/* 写入`Nil` */
static inline int write_nil(luaL_Buffer *B) {
  luaL_addchar(B, XRTYPE_NIL);
  return 1;
}

/* 写入`NULL` */
static inline int write_null(luaL_Buffer *B) {
  luaL_addchar(B, XRTYPE_USERDATA);
  return 1;
}

/* 写入`Boolean` */
static inline int write_boolean(luaL_Buffer *B, int boolean) {
  char buffer[] = {XRTYPE_BOOLEAN, boolean ? 1 : 0};
  luaL_addlstring(B, buffer, 2);
  return 2;
}

/* 写入`Integer` */
static inline int write_integer(lua_State *L, luaL_Buffer *B, size_t idx) {
  lua_Integer i = lua_tointeger(L, idx);
  if (i >=0 && i <= 255)
  {
    luaL_addchar(B, XRTYPE_INT8);
    luaL_addchar(B, i);
    return 2;
  }
  else if (i >= 0 && i <= 65535)
  {
    union { uint16_t i; char buffer[4]; } v;
    v.i = i;
    luaL_addchar(B, XRTYPE_INT16);
    luaL_addlstring(B, v.buffer, 2);
    return 3;
  }
  else if (i >= 0 && i <= 4294967295)
  {
    union { uint32_t i; char buffer[4]; } v;
    v.i = i;
    luaL_addchar(B, XRTYPE_INT32);
    luaL_addlstring(B, v.buffer, 4);
    return 5;
  }
  else
  {
    union { int64_t i; char buffer[8]; } v;
    v.i = i;
    luaL_addchar(B, XRTYPE_INT64);
    luaL_addlstring(B, v.buffer, 8);
    return 9;
  }
}

/* 写入`Number` */
static inline int write_number(lua_State *L, luaL_Buffer *B, size_t idx) {
  union { lua_Number n; char buffer[8]; } v;
  v.n = lua_tonumber(L, idx);
  luaL_addchar(B, XRTYPE_NUMBER);
  luaL_addlstring(B, v.buffer, 8);
  return 9;
}

/* 写入`CString` */
static inline int write_cstring(lua_State *L, luaL_Buffer *B, size_t idx) {
  size_t csize;
  const char* cstring = lua_tolstring(L, idx, &csize);
  /* 短字符串用`1`个字节长度表示 */
  if (csize <= 255) {
    luaL_addchar(B, XRTYPE_SSTRING);
    luaL_addchar(B, csize);
    if (csize > 0)
      luaL_addlstring(B, cstring, csize);
    return 2 + csize;
  }
  /* 长字符串用`4`字节表示 */
  luaL_addchar(B, XRTYPE_LSTRING);
  char buffsize[4];
  xrio_write_len(buffsize, csize);
  luaL_addlstring(B, buffsize, 4);
  luaL_addlstring(B, cstring, csize);
  return 5 + csize;
}

static inline int encoder_write(lua_State *L, int idx, luaL_Buffer *B);
/* 写入`Table` */
static inline int write_table(lua_State *L, luaL_Buffer *B, size_t idx) {
  size_t tsize = 2;
  int kidx =  lua_gettop(L) + 1;
  int vidx =  lua_gettop(L) + 2;
  luaL_addchar(B, XRTYPE_TABLE);
  lua_pushnil(L);
  while (lua_next(L, idx))
  {
    /* 计算表编码后的总长度 */
    tsize += encoder_write(L, kidx, B) + encoder_write(L, vidx, B);
    lua_pop(L, 1);
  }
  luaL_addchar(B, 0x00);
  return tsize;
}

static inline int encoder_write(lua_State *L, int idx, luaL_Buffer *B) {
  switch (lua_type(L, idx)) {
    case LUA_TNIL:
      return write_nil(B);
    case LUA_TUSERDATA:
    case LUA_TLIGHTUSERDATA:
      return write_null(B);
    case LUA_TBOOLEAN:
      return write_boolean(B, lua_toboolean(L, idx));
    case LUA_TNUMBER:
      if (lua_isinteger(L, idx))
        return write_integer(L, B, idx);
      else
        return write_number(L, B, idx);
    case LUA_TSTRING:
      return write_cstring(L, B, idx);
    case LUA_TTABLE:
      return write_table(L, B, idx);
    default:
      return luaL_error(L, "encode error: Invalid type '%s', (parameter index: %d).", lua_typename(L, lua_type(L, idx)), idx);
  }
}

int encoder(lua_State *L) {
  int top = lua_gettop(L);
  if (top == 0) {
    lua_pushlstring(L, "\x00\x00\x00\x00", 4);
    return 1;
  }
  size_t bsize = 0;
  luaL_Buffer B;
  char* hsize = luaL_buffinitsize(L, &B, 4);
  luaL_addlstring(&B, "\x00\x00\x00\x00", 4);
  for (int i = 1; i <= top; i++)
    bsize += encoder_write(L, i, &B);
  /* 计算长度 */
  xrio_write_len(hsize, bsize);
  luaL_pushresult(&B);
  return 1;
}