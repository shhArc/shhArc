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


#ifndef LUAPARAMETERS_H
#define LUAPARAMETERS_H


#include "../Common/SecureStl.h"
#include "../Common/Debug.h"
#include "../Common/Dictionary.h"
#include "../Common/TypeLog.h"
#include "../VM/Parameters.h"
#include "LuaWrapper.h"
#include <Lua/src/lstate.h>

#define luaL_getmetatable(L,n)	(lua_getfield(L, LUA_REGISTRYINDEX, (n)))

namespace shh {

	class LuaParameters
	{

	public:

		static lua_State* GetCurrentLuaState();

		static inline void* LuaApiGetStackUserData(lua_State* L, int arg);
		template <class T> static inline void* LuaApiAddUserData(T* data);

		static void Get(lua_State* L, int arg, bool& b);
		static void Get(lua_State* L, int arg, double& number);
		static void Get(lua_State* L, int arg, float& number);
		static void Get(lua_State* L, int arg, int& number);
		static void Get(lua_State* L, int arg, unsigned int& number);
		static void Get(lua_State* L, int arg, long& number);
		static void Get(lua_State* L, int arg, unsigned long& number);
		static void Get(lua_State* L, int arg, long long& number);
		static void Get(lua_State* L, int arg, unsigned long long& number);
		static void Get(lua_State* L, int arg, std::string& str);
		static void Get(lua_State* L, int arg, VariantKeyDictionary& dict);
		static void Get(lua_State* L, int arg, const char** str);
		static void Get(lua_State* L, int arg, void** data);
		template<class T> static inline void Get(lua_State* L, int arg, GCPtr<T>& userPtr) { userPtr = *static_cast<GCPtr<T>*>(LuaApiGetStackUserData(L, arg)); }
		template<class T> static inline void Get(lua_State* L, int arg, T& userType) { userType = *static_cast<const T*>(LuaApiGetStackUserData(L, arg)); }
		template<class T> static inline T* Set(lua_State* L, T& data, unsigned int argNum, unsigned int firstReturnArg);


		static void Push(bool* b);
		static void Push(double* number);
		static void Push(float* number);
		static void Push(int* number);
		static void Push(unsigned int* number);
		static void Push(long* number);
		static void Push(unsigned long* number);
		static void Push(long long* number);
		static void Push(unsigned long long* number);
		static void Push(std::string* str);
		static void Push(VariantKeyDictionary* dict);
		static void Push(const char** str);
		static void Push(void** data);
		template<class T> static inline void Push(GCPtr<T>* userPtr) { LuaApiAddUserData(new GCPtr<T>(*userPtr)); }
		template<class T> static inline void Push(T* userType) { LuaApiAddUserData(new T(*userType)); }
		template<class T> static inline void Return(T* data, unsigned int argNum, unsigned int firstReturnArg)
		{
			if (argNum >= firstReturnArg)
				Push(data);
		}
	};


	// --------------------------------------------------------------------------						
	// Function:	LuaApiGetStackUserData
	// Description:	gets an arg from the lua stack that is a user data type
	// Arguments:	lua state, stack arg number
	// Returns:		pointer to user data
	// --------------------------------------------------------------------------
	inline void* LuaParameters::LuaApiGetStackUserData(lua_State* L, int arg)
	{

		TValue* o;
		if (arg <= 0)
			o = s2v(L->top.p + arg);
		else
			o = s2v(L->ci->func.p + arg);

		if (ttype(o) != LUA_TUSERDATA)
			return NULL;

		shh::LuaUserData* lud = static_cast<shh::LuaUserData*>(lua_touserdata(L, arg));
		DEBUG_ASSERT(lud->myShhType == shh::LuaUserData::ourMarker);
		return lud->myData;
	}


	// --------------------------------------------------------------------------						
	// Function:	LuaApiAddUserData
	// Description:	adds data to a lua type
	// Arguments:	data
	// Returns:		lua user data
	// --------------------------------------------------------------------------
	template <class T> inline void* LuaParameters::LuaApiAddUserData(T* data)
	{
		lua_State* L = GetCurrentLuaState();
		void* p = lua_newuserdata(L, sizeof(LuaUserData));
		shh::LuaUserData* lud = static_cast<shh::LuaUserData*>(p);
		lud->myShhType = (void*)LuaUserData::ourMarker;
		lud->myData = data;
		lud->myTypeId = TypeLog<T>::GetTypeId();
		lud->myType = TypeLog<T>::GetStaticType();
		int ok = luaL_getmetatable(L, TypeLog<T>::GetTypeName().c_str());
		DEBUG_ASSERT(ok);
		lua_setmetatable(L, -2);
		return p;
	}


	// --------------------------------------------------------------------------						
	// Function:	Set
	// Description:	sets a C function call arg to that from a lua stack
	// Arguments:	lua state,  variabvle to set, C function arg number
	//				index of first return arg of C functions
	// Returns:		if successful
	// --------------------------------------------------------------------------
	template<class T> inline T* LuaParameters::Set(lua_State* L, T& data, unsigned int argNum, unsigned int firstReturnArg)
	{
		if (argNum < firstReturnArg)
		{
			Get(L, argNum, data);
			return &data;
		}
		else
		{
			return &data;
		}
	}


}
#endif