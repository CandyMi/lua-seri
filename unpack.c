#include "xrpack.h"

static inline int unpacker_table(lua_State *L, struct xrio_msg *msg);

static inline int unpacker_set(lua_State *L, struct xrio_msg *msg) {
  switch(msg->type)
  {
    case XRTYPE_NIL:
      lua_pushnil(L);
      break;
    case XRTYPE_USERDATA:
      lua_pushlightuserdata(L, msg->data);
      break;
    case XRTYPE_BOOLEAN:
      lua_pushboolean(L, msg->data ? 1 : 0);
      break;
    case XRTYPE_NUMBER :
      lua_pushnumber(L, msg->n);
      break;
    case XRTYPE_INTEGER :
      lua_pushinteger(L, msg->i);
      break;
    case XRTYPE_STRING :
      lua_pushlstring(L, msg->data, msg->len);
      if (msg->len > 0)
        xrio_free(msg->data);
      break;
    case XRTYPE_TABLE :
      lua_newtable(L);
      if (msg->data)
        unpacker_table(L, msg->data);
      break;
    default:
      return luaL_error(L, "decode error: Invalid type.");
  }
  return 0;
}

static inline int unpacker_table(lua_State *L, struct xrio_msg *msg) {
  struct xrio_msg *pk = NULL; struct xrio_msg *pv = NULL;
  struct xrio_msg *k = msg; struct xrio_msg *v = msg->next;
  while (k && v)
  {
    unpacker_set(L, k);
    unpacker_set(L, v);
    lua_rawset(L, -3);
    pk = k; pv = v;
    k = v->next;
    if (k)
      v = k->next;
    xrio_free(pk); xrio_free(pv);
  }
  return 0;
}

int unpacker(lua_State *L) {
  int nret = 0; 
  struct xrio_msg *head = NULL;
  struct xrio_msg *msg = lua_touserdata(L, 1);
  while (msg)
  {
    unpacker_set(L, msg);
    /* 清理资源 */
    head = msg;
    msg = msg->next;
    xrio_free(head);
    nret++;
  }
  return nret;
}