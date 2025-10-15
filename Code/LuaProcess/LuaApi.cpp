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


#ifdef _MSC_VER
#pragma warning( disable : 4786 4503 )
#endif




#include "../Config/GCPtr.h"
#include "../Common/SecureStl.h"
#include "../Arc/Type.inl"
#include "LuaApi.h"
#include "LuaWrapper.h"
#include "LuaProcess.h"
#include "LuaType.h"
#include <string>
#include <stdarg.h>


namespace shh {

	// --------------------------------------------------------------------------						
	// Function:	LuaThrowScriptError
	// Description:	throws a lua error
	// Arguments:	format, data to print
	// Returns:		none
	// --------------------------------------------------------------------------
	void LuaThrowScriptError(const char* format, ...)
	{
		va_list ap;
		va_start(ap, format);
		LuaApi::ThrowScriptError(format, ap);
		va_end(ap);
	}


	// --------------------------------------------------------------------------						
	// Function:	LuaDispatcher
	// Description:	Lua C function dispatcher for overloaded functions call from 
	//				lua
	// Arguments:	lua state
	// Returns:		none
	// --------------------------------------------------------------------------
	int LuaApi::LuaDispatcher(lua_State* L)
	{
		// get overload table
		void* data = (LuaUserData*)LuaGetUpValueUserData(L, 1);
		GCPtr<Registry::OverloadTable>* ot = static_cast<GCPtr<Registry::OverloadTable>*>(data);
		Registry::OverloadTable& t = *(ot->GetObject());

		// get argument tyoes from calling lua code
		LuaDispatchData dispatchData;
		dispatchData.L = L;
		Registry::OverloadTable::ArgumentTypes argTypes;
		int numArgs = LuaGetCallStackSize(L);
		for (int a = 1; a <= numArgs; a++)
		{
			TValue* v = LuaGetStackValue(L, a);
			LuaTypeId id = LuaGetTypeId(v);
			if (id < 0)
				argTypes.push_back(-id);
			else if (id == LUA_TNUMBER)
			{
				if (LuaGetSubType(v) == LUA_VNUMINT)
					argTypes.push_back(Type<long long>::GetTypeId());
				else
					argTypes.push_back(Type<double>::GetTypeId());
			}
			else if (id == LUA_TSTRING)
				argTypes.push_back(Type<std::string>::GetTypeId());
			else if (id == LUA_TBOOLEAN)
				argTypes.push_back(Type<bool>::GetTypeId());
			else if (id == LUA_TTABLE)
				argTypes.push_back(Type<VariantKeyDictionary>::GetTypeId());
			else
				argTypes.push_back(0);

		}

		// call using the the overload table
		ExecutionState state = (*ot)->Call(Lua, argTypes, LuaTypeBase::GetSharedTypes(), &dispatchData);
		if (state != ExecutionOk)
		{
			// error
			GetCurrentLuaProcess()->myState = state;
			LuaApi::ThrowScriptError(dispatchData.myMessage.c_str());
		}
		return dispatchData.numReturnValues;
	}


	// --------------------------------------------------------------------------						
	// Function:	ThrowScriptError
	// Description:	throws a lua error
	// Arguments:	format, data to print
	// Returns:		none
	// --------------------------------------------------------------------------
	void LuaApi::ThrowScriptError(const char* format, ...)
	{
		GetCurrentLuaProcess()->myScriptError = true;

		// Variable argument list support.
		va_list ap;
		va_start(ap, format);

		const int bufferSize = 1024;
		char error[bufferSize];

		vsnprintf(error, bufferSize, format, ap);

		// End variable argument list support.
		va_end(ap);

		lua_error(LuaProcess::GetCurrentLuaState());
	}





	// --------------------------------------------------------------------------						
	// Function:	OpenNamespace
	// Description:	opens and if  creates a name space in the current process
	// Arguments:	name space
	// Returns:		none
	// --------------------------------------------------------------------------
	void LuaApi::OpenNamespace(const std::string& name)
	{
		GetCurrentLuaProcess()->OpenNamespace(name);
	}


	// --------------------------------------------------------------------------						
	// Function:	CloseNamespace
	// Description:	closes the currently open namespace in the current process
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void LuaApi::CloseNamespace()
	{
		GetCurrentLuaProcess()->CloseNamespace();
	}


	// --------------------------------------------------------------------------						
	// Function:	GetLuaCurrentProcess
	// Description:	Gets the currently active LuaProcess 
	// Arguments:	none
	// Returns:		LuaProcess
	// --------------------------------------------------------------------------
	const GCPtr<LuaProcess> LuaApi::GetCurrentLuaProcess()
	{
		GCPtr<LuaProcess> lp;
		lp.StaticCast(Scheduler::GetCurrentProcess());
		return lp;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetCurrentLuaState
	// Description:	returns the lua state of the current process 
	// Arguments:	none
	// Returns:		lua state
	// --------------------------------------------------------------------------						
	lua_State* LuaApi::GetCurrentLuaState() 
	{ 
		return LuaProcess::GetCurrentLuaState(); 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetNumArgs
	// Description:	Returns the number of arguments in the lua state stack 
	// Arguments:	lua state
	// Returns:		num args
	// --------------------------------------------------------------------------
	unsigned int LuaApi::GetNumArgs(lua_State* L)
	{
		return LuaGetCallStackSize(L);
	}


	// --------------------------------------------------------------------------						
	// Function:	CheckNumArguments
	// Description:	Throws error if the number of arguments in the lua state 
	//				stack is not the same as that given
	// Arguments:	lua state, num args required
	// Returns:		none
	// --------------------------------------------------------------------------
	void LuaApi::CheckNumArguments(lua_State* L, unsigned int n)
	{
		unsigned int size = LuaGetCallStackSize(L);
		if (size != n)
			ThrowScriptError("Invalid number of arguments, %d expected, %d got.", n, size);
	}


	// --------------------------------------------------------------------------						
	// Function:	CheckNumArgumentsGreaterOrEqual
	// Description:	Throws error if the number of arguments is lass than given
	// Arguments:	lua state, num args 
	// Returns:		none
	// --------------------------------------------------------------------------
	void LuaApi::CheckNumArgumentsGreaterOrEqual(lua_State* L, unsigned int n)
	{
		unsigned int size = LuaGetCallStackSize(L);
		if (size < n)
			ThrowScriptError("Invalid number of arguments, %d or more expected, %d got.", n, size);
	}


	// --------------------------------------------------------------------------						
	// Function:	CheckNumArgumentRange
	// Description:	Checks if no args on stack is equal to one of the given 
	//				arg sizes, thows error if not
	// Arguments:	lua state, args sizes, number of arg sizes to to check
	// Returns:		num args
	// --------------------------------------------------------------------------
	int LuaApi::CheckNumArgumentRange(lua_State* L, unsigned int args[], unsigned int numSizes)
	{
		int numArgs = LuaGetCallStackSize(L);
		for (int s = 0; s != numSizes; s++)
		{
			if (numArgs == args[s])
				return numArgs;
		}
		std::string error;
		for (int s = 0; s != numSizes; s++)
		{
			error += args[s];
			error += ", ";
		}
		ThrowScriptError("Invalid number of arguments, %s expected, %d got", error.c_str(), numArgs);
		return numArgs;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetArgumentType
	// Description:	Gets the data type of the argument number asked for
	// Arguments:	lua state, args number 
	// Returns:		type id
	// --------------------------------------------------------------------------
	const int LuaApi::GetArgumentType(lua_State* L, int arg) 
	{ 
		return LuaGetStackTypeId(LuaProcess::GetCurrentLuaState(), arg); 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetArgument
	// Description:	Gets the argument number asked for of type requested.
	// Arguments:	lua state, args numbeer, value of type to return
	//				throws error if fails
	// Returns:		none but referenced argument is set
	// --------------------------------------------------------------------------
	void LuaApi::GetArgument(lua_State* L, int arg, bool& b)
	{
		if (LuaGetStackSize(L) < arg || LuaGetStackTypeId(L, arg) == LUA_TBOOLEAN)
			ThrowScriptError("Invalid argument %d.", arg + 1);
		b = lua_toboolean(L, arg);
	}
	void LuaApi::GetArgument(lua_State* L, int arg, double& number)
	{
		TValue* v = LuaGetStackValue(L, arg);
		if (LuaGetStackSize(L) < arg || LuaGetTypeId(v) != LUA_TNUMBER || LuaGetSubType(v) != LUA_VNUMFLT)
			ThrowScriptError("Invalid argument %d.", arg + 1);
		number = lua_tonumber(L, arg);
	}
	void LuaApi::GetArgument(lua_State* L, int arg, float& number)
	{
		TValue* v = LuaGetStackValue(L, arg);
		if (LuaGetStackSize(L) < arg || LuaGetTypeId(v) != LUA_TNUMBER || LuaGetSubType(v) != LUA_VNUMFLT)
			ThrowScriptError("Invalid argument %d.", arg + 1);
		number = (float)lua_tonumber(L, arg);
	}
	void LuaApi::GetArgument(lua_State* L, int arg, int& number)
	{
		TValue* v = LuaGetStackValue(L, arg);
		if (LuaGetStackSize(L) < arg || LuaGetTypeId(v) != LUA_TNUMBER || LuaGetSubType(v) != LUA_VNUMINT)
			ThrowScriptError("Invalid argument %d.", arg + 1);
		number = (int)lua_tointeger(L, arg);
	}
	void LuaApi::GetArgument(lua_State* L, int arg, unsigned int& number)
	{
		TValue* v = LuaGetStackValue(L, arg);
		if (LuaGetStackSize(L) < arg || LuaGetTypeId(v) != LUA_TNUMBER || LuaGetSubType(v) != LUA_VNUMINT)
			ThrowScriptError("Invalid argument %d.", arg + 1);
		number = (unsigned int)lua_tointeger(L, arg);
	}
	void LuaApi::GetArgument(lua_State* L, int arg, long& number)
	{
		TValue* v = LuaGetStackValue(L, arg);
		if (LuaGetStackSize(L) < arg || LuaGetTypeId(v) != LUA_TNUMBER || LuaGetSubType(v) != LUA_VNUMINT)
			ThrowScriptError("Invalid argument %d.", arg + 1);
		number = (long)lua_tointeger(L, arg);
	}
	void LuaApi::GetArgument(lua_State* L, int arg, unsigned long& number)
	{
		TValue* v = LuaGetStackValue(L, arg);
		if (LuaGetStackSize(L) < arg || LuaGetTypeId(v) != LUA_TNUMBER || LuaGetSubType(v) != LUA_VNUMINT)
			ThrowScriptError("Invalid argument %d.", arg + 1);
		number = (unsigned long)lua_tointeger(L, arg);
	}
	void LuaApi::GetArgument(lua_State* L, int arg, long long& number)
	{
		TValue* v = LuaGetStackValue(L, arg);
		if (LuaGetStackSize(L) < arg || LuaGetTypeId(v) != LUA_TNUMBER || LuaGetSubType(v) != LUA_VNUMINT)
			ThrowScriptError("Invalid argument %d.", arg + 1);
		number = (long long)lua_tointeger(L, arg);
	}
	void LuaApi::GetArgument(lua_State* L, int arg, unsigned long long& number)
	{
		TValue* v = LuaGetStackValue(L, arg);
		if (LuaGetStackSize(L) < arg || LuaGetTypeId(v) != LUA_TNUMBER || LuaGetSubType(v) != LUA_VNUMINT)
			ThrowScriptError("Invalid argument %d.", arg + 1);
		number = (unsigned long long)lua_tointeger(L, arg);
	}
	void LuaApi::GetArgument(lua_State* L, int arg, std::string& str)
	{
		if (LuaGetStackSize(L) < arg || LuaGetStackTypeId(L, arg) != LUA_TSTRING)
			ThrowScriptError("Invalid argument %d.", arg + 1);
		str = lua_tostring(L, arg);
	}
	void LuaApi::GetArgument(lua_State* L, int arg, const char** str)
	{
		if (LuaGetStackSize(L) < arg || LuaGetStackTypeId(L, arg) != LUA_TSTRING)
			ThrowScriptError("Invalid argument %d.", arg + 1);
		*str = lua_tostring(L, arg);
	}
	void LuaApi::GetArgument(lua_State* L, int arg, VariantKeyDictionary& dict)
	{
		if (LuaGetStackSize(L) < arg || LuaGetStackTypeId(L, arg) != LUA_TTABLE)
			ThrowScriptError("Invalid argument %d.", arg + 1);
		TValue* toClone = LuaGetStackValue(L, arg);
		LuaHelperFunctions::PopDictionary(L, toClone, &dict);

	}
	void LuaApi::GetArgument(lua_State* L, int arg, void ** data)
	{
		if (LuaGetStackSize(L) < arg || LuaGetStackTypeId(L, arg) >= 0)
			ThrowScriptError("Invalid argument %d.", arg + 1);
		*data = LuaGetStackUserData(L, arg);
	}


	// --------------------------------------------------------------------------						
	// Function:	GetArg
	// Description:	gets the script argument of given number from stack
	// Arguments:	Lua state, num of arg required
	// Returns:		argument as lua object
	// --------------------------------------------------------------------------
	const TValue* LuaApi::GetArg(lua_State* L, int arg) 
	{ 
		return LuaGetValue(LuaGetCodeBase(L) + arg); 
	}


	// --------------------------------------------------------------------------						
	// Function:	RenameTableValue
	// Description:	renames a variable in lua table, expects table on stack
	// Arguments:	variable value, new name
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool LuaApi::RenameTableValue(const std::string& oldName, const std::string& newName)
	{
		lua_State* L = GetCurrentLuaState();
		// 

		// place value of new name in table
		lua_pushlstring(L, newName.c_str(), newName.length());
		const TValue* value = LuaGetTableValue(LuaGetTable(LuaGetStackValue(L, -2)), LuaNewString(L, oldName.c_str()));

		if(LuaGetTypeId(value) == LUA_TNIL)
		{
			lua_pop(L, 1);
			return false;
		}

		LuaSetStackValue(L, 0, value);
		LuaIncStack(L);

		lua_settable(L, LuaGetStackSize(L) - 2);

		// clear value of old name
		lua_pushlstring(L, oldName.c_str(), oldName.length());
		LuaSetNilStack(L, 0);
		LuaIncStack(L);

		lua_settable(L, LuaGetStackSize(L) - 2);

		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetErrorMessageDetails
	// Description:	create a full line numbered error message, but without
	//				the problem detail
	// Arguments:	full line numbered etc error message to be added to
	// Returns:		none
	// --------------------------------------------------------------------------
	void LuaApi::GetErrorMessageDetails(std::string& errorMessage)
	{
		lua_State* L = GetCurrentLuaState();
		lua_Debug ar2;
		lua_getstack(L, 1, &ar2);
	
		lua_getinfo(L, "Slunf", &ar2);

		std::string cFunctionBeingCalled;
		if (ar2.what == "C")
		{
			cFunctionBeingCalled = "\nC++ function: ";
			cFunctionBeingCalled += ar2.name ? ar2.name : "<unknown>";
			lua_getstack(L, 2, &ar2);
			lua_getinfo(L, "Slunf", &ar2);
		}

		if (ar2.source[0] == '@')
		{
			errorMessage += "\n";
			std::string file = ar2.source;
			errorMessage += file.substr(1, file.length());
			errorMessage += ",";

			char line[50];
			_snprintf_s(line, 49, "%d", ar2.currentline);
			errorMessage += " line ";
			errorMessage += line;
		}

		if (ar2.name != NULL)
		{
			errorMessage += "\nFunction name: ";
			errorMessage += ar2.name;
		}

		if (!cFunctionBeingCalled.empty())
		{
			errorMessage += cFunctionBeingCalled;
		}
	}

} // namespace shh