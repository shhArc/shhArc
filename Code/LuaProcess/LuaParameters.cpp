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



#include "../LuaProcess/LuaType.h"
#include "../LuaProcess/LuaProcess.h"
#include "LuaParameters.h"
#include "LuaApi.h"


namespace shh {

	lua_State* LuaParameters::GetCurrentLuaState() { return LuaProcess::GetCurrentLuaState(); };

	// --------------------------------------------------------------------------						
	// Function:	Get
	// Description:	gets argument from stacl
	// Arguments:	lua state, argument number, value to return
	// Returns:		none
	// --------------------------------------------------------------------------
	void LuaParameters::Get(lua_State* L, int arg, bool& b) { b = lua_toboolean(L, arg); }
	void LuaParameters::Get(lua_State* L, int arg, double& number) { number = (double)lua_tonumber(L, arg); }
	void LuaParameters::Get(lua_State* L, int arg, float& number) { number = (float)lua_tonumber(L, arg); }
	void LuaParameters::Get(lua_State* L, int arg, int& number) { number = (int)lua_tointeger(L, arg); }
	void LuaParameters::Get(lua_State* L, int arg, unsigned int& number) { number = (unsigned int)lua_tointeger(L, arg); }
	void LuaParameters::Get(lua_State* L, int arg, long& number) { number = (long)lua_tointeger(L, arg); }
	void LuaParameters::Get(lua_State* L, int arg, unsigned long& number) { number = (unsigned long)lua_tointeger(L, arg); }
	void LuaParameters::Get(lua_State* L, int arg, long long& number) { number = (long long)lua_tointeger(L, arg); }
	void LuaParameters::Get(lua_State* L, int arg, unsigned long long& number) { number = (unsigned long long)lua_tointeger(L, arg); }
	void LuaParameters::Get(lua_State* L, int arg, std::string& str) { str = lua_tostring(L, arg); }
	void LuaParameters::Get(lua_State* L, int arg, VariantKeyDictionary& dict) { const TValue* o = LuaApi::GetArg(L, arg); LuaHelperFunctions::PopDictionary(L, o, &dict); }
	void LuaParameters::Get(lua_State* L, int arg, const char** str) { THROW("Undefined Lua send arg type"); }
	void LuaParameters::Get(lua_State* L, int arg, void** data) { THROW("Undefined Lua send arg type"); }
	


	// --------------------------------------------------------------------------						
	// Function:	Push
	// Description:	Pushes argument on stack
	// Arguments:	lua state, argument number, value to return
	// Returns:		none
	// --------------------------------------------------------------------------
	void LuaParameters::Push(bool* b) { LuaType<bool>::ExternalPushBoolean(b); }
	void LuaParameters::Push(double* number) { LuaType<double>::ExternalPushFloat(number); }
	void LuaParameters::Push(float* number) { LuaType<float>::ExternalPushFloat(number); }
	void LuaParameters::Push(int* number) { LuaType<int>::ExternalPushInteger(number); }
	void LuaParameters::Push(unsigned int* number) { LuaType<unsigned int>::ExternalPushInteger(number); }
	void LuaParameters::Push(long* number) { LuaType<long>::ExternalPushInteger(number); }
	void LuaParameters::Push(unsigned long* number) { LuaType<unsigned long>::ExternalPushInteger(number); }
	void LuaParameters::Push(long long* number) { LuaType<long long>::ExternalPushInteger(number); }
	void LuaParameters::Push(unsigned long long* number) { LuaType<unsigned long long>::ExternalPushInteger(number); }
	void LuaParameters::Push(std::string* str) { LuaType<std::string>::ExternalPushString(str); }
	void LuaParameters::Push(VariantKeyDictionary* dict) { lua_State* L = LuaApi::GetCurrentLuaState();  LuaHelperFunctions::PushDictionary(L, dict); }
	void LuaParameters::Push(const char** str) { THROW("Undefined Lua return type"); }
	void LuaParameters::Push(void** data) { THROW("Undefined Lua return type"); }


}//namespace shh

