/////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////

#ifndef LUAAPI_H
#define LUAAPI_H

#include "../Common/SecureStl.h"
#include "../Common/Enums.h"
#include "../Common/Dictionary.h"
#include "../Config/GCPtr.h"
#include "../LuaProcess/LuaWrapper.h"
#include <string>

#include <Lua/src/lstate.h>
#include <Lua/src/lapi.h>


namespace shh {

	class LuaProcess;

	void LuaThrowScriptError(const char* format, ...);

	class LuaApi
	{
	public:

		static int LuaDispatcher(lua_State* L);
		static void ThrowScriptError(const char* format, ...);
		static void OpenNamespace(const std::string& name);
		static void CloseNamespace();
		static const GCPtr<LuaProcess> GetCurrentLuaProcess();
		static lua_State* GetCurrentLuaState();
		static unsigned int GetNumArgs(lua_State* L);
		static void CheckNumArguments(lua_State* L, unsigned int n);
		static void CheckNumArgumentsGreaterOrEqual(lua_State* L, unsigned int n);
		static int CheckNumArgumentRange(lua_State* L, unsigned int args[], unsigned int numSizes);

		static const int GetArgumentType(lua_State* L, int arg);

		static void GetArgument(lua_State* L, int arg, bool& b);
		static void GetArgument(lua_State* L, int arg, double& number);
		static void GetArgument(lua_State* L, int arg, float& number);
		static void GetArgument(lua_State* L, int arg, int& number);
		static void GetArgument(lua_State* L, int arg, unsigned int& number);
		static void GetArgument(lua_State* L, int arg, long& number);
		static void GetArgument(lua_State* L, int arg, unsigned long& number);
		static void GetArgument(lua_State* L, int arg, long long& number);
		static void GetArgument(lua_State* L, int arg, unsigned long long& number);
		static void GetArgument(lua_State* L, int arg, std::string& str);
		static void GetArgument(lua_State* L, int arg, const char** str);
		static void GetArgument(lua_State* L, int arg, VariantKeyDictionary& dict);
		static void GetArgument(lua_State* L, int arg, void ** data);

		static const TValue* GetArg(lua_State* L, int arg);
	
		static bool RenameTableValue(const std::string& oldName, const std::string& newName);
		static void GetErrorMessageDetails(std::string& errorMessage);
	};
} // namespace shh


#include"../Config/LuaRegister.h"

#endif