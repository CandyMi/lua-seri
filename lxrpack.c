#include "xrpack.h"

/* 内存数据打包 */
int lxrio_pack(lua_State *L){
  int top = lua_gettop(L);
  if (top > 0)
    return packer(L);
  lua_pushlightuserdata(L, NULL);
  return 1;
}

/* 内存数据解包 */
int lxrio_unpack(lua_State *L){
  if (lua_type(L, 1) != LUA_TLIGHTUSERDATA)
    return luaL_error(L, "Invalid `xrunpack` arguments. (%d, %s)", lua_type(L, 1), lua_typename(L, lua_type(L, 1)));
  struct xrio_lua_msg* msg = lua_touserdata(L, 1);
  if (!msg)
    return 0;
  lua_settop(L, 1);
  return unpacker(L);
}

/* 网络数据打包 */
int lxrio_encode(lua_State *L) {
  return encoder(L);
}

/* 网络数据解包 */
int lxrio_decode(lua_State *L) {
  return decoder(L);
}

LUAMOD_API int luaopen_lxrpack(lua_State *L) {
  luaL_checkversion(L);
  luaL_Reg lpack_libs[] = {
    { "pack",   lxrio_pack   },
    { "unpack", lxrio_unpack },
    { "encode", lxrio_encode },
    { "decode", lxrio_decode },
    { NULL, NULL },
  };
  luaL_newlib(L, lpack_libs);
  return 1;
}
