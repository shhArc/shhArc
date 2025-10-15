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

#ifndef LUAAPITEMPLATE_H
#define LUAAPITEMPLATE_H

#include "../Config/GCPtr.h"
#include "../Common/SecureStl.h"
#include "../Common/TypeLog.h"
#include "../Arc/Type.h"
#include "../LuaProcess/LuaType.h"
#include "LuaWrapper.h"
#include "LuaProcess.h"
#include "LuaType.h"
#include "LuaApi.h"


#include <string>


namespace shh {

	class LuaApiTemplate
	{
	public:

		template<class T> static void GetArgument(lua_State* L, int arg, GCPtr<T>& userPtr);
		template<class T> static void GetArgument(lua_State* L, int arg, T*& userType);
		template<class T> static T* Return(lua_State* L, const T& userType);
		template<class T> static unsigned int RegisterType(const T* example, const std::string& name, StringFunction stringer = NULL, ValueFunction valuer = NULL, bool allowInterVM = true, bool regsiterConstructors = true);
		template<class T> static void RegisterMemberFunction(const T* dummy, const std::string& name, LuaCFunction fn, void* data = NULL, LuaTypeId dataType = 0);
		template<class T> static void RegisterVariable(const std::string& name, const T& value);
		template <class T> static bool GetVariable(const std::string& tableName, const std::string& s, T& ret);
		template < class T > static bool SetVariable(const std::string& tableName, const std::string& s, const T& val, bool overWrite);
	};

	// --------------------------------------------------------------------------						
	// Function:	GetArgument
	// Description:	Gets the argument number asked for of type requested.
	// Arguments:	lua state, args number, value of type to return
	//				throws error if fails
	// Returns:		none but referenced argument is set
	// --------------------------------------------------------------------------
	template<class T> void LuaApiTemplate::GetArgument(lua_State* L, int arg, GCPtr<T>& userPtr)
	{
		if (LuaGetStackSize(L) < arg || LuaGetStackTypeId(L, arg) >= 0)
			LuaApi::ThrowScriptError("Invalid argument %d.", arg + 1);

		GCPtr<T>* u;
		LuaType< GCPtr<T> >::GetArgument(L, arg, u);
		userPtr = *u;
	}
	template<class T> void LuaApiTemplate::GetArgument(lua_State* L, int arg, T*& userType)
	{
		if (LuaGetStackSize(L) < arg || LuaGetStackTypeId(L, arg) >= 0)
			LuaApi::ThrowScriptError("Invalid argument %d.", arg + 1);

		userType = NULL;
		LuaType<T>::GetArgument(L, arg, userType);
	}


	// --------------------------------------------------------------------------						
	// Function:	Return
	// Description:	Returns value of templated type onto lua state
	// Arguments:	lus state, object of type to be pushed
	// Returns:		pointer to new data type pushed, else NULL
	// --------------------------------------------------------------------------						
	template<class T> T* LuaApiTemplate::Return(lua_State* L, const T& userType)
	{
		return LuaType<T>::Push(L, userType);
	}


	// --------------------------------------------------------------------------						
	// Function:	RegisterType
	// Description:	Registers a user data type with the process
	// Arguments:	example of user data type
	//				name as appears in lua script
	//				function to be used to convert value to string
	//				function to be used to convert string to value
	//				whether to allow this type to be passed between
	//				VMs when messaging
	//				whether to add default constructors
	// Returns:		type id on register type
	// --------------------------------------------------------------------------
	template<class T> unsigned int LuaApiTemplate::RegisterType(const T* example, const std::string& name, StringFunction stringer, ValueFunction valuer, bool allowInterVM, bool regsiterConstructors)
	{
		int id = LuaType< T >::Register(name, stringer, valuer, allowInterVM, regsiterConstructors);
		TypeLog<T>::Register(id, Type<T>::GetStatic());
		TypeLog<T>::Syncronize();
		return id;
	}


	// --------------------------------------------------------------------------						
	// Function:	RegisterMemberFunction
	// Description:	Registers a member function in the current process
	// Arguments:	example to data type function is to be a member of
	//				name of function, function, upvalue data
	//				for closure, type of the upvalue data
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class T> void LuaApiTemplate::RegisterMemberFunction (const T* dummy, const std::string& name, LuaCFunction fn, void* data, LuaTypeId dataType)
	{
		LuaType< T >::RegisterMemberFunction(name, fn, data, dataType);
	}


	// --------------------------------------------------------------------------						
	// Function:	RegisterVariable
	// Description:	registers a variable with the process
	// Arguments:	variable name, variable value;
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class T> void LuaApiTemplate::RegisterVariable(const std::string& name, const T& value)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		LuaGetGlobalsStack(L);
		LuaType<T>::SetVariable(*LuaGetStackValue(L, -1), name, value, true);
		LuaDecStack(L);
	}


	// --------------------------------------------------------------------------						
	// Function:	GetVariable
	// Description:	gets a variable from a table
	// Arguments:	table name, variable name, reference of value to return
	// Returns:		if successful
	// --------------------------------------------------------------------------
	template <class T> bool LuaApiTemplate::GetVariable(const std::string& tableName, const std::string& s, T& ret)
	{
		return LuaType<T>::GetVariable(tableName, s, ret, LuaApi::GetCurrentLuaProcess());
	}


	// --------------------------------------------------------------------------						
	// Function:	SetVariable
	// Description:	sets a variable in a tavle
	// Arguments:	table name, variable name, value to set, whether to overwrite
	//				if variable already exists
	// Returns:		if successful
	// --------------------------------------------------------------------------
	template < class T > bool LuaApiTemplate::SetVariable(const std::string& tableName, const std::string& s, const T& val, bool overWrite)
	{
		return LuaType<T>::SetVariable(tableName, s, val, LuaApi::GetCurrentLuaProcess(), overWrite);
	}




} // namespace shh

#endif