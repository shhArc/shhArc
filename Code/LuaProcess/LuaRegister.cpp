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



#include "../Common/TypeLog.h"
#include "../VM/Parameters.h"
#include "LuaApi.h"
#include "LuaApi.h"


namespace shh {

const std::type_info &HeadType( int length, NullType* )
{
	static const NullType n = NullType();	
	static const std::type_info &typeInfo = typeid( n );
	return typeInfo;
}



// CALL 0
// version for non member and static member calling
template<  typename F>
class LuaCall0 : public Registry::CallInterface
{
public:

	LuaCall0(std::string fumctionName, F funmction, unsigned int firstReturnArg) :
		myFunctionName(fumctionName),
		myFunction(funmction),
		myFirstReturnArg(firstReturnArg)
	{}

	virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
	virtual void* GetFunction() { return myFunction; }

	template< typename F > ExecutionState CallPolicy(void* data) const
	{
		LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
		dispatchData->numReturnValues = 0;
		return myFunction();
	}


private:

	std::string myFunctionName;
	F myFunction;
	unsigned int myFirstReturnArg;
};


// --------------------------------------------------------------------------						
// Function:	LuaRegisterFunction
// Description:	registers a C function that takes not arguments
// Arguments:	lua state,  function name, C function to register, user data 
//				attached to function, type of user data attached
// Returns:		if successful
// --------------------------------------------------------------------------
static void LuaRegisterFunctionX(lua_State* L, const std::string name, LuaCFunction& fn, void* data, LuaTypeId dataType)
{
	lua_pushglobaltable(L);
	if (data != NULL)
	{
		void* p = lua_newuserdata(L, sizeof(shh::LuaUserData));
		shh::LuaUserData* lud = static_cast<shh::LuaUserData*>(p);
		lud->myShhType = (void*)shh::LuaUserData::ourMarker;
		lud->myData = data;
		const shh::BaseType* bt = shh::Registry::GetRegistry().GetType(dataType);
		DEBUG_ASSERT(bt);
		if (bt == NULL)
		{
			// IF IT IS NOT A SHH LUATYPE IT CANT BE CLONED
			lud->myTypeId = 0;
			lud->myType = NULL;
		}
		else
		{
			lud->myTypeId = dataType;
			lud->myType = bt;
			int ok = luaL_getmetatable(L, bt->GetName().c_str());
			DEBUG_ASSERT(ok);
			lua_setmetatable(L, -2);
		}
		lua_pushcclosure(L, fn, 1);
	}
	else
	{
		lua_pushcfunction(L, fn);
	}
	lua_setfield(L, -2, name.c_str());
	L->top.p--;
}



// --------------------------------------------------------------------------						
// Function:	RegisterLuaFunction
// Description:	Registers a function in the current process
// Arguments:	name of function, function, upvalue data
//				for closure, type of the upvalue data
// Returns:		none
// --------------------------------------------------------------------------						
void RegisterLuaFunction(const std::string& name, LuaCFunction fn, void* data, LuaTypeId dataType)
{
	LuaRegisterFunctionX(LuaApi::GetCurrentLuaState(), name.c_str(), fn, data, dataType);
}




void RegisterLuaFunction(const std::string& functionName, ExecutionState(*f)(), unsigned int firstReturnArg, const GCPtr<Module>& module)
{
	lua_State* L = LuaApi::GetCurrentLuaState();
	typedef ExecutionState(*Function)();
	typedef TYPELIST_0 Arguments;
	typedef Parameters< Function, Arguments > FunctionType;
	typedef LuaCall0< Function > Call;
	GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
	FunctionType parameter(f, firstReturnArg);
	Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
	GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
	if (!ok.IsValid())
	{
		std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
		THROW(msg.c_str());
	}
	GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
	RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
	return;
}


// --------------------------------------------------------------------------						
// Function:	RegisterLuaMemberFunction
// Description:	Registers a member function in the current process
// Arguments:	example of owner type of the member function
//				name of function, function, upvalue data
//				for closure, type of the upvalue data
// Returns:		none
// --------------------------------------------------------------------------
template< typename C >
void RegisterLuaMemberFunction(const C* ownerExampe, const std::string& functionName, ExecutionState(*f)(), unsigned int firstReturnArg, const GCPtr<Module>& module)
{
	lua_State* L = LuaApi::GetCurrentLuaState();
	typedef ExecutionState(*Function)();
	typedef TYPELIST_0 Arguments;
	typedef Parameters< Function, Arguments > FunctionType;
	typedef LuaCall0< Function > Call;
	GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
	FunctionType parameter(f, firstReturnArg);
	Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
	GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
	if (!ok.IsValid())
	{
		std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
		THROW(msg.c_str());
	}
	GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
	RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
	return;
}
}//namespace shh
