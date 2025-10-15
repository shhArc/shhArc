///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 David K Bhowmik. All rights reserved.
// This file is part of shhArc.
//
// This Software is available under the MIT License with a No Modification clause.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this file and associated documentation files (the "Software"), to use the
// Software for any purpose, including commercial applications, provided that:
//
//   1. You do NOT modify, alter, or create derivative works of this file.
//   2. Redistributions must include this notice and may only distribute it 
//      unmodified.
//
// The full MIT License text, including this Custom No Modification clause,
// is available in the LICENSE file in the root of the project.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
//
// You may alternatively use this source under the terms of a specific 
// version of the shhArc Unrestricted License provided you have obtained 
// such a license from the copyright holder.
///////////////////////////////////////////////////////////////////////////////



#include <Lua/src/lstate.h>
#include <Lua/src/ltable.h>
#include <Lua/src/lauxlib.h>
#include <Lua/src/lstring.h>
#include <Lua/src/lzio.h>
#include <Lua/src/lauxlib.h>
#include <Lua/src/lvm.h>
#include <Lua/src/lualib.h>
#include <Lua/src/lualib.h>
#include <Lua/src/lapi.h>

#include "../Common/Debug.h"
#include "../Common/Dictionary.h"


#include "../Config/GCPtr.h"
#include <vector>
#include <string>

#ifndef LUAWRAPPER_H
#define LUAWRAPPER_H


typedef struct luaL_Reg LuaFunctionReg;
typedef lua_CFunction LuaCFunction;
typedef unsigned int LuaSize;
typedef int LuaTypeId;
typedef GCObject LuaGCObject;

namespace shh
{
    class BaseType;

    typedef struct LuaUserData
    {
        void* myShhType;
        void* myData;
        LuaTypeId myTypeId;
        const BaseType* myType;
        static const void* ourMarker;
    } LuaUserData;
}

LUAI_FUNC LClosure* luaF_newLclosure(lua_State* L, int nupvals);
LUAI_FUNC Proto* luaF_newproto(lua_State* L);
LUAI_FUNC void luaF_initupvals(lua_State* L, LClosure* cl);


inline void LuaSetCollectable(TValue *o){ o->tt_ = ctb(o->tt_); }
inline bool LuaIsCollectable(const TValue* o) { return iscollectable(o); }

inline GCObject* LuaGetGCObject(const TValue* o) { return val_(o).gc; }
inline Udata* LuaGetUdata(const TValue* o) { return gco2u(LuaGetGCObject(o)); }
inline LClosure* LuaGetLClosure(const TValue* o) { return gco2lcl(LuaGetGCObject(o)); }
inline CClosure* LuaGetCClosure(const TValue* o) { return gco2ccl(LuaGetGCObject(o)); }
inline Closure* LuaGetClosure(const TValue* o) { return gco2cl(LuaGetGCObject(o)); }
inline Proto* LuaGetProto(const TValue* o) { return gco2p(LuaGetGCObject(o)); }
inline Table* LuaGetTable(const TValue* o) { return hvalue(o); }
inline char* LuaGetString(const TValue* o) { return svalue(o); }
inline TString* LuaGetTString(const TValue* o) { return (TString*)(o->value_.gc); }
inline lua_Number LuaGetNumber(const TValue* o) { return nvalue(o); }
inline lua_Integer LuaGetInteger(const TValue* o) { return ivalue(o); }
inline bool LuaGetBoolean(const TValue* o) { return !l_isfalse(o); }

inline void LuaSetValue(TValue* o, const TValue* v) { *o = *v; }
inline void LuaSetValue(TValue* o, const Proto* v) { rawtt(o) = LUA_VPROTO; val_(o).gc = (GCObject*)v; }
inline void LuaSetValue(TValue* o, const lua_Integer* v) { rawtt(o) = LUA_VNUMINT; val_(o).i = *v; }
inline void LuaSetValue(TValue* o, const Table* v) { rawtt(o) = LUA_VTABLE; *LuaGetTable(o) = *v; }
inline void LuaSetValue(TValue* o, const Value* v) { val_(o) = *v; }

inline void LuaSetNilValue(TValue& o) { setnilvalue(&o); }
inline void LuaSetFloat(TValue* o, double v) {   setfltvalue(o, v); }
inline void LuaSetInteger(TValue* o, int v) { setivalue(o, v); }
inline void LuaSetString(TValue* o, TString* v) { rawtt(o) = LUA_TSTRING; val_(o).gc = (GCObject*)v; }
inline TString* LuaNewString(lua_State* L, const char* str) { return luaS_new(L, str); }


inline void LuaIncStack(lua_State* L) { L->top.p++; }
inline void LuaDecStack(lua_State* L){ L->top.p--; }
inline StkId& LuaGetTopStack(lua_State* L){ return L->top.p; }
inline StkId& LuaGetCodeBase(lua_State* L){ return L->ci->func.p; }
inline int LuaGetStackSize(lua_State* L) { return lua_gettop(L); }
inline unsigned int  LuaGetCallStackSize(lua_State* L){ return (unsigned int)(LuaGetTopStack(L) - LuaGetCodeBase(L)-1); }
inline void EnsureFreeStack(lua_State* L, int free)
{
    if (L->stack_last.p - L->top.p <= free)
        luaD_growstack(L, free, 0);
}


inline Value* LuaGetValue(TValue *o){ return &o->value_;}
inline TValue* LuaGetValue(StkId s){ return s2v(s); }
inline TValue* LuaGetStackValue(lua_State* L, int idx)
{
    if (idx <= 0)
        return s2v(L->top.p + idx);
    else
        return s2v(L->ci->func.p + idx);

}
inline const TValue* LuaGetTableValue(Table* t, TString* name) { return luaH_getstr(t, name); }
inline const TValue* LuaGetTableValue(Table* t, TValue* key) { return luaH_get(t, key); }

inline unsigned int LuaGetFunctionType(const TValue* o) { return ttypetag(o); }
inline unsigned int LuaGetSubType(const TValue* o) { return o->tt_; }
inline void LuaSetFloatSubType(TValue* o) { settt_(o, LUA_VNUMFLT); }
inline void LuaSetIntegerSubType(TValue* o) { settt_(o, LUA_VNUMINT); }

inline LuaTypeId LuaGetTypeId(const TValue* o)
{
    if (ttype(o) != LUA_TUSERDATA)
        return ttype(o);

    Udata* u = LuaGetUdata(o);
    if (u->metatable)
    {
        shh::LuaUserData* lud = static_cast<shh::LuaUserData*>((void*)getudatamem(u));
        if(lud->myShhType == shh::LuaUserData::ourMarker)
            return -lud->myTypeId;
    }

    return LUA_TUSERDATA;
}


inline const shh::BaseType* LuaGetType(lua_State* L, const TValue* o)
{
    if (ttype(o) != LUA_TUSERDATA)
        return NULL;

    *s2v(L->top.p) = *o;
    LuaIncStack(L);
    shh::LuaUserData *lud = static_cast<shh::LuaUserData*>(lua_touserdata(L, -1));
    LuaDecStack(L);
    return lud->myType;
}


inline void* LuaGetUserData(lua_State* L, const TValue* o)
{
    *s2v(L->top.p) = *o;
    LuaIncStack(L);
    shh::LuaUserData* lud = static_cast<shh::LuaUserData*>(lua_touserdata(L, -1));
    LuaDecStack(L);
    if (lud->myShhType == shh::LuaUserData::ourMarker)
        return lud->myData;
    else
        return lud;
}


inline void* LuaGetUpValueUserData(lua_State* L, const int idx)
{
    shh::LuaUserData* lud = static_cast<shh::LuaUserData*>(lua_touserdata(L, lua_upvalueindex(idx)));
    if (lud->myShhType == shh::LuaUserData::ourMarker)
        return lud->myData;
    else
        return lud;
}


inline LuaTypeId LuaGetStackTypeId(lua_State* L, int arg)
{
    TValue* o = LuaGetStackValue(L, arg);
    return LuaGetTypeId(o);
}


inline const void * LuaGetStackType(lua_State* L, int arg)
{
    TValue* o = LuaGetStackValue(L, arg);
    if (ttype(o) != LUA_TUSERDATA)
        return NULL;

    shh::LuaUserData* lud = static_cast<shh::LuaUserData*>(lua_touserdata(L, arg));
    return lud->myType;
}


inline void *LuaGetStackUserData(lua_State* L, int arg)
{
    TValue* o = LuaGetStackValue(L, arg);
    if (ttype(o) != LUA_TUSERDATA)
        return NULL;

    shh::LuaUserData* lud = static_cast<shh::LuaUserData*>(lua_touserdata(L, arg));
    DEBUG_ASSERT(lud->myShhType == shh::LuaUserData::ourMarker);
    return lud->myData;
}


template < typename T > void LuaSetStackValue(lua_State* L, int idx, const T* v) { LuaSetValue(s2v(L->top.p + idx), v); }
inline void LuaSetNilStack(lua_State* L, int idx) { LuaSetNilValue(*s2v(L->top.p + idx)); }

inline TValue* LuaGetGlobalsValue(lua_State* L){ return (&hvalue(&G(L)->l_registry)->array[LUA_RIDX_GLOBALS - 1]); }
inline TValue* LuaGetRegistry(lua_State* L){ return &G(L)->l_registry; }
inline void LuaSetRegistry(lua_State* L, TValue* r){ G(L)->l_registry = *r; }
inline void LuaGetGlobalsStack(lua_State* L){ lua_pushglobaltable(L); }
inline Table* LuaGetGlobalsTable(lua_State* L){ return LuaGetTable(LuaGetGlobalsValue(L)); }

inline void LuaSetGlobals(lua_State* L)
{
    TValue* t = LuaGetStackValue(L, -1);
    hvalue(&G(L)->l_registry)->array[LUA_RIDX_GLOBALS - 1] = *t;
    LuaDecStack(L);
}

inline unsigned int LuaInitTable(lua_State* L, const TValue *tableValue, unsigned int arraySize, unsigned int nodeSize) 
{
    Table* t = LuaGetTable(tableValue);
    luaH_resize(L, t, arraySize, nodeSize);
    return twoto(t->lsizenode);
}


#endif