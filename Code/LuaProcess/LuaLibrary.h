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
#ifndef LUALIBRARY_H
#define LUALIBRARY_H

#include "../Common/SecureStl.h"
#include "LuaWrapper.h"
#include <string>
#include <vector>
#include <map>

namespace shh {

	typedef std::vector <std::string> LuaExcludedFunctions;
	typedef std::map<std::string, LuaCFunction> LuaFunctions;

	void LuaClearLibs();
	void LuaAddLib(const std::string& name, const LuaCFunction func);
	void RegisterLuaLibs(lua_State* L);
	int LuaGetMetaTable(lua_State* L);
	int LuaSetMetaTable(lua_State* L);
	int LuaGetUpValue(lua_State* L);
	int LuaSetUpValue(lua_State* L);
	int LuaValueToString(lua_State* L);
	int LuaTypeIdToString(lua_State* L);
	int LuaCoroutineYield(lua_State* L);

} // namespace shh
#endif