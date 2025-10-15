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

#include "LuaType.h"
#include "LuaProcess.h"

namespace shh {

	LuaTypeBase::GarbageCheckers LuaTypeBase::ourGarbageCheckers;
	Registry::OverloadTable::SharedTypes LuaTypeBase::ourSharedTypes;


	// LyaTypeBase ///////////////////////////////////////////////////////////////

	// --------------------------------------------------------------------------						
	// Function:	SetCastableTypes
	// Description:	sets types which can be cast to each other
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void LuaTypeBase::SetCastableTypes()
	{
		Registry::OverloadTable::ArgumentTypes st;
		st.push_back(Type <float>::GetTypeId());
		ourSharedTypes[Type < double>::GetTypeId()] = st;
		st.clear();

		st.push_back(Type <double>::GetTypeId());
		ourSharedTypes[Type <float>::GetTypeId()] = st;
		st.clear();

		st.push_back(Type <unsigned int>::GetTypeId());
		st.push_back(Type <long>::GetTypeId());
		st.push_back(Type <unsigned long>::GetTypeId());
		st.push_back(Type <long long>::GetTypeId());
		st.push_back(Type <unsigned long long>::GetTypeId());
		ourSharedTypes[Type <int>::GetTypeId()] = st;
		st.clear();

		st.push_back(Type <int>::GetTypeId());
		st.push_back(Type <long>::GetTypeId());
		st.push_back(Type <unsigned long>::GetTypeId());
		st.push_back(Type <long long>::GetTypeId());
		st.push_back(Type <unsigned long long>::GetTypeId());
		ourSharedTypes[Type <unsigned int>::GetTypeId()] = st;
		st.clear();

		st.push_back(Type <int>::GetTypeId());
		st.push_back(Type <unsigned int>::GetTypeId());
		st.push_back(Type <unsigned long>::GetTypeId());
		st.push_back(Type <long long>::GetTypeId());
		st.push_back(Type <unsigned long long>::GetTypeId());
		ourSharedTypes[Type <long>::GetTypeId()] = st;
		st.clear();

		st.push_back(Type <int>::GetTypeId());
		st.push_back(Type <unsigned int>::GetTypeId());
		st.push_back(Type <long>::GetTypeId());
		st.push_back(Type <long long>::GetTypeId());
		st.push_back(Type <unsigned long long>::GetTypeId());
		ourSharedTypes[Type <unsigned long>::GetTypeId()] = st;
		st.clear();

		st.push_back(Type <int>::GetTypeId());
		st.push_back(Type <unsigned int>::GetTypeId());
		st.push_back(Type <long>::GetTypeId());
		st.push_back(Type <unsigned long>::GetTypeId());
		st.push_back(Type <unsigned long long>::GetTypeId());
		ourSharedTypes[Type <long long>::GetTypeId()] = st;
		st.clear();

		st.push_back(Type <int>::GetTypeId());
		st.push_back(Type <unsigned int>::GetTypeId());
		st.push_back(Type <long>::GetTypeId());
		st.push_back(Type <unsigned long>::GetTypeId());
		st.push_back(Type <long long>::GetTypeId());
		ourSharedTypes[Type <unsigned long long>::GetTypeId()] = st;
		st.clear();

		return;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetTable
	// Description:	get table of name from global table
	// Arguments:	name ("" = global table), table got, LuaProcess to get from
	// Returns:		true if sucseeds
	// --------------------------------------------------------------------------						
	bool LuaTypeBase::GetTable(const std::string& tableName, TValue& table, GCPtr<LuaProcess>& luaProcess) 
	{ 
		return luaProcess->GetTable(tableName, table); 
	}


	// --------------------------------------------------------------------------						
	// Function:	PushString
	// Description:	pushes string lua state,
	// Arguments:	lua state, string to push
	// Returns:		none
	// --------------------------------------------------------------------------
	void LuaTypeBase::PushString(lua_State* L, const std::string& value) 
	{ 
		lua_pushstring(L, GetString(&value)); 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetLuaState
	// Description:	returns the lua state with current LuaProcess
	// Arguments:	if to throw an error message when fails
	// Returns:		lua state requested
	// --------------------------------------------------------------------------	
	lua_State* LuaTypeBase::GetLuaState(bool throwError) 
	{ 
		return LuaProcess::GetCurrentLuaState(throwError); 
	}


	
	
} // namespace shh