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

#include "../Config/Config.h"
#include "LuaLibrary.h"
#include "LuaApi.h"
#include "LuaWrapper.h"
#include "LuaType.h"
#include "LuaProcess.h"

extern int luaB_getmetatable(lua_State* L);
extern int luaB_setmetatable(lua_State* L);
extern int luaB_yield(lua_State* L);


namespace shh {



	static ::LuaFunctionReg defaultloadedlibs[] =
	{
#if LUA_DEBUG_LIB
		{LUA_DBLIBNAME, luaopen_debug},
#endif
		{LUA_GNAME, luaopen_base }, 
		{LUA_LOADLIBNAME, luaopen_package}, 
		{LUA_COLIBNAME, luaopen_coroutine}, 
		{LUA_TABLIBNAME, luaopen_table}, 
		{LUA_IOLIBNAME, luaopen_io}, 
		{LUA_OSLIBNAME, luaopen_os}, 
		{LUA_STRLIBNAME, luaopen_string}, 
		{LUA_MATHLIBNAME, luaopen_math}, 
		{LUA_UTF8LIBNAME, luaopen_utf8}
		
	};

#ifdef LUA_DEBUG_LIB
	static int defaultNumLoaded = 10;
#else
	static int defaultNumLoaded = 9;
#endif

	// Funcitons to overwrite
	static const struct ::LuaFunctionReg defaultOverrided[] = 
	{
	  {"getmetatable", LuaGetMetaTable},
	  {"setmetatable", LuaSetMetaTable},
	  {"debug.getupvalue", LuaGetUpValue},
	  {"debug.setupvalue", LuaSetUpValue},
	  {"tostring", LuaValueToString},
	  {"type", LuaTypeIdToString},
	  {"coroutine.yield", LuaCoroutineYield},
	};

	static int defaultNumOverrided = 7;


	// Functions to exclude:
	static const char* defaultExcluded[] = {
		//LUA_ERRORMESSAGE,
		"dofile",
		"collectgarbage",
		"globals",
		"getglobal",
		"setglobal",
		"newtag",
		"rawget",
		"rawset",
		"rawgettable",
		"rawsettable"
	};

	static int defaultNumExcluded = 11 - 1;



	LuaCFunction typeFunction;
	LuaCFunction stringFunction;
	static LuaFunctions loadedlibs;
	static LuaExcludedFunctions excluded;
	static LuaFunctions overrided;
	

	// --------------------------------------------------------------------------						
	// Function:	LuaClearLibs
	// Description:	clears list of libraries to be registered
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void LuaClearLibs() 
	{ 
		loadedlibs.clear(); 
	}


	// --------------------------------------------------------------------------						
	// Function:	LuaAddLib
	// Description:	adds a library to register
	// Arguments:	library name, library register func
	// Returns:		none
	// --------------------------------------------------------------------------
	void LuaAddLib(const std::string &name, const LuaCFunction func) 
	{ 
		loadedlibs[name] = func; 
	}


	// --------------------------------------------------------------------------						
	// Function:	SetLuaLibRegistered
	// Description:	sets list of libraries to be registered
	// Arguments:	library list
	// Returns:		none
	// --------------------------------------------------------------------------
	void SetLuaLibRegistered(const LuaFunctions &libs) 
	{ 
		loadedlibs = libs; 
	}


	// --------------------------------------------------------------------------						
	// Function:	LuaExcludedFunctions
	// Description:	sets list of functions to be exclusded
	// Arguments:	function list
	// Returns:		none
	// --------------------------------------------------------------------------
	void SetLuaLibExcluded(const LuaExcludedFunctions& excludes)
	{ 
		excluded = excludes; 
	}


	// --------------------------------------------------------------------------						
	// Function:	SetLuaLibOverided
	// Description:	sets functions to be overided
	// Arguments:	list of function names and functions 
	// Returns:		none
	// --------------------------------------------------------------------------
	void SetLuaLibOverided(const LuaFunctions& overrides) 
	{ 
		overrided = overrides; 
	}


	// --------------------------------------------------------------------------						
	// Function:	RegisterLuaLibs
	// Description:	registers libraries
	// Arguments:	lua state
	// Returns:		none
	// --------------------------------------------------------------------------
	void RegisterLuaLibs(lua_State* L)
	{
		if (loadedlibs.empty())
		{ 
			// use default lib config defined above
			for (int j = 0; j < defaultNumLoaded; j++)
				loadedlibs[defaultloadedlibs[j].name] = defaultloadedlibs[j].func;
		}

		for (LuaFunctions::iterator lit = loadedlibs.begin(); lit != loadedlibs.end(); lit++)
		{
			// register library
			luaL_requiref(L, lit->first.c_str(), lit->second, 1);
			lua_pop(L, 1);
		}

		if (excluded.empty())
		{
			// use default excluded funcs
			for (int i = 0; i < defaultNumExcluded; i++)
				excluded.push_back(defaultExcluded[i]);
		}

		for (int i = 0; i < excluded.size(); i++)
		{
			// black out excluded funds
			lua_pushnil(L);
			lua_setglobal(L, excluded[i].c_str());
		}

	
		if (overrided.empty())
		{
			// use default overrided fundcs
			for (int j = 0; j < defaultNumOverrided; j++)
				overrided[defaultOverrided[j].name] = defaultOverrided[j].func;
		}



		// override normal functions in special circumstances
		for (LuaFunctions::iterator oit = overrided.begin(); oit != overrided.end(); oit++)
		{
			if (oit->first == "tostring")
			{
				lua_getglobal(L, oit->first.c_str());
				stringFunction = lua_tocfunction(L, -1);
			}
			else if (oit->first == "type")
			{
				lua_getglobal(L, oit->first.c_str());
				typeFunction = lua_tocfunction(L, -1);
			}
			std::size_t found = oit->first.find('.');
			if (found != std::string::npos)
			{
				std::string tableName = oit->first.substr(0, found);
				std::string functionName = oit->first.substr(found + 1, oit->first.size());
				const TValue* value = LuaGetTableValue(LuaGetGlobalsTable(L), LuaNewString(L, tableName.c_str()));
				if(LuaGetTypeId(value) == LUA_TTABLE)
				{
					LuaSetStackValue(L, 0, value);
					LuaIncStack(L);
					lua_pushcclosure(L, oit->second, 0);
					lua_setfield(L, -2, functionName.c_str());
					LuaDecStack(L);
				}
			}
			else
			{
				lua_pushcclosure(L, oit->second, 0);
				lua_setglobal(L, oit->first.c_str());
			}
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	LuaGetMetaTable
	// Description:	overide function for getmetatable
	// Arguments:	lua state
	// Returns:		none
	// --------------------------------------------------------------------------
	int LuaGetMetaTable(lua_State* L)
	{
		LuaApi::CheckNumArguments(L, 1);
		LuaTypeId type = LuaApi::GetArgumentType(L, 1);
		if(type < 0 || type == LUA_TUSERDATA || type == LUA_TLIGHTUSERDATA)
		{
			LuaApi::ThrowScriptError("Function getmetatable is not permitted on engine types");
			return 0;
		}
		return luaB_getmetatable(L);
	}


	// --------------------------------------------------------------------------						
	// Function:	LuaSetMetaTable
	// Description:	overide function for setmetatable
	// Arguments:	lua state
	// Returns:		none
	// --------------------------------------------------------------------------
	int LuaSetMetaTable(lua_State* L)
	{
		LuaApi::CheckNumArguments(L, 2);
		LuaTypeId type = LuaApi::GetArgumentType(L, 1);
		if(type < 0 || type == LUA_TUSERDATA || type == LUA_TLIGHTUSERDATA)
		{
			LuaApi::ThrowScriptError("Function setmetatable is not permitted on engine types");
			return 0;
		}
		return luaB_setmetatable(L);
	}


	// --------------------------------------------------------------------------						
	// Function:	LuaGetUpValue
	// Description:	overide function for getmetatable
	// Arguments:	lua state
	// Returns:		none
	// --------------------------------------------------------------------------
	int LuaGetUpValue(lua_State* L)
	{
		LuaApi::ThrowScriptError("Function getup value is not permitted");
		return 0;
	}


	// --------------------------------------------------------------------------						
	// Function:	LuaSetU0pValue
	// Description:	overide function for setmetatable
	// Arguments:	lua state
	// Returns:		none
	// --------------------------------------------------------------------------
	int LuaSetUpValue(lua_State* L)
	{
		LuaApi::ThrowScriptError("Function setupvalue is not permitted");
		return 0;
	}

	// --------------------------------------------------------------------------						
	// Function:	LuaValueToString
	// Description:	overide function for tostring
	// Arguments:	lua state
	// Returns:		none
	// --------------------------------------------------------------------------
	int LuaValueToString(lua_State* L)
	{
		LuaApi::CheckNumArguments(L, 1);
		LuaTypeId type = LuaTypeBase::GetArgumentType(L, 1);
		if (type < 0)
		{
			StringFunction func = Registry::GetRegistry().GetStringFunction(type);
			void *data = *static_cast<void**>(lua_touserdata(L, 1));
			std::string buff = (*func)(data);
			lua_pushstring(L, buff.c_str());
			return 1;
		}

		return stringFunction(L);

	}


	// --------------------------------------------------------------------------						
	// Function:	LuaValueToString
	// Description:	overide function for type
	// Arguments:	lua state
	// Returns:		none
	// --------------------------------------------------------------------------
	int LuaTypeIdToString(lua_State* L)
	{
		LuaApi::CheckNumArguments(L, 1);
		LuaTypeId type = LuaTypeBase::GetArgumentType(L, 1);
		if (type < 0)
		{
			std::string buff = Registry::GetRegistry().GetTypeName(type);
			lua_pushstring(L, buff.c_str());
			return 1;
		}

		return typeFunction(L);
	}


	// --------------------------------------------------------------------------						
	// Function:	LuaCoroutineYield
	// Description:	overide function for yield
	// Arguments:	lua state
	// Returns:		none
	// --------------------------------------------------------------------------
	int LuaCoroutineYield(lua_State* L)
	{
		if (LuaApi::GetCurrentLuaProcess()->GetLuaState() == L)
		{
			LuaApi::ThrowScriptError("Can only call yield within a coroutine");
			return 0;
		}

		return luaB_yield(L);
	}

} 