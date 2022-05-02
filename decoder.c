#include "xrpack.h"

#define xrio_invalid_check(L, ilen, len) if (ilen > len) return luaL_error(L, "buffer was Invalid.");

/* 读取`Nil` */
static inline int read_nil(lua_State *L) {
  lua_pushnil(L);
  return 1;
}

/* 读取`Null` */
static inline int read_null(lua_State *L) {
  lua_pushlightuserdata(L, NULL);
  return 1;
}

/* 读取`Boolean` */
static inline int read_boolean(lua_State *L, const char *buffer, size_t len) {
  xrio_invalid_check(L, 1, len);
  lua_pushboolean(L, *buffer ? 1 : 0);
  return 2;
}

/* 读取`Integer` */
static inline int read_int8(lua_State *L, const char *buffer, size_t len) {
  xrio_invalid_check(L, 1, len);
  lua_pushinteger(L, (uint8_t)buffer[0]);
  return 2;
}

static inline int read_int16(lua_State *L, const char *buffer, size_t len) {
  xrio_invalid_check(L, 2, len);
  lua_pushinteger(L, (uint16_t)buffer[0] | (uint16_t)buffer[1] << 8);
  return 3;
}

static inline int read_int32(lua_State *L, const char *buffer, size_t len) {
  xrio_invalid_check(L, 4, len);
  union { uint32_t i; char buffer[4]; } v;
  memmove(v.buffer, buffer, 4);
  lua_pushinteger(L, v.i);
  return 5;  
}

static inline int read_int64(lua_State *L, const char *buffer, size_t len) {
  xrio_invalid_check(L, 8, len);
  union { int64_t i; char buffer[8]; } v;
  memmove(v.buffer, buffer, 8);
  lua_pushinteger(L, v.i);
  return 9;
}

/* 读取`Number` */
static inline int read_number(lua_State *L, const char *buffer, size_t len) {
  xrio_invalid_check(L, 8, len);
  union { lua_Number n; char buffer[8]; } v;
  memmove(v.buffer, buffer, 8);
  lua_pushnumber(L, v.n);
  return 9;
}

/* 读取`CString` */
static inline int read_sstring(lua_State *L, const char *buffer, size_t len){
  uint8_t csize = 0;
  xrio_invalid_check(L, 1, len);
  csize = (uint8_t)buffer[0];
  if (csize)
    xrio_invalid_check(L, 1 + csize, len);
  lua_pushlstring(L, buffer + 1, csize);
  return 2 + csize;
}

static inline int read_lstring(lua_State *L, const char *buffer, size_t len){
  uint32_t csize = 0;
  xrio_invalid_check(L, 4, len);
  csize = buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0];
  if (csize)
    xrio_invalid_check(L, csize + 4, len);
  lua_pushlstring(L, buffer + 4, csize);
  return 5 + csize;
}

static inline int decoder_read(lua_State *L, const char *buffer, size_t *pos, size_t len);
/* 读取`Table` */
static inline int read_table(lua_State *L, const char *buffer, size_t len) {
  int offset = 0;
  int bsize = len;
  size_t position = 0;
  lua_newtable(L);
  int top = lua_gettop(L);
  while (bsize)
  {
    offset = decoder_read(L, buffer, &position, bsize);
    if (offset == -1)
      break;
    /* 检查堆栈字段 */
    if (top + 2 == lua_gettop(L))
      lua_rawset(L, top);
    /* 计算偏移值 */
    bsize -= offset; position += offset;
  }
  return position;
}

static inline int decoder_read(lua_State *L, const char *buffer, size_t *pos, size_t len) {
  size_t offset = 0;
  switch (buffer[*pos])
  {
    case XRTYPE_NIL:
      offset = read_nil(L);
      break;
    case XRTYPE_USERDATA:
      offset = read_null(L);
      break;
    case XRTYPE_BOOLEAN:
      offset = read_boolean(L, buffer + *pos + 1, len - 1);
      break;
    case XRTYPE_INT8:
      offset = read_int8(L, buffer + *pos + 1, len - 1);
      break;
    case XRTYPE_INT16:
      offset = read_int16(L, buffer + *pos + 1, len - 1);
      break;
    case XRTYPE_INT32:
      offset = read_int32(L, buffer + *pos + 1, len - 1);
      break;
    case XRTYPE_INT64:
      offset = read_int64(L, buffer + *pos + 1, len - 1);
      break;
    case XRTYPE_NUMBER:
      offset = read_number(L, buffer + *pos + 1, len - 1);
      break;
    case XRTYPE_SSTRING:
      offset = read_sstring(L, buffer + *pos + 1, len - 1);
      break;
    case XRTYPE_LSTRING:
      offset = read_lstring(L, buffer + *pos + 1, len - 1);
      break;
    case XRTYPE_TABLE:
      offset = read_table(L, buffer + *pos + 1, len);
      offset += 2;
      break;
    case 0x00:
      offset = -1;
      break;
    default:
      return luaL_error(L, "decode error: Invalid type. (%d, %d)", *pos, len);
  }
  return offset;
}

int decoder(lua_State *L) {
  size_t bsize;
  const char * buffer = luaL_checklstring(L, 1, &bsize);
  if (!buffer || bsize < 4)
    return luaL_error(L, "Invalid decoder buffer.");

  size_t nret = 0;
  size_t position = 0;
  size_t offset = 0;
  int blen = bsize - 4;
  buffer = buffer + 4;

  while (blen)
  {
    offset = decoder_read(L, buffer, &position, blen);
    blen -= offset; position += offset;
    nret++;
  }
  return nret;
}