#include "xrpack.h"

static inline int32_t packer_table(lua_State *L, int32_t idx, struct xrio_msg *msg);

static inline int32_t packer_set(lua_State *L, int32_t idx, struct xrio_msg *msg) {
  const char *text;
  switch (lua_type(L, idx))
  {
    case LUA_TNIL:
      msg->type = XRTYPE_NIL;
      break;
    case LUA_TLIGHTUSERDATA:
      msg->type = XRTYPE_USERDATA;
      msg->data = (void*)lua_touserdata(L, idx);
      break;
    case LUA_TBOOLEAN:
      msg->type = XRTYPE_BOOLEAN;
      msg->data = lua_toboolean(L, idx) ? (void*)1 : NULL;
      break;
    case LUA_TNUMBER:
      if (lua_isinteger(L, idx))
      {
        msg->type = XRTYPE_INTEGER;
        msg->i = lua_tointeger(L, idx);
      }
      else
      {
        msg->type = XRTYPE_NUMBER;
        msg->n = lua_tonumber(L, idx);
      }
      break;
    case LUA_TSTRING:
      msg->type = XRTYPE_STRING;
      text = lua_tolstring(L, idx, (size_t*)&msg->len);
      if (msg->len > 0) 
        msg->data = memmove(xrio_malloc(msg->len), text, msg->len);
      break;      
    case LUA_TTABLE:
      msg->type = XRTYPE_TABLE;
      packer_table(L, idx, msg);
      break;
    default:
      return luaL_error(L, "encode error: Invalid type '%s', (parameter index: %d).", lua_typename(L, lua_type(L, idx)), idx);
  }
  return 0;
}

static inline int32_t packer_table(lua_State *L, int32_t idx, struct xrio_msg *msg) {
  struct xrio_msg* parent = NULL;
  struct xrio_msg* k = NULL;
  struct xrio_msg* v = NULL;
  size_t top = lua_gettop(L);
  size_t kpos = top + 1;
  size_t vpos = top + 2;
  lua_pushnil(L);
  while (lua_next(L, idx))
  {
    if (!parent) {
      k = parent = xrio_malloc(sizeof(struct xrio_msg));
      v = parent->next = xrio_malloc(sizeof(struct xrio_msg));
    } else {
      k = v->next = xrio_malloc(sizeof(struct xrio_msg));
      v = k->next = xrio_malloc(sizeof(struct xrio_msg));
    }
    /* 初始化 */
    k->data = v->data = v->next = NULL;
    /* 不能用负索引 */
    packer_set(L, kpos, k);
    packer_set(L, vpos, v);
    /* 弹出栈 */
    lua_pop(L, 1);
  }
  // printf("%p, %p, %p\n", parent, k, v);
  msg->data = parent;
  return 0;
}

int packer(lua_State *L) {
  struct xrio_msg *parent = NULL;
  struct xrio_msg *msg = NULL;
  for (int i = 1; i <= lua_gettop(L); i++) {
    if (!msg)
      msg = parent = xrio_malloc(sizeof(struct xrio_msg));
    else
      msg = msg->next = xrio_malloc(sizeof(struct xrio_msg));
    msg->type = msg->len = 0;
    msg->data = msg->next = NULL;
    packer_set(L, i, msg);
  }
  lua_pushlightuserdata(L, parent);
  return 1;
}