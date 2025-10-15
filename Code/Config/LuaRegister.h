///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 David K Bhowmik. All rights reserved.
//
/// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUAREGISTER_H
#define LUAREGISTER_H

#include "../Common/SecureStl.h"
#include "../Common/TypeLog.h"
#include "../VM/Parameters.h"
#include "../LuaProcess/LuaParameters.h"
#include "../Common/TypeList.h"
#include "../LuaProcess/LuaDispatchData.h"
#include "../LuaProcess/LuaWrapper.h"
#include <Lua/src/lstate.h>
#include <Lua/src/lapi.h>
#define luaL_getmetatable(L,n)	(lua_getfield(L, LUA_REGISTRYINDEX, (n)))


namespace shh
{


	//////////////////////////////////////////////////////////////////////////////////////////////
	// Lua C type function taking a lua_State as argument
	void LuaRegisterFunction(lua_State* L, const std::string name, LuaCFunction& fn, void* data, LuaTypeId dataType);
	void RegisterLuaFunction(const std::string& name, LuaCFunction fn, void* data = NULL, LuaTypeId dataType = 0);

	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall0

	void RegisterLuaFunction(const std::string& functionName, ExecutionState(*f)(), unsigned int firstReturnArg, const GCPtr<Module>& module);
	template<class T> inline void RegisterLuaMemberFunction(const T* dummy, const std::string& name, LuaCFunction fn, void* data = NULL, LuaTypeId dataType = 0)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		int m = luaL_getmetatable(L, TypeLog<T>::GetTypeName().c_str());

		if (data != NULL)
		{
			void* p = lua_newuserdata(L, sizeof(LuaUserData));
			LuaUserData* lud = static_cast<LuaUserData*>(p);
			lud->myShhType = (void*)LuaUserData::ourMarker;
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

	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall1

	template<  typename F, typename P1 >
	class LuaCall1 : public Registry::CallInterface
	{
	public:

		LuaCall1(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 1 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&);
		typedef TYPELIST_1(P1) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall1< Function, P1 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&);
		typedef TYPELIST_1(P1) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall1< Function, P1 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall2

	template<  typename F, typename P1, typename P2 >
	class LuaCall2 : public Registry::CallInterface
	{
	public:

		LuaCall2(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 2 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&);
		typedef TYPELIST_2(P1, P2) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall2< Function, P1, P2 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&);
		typedef TYPELIST_2(P1, P2) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall2< Function, P1, P2 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall3

	template<  typename F, typename P1, typename P2, typename P3 >
	class LuaCall3 : public Registry::CallInterface
	{
	public:

		LuaCall3(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 3 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&);
		typedef TYPELIST_3(P1, P2, P3) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall3< Function, P1, P2, P3 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&);
		typedef TYPELIST_3(P1, P2, P3) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall3< Function, P1, P2, P3 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall4

	template<  typename F, typename P1, typename P2, typename P3, typename P4 >
	class LuaCall4 : public Registry::CallInterface
	{
	public:

		LuaCall4(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 4 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&);
		typedef TYPELIST_4(P1, P2, P3, P4) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall4< Function, P1, P2, P3, P4 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&);
		typedef TYPELIST_4(P1, P2, P3, P4) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall4< Function, P1, P2, P3, P4 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall5

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5 >
	class LuaCall5 : public Registry::CallInterface
	{
	public:

		LuaCall5(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 5 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&);
		typedef TYPELIST_5(P1, P2, P3, P4, P5) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall5< Function, P1, P2, P3, P4, P5 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&);
		typedef TYPELIST_5(P1, P2, P3, P4, P5) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall5< Function, P1, P2, P3, P4, P5 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall6

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6 >
	class LuaCall6 : public Registry::CallInterface
	{
	public:

		LuaCall6(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 6 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&);
		typedef TYPELIST_6(P1, P2, P3, P4, P5, P6) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall6< Function, P1, P2, P3, P4, P5, P6 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&);
		typedef TYPELIST_6(P1, P2, P3, P4, P5, P6) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall6< Function, P1, P2, P3, P4, P5, P6 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall7

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7 >
	class LuaCall7 : public Registry::CallInterface
	{
	public:

		LuaCall7(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 7 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&);
		typedef TYPELIST_7(P1, P2, P3, P4, P5, P6, P7) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall7< Function, P1, P2, P3, P4, P5, P6, P7 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&);
		typedef TYPELIST_7(P1, P2, P3, P4, P5, P6, P7) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall7< Function, P1, P2, P3, P4, P5, P6, P7 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall8

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8 >
	class LuaCall8 : public Registry::CallInterface
	{
	public:

		LuaCall8(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 8 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&);
		typedef TYPELIST_8(P1, P2, P3, P4, P5, P6, P7, P8) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall8< Function, P1, P2, P3, P4, P5, P6, P7, P8 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&);
		typedef TYPELIST_8(P1, P2, P3, P4, P5, P6, P7, P8) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall8< Function, P1, P2, P3, P4, P5, P6, P7, P8 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall9

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9 >
	class LuaCall9 : public Registry::CallInterface
	{
	public:

		LuaCall9(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 9 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&);
		typedef TYPELIST_9(P1, P2, P3, P4, P5, P6, P7, P8, P9) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall9< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&);
		typedef TYPELIST_9(P1, P2, P3, P4, P5, P6, P7, P8, P9) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall9< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall10

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10 >
	class LuaCall10 : public Registry::CallInterface
	{
	public:

		LuaCall10(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 10 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&);
		typedef TYPELIST_10(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall10< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&);
		typedef TYPELIST_10(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall10< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall11

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11 >
	class LuaCall11 : public Registry::CallInterface
	{
	public:

		LuaCall11(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 11 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&);
		typedef TYPELIST_11(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall11< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&);
		typedef TYPELIST_11(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall11< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall12

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12 >
	class LuaCall12 : public Registry::CallInterface
	{
	public:

		LuaCall12(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 12 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&);
		typedef TYPELIST_12(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall12< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&);
		typedef TYPELIST_12(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall12< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall13

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13 >
	class LuaCall13 : public Registry::CallInterface
	{
	public:

		LuaCall13(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 13 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&);
		typedef TYPELIST_13(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall13< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&);
		typedef TYPELIST_13(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall13< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall14

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14 >
	class LuaCall14 : public Registry::CallInterface
	{
	public:

		LuaCall14(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 14 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&);
		typedef TYPELIST_14(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall14< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&);
		typedef TYPELIST_14(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall14< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall15

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15 >
	class LuaCall15 : public Registry::CallInterface
	{
	public:

		LuaCall15(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 15 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&);
		typedef TYPELIST_15(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall15< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&);
		typedef TYPELIST_15(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall15< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall16

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16 >
	class LuaCall16 : public Registry::CallInterface
	{
	public:

		LuaCall16(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 16 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&);
		typedef TYPELIST_16(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall16< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&);
		typedef TYPELIST_16(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall16< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall17

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17 >
	class LuaCall17 : public Registry::CallInterface
	{
	public:

		LuaCall17(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 17 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&);
		typedef TYPELIST_17(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall17< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&);
		typedef TYPELIST_17(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall17< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall18

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18 >
	class LuaCall18 : public Registry::CallInterface
	{
	public:

		LuaCall18(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 18 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&);
		typedef TYPELIST_18(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall18< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&);
		typedef TYPELIST_18(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall18< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall19

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19 >
	class LuaCall19 : public Registry::CallInterface
	{
	public:

		LuaCall19(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 19 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&);
		typedef TYPELIST_19(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall19< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&);
		typedef TYPELIST_19(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall19< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall20

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20 >
	class LuaCall20 : public Registry::CallInterface
	{
	public:

		LuaCall20(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 20 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			P20 p20; P20* s20 = LuaParameters::Set(dispatchData->L, p20, 20, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19, *s20);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			LuaParameters::Return(s20, 20, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&);
		typedef TYPELIST_20(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall20< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&);
		typedef TYPELIST_20(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall20< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall21

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21 >
	class LuaCall21 : public Registry::CallInterface
	{
	public:

		LuaCall21(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 21 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			P20 p20; P20* s20 = LuaParameters::Set(dispatchData->L, p20, 20, myFirstReturnArg); 
			P21 p21; P21* s21 = LuaParameters::Set(dispatchData->L, p21, 21, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19, *s20, *s21);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			LuaParameters::Return(s20, 20, myFirstReturnArg);
			LuaParameters::Return(s21, 21, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&);
		typedef TYPELIST_21(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall21< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&);
		typedef TYPELIST_21(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall21< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall22

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22 >
	class LuaCall22 : public Registry::CallInterface
	{
	public:

		LuaCall22(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 22 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			P20 p20; P20* s20 = LuaParameters::Set(dispatchData->L, p20, 20, myFirstReturnArg); 
			P21 p21; P21* s21 = LuaParameters::Set(dispatchData->L, p21, 21, myFirstReturnArg); 
			P22 p22; P22* s22 = LuaParameters::Set(dispatchData->L, p22, 22, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19, *s20, *s21, *s22);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			LuaParameters::Return(s20, 20, myFirstReturnArg);
			LuaParameters::Return(s21, 21, myFirstReturnArg);
			LuaParameters::Return(s22, 22, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&);
		typedef TYPELIST_22(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall22< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&);
		typedef TYPELIST_22(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall22< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall23

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23 >
	class LuaCall23 : public Registry::CallInterface
	{
	public:

		LuaCall23(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 23 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			P20 p20; P20* s20 = LuaParameters::Set(dispatchData->L, p20, 20, myFirstReturnArg); 
			P21 p21; P21* s21 = LuaParameters::Set(dispatchData->L, p21, 21, myFirstReturnArg); 
			P22 p22; P22* s22 = LuaParameters::Set(dispatchData->L, p22, 22, myFirstReturnArg); 
			P23 p23; P23* s23 = LuaParameters::Set(dispatchData->L, p23, 23, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19, *s20, *s21, *s22, *s23);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			LuaParameters::Return(s20, 20, myFirstReturnArg);
			LuaParameters::Return(s21, 21, myFirstReturnArg);
			LuaParameters::Return(s22, 22, myFirstReturnArg);
			LuaParameters::Return(s23, 23, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&);
		typedef TYPELIST_23(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall23< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&);
		typedef TYPELIST_23(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall23< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall24

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24 >
	class LuaCall24 : public Registry::CallInterface
	{
	public:

		LuaCall24(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 24 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			P20 p20; P20* s20 = LuaParameters::Set(dispatchData->L, p20, 20, myFirstReturnArg); 
			P21 p21; P21* s21 = LuaParameters::Set(dispatchData->L, p21, 21, myFirstReturnArg); 
			P22 p22; P22* s22 = LuaParameters::Set(dispatchData->L, p22, 22, myFirstReturnArg); 
			P23 p23; P23* s23 = LuaParameters::Set(dispatchData->L, p23, 23, myFirstReturnArg); 
			P24 p24; P24* s24 = LuaParameters::Set(dispatchData->L, p24, 24, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19, *s20, *s21, *s22, *s23, *s24);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			LuaParameters::Return(s20, 20, myFirstReturnArg);
			LuaParameters::Return(s21, 21, myFirstReturnArg);
			LuaParameters::Return(s22, 22, myFirstReturnArg);
			LuaParameters::Return(s23, 23, myFirstReturnArg);
			LuaParameters::Return(s24, 24, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&);
		typedef TYPELIST_24(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall24< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&);
		typedef TYPELIST_24(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall24< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall25

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25 >
	class LuaCall25 : public Registry::CallInterface
	{
	public:

		LuaCall25(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 25 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			P20 p20; P20* s20 = LuaParameters::Set(dispatchData->L, p20, 20, myFirstReturnArg); 
			P21 p21; P21* s21 = LuaParameters::Set(dispatchData->L, p21, 21, myFirstReturnArg); 
			P22 p22; P22* s22 = LuaParameters::Set(dispatchData->L, p22, 22, myFirstReturnArg); 
			P23 p23; P23* s23 = LuaParameters::Set(dispatchData->L, p23, 23, myFirstReturnArg); 
			P24 p24; P24* s24 = LuaParameters::Set(dispatchData->L, p24, 24, myFirstReturnArg); 
			P25 p25; P25* s25 = LuaParameters::Set(dispatchData->L, p25, 25, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19, *s20, *s21, *s22, *s23, *s24, *s25);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			LuaParameters::Return(s20, 20, myFirstReturnArg);
			LuaParameters::Return(s21, 21, myFirstReturnArg);
			LuaParameters::Return(s22, 22, myFirstReturnArg);
			LuaParameters::Return(s23, 23, myFirstReturnArg);
			LuaParameters::Return(s24, 24, myFirstReturnArg);
			LuaParameters::Return(s25, 25, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&);
		typedef TYPELIST_25(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall25< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&);
		typedef TYPELIST_25(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall25< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall26

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26 >
	class LuaCall26 : public Registry::CallInterface
	{
	public:

		LuaCall26(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 26 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			P20 p20; P20* s20 = LuaParameters::Set(dispatchData->L, p20, 20, myFirstReturnArg); 
			P21 p21; P21* s21 = LuaParameters::Set(dispatchData->L, p21, 21, myFirstReturnArg); 
			P22 p22; P22* s22 = LuaParameters::Set(dispatchData->L, p22, 22, myFirstReturnArg); 
			P23 p23; P23* s23 = LuaParameters::Set(dispatchData->L, p23, 23, myFirstReturnArg); 
			P24 p24; P24* s24 = LuaParameters::Set(dispatchData->L, p24, 24, myFirstReturnArg); 
			P25 p25; P25* s25 = LuaParameters::Set(dispatchData->L, p25, 25, myFirstReturnArg); 
			P26 p26; P26* s26 = LuaParameters::Set(dispatchData->L, p26, 26, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19, *s20, *s21, *s22, *s23, *s24, *s25, *s26);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			LuaParameters::Return(s20, 20, myFirstReturnArg);
			LuaParameters::Return(s21, 21, myFirstReturnArg);
			LuaParameters::Return(s22, 22, myFirstReturnArg);
			LuaParameters::Return(s23, 23, myFirstReturnArg);
			LuaParameters::Return(s24, 24, myFirstReturnArg);
			LuaParameters::Return(s25, 25, myFirstReturnArg);
			LuaParameters::Return(s26, 26, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&);
		typedef TYPELIST_26(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall26< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&);
		typedef TYPELIST_26(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall26< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall27

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27 >
	class LuaCall27 : public Registry::CallInterface
	{
	public:

		LuaCall27(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 27 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			P20 p20; P20* s20 = LuaParameters::Set(dispatchData->L, p20, 20, myFirstReturnArg); 
			P21 p21; P21* s21 = LuaParameters::Set(dispatchData->L, p21, 21, myFirstReturnArg); 
			P22 p22; P22* s22 = LuaParameters::Set(dispatchData->L, p22, 22, myFirstReturnArg); 
			P23 p23; P23* s23 = LuaParameters::Set(dispatchData->L, p23, 23, myFirstReturnArg); 
			P24 p24; P24* s24 = LuaParameters::Set(dispatchData->L, p24, 24, myFirstReturnArg); 
			P25 p25; P25* s25 = LuaParameters::Set(dispatchData->L, p25, 25, myFirstReturnArg); 
			P26 p26; P26* s26 = LuaParameters::Set(dispatchData->L, p26, 26, myFirstReturnArg); 
			P27 p27; P27* s27 = LuaParameters::Set(dispatchData->L, p27, 27, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19, *s20, *s21, *s22, *s23, *s24, *s25, *s26, *s27);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			LuaParameters::Return(s20, 20, myFirstReturnArg);
			LuaParameters::Return(s21, 21, myFirstReturnArg);
			LuaParameters::Return(s22, 22, myFirstReturnArg);
			LuaParameters::Return(s23, 23, myFirstReturnArg);
			LuaParameters::Return(s24, 24, myFirstReturnArg);
			LuaParameters::Return(s25, 25, myFirstReturnArg);
			LuaParameters::Return(s26, 26, myFirstReturnArg);
			LuaParameters::Return(s27, 27, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&);
		typedef TYPELIST_27(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall27< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&);
		typedef TYPELIST_27(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall27< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall28

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28 >
	class LuaCall28 : public Registry::CallInterface
	{
	public:

		LuaCall28(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 28 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			P20 p20; P20* s20 = LuaParameters::Set(dispatchData->L, p20, 20, myFirstReturnArg); 
			P21 p21; P21* s21 = LuaParameters::Set(dispatchData->L, p21, 21, myFirstReturnArg); 
			P22 p22; P22* s22 = LuaParameters::Set(dispatchData->L, p22, 22, myFirstReturnArg); 
			P23 p23; P23* s23 = LuaParameters::Set(dispatchData->L, p23, 23, myFirstReturnArg); 
			P24 p24; P24* s24 = LuaParameters::Set(dispatchData->L, p24, 24, myFirstReturnArg); 
			P25 p25; P25* s25 = LuaParameters::Set(dispatchData->L, p25, 25, myFirstReturnArg); 
			P26 p26; P26* s26 = LuaParameters::Set(dispatchData->L, p26, 26, myFirstReturnArg); 
			P27 p27; P27* s27 = LuaParameters::Set(dispatchData->L, p27, 27, myFirstReturnArg); 
			P28 p28; P28* s28 = LuaParameters::Set(dispatchData->L, p28, 28, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19, *s20, *s21, *s22, *s23, *s24, *s25, *s26, *s27, *s28);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			LuaParameters::Return(s20, 20, myFirstReturnArg);
			LuaParameters::Return(s21, 21, myFirstReturnArg);
			LuaParameters::Return(s22, 22, myFirstReturnArg);
			LuaParameters::Return(s23, 23, myFirstReturnArg);
			LuaParameters::Return(s24, 24, myFirstReturnArg);
			LuaParameters::Return(s25, 25, myFirstReturnArg);
			LuaParameters::Return(s26, 26, myFirstReturnArg);
			LuaParameters::Return(s27, 27, myFirstReturnArg);
			LuaParameters::Return(s28, 28, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&);
		typedef TYPELIST_28(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall28< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&);
		typedef TYPELIST_28(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall28< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall29

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29 >
	class LuaCall29 : public Registry::CallInterface
	{
	public:

		LuaCall29(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 29 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			P20 p20; P20* s20 = LuaParameters::Set(dispatchData->L, p20, 20, myFirstReturnArg); 
			P21 p21; P21* s21 = LuaParameters::Set(dispatchData->L, p21, 21, myFirstReturnArg); 
			P22 p22; P22* s22 = LuaParameters::Set(dispatchData->L, p22, 22, myFirstReturnArg); 
			P23 p23; P23* s23 = LuaParameters::Set(dispatchData->L, p23, 23, myFirstReturnArg); 
			P24 p24; P24* s24 = LuaParameters::Set(dispatchData->L, p24, 24, myFirstReturnArg); 
			P25 p25; P25* s25 = LuaParameters::Set(dispatchData->L, p25, 25, myFirstReturnArg); 
			P26 p26; P26* s26 = LuaParameters::Set(dispatchData->L, p26, 26, myFirstReturnArg); 
			P27 p27; P27* s27 = LuaParameters::Set(dispatchData->L, p27, 27, myFirstReturnArg); 
			P28 p28; P28* s28 = LuaParameters::Set(dispatchData->L, p28, 28, myFirstReturnArg); 
			P29 p29; P29* s29 = LuaParameters::Set(dispatchData->L, p29, 29, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19, *s20, *s21, *s22, *s23, *s24, *s25, *s26, *s27, *s28, *s29);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			LuaParameters::Return(s20, 20, myFirstReturnArg);
			LuaParameters::Return(s21, 21, myFirstReturnArg);
			LuaParameters::Return(s22, 22, myFirstReturnArg);
			LuaParameters::Return(s23, 23, myFirstReturnArg);
			LuaParameters::Return(s24, 24, myFirstReturnArg);
			LuaParameters::Return(s25, 25, myFirstReturnArg);
			LuaParameters::Return(s26, 26, myFirstReturnArg);
			LuaParameters::Return(s27, 27, myFirstReturnArg);
			LuaParameters::Return(s28, 28, myFirstReturnArg);
			LuaParameters::Return(s29, 29, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&);
		typedef TYPELIST_29(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall29< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&);
		typedef TYPELIST_29(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall29< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall30

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30 >
	class LuaCall30 : public Registry::CallInterface
	{
	public:

		LuaCall30(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 30 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			P20 p20; P20* s20 = LuaParameters::Set(dispatchData->L, p20, 20, myFirstReturnArg); 
			P21 p21; P21* s21 = LuaParameters::Set(dispatchData->L, p21, 21, myFirstReturnArg); 
			P22 p22; P22* s22 = LuaParameters::Set(dispatchData->L, p22, 22, myFirstReturnArg); 
			P23 p23; P23* s23 = LuaParameters::Set(dispatchData->L, p23, 23, myFirstReturnArg); 
			P24 p24; P24* s24 = LuaParameters::Set(dispatchData->L, p24, 24, myFirstReturnArg); 
			P25 p25; P25* s25 = LuaParameters::Set(dispatchData->L, p25, 25, myFirstReturnArg); 
			P26 p26; P26* s26 = LuaParameters::Set(dispatchData->L, p26, 26, myFirstReturnArg); 
			P27 p27; P27* s27 = LuaParameters::Set(dispatchData->L, p27, 27, myFirstReturnArg); 
			P28 p28; P28* s28 = LuaParameters::Set(dispatchData->L, p28, 28, myFirstReturnArg); 
			P29 p29; P29* s29 = LuaParameters::Set(dispatchData->L, p29, 29, myFirstReturnArg); 
			P30 p30; P30* s30 = LuaParameters::Set(dispatchData->L, p30, 30, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19, *s20, *s21, *s22, *s23, *s24, *s25, *s26, *s27, *s28, *s29, *s30);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			LuaParameters::Return(s20, 20, myFirstReturnArg);
			LuaParameters::Return(s21, 21, myFirstReturnArg);
			LuaParameters::Return(s22, 22, myFirstReturnArg);
			LuaParameters::Return(s23, 23, myFirstReturnArg);
			LuaParameters::Return(s24, 24, myFirstReturnArg);
			LuaParameters::Return(s25, 25, myFirstReturnArg);
			LuaParameters::Return(s26, 26, myFirstReturnArg);
			LuaParameters::Return(s27, 27, myFirstReturnArg);
			LuaParameters::Return(s28, 28, myFirstReturnArg);
			LuaParameters::Return(s29, 29, myFirstReturnArg);
			LuaParameters::Return(s30, 30, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&);
		typedef TYPELIST_30(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall30< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&);
		typedef TYPELIST_30(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall30< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall31

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31 >
	class LuaCall31 : public Registry::CallInterface
	{
	public:

		LuaCall31(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 31 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			P20 p20; P20* s20 = LuaParameters::Set(dispatchData->L, p20, 20, myFirstReturnArg); 
			P21 p21; P21* s21 = LuaParameters::Set(dispatchData->L, p21, 21, myFirstReturnArg); 
			P22 p22; P22* s22 = LuaParameters::Set(dispatchData->L, p22, 22, myFirstReturnArg); 
			P23 p23; P23* s23 = LuaParameters::Set(dispatchData->L, p23, 23, myFirstReturnArg); 
			P24 p24; P24* s24 = LuaParameters::Set(dispatchData->L, p24, 24, myFirstReturnArg); 
			P25 p25; P25* s25 = LuaParameters::Set(dispatchData->L, p25, 25, myFirstReturnArg); 
			P26 p26; P26* s26 = LuaParameters::Set(dispatchData->L, p26, 26, myFirstReturnArg); 
			P27 p27; P27* s27 = LuaParameters::Set(dispatchData->L, p27, 27, myFirstReturnArg); 
			P28 p28; P28* s28 = LuaParameters::Set(dispatchData->L, p28, 28, myFirstReturnArg); 
			P29 p29; P29* s29 = LuaParameters::Set(dispatchData->L, p29, 29, myFirstReturnArg); 
			P30 p30; P30* s30 = LuaParameters::Set(dispatchData->L, p30, 30, myFirstReturnArg); 
			P31 p31; P31* s31 = LuaParameters::Set(dispatchData->L, p31, 31, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19, *s20, *s21, *s22, *s23, *s24, *s25, *s26, *s27, *s28, *s29, *s30, *s31);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			LuaParameters::Return(s20, 20, myFirstReturnArg);
			LuaParameters::Return(s21, 21, myFirstReturnArg);
			LuaParameters::Return(s22, 22, myFirstReturnArg);
			LuaParameters::Return(s23, 23, myFirstReturnArg);
			LuaParameters::Return(s24, 24, myFirstReturnArg);
			LuaParameters::Return(s25, 25, myFirstReturnArg);
			LuaParameters::Return(s26, 26, myFirstReturnArg);
			LuaParameters::Return(s27, 27, myFirstReturnArg);
			LuaParameters::Return(s28, 28, myFirstReturnArg);
			LuaParameters::Return(s29, 29, myFirstReturnArg);
			LuaParameters::Return(s30, 30, myFirstReturnArg);
			LuaParameters::Return(s31, 31, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&);
		typedef TYPELIST_31(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall31< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&);
		typedef TYPELIST_31(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall31< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall32

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32 >
	class LuaCall32 : public Registry::CallInterface
	{
	public:

		LuaCall32(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 32 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			P20 p20; P20* s20 = LuaParameters::Set(dispatchData->L, p20, 20, myFirstReturnArg); 
			P21 p21; P21* s21 = LuaParameters::Set(dispatchData->L, p21, 21, myFirstReturnArg); 
			P22 p22; P22* s22 = LuaParameters::Set(dispatchData->L, p22, 22, myFirstReturnArg); 
			P23 p23; P23* s23 = LuaParameters::Set(dispatchData->L, p23, 23, myFirstReturnArg); 
			P24 p24; P24* s24 = LuaParameters::Set(dispatchData->L, p24, 24, myFirstReturnArg); 
			P25 p25; P25* s25 = LuaParameters::Set(dispatchData->L, p25, 25, myFirstReturnArg); 
			P26 p26; P26* s26 = LuaParameters::Set(dispatchData->L, p26, 26, myFirstReturnArg); 
			P27 p27; P27* s27 = LuaParameters::Set(dispatchData->L, p27, 27, myFirstReturnArg); 
			P28 p28; P28* s28 = LuaParameters::Set(dispatchData->L, p28, 28, myFirstReturnArg); 
			P29 p29; P29* s29 = LuaParameters::Set(dispatchData->L, p29, 29, myFirstReturnArg); 
			P30 p30; P30* s30 = LuaParameters::Set(dispatchData->L, p30, 30, myFirstReturnArg); 
			P31 p31; P31* s31 = LuaParameters::Set(dispatchData->L, p31, 31, myFirstReturnArg); 
			P32 p32; P32* s32 = LuaParameters::Set(dispatchData->L, p32, 32, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19, *s20, *s21, *s22, *s23, *s24, *s25, *s26, *s27, *s28, *s29, *s30, *s31, *s32);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			LuaParameters::Return(s20, 20, myFirstReturnArg);
			LuaParameters::Return(s21, 21, myFirstReturnArg);
			LuaParameters::Return(s22, 22, myFirstReturnArg);
			LuaParameters::Return(s23, 23, myFirstReturnArg);
			LuaParameters::Return(s24, 24, myFirstReturnArg);
			LuaParameters::Return(s25, 25, myFirstReturnArg);
			LuaParameters::Return(s26, 26, myFirstReturnArg);
			LuaParameters::Return(s27, 27, myFirstReturnArg);
			LuaParameters::Return(s28, 28, myFirstReturnArg);
			LuaParameters::Return(s29, 29, myFirstReturnArg);
			LuaParameters::Return(s30, 30, myFirstReturnArg);
			LuaParameters::Return(s31, 31, myFirstReturnArg);
			LuaParameters::Return(s32, 32, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&);
		typedef TYPELIST_32(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall32< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&);
		typedef TYPELIST_32(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall32< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall33

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33 >
	class LuaCall33 : public Registry::CallInterface
	{
	public:

		LuaCall33(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 33 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			P20 p20; P20* s20 = LuaParameters::Set(dispatchData->L, p20, 20, myFirstReturnArg); 
			P21 p21; P21* s21 = LuaParameters::Set(dispatchData->L, p21, 21, myFirstReturnArg); 
			P22 p22; P22* s22 = LuaParameters::Set(dispatchData->L, p22, 22, myFirstReturnArg); 
			P23 p23; P23* s23 = LuaParameters::Set(dispatchData->L, p23, 23, myFirstReturnArg); 
			P24 p24; P24* s24 = LuaParameters::Set(dispatchData->L, p24, 24, myFirstReturnArg); 
			P25 p25; P25* s25 = LuaParameters::Set(dispatchData->L, p25, 25, myFirstReturnArg); 
			P26 p26; P26* s26 = LuaParameters::Set(dispatchData->L, p26, 26, myFirstReturnArg); 
			P27 p27; P27* s27 = LuaParameters::Set(dispatchData->L, p27, 27, myFirstReturnArg); 
			P28 p28; P28* s28 = LuaParameters::Set(dispatchData->L, p28, 28, myFirstReturnArg); 
			P29 p29; P29* s29 = LuaParameters::Set(dispatchData->L, p29, 29, myFirstReturnArg); 
			P30 p30; P30* s30 = LuaParameters::Set(dispatchData->L, p30, 30, myFirstReturnArg); 
			P31 p31; P31* s31 = LuaParameters::Set(dispatchData->L, p31, 31, myFirstReturnArg); 
			P32 p32; P32* s32 = LuaParameters::Set(dispatchData->L, p32, 32, myFirstReturnArg); 
			P33 p33; P33* s33 = LuaParameters::Set(dispatchData->L, p33, 33, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19, *s20, *s21, *s22, *s23, *s24, *s25, *s26, *s27, *s28, *s29, *s30, *s31, *s32, *s33);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			LuaParameters::Return(s20, 20, myFirstReturnArg);
			LuaParameters::Return(s21, 21, myFirstReturnArg);
			LuaParameters::Return(s22, 22, myFirstReturnArg);
			LuaParameters::Return(s23, 23, myFirstReturnArg);
			LuaParameters::Return(s24, 24, myFirstReturnArg);
			LuaParameters::Return(s25, 25, myFirstReturnArg);
			LuaParameters::Return(s26, 26, myFirstReturnArg);
			LuaParameters::Return(s27, 27, myFirstReturnArg);
			LuaParameters::Return(s28, 28, myFirstReturnArg);
			LuaParameters::Return(s29, 29, myFirstReturnArg);
			LuaParameters::Return(s30, 30, myFirstReturnArg);
			LuaParameters::Return(s31, 31, myFirstReturnArg);
			LuaParameters::Return(s32, 32, myFirstReturnArg);
			LuaParameters::Return(s33, 33, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&);
		typedef TYPELIST_33(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall33< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&);
		typedef TYPELIST_33(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall33< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall34

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34 >
	class LuaCall34 : public Registry::CallInterface
	{
	public:

		LuaCall34(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 34 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			P20 p20; P20* s20 = LuaParameters::Set(dispatchData->L, p20, 20, myFirstReturnArg); 
			P21 p21; P21* s21 = LuaParameters::Set(dispatchData->L, p21, 21, myFirstReturnArg); 
			P22 p22; P22* s22 = LuaParameters::Set(dispatchData->L, p22, 22, myFirstReturnArg); 
			P23 p23; P23* s23 = LuaParameters::Set(dispatchData->L, p23, 23, myFirstReturnArg); 
			P24 p24; P24* s24 = LuaParameters::Set(dispatchData->L, p24, 24, myFirstReturnArg); 
			P25 p25; P25* s25 = LuaParameters::Set(dispatchData->L, p25, 25, myFirstReturnArg); 
			P26 p26; P26* s26 = LuaParameters::Set(dispatchData->L, p26, 26, myFirstReturnArg); 
			P27 p27; P27* s27 = LuaParameters::Set(dispatchData->L, p27, 27, myFirstReturnArg); 
			P28 p28; P28* s28 = LuaParameters::Set(dispatchData->L, p28, 28, myFirstReturnArg); 
			P29 p29; P29* s29 = LuaParameters::Set(dispatchData->L, p29, 29, myFirstReturnArg); 
			P30 p30; P30* s30 = LuaParameters::Set(dispatchData->L, p30, 30, myFirstReturnArg); 
			P31 p31; P31* s31 = LuaParameters::Set(dispatchData->L, p31, 31, myFirstReturnArg); 
			P32 p32; P32* s32 = LuaParameters::Set(dispatchData->L, p32, 32, myFirstReturnArg); 
			P33 p33; P33* s33 = LuaParameters::Set(dispatchData->L, p33, 33, myFirstReturnArg); 
			P34 p34; P34* s34 = LuaParameters::Set(dispatchData->L, p34, 34, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19, *s20, *s21, *s22, *s23, *s24, *s25, *s26, *s27, *s28, *s29, *s30, *s31, *s32, *s33, *s34);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			LuaParameters::Return(s20, 20, myFirstReturnArg);
			LuaParameters::Return(s21, 21, myFirstReturnArg);
			LuaParameters::Return(s22, 22, myFirstReturnArg);
			LuaParameters::Return(s23, 23, myFirstReturnArg);
			LuaParameters::Return(s24, 24, myFirstReturnArg);
			LuaParameters::Return(s25, 25, myFirstReturnArg);
			LuaParameters::Return(s26, 26, myFirstReturnArg);
			LuaParameters::Return(s27, 27, myFirstReturnArg);
			LuaParameters::Return(s28, 28, myFirstReturnArg);
			LuaParameters::Return(s29, 29, myFirstReturnArg);
			LuaParameters::Return(s30, 30, myFirstReturnArg);
			LuaParameters::Return(s31, 31, myFirstReturnArg);
			LuaParameters::Return(s32, 32, myFirstReturnArg);
			LuaParameters::Return(s33, 33, myFirstReturnArg);
			LuaParameters::Return(s34, 34, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&);
		typedef TYPELIST_34(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall34< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&);
		typedef TYPELIST_34(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall34< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall35

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35 >
	class LuaCall35 : public Registry::CallInterface
	{
	public:

		LuaCall35(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 35 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			P20 p20; P20* s20 = LuaParameters::Set(dispatchData->L, p20, 20, myFirstReturnArg); 
			P21 p21; P21* s21 = LuaParameters::Set(dispatchData->L, p21, 21, myFirstReturnArg); 
			P22 p22; P22* s22 = LuaParameters::Set(dispatchData->L, p22, 22, myFirstReturnArg); 
			P23 p23; P23* s23 = LuaParameters::Set(dispatchData->L, p23, 23, myFirstReturnArg); 
			P24 p24; P24* s24 = LuaParameters::Set(dispatchData->L, p24, 24, myFirstReturnArg); 
			P25 p25; P25* s25 = LuaParameters::Set(dispatchData->L, p25, 25, myFirstReturnArg); 
			P26 p26; P26* s26 = LuaParameters::Set(dispatchData->L, p26, 26, myFirstReturnArg); 
			P27 p27; P27* s27 = LuaParameters::Set(dispatchData->L, p27, 27, myFirstReturnArg); 
			P28 p28; P28* s28 = LuaParameters::Set(dispatchData->L, p28, 28, myFirstReturnArg); 
			P29 p29; P29* s29 = LuaParameters::Set(dispatchData->L, p29, 29, myFirstReturnArg); 
			P30 p30; P30* s30 = LuaParameters::Set(dispatchData->L, p30, 30, myFirstReturnArg); 
			P31 p31; P31* s31 = LuaParameters::Set(dispatchData->L, p31, 31, myFirstReturnArg); 
			P32 p32; P32* s32 = LuaParameters::Set(dispatchData->L, p32, 32, myFirstReturnArg); 
			P33 p33; P33* s33 = LuaParameters::Set(dispatchData->L, p33, 33, myFirstReturnArg); 
			P34 p34; P34* s34 = LuaParameters::Set(dispatchData->L, p34, 34, myFirstReturnArg); 
			P35 p35; P35* s35 = LuaParameters::Set(dispatchData->L, p35, 35, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19, *s20, *s21, *s22, *s23, *s24, *s25, *s26, *s27, *s28, *s29, *s30, *s31, *s32, *s33, *s34, *s35);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			LuaParameters::Return(s20, 20, myFirstReturnArg);
			LuaParameters::Return(s21, 21, myFirstReturnArg);
			LuaParameters::Return(s22, 22, myFirstReturnArg);
			LuaParameters::Return(s23, 23, myFirstReturnArg);
			LuaParameters::Return(s24, 24, myFirstReturnArg);
			LuaParameters::Return(s25, 25, myFirstReturnArg);
			LuaParameters::Return(s26, 26, myFirstReturnArg);
			LuaParameters::Return(s27, 27, myFirstReturnArg);
			LuaParameters::Return(s28, 28, myFirstReturnArg);
			LuaParameters::Return(s29, 29, myFirstReturnArg);
			LuaParameters::Return(s30, 30, myFirstReturnArg);
			LuaParameters::Return(s31, 31, myFirstReturnArg);
			LuaParameters::Return(s32, 32, myFirstReturnArg);
			LuaParameters::Return(s33, 33, myFirstReturnArg);
			LuaParameters::Return(s34, 34, myFirstReturnArg);
			LuaParameters::Return(s35, 35, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&);
		typedef TYPELIST_35(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall35< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&);
		typedef TYPELIST_35(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall35< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall36

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36 >
	class LuaCall36 : public Registry::CallInterface
	{
	public:

		LuaCall36(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 36 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			P20 p20; P20* s20 = LuaParameters::Set(dispatchData->L, p20, 20, myFirstReturnArg); 
			P21 p21; P21* s21 = LuaParameters::Set(dispatchData->L, p21, 21, myFirstReturnArg); 
			P22 p22; P22* s22 = LuaParameters::Set(dispatchData->L, p22, 22, myFirstReturnArg); 
			P23 p23; P23* s23 = LuaParameters::Set(dispatchData->L, p23, 23, myFirstReturnArg); 
			P24 p24; P24* s24 = LuaParameters::Set(dispatchData->L, p24, 24, myFirstReturnArg); 
			P25 p25; P25* s25 = LuaParameters::Set(dispatchData->L, p25, 25, myFirstReturnArg); 
			P26 p26; P26* s26 = LuaParameters::Set(dispatchData->L, p26, 26, myFirstReturnArg); 
			P27 p27; P27* s27 = LuaParameters::Set(dispatchData->L, p27, 27, myFirstReturnArg); 
			P28 p28; P28* s28 = LuaParameters::Set(dispatchData->L, p28, 28, myFirstReturnArg); 
			P29 p29; P29* s29 = LuaParameters::Set(dispatchData->L, p29, 29, myFirstReturnArg); 
			P30 p30; P30* s30 = LuaParameters::Set(dispatchData->L, p30, 30, myFirstReturnArg); 
			P31 p31; P31* s31 = LuaParameters::Set(dispatchData->L, p31, 31, myFirstReturnArg); 
			P32 p32; P32* s32 = LuaParameters::Set(dispatchData->L, p32, 32, myFirstReturnArg); 
			P33 p33; P33* s33 = LuaParameters::Set(dispatchData->L, p33, 33, myFirstReturnArg); 
			P34 p34; P34* s34 = LuaParameters::Set(dispatchData->L, p34, 34, myFirstReturnArg); 
			P35 p35; P35* s35 = LuaParameters::Set(dispatchData->L, p35, 35, myFirstReturnArg); 
			P36 p36; P36* s36 = LuaParameters::Set(dispatchData->L, p36, 36, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19, *s20, *s21, *s22, *s23, *s24, *s25, *s26, *s27, *s28, *s29, *s30, *s31, *s32, *s33, *s34, *s35, *s36);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			LuaParameters::Return(s20, 20, myFirstReturnArg);
			LuaParameters::Return(s21, 21, myFirstReturnArg);
			LuaParameters::Return(s22, 22, myFirstReturnArg);
			LuaParameters::Return(s23, 23, myFirstReturnArg);
			LuaParameters::Return(s24, 24, myFirstReturnArg);
			LuaParameters::Return(s25, 25, myFirstReturnArg);
			LuaParameters::Return(s26, 26, myFirstReturnArg);
			LuaParameters::Return(s27, 27, myFirstReturnArg);
			LuaParameters::Return(s28, 28, myFirstReturnArg);
			LuaParameters::Return(s29, 29, myFirstReturnArg);
			LuaParameters::Return(s30, 30, myFirstReturnArg);
			LuaParameters::Return(s31, 31, myFirstReturnArg);
			LuaParameters::Return(s32, 32, myFirstReturnArg);
			LuaParameters::Return(s33, 33, myFirstReturnArg);
			LuaParameters::Return(s34, 34, myFirstReturnArg);
			LuaParameters::Return(s35, 35, myFirstReturnArg);
			LuaParameters::Return(s36, 36, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&);
		typedef TYPELIST_36(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall36< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&);
		typedef TYPELIST_36(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall36< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall37

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37 >
	class LuaCall37 : public Registry::CallInterface
	{
	public:

		LuaCall37(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 37 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			P20 p20; P20* s20 = LuaParameters::Set(dispatchData->L, p20, 20, myFirstReturnArg); 
			P21 p21; P21* s21 = LuaParameters::Set(dispatchData->L, p21, 21, myFirstReturnArg); 
			P22 p22; P22* s22 = LuaParameters::Set(dispatchData->L, p22, 22, myFirstReturnArg); 
			P23 p23; P23* s23 = LuaParameters::Set(dispatchData->L, p23, 23, myFirstReturnArg); 
			P24 p24; P24* s24 = LuaParameters::Set(dispatchData->L, p24, 24, myFirstReturnArg); 
			P25 p25; P25* s25 = LuaParameters::Set(dispatchData->L, p25, 25, myFirstReturnArg); 
			P26 p26; P26* s26 = LuaParameters::Set(dispatchData->L, p26, 26, myFirstReturnArg); 
			P27 p27; P27* s27 = LuaParameters::Set(dispatchData->L, p27, 27, myFirstReturnArg); 
			P28 p28; P28* s28 = LuaParameters::Set(dispatchData->L, p28, 28, myFirstReturnArg); 
			P29 p29; P29* s29 = LuaParameters::Set(dispatchData->L, p29, 29, myFirstReturnArg); 
			P30 p30; P30* s30 = LuaParameters::Set(dispatchData->L, p30, 30, myFirstReturnArg); 
			P31 p31; P31* s31 = LuaParameters::Set(dispatchData->L, p31, 31, myFirstReturnArg); 
			P32 p32; P32* s32 = LuaParameters::Set(dispatchData->L, p32, 32, myFirstReturnArg); 
			P33 p33; P33* s33 = LuaParameters::Set(dispatchData->L, p33, 33, myFirstReturnArg); 
			P34 p34; P34* s34 = LuaParameters::Set(dispatchData->L, p34, 34, myFirstReturnArg); 
			P35 p35; P35* s35 = LuaParameters::Set(dispatchData->L, p35, 35, myFirstReturnArg); 
			P36 p36; P36* s36 = LuaParameters::Set(dispatchData->L, p36, 36, myFirstReturnArg); 
			P37 p37; P37* s37 = LuaParameters::Set(dispatchData->L, p37, 37, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19, *s20, *s21, *s22, *s23, *s24, *s25, *s26, *s27, *s28, *s29, *s30, *s31, *s32, *s33, *s34, *s35, *s36, *s37);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			LuaParameters::Return(s20, 20, myFirstReturnArg);
			LuaParameters::Return(s21, 21, myFirstReturnArg);
			LuaParameters::Return(s22, 22, myFirstReturnArg);
			LuaParameters::Return(s23, 23, myFirstReturnArg);
			LuaParameters::Return(s24, 24, myFirstReturnArg);
			LuaParameters::Return(s25, 25, myFirstReturnArg);
			LuaParameters::Return(s26, 26, myFirstReturnArg);
			LuaParameters::Return(s27, 27, myFirstReturnArg);
			LuaParameters::Return(s28, 28, myFirstReturnArg);
			LuaParameters::Return(s29, 29, myFirstReturnArg);
			LuaParameters::Return(s30, 30, myFirstReturnArg);
			LuaParameters::Return(s31, 31, myFirstReturnArg);
			LuaParameters::Return(s32, 32, myFirstReturnArg);
			LuaParameters::Return(s33, 33, myFirstReturnArg);
			LuaParameters::Return(s34, 34, myFirstReturnArg);
			LuaParameters::Return(s35, 35, myFirstReturnArg);
			LuaParameters::Return(s36, 36, myFirstReturnArg);
			LuaParameters::Return(s37, 37, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&);
		typedef TYPELIST_37(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall37< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&);
		typedef TYPELIST_37(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall37< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall38

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38 >
	class LuaCall38 : public Registry::CallInterface
	{
	public:

		LuaCall38(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 38 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			P20 p20; P20* s20 = LuaParameters::Set(dispatchData->L, p20, 20, myFirstReturnArg); 
			P21 p21; P21* s21 = LuaParameters::Set(dispatchData->L, p21, 21, myFirstReturnArg); 
			P22 p22; P22* s22 = LuaParameters::Set(dispatchData->L, p22, 22, myFirstReturnArg); 
			P23 p23; P23* s23 = LuaParameters::Set(dispatchData->L, p23, 23, myFirstReturnArg); 
			P24 p24; P24* s24 = LuaParameters::Set(dispatchData->L, p24, 24, myFirstReturnArg); 
			P25 p25; P25* s25 = LuaParameters::Set(dispatchData->L, p25, 25, myFirstReturnArg); 
			P26 p26; P26* s26 = LuaParameters::Set(dispatchData->L, p26, 26, myFirstReturnArg); 
			P27 p27; P27* s27 = LuaParameters::Set(dispatchData->L, p27, 27, myFirstReturnArg); 
			P28 p28; P28* s28 = LuaParameters::Set(dispatchData->L, p28, 28, myFirstReturnArg); 
			P29 p29; P29* s29 = LuaParameters::Set(dispatchData->L, p29, 29, myFirstReturnArg); 
			P30 p30; P30* s30 = LuaParameters::Set(dispatchData->L, p30, 30, myFirstReturnArg); 
			P31 p31; P31* s31 = LuaParameters::Set(dispatchData->L, p31, 31, myFirstReturnArg); 
			P32 p32; P32* s32 = LuaParameters::Set(dispatchData->L, p32, 32, myFirstReturnArg); 
			P33 p33; P33* s33 = LuaParameters::Set(dispatchData->L, p33, 33, myFirstReturnArg); 
			P34 p34; P34* s34 = LuaParameters::Set(dispatchData->L, p34, 34, myFirstReturnArg); 
			P35 p35; P35* s35 = LuaParameters::Set(dispatchData->L, p35, 35, myFirstReturnArg); 
			P36 p36; P36* s36 = LuaParameters::Set(dispatchData->L, p36, 36, myFirstReturnArg); 
			P37 p37; P37* s37 = LuaParameters::Set(dispatchData->L, p37, 37, myFirstReturnArg); 
			P38 p38; P38* s38 = LuaParameters::Set(dispatchData->L, p38, 38, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19, *s20, *s21, *s22, *s23, *s24, *s25, *s26, *s27, *s28, *s29, *s30, *s31, *s32, *s33, *s34, *s35, *s36, *s37, *s38);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			LuaParameters::Return(s20, 20, myFirstReturnArg);
			LuaParameters::Return(s21, 21, myFirstReturnArg);
			LuaParameters::Return(s22, 22, myFirstReturnArg);
			LuaParameters::Return(s23, 23, myFirstReturnArg);
			LuaParameters::Return(s24, 24, myFirstReturnArg);
			LuaParameters::Return(s25, 25, myFirstReturnArg);
			LuaParameters::Return(s26, 26, myFirstReturnArg);
			LuaParameters::Return(s27, 27, myFirstReturnArg);
			LuaParameters::Return(s28, 28, myFirstReturnArg);
			LuaParameters::Return(s29, 29, myFirstReturnArg);
			LuaParameters::Return(s30, 30, myFirstReturnArg);
			LuaParameters::Return(s31, 31, myFirstReturnArg);
			LuaParameters::Return(s32, 32, myFirstReturnArg);
			LuaParameters::Return(s33, 33, myFirstReturnArg);
			LuaParameters::Return(s34, 34, myFirstReturnArg);
			LuaParameters::Return(s35, 35, myFirstReturnArg);
			LuaParameters::Return(s36, 36, myFirstReturnArg);
			LuaParameters::Return(s37, 37, myFirstReturnArg);
			LuaParameters::Return(s38, 38, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&);
		typedef TYPELIST_38(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall38< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&);
		typedef TYPELIST_38(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall38< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall39

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39 >
	class LuaCall39 : public Registry::CallInterface
	{
	public:

		LuaCall39(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 39 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			P20 p20; P20* s20 = LuaParameters::Set(dispatchData->L, p20, 20, myFirstReturnArg); 
			P21 p21; P21* s21 = LuaParameters::Set(dispatchData->L, p21, 21, myFirstReturnArg); 
			P22 p22; P22* s22 = LuaParameters::Set(dispatchData->L, p22, 22, myFirstReturnArg); 
			P23 p23; P23* s23 = LuaParameters::Set(dispatchData->L, p23, 23, myFirstReturnArg); 
			P24 p24; P24* s24 = LuaParameters::Set(dispatchData->L, p24, 24, myFirstReturnArg); 
			P25 p25; P25* s25 = LuaParameters::Set(dispatchData->L, p25, 25, myFirstReturnArg); 
			P26 p26; P26* s26 = LuaParameters::Set(dispatchData->L, p26, 26, myFirstReturnArg); 
			P27 p27; P27* s27 = LuaParameters::Set(dispatchData->L, p27, 27, myFirstReturnArg); 
			P28 p28; P28* s28 = LuaParameters::Set(dispatchData->L, p28, 28, myFirstReturnArg); 
			P29 p29; P29* s29 = LuaParameters::Set(dispatchData->L, p29, 29, myFirstReturnArg); 
			P30 p30; P30* s30 = LuaParameters::Set(dispatchData->L, p30, 30, myFirstReturnArg); 
			P31 p31; P31* s31 = LuaParameters::Set(dispatchData->L, p31, 31, myFirstReturnArg); 
			P32 p32; P32* s32 = LuaParameters::Set(dispatchData->L, p32, 32, myFirstReturnArg); 
			P33 p33; P33* s33 = LuaParameters::Set(dispatchData->L, p33, 33, myFirstReturnArg); 
			P34 p34; P34* s34 = LuaParameters::Set(dispatchData->L, p34, 34, myFirstReturnArg); 
			P35 p35; P35* s35 = LuaParameters::Set(dispatchData->L, p35, 35, myFirstReturnArg); 
			P36 p36; P36* s36 = LuaParameters::Set(dispatchData->L, p36, 36, myFirstReturnArg); 
			P37 p37; P37* s37 = LuaParameters::Set(dispatchData->L, p37, 37, myFirstReturnArg); 
			P38 p38; P38* s38 = LuaParameters::Set(dispatchData->L, p38, 38, myFirstReturnArg); 
			P39 p39; P39* s39 = LuaParameters::Set(dispatchData->L, p39, 39, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19, *s20, *s21, *s22, *s23, *s24, *s25, *s26, *s27, *s28, *s29, *s30, *s31, *s32, *s33, *s34, *s35, *s36, *s37, *s38, *s39);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			LuaParameters::Return(s20, 20, myFirstReturnArg);
			LuaParameters::Return(s21, 21, myFirstReturnArg);
			LuaParameters::Return(s22, 22, myFirstReturnArg);
			LuaParameters::Return(s23, 23, myFirstReturnArg);
			LuaParameters::Return(s24, 24, myFirstReturnArg);
			LuaParameters::Return(s25, 25, myFirstReturnArg);
			LuaParameters::Return(s26, 26, myFirstReturnArg);
			LuaParameters::Return(s27, 27, myFirstReturnArg);
			LuaParameters::Return(s28, 28, myFirstReturnArg);
			LuaParameters::Return(s29, 29, myFirstReturnArg);
			LuaParameters::Return(s30, 30, myFirstReturnArg);
			LuaParameters::Return(s31, 31, myFirstReturnArg);
			LuaParameters::Return(s32, 32, myFirstReturnArg);
			LuaParameters::Return(s33, 33, myFirstReturnArg);
			LuaParameters::Return(s34, 34, myFirstReturnArg);
			LuaParameters::Return(s35, 35, myFirstReturnArg);
			LuaParameters::Return(s36, 36, myFirstReturnArg);
			LuaParameters::Return(s37, 37, myFirstReturnArg);
			LuaParameters::Return(s38, 38, myFirstReturnArg);
			LuaParameters::Return(s39, 39, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&);
		typedef TYPELIST_39(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall39< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&);
		typedef TYPELIST_39(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall39< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall40

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40 >
	class LuaCall40 : public Registry::CallInterface
	{
	public:

		LuaCall40(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 40 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			P20 p20; P20* s20 = LuaParameters::Set(dispatchData->L, p20, 20, myFirstReturnArg); 
			P21 p21; P21* s21 = LuaParameters::Set(dispatchData->L, p21, 21, myFirstReturnArg); 
			P22 p22; P22* s22 = LuaParameters::Set(dispatchData->L, p22, 22, myFirstReturnArg); 
			P23 p23; P23* s23 = LuaParameters::Set(dispatchData->L, p23, 23, myFirstReturnArg); 
			P24 p24; P24* s24 = LuaParameters::Set(dispatchData->L, p24, 24, myFirstReturnArg); 
			P25 p25; P25* s25 = LuaParameters::Set(dispatchData->L, p25, 25, myFirstReturnArg); 
			P26 p26; P26* s26 = LuaParameters::Set(dispatchData->L, p26, 26, myFirstReturnArg); 
			P27 p27; P27* s27 = LuaParameters::Set(dispatchData->L, p27, 27, myFirstReturnArg); 
			P28 p28; P28* s28 = LuaParameters::Set(dispatchData->L, p28, 28, myFirstReturnArg); 
			P29 p29; P29* s29 = LuaParameters::Set(dispatchData->L, p29, 29, myFirstReturnArg); 
			P30 p30; P30* s30 = LuaParameters::Set(dispatchData->L, p30, 30, myFirstReturnArg); 
			P31 p31; P31* s31 = LuaParameters::Set(dispatchData->L, p31, 31, myFirstReturnArg); 
			P32 p32; P32* s32 = LuaParameters::Set(dispatchData->L, p32, 32, myFirstReturnArg); 
			P33 p33; P33* s33 = LuaParameters::Set(dispatchData->L, p33, 33, myFirstReturnArg); 
			P34 p34; P34* s34 = LuaParameters::Set(dispatchData->L, p34, 34, myFirstReturnArg); 
			P35 p35; P35* s35 = LuaParameters::Set(dispatchData->L, p35, 35, myFirstReturnArg); 
			P36 p36; P36* s36 = LuaParameters::Set(dispatchData->L, p36, 36, myFirstReturnArg); 
			P37 p37; P37* s37 = LuaParameters::Set(dispatchData->L, p37, 37, myFirstReturnArg); 
			P38 p38; P38* s38 = LuaParameters::Set(dispatchData->L, p38, 38, myFirstReturnArg); 
			P39 p39; P39* s39 = LuaParameters::Set(dispatchData->L, p39, 39, myFirstReturnArg); 
			P40 p40; P40* s40 = LuaParameters::Set(dispatchData->L, p40, 40, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19, *s20, *s21, *s22, *s23, *s24, *s25, *s26, *s27, *s28, *s29, *s30, *s31, *s32, *s33, *s34, *s35, *s36, *s37, *s38, *s39, *s40);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			LuaParameters::Return(s20, 20, myFirstReturnArg);
			LuaParameters::Return(s21, 21, myFirstReturnArg);
			LuaParameters::Return(s22, 22, myFirstReturnArg);
			LuaParameters::Return(s23, 23, myFirstReturnArg);
			LuaParameters::Return(s24, 24, myFirstReturnArg);
			LuaParameters::Return(s25, 25, myFirstReturnArg);
			LuaParameters::Return(s26, 26, myFirstReturnArg);
			LuaParameters::Return(s27, 27, myFirstReturnArg);
			LuaParameters::Return(s28, 28, myFirstReturnArg);
			LuaParameters::Return(s29, 29, myFirstReturnArg);
			LuaParameters::Return(s30, 30, myFirstReturnArg);
			LuaParameters::Return(s31, 31, myFirstReturnArg);
			LuaParameters::Return(s32, 32, myFirstReturnArg);
			LuaParameters::Return(s33, 33, myFirstReturnArg);
			LuaParameters::Return(s34, 34, myFirstReturnArg);
			LuaParameters::Return(s35, 35, myFirstReturnArg);
			LuaParameters::Return(s36, 36, myFirstReturnArg);
			LuaParameters::Return(s37, 37, myFirstReturnArg);
			LuaParameters::Return(s38, 38, myFirstReturnArg);
			LuaParameters::Return(s39, 39, myFirstReturnArg);
			LuaParameters::Return(s40, 40, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&);
		typedef TYPELIST_40(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall40< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&);
		typedef TYPELIST_40(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall40< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall41

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40, typename P41 >
	class LuaCall41 : public Registry::CallInterface
	{
	public:

		LuaCall41(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 41 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			P20 p20; P20* s20 = LuaParameters::Set(dispatchData->L, p20, 20, myFirstReturnArg); 
			P21 p21; P21* s21 = LuaParameters::Set(dispatchData->L, p21, 21, myFirstReturnArg); 
			P22 p22; P22* s22 = LuaParameters::Set(dispatchData->L, p22, 22, myFirstReturnArg); 
			P23 p23; P23* s23 = LuaParameters::Set(dispatchData->L, p23, 23, myFirstReturnArg); 
			P24 p24; P24* s24 = LuaParameters::Set(dispatchData->L, p24, 24, myFirstReturnArg); 
			P25 p25; P25* s25 = LuaParameters::Set(dispatchData->L, p25, 25, myFirstReturnArg); 
			P26 p26; P26* s26 = LuaParameters::Set(dispatchData->L, p26, 26, myFirstReturnArg); 
			P27 p27; P27* s27 = LuaParameters::Set(dispatchData->L, p27, 27, myFirstReturnArg); 
			P28 p28; P28* s28 = LuaParameters::Set(dispatchData->L, p28, 28, myFirstReturnArg); 
			P29 p29; P29* s29 = LuaParameters::Set(dispatchData->L, p29, 29, myFirstReturnArg); 
			P30 p30; P30* s30 = LuaParameters::Set(dispatchData->L, p30, 30, myFirstReturnArg); 
			P31 p31; P31* s31 = LuaParameters::Set(dispatchData->L, p31, 31, myFirstReturnArg); 
			P32 p32; P32* s32 = LuaParameters::Set(dispatchData->L, p32, 32, myFirstReturnArg); 
			P33 p33; P33* s33 = LuaParameters::Set(dispatchData->L, p33, 33, myFirstReturnArg); 
			P34 p34; P34* s34 = LuaParameters::Set(dispatchData->L, p34, 34, myFirstReturnArg); 
			P35 p35; P35* s35 = LuaParameters::Set(dispatchData->L, p35, 35, myFirstReturnArg); 
			P36 p36; P36* s36 = LuaParameters::Set(dispatchData->L, p36, 36, myFirstReturnArg); 
			P37 p37; P37* s37 = LuaParameters::Set(dispatchData->L, p37, 37, myFirstReturnArg); 
			P38 p38; P38* s38 = LuaParameters::Set(dispatchData->L, p38, 38, myFirstReturnArg); 
			P39 p39; P39* s39 = LuaParameters::Set(dispatchData->L, p39, 39, myFirstReturnArg); 
			P40 p40; P40* s40 = LuaParameters::Set(dispatchData->L, p40, 40, myFirstReturnArg); 
			P41 p41; P41* s41 = LuaParameters::Set(dispatchData->L, p41, 41, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19, *s20, *s21, *s22, *s23, *s24, *s25, *s26, *s27, *s28, *s29, *s30, *s31, *s32, *s33, *s34, *s35, *s36, *s37, *s38, *s39, *s40, *s41);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			LuaParameters::Return(s20, 20, myFirstReturnArg);
			LuaParameters::Return(s21, 21, myFirstReturnArg);
			LuaParameters::Return(s22, 22, myFirstReturnArg);
			LuaParameters::Return(s23, 23, myFirstReturnArg);
			LuaParameters::Return(s24, 24, myFirstReturnArg);
			LuaParameters::Return(s25, 25, myFirstReturnArg);
			LuaParameters::Return(s26, 26, myFirstReturnArg);
			LuaParameters::Return(s27, 27, myFirstReturnArg);
			LuaParameters::Return(s28, 28, myFirstReturnArg);
			LuaParameters::Return(s29, 29, myFirstReturnArg);
			LuaParameters::Return(s30, 30, myFirstReturnArg);
			LuaParameters::Return(s31, 31, myFirstReturnArg);
			LuaParameters::Return(s32, 32, myFirstReturnArg);
			LuaParameters::Return(s33, 33, myFirstReturnArg);
			LuaParameters::Return(s34, 34, myFirstReturnArg);
			LuaParameters::Return(s35, 35, myFirstReturnArg);
			LuaParameters::Return(s36, 36, myFirstReturnArg);
			LuaParameters::Return(s37, 37, myFirstReturnArg);
			LuaParameters::Return(s38, 38, myFirstReturnArg);
			LuaParameters::Return(s39, 39, myFirstReturnArg);
			LuaParameters::Return(s40, 40, myFirstReturnArg);
			LuaParameters::Return(s41, 41, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40, typename P41 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&);
		typedef TYPELIST_41(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall41< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40, typename P41 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&);
		typedef TYPELIST_41(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall41< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall42

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40, typename P41, typename P42 >
	class LuaCall42 : public Registry::CallInterface
	{
	public:

		LuaCall42(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 42 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			P20 p20; P20* s20 = LuaParameters::Set(dispatchData->L, p20, 20, myFirstReturnArg); 
			P21 p21; P21* s21 = LuaParameters::Set(dispatchData->L, p21, 21, myFirstReturnArg); 
			P22 p22; P22* s22 = LuaParameters::Set(dispatchData->L, p22, 22, myFirstReturnArg); 
			P23 p23; P23* s23 = LuaParameters::Set(dispatchData->L, p23, 23, myFirstReturnArg); 
			P24 p24; P24* s24 = LuaParameters::Set(dispatchData->L, p24, 24, myFirstReturnArg); 
			P25 p25; P25* s25 = LuaParameters::Set(dispatchData->L, p25, 25, myFirstReturnArg); 
			P26 p26; P26* s26 = LuaParameters::Set(dispatchData->L, p26, 26, myFirstReturnArg); 
			P27 p27; P27* s27 = LuaParameters::Set(dispatchData->L, p27, 27, myFirstReturnArg); 
			P28 p28; P28* s28 = LuaParameters::Set(dispatchData->L, p28, 28, myFirstReturnArg); 
			P29 p29; P29* s29 = LuaParameters::Set(dispatchData->L, p29, 29, myFirstReturnArg); 
			P30 p30; P30* s30 = LuaParameters::Set(dispatchData->L, p30, 30, myFirstReturnArg); 
			P31 p31; P31* s31 = LuaParameters::Set(dispatchData->L, p31, 31, myFirstReturnArg); 
			P32 p32; P32* s32 = LuaParameters::Set(dispatchData->L, p32, 32, myFirstReturnArg); 
			P33 p33; P33* s33 = LuaParameters::Set(dispatchData->L, p33, 33, myFirstReturnArg); 
			P34 p34; P34* s34 = LuaParameters::Set(dispatchData->L, p34, 34, myFirstReturnArg); 
			P35 p35; P35* s35 = LuaParameters::Set(dispatchData->L, p35, 35, myFirstReturnArg); 
			P36 p36; P36* s36 = LuaParameters::Set(dispatchData->L, p36, 36, myFirstReturnArg); 
			P37 p37; P37* s37 = LuaParameters::Set(dispatchData->L, p37, 37, myFirstReturnArg); 
			P38 p38; P38* s38 = LuaParameters::Set(dispatchData->L, p38, 38, myFirstReturnArg); 
			P39 p39; P39* s39 = LuaParameters::Set(dispatchData->L, p39, 39, myFirstReturnArg); 
			P40 p40; P40* s40 = LuaParameters::Set(dispatchData->L, p40, 40, myFirstReturnArg); 
			P41 p41; P41* s41 = LuaParameters::Set(dispatchData->L, p41, 41, myFirstReturnArg); 
			P42 p42; P42* s42 = LuaParameters::Set(dispatchData->L, p42, 42, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19, *s20, *s21, *s22, *s23, *s24, *s25, *s26, *s27, *s28, *s29, *s30, *s31, *s32, *s33, *s34, *s35, *s36, *s37, *s38, *s39, *s40, *s41, *s42);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			LuaParameters::Return(s20, 20, myFirstReturnArg);
			LuaParameters::Return(s21, 21, myFirstReturnArg);
			LuaParameters::Return(s22, 22, myFirstReturnArg);
			LuaParameters::Return(s23, 23, myFirstReturnArg);
			LuaParameters::Return(s24, 24, myFirstReturnArg);
			LuaParameters::Return(s25, 25, myFirstReturnArg);
			LuaParameters::Return(s26, 26, myFirstReturnArg);
			LuaParameters::Return(s27, 27, myFirstReturnArg);
			LuaParameters::Return(s28, 28, myFirstReturnArg);
			LuaParameters::Return(s29, 29, myFirstReturnArg);
			LuaParameters::Return(s30, 30, myFirstReturnArg);
			LuaParameters::Return(s31, 31, myFirstReturnArg);
			LuaParameters::Return(s32, 32, myFirstReturnArg);
			LuaParameters::Return(s33, 33, myFirstReturnArg);
			LuaParameters::Return(s34, 34, myFirstReturnArg);
			LuaParameters::Return(s35, 35, myFirstReturnArg);
			LuaParameters::Return(s36, 36, myFirstReturnArg);
			LuaParameters::Return(s37, 37, myFirstReturnArg);
			LuaParameters::Return(s38, 38, myFirstReturnArg);
			LuaParameters::Return(s39, 39, myFirstReturnArg);
			LuaParameters::Return(s40, 40, myFirstReturnArg);
			LuaParameters::Return(s41, 41, myFirstReturnArg);
			LuaParameters::Return(s42, 42, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40, typename P41, typename P42 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&);
		typedef TYPELIST_42(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall42< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40, typename P41, typename P42 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&);
		typedef TYPELIST_42(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall42< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall43

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40, typename P41, typename P42, typename P43 >
	class LuaCall43 : public Registry::CallInterface
	{
	public:

		LuaCall43(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 43 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			P20 p20; P20* s20 = LuaParameters::Set(dispatchData->L, p20, 20, myFirstReturnArg); 
			P21 p21; P21* s21 = LuaParameters::Set(dispatchData->L, p21, 21, myFirstReturnArg); 
			P22 p22; P22* s22 = LuaParameters::Set(dispatchData->L, p22, 22, myFirstReturnArg); 
			P23 p23; P23* s23 = LuaParameters::Set(dispatchData->L, p23, 23, myFirstReturnArg); 
			P24 p24; P24* s24 = LuaParameters::Set(dispatchData->L, p24, 24, myFirstReturnArg); 
			P25 p25; P25* s25 = LuaParameters::Set(dispatchData->L, p25, 25, myFirstReturnArg); 
			P26 p26; P26* s26 = LuaParameters::Set(dispatchData->L, p26, 26, myFirstReturnArg); 
			P27 p27; P27* s27 = LuaParameters::Set(dispatchData->L, p27, 27, myFirstReturnArg); 
			P28 p28; P28* s28 = LuaParameters::Set(dispatchData->L, p28, 28, myFirstReturnArg); 
			P29 p29; P29* s29 = LuaParameters::Set(dispatchData->L, p29, 29, myFirstReturnArg); 
			P30 p30; P30* s30 = LuaParameters::Set(dispatchData->L, p30, 30, myFirstReturnArg); 
			P31 p31; P31* s31 = LuaParameters::Set(dispatchData->L, p31, 31, myFirstReturnArg); 
			P32 p32; P32* s32 = LuaParameters::Set(dispatchData->L, p32, 32, myFirstReturnArg); 
			P33 p33; P33* s33 = LuaParameters::Set(dispatchData->L, p33, 33, myFirstReturnArg); 
			P34 p34; P34* s34 = LuaParameters::Set(dispatchData->L, p34, 34, myFirstReturnArg); 
			P35 p35; P35* s35 = LuaParameters::Set(dispatchData->L, p35, 35, myFirstReturnArg); 
			P36 p36; P36* s36 = LuaParameters::Set(dispatchData->L, p36, 36, myFirstReturnArg); 
			P37 p37; P37* s37 = LuaParameters::Set(dispatchData->L, p37, 37, myFirstReturnArg); 
			P38 p38; P38* s38 = LuaParameters::Set(dispatchData->L, p38, 38, myFirstReturnArg); 
			P39 p39; P39* s39 = LuaParameters::Set(dispatchData->L, p39, 39, myFirstReturnArg); 
			P40 p40; P40* s40 = LuaParameters::Set(dispatchData->L, p40, 40, myFirstReturnArg); 
			P41 p41; P41* s41 = LuaParameters::Set(dispatchData->L, p41, 41, myFirstReturnArg); 
			P42 p42; P42* s42 = LuaParameters::Set(dispatchData->L, p42, 42, myFirstReturnArg); 
			P43 p43; P43* s43 = LuaParameters::Set(dispatchData->L, p43, 43, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19, *s20, *s21, *s22, *s23, *s24, *s25, *s26, *s27, *s28, *s29, *s30, *s31, *s32, *s33, *s34, *s35, *s36, *s37, *s38, *s39, *s40, *s41, *s42, *s43);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			LuaParameters::Return(s20, 20, myFirstReturnArg);
			LuaParameters::Return(s21, 21, myFirstReturnArg);
			LuaParameters::Return(s22, 22, myFirstReturnArg);
			LuaParameters::Return(s23, 23, myFirstReturnArg);
			LuaParameters::Return(s24, 24, myFirstReturnArg);
			LuaParameters::Return(s25, 25, myFirstReturnArg);
			LuaParameters::Return(s26, 26, myFirstReturnArg);
			LuaParameters::Return(s27, 27, myFirstReturnArg);
			LuaParameters::Return(s28, 28, myFirstReturnArg);
			LuaParameters::Return(s29, 29, myFirstReturnArg);
			LuaParameters::Return(s30, 30, myFirstReturnArg);
			LuaParameters::Return(s31, 31, myFirstReturnArg);
			LuaParameters::Return(s32, 32, myFirstReturnArg);
			LuaParameters::Return(s33, 33, myFirstReturnArg);
			LuaParameters::Return(s34, 34, myFirstReturnArg);
			LuaParameters::Return(s35, 35, myFirstReturnArg);
			LuaParameters::Return(s36, 36, myFirstReturnArg);
			LuaParameters::Return(s37, 37, myFirstReturnArg);
			LuaParameters::Return(s38, 38, myFirstReturnArg);
			LuaParameters::Return(s39, 39, myFirstReturnArg);
			LuaParameters::Return(s40, 40, myFirstReturnArg);
			LuaParameters::Return(s41, 41, myFirstReturnArg);
			LuaParameters::Return(s42, 42, myFirstReturnArg);
			LuaParameters::Return(s43, 43, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40, typename P41, typename P42, typename P43 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&);
		typedef TYPELIST_43(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall43< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40, typename P41, typename P42, typename P43 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&);
		typedef TYPELIST_43(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall43< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall44

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40, typename P41, typename P42, typename P43, typename P44 >
	class LuaCall44 : public Registry::CallInterface
	{
	public:

		LuaCall44(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 44 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			P20 p20; P20* s20 = LuaParameters::Set(dispatchData->L, p20, 20, myFirstReturnArg); 
			P21 p21; P21* s21 = LuaParameters::Set(dispatchData->L, p21, 21, myFirstReturnArg); 
			P22 p22; P22* s22 = LuaParameters::Set(dispatchData->L, p22, 22, myFirstReturnArg); 
			P23 p23; P23* s23 = LuaParameters::Set(dispatchData->L, p23, 23, myFirstReturnArg); 
			P24 p24; P24* s24 = LuaParameters::Set(dispatchData->L, p24, 24, myFirstReturnArg); 
			P25 p25; P25* s25 = LuaParameters::Set(dispatchData->L, p25, 25, myFirstReturnArg); 
			P26 p26; P26* s26 = LuaParameters::Set(dispatchData->L, p26, 26, myFirstReturnArg); 
			P27 p27; P27* s27 = LuaParameters::Set(dispatchData->L, p27, 27, myFirstReturnArg); 
			P28 p28; P28* s28 = LuaParameters::Set(dispatchData->L, p28, 28, myFirstReturnArg); 
			P29 p29; P29* s29 = LuaParameters::Set(dispatchData->L, p29, 29, myFirstReturnArg); 
			P30 p30; P30* s30 = LuaParameters::Set(dispatchData->L, p30, 30, myFirstReturnArg); 
			P31 p31; P31* s31 = LuaParameters::Set(dispatchData->L, p31, 31, myFirstReturnArg); 
			P32 p32; P32* s32 = LuaParameters::Set(dispatchData->L, p32, 32, myFirstReturnArg); 
			P33 p33; P33* s33 = LuaParameters::Set(dispatchData->L, p33, 33, myFirstReturnArg); 
			P34 p34; P34* s34 = LuaParameters::Set(dispatchData->L, p34, 34, myFirstReturnArg); 
			P35 p35; P35* s35 = LuaParameters::Set(dispatchData->L, p35, 35, myFirstReturnArg); 
			P36 p36; P36* s36 = LuaParameters::Set(dispatchData->L, p36, 36, myFirstReturnArg); 
			P37 p37; P37* s37 = LuaParameters::Set(dispatchData->L, p37, 37, myFirstReturnArg); 
			P38 p38; P38* s38 = LuaParameters::Set(dispatchData->L, p38, 38, myFirstReturnArg); 
			P39 p39; P39* s39 = LuaParameters::Set(dispatchData->L, p39, 39, myFirstReturnArg); 
			P40 p40; P40* s40 = LuaParameters::Set(dispatchData->L, p40, 40, myFirstReturnArg); 
			P41 p41; P41* s41 = LuaParameters::Set(dispatchData->L, p41, 41, myFirstReturnArg); 
			P42 p42; P42* s42 = LuaParameters::Set(dispatchData->L, p42, 42, myFirstReturnArg); 
			P43 p43; P43* s43 = LuaParameters::Set(dispatchData->L, p43, 43, myFirstReturnArg); 
			P44 p44; P44* s44 = LuaParameters::Set(dispatchData->L, p44, 44, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19, *s20, *s21, *s22, *s23, *s24, *s25, *s26, *s27, *s28, *s29, *s30, *s31, *s32, *s33, *s34, *s35, *s36, *s37, *s38, *s39, *s40, *s41, *s42, *s43, *s44);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			LuaParameters::Return(s20, 20, myFirstReturnArg);
			LuaParameters::Return(s21, 21, myFirstReturnArg);
			LuaParameters::Return(s22, 22, myFirstReturnArg);
			LuaParameters::Return(s23, 23, myFirstReturnArg);
			LuaParameters::Return(s24, 24, myFirstReturnArg);
			LuaParameters::Return(s25, 25, myFirstReturnArg);
			LuaParameters::Return(s26, 26, myFirstReturnArg);
			LuaParameters::Return(s27, 27, myFirstReturnArg);
			LuaParameters::Return(s28, 28, myFirstReturnArg);
			LuaParameters::Return(s29, 29, myFirstReturnArg);
			LuaParameters::Return(s30, 30, myFirstReturnArg);
			LuaParameters::Return(s31, 31, myFirstReturnArg);
			LuaParameters::Return(s32, 32, myFirstReturnArg);
			LuaParameters::Return(s33, 33, myFirstReturnArg);
			LuaParameters::Return(s34, 34, myFirstReturnArg);
			LuaParameters::Return(s35, 35, myFirstReturnArg);
			LuaParameters::Return(s36, 36, myFirstReturnArg);
			LuaParameters::Return(s37, 37, myFirstReturnArg);
			LuaParameters::Return(s38, 38, myFirstReturnArg);
			LuaParameters::Return(s39, 39, myFirstReturnArg);
			LuaParameters::Return(s40, 40, myFirstReturnArg);
			LuaParameters::Return(s41, 41, myFirstReturnArg);
			LuaParameters::Return(s42, 42, myFirstReturnArg);
			LuaParameters::Return(s43, 43, myFirstReturnArg);
			LuaParameters::Return(s44, 44, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40, typename P41, typename P42, typename P43, typename P44 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&, P44&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&, P44&);
		typedef TYPELIST_44(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43, P44) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall44< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43, P44 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40, typename P41, typename P42, typename P43, typename P44 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&, P44&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&, P44&);
		typedef TYPELIST_44(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43, P44) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall44< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43, P44 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall45

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40, typename P41, typename P42, typename P43, typename P44, typename P45 >
	class LuaCall45 : public Registry::CallInterface
	{
	public:

		LuaCall45(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 45 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			P20 p20; P20* s20 = LuaParameters::Set(dispatchData->L, p20, 20, myFirstReturnArg); 
			P21 p21; P21* s21 = LuaParameters::Set(dispatchData->L, p21, 21, myFirstReturnArg); 
			P22 p22; P22* s22 = LuaParameters::Set(dispatchData->L, p22, 22, myFirstReturnArg); 
			P23 p23; P23* s23 = LuaParameters::Set(dispatchData->L, p23, 23, myFirstReturnArg); 
			P24 p24; P24* s24 = LuaParameters::Set(dispatchData->L, p24, 24, myFirstReturnArg); 
			P25 p25; P25* s25 = LuaParameters::Set(dispatchData->L, p25, 25, myFirstReturnArg); 
			P26 p26; P26* s26 = LuaParameters::Set(dispatchData->L, p26, 26, myFirstReturnArg); 
			P27 p27; P27* s27 = LuaParameters::Set(dispatchData->L, p27, 27, myFirstReturnArg); 
			P28 p28; P28* s28 = LuaParameters::Set(dispatchData->L, p28, 28, myFirstReturnArg); 
			P29 p29; P29* s29 = LuaParameters::Set(dispatchData->L, p29, 29, myFirstReturnArg); 
			P30 p30; P30* s30 = LuaParameters::Set(dispatchData->L, p30, 30, myFirstReturnArg); 
			P31 p31; P31* s31 = LuaParameters::Set(dispatchData->L, p31, 31, myFirstReturnArg); 
			P32 p32; P32* s32 = LuaParameters::Set(dispatchData->L, p32, 32, myFirstReturnArg); 
			P33 p33; P33* s33 = LuaParameters::Set(dispatchData->L, p33, 33, myFirstReturnArg); 
			P34 p34; P34* s34 = LuaParameters::Set(dispatchData->L, p34, 34, myFirstReturnArg); 
			P35 p35; P35* s35 = LuaParameters::Set(dispatchData->L, p35, 35, myFirstReturnArg); 
			P36 p36; P36* s36 = LuaParameters::Set(dispatchData->L, p36, 36, myFirstReturnArg); 
			P37 p37; P37* s37 = LuaParameters::Set(dispatchData->L, p37, 37, myFirstReturnArg); 
			P38 p38; P38* s38 = LuaParameters::Set(dispatchData->L, p38, 38, myFirstReturnArg); 
			P39 p39; P39* s39 = LuaParameters::Set(dispatchData->L, p39, 39, myFirstReturnArg); 
			P40 p40; P40* s40 = LuaParameters::Set(dispatchData->L, p40, 40, myFirstReturnArg); 
			P41 p41; P41* s41 = LuaParameters::Set(dispatchData->L, p41, 41, myFirstReturnArg); 
			P42 p42; P42* s42 = LuaParameters::Set(dispatchData->L, p42, 42, myFirstReturnArg); 
			P43 p43; P43* s43 = LuaParameters::Set(dispatchData->L, p43, 43, myFirstReturnArg); 
			P44 p44; P44* s44 = LuaParameters::Set(dispatchData->L, p44, 44, myFirstReturnArg); 
			P45 p45; P45* s45 = LuaParameters::Set(dispatchData->L, p45, 45, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19, *s20, *s21, *s22, *s23, *s24, *s25, *s26, *s27, *s28, *s29, *s30, *s31, *s32, *s33, *s34, *s35, *s36, *s37, *s38, *s39, *s40, *s41, *s42, *s43, *s44, *s45);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			LuaParameters::Return(s20, 20, myFirstReturnArg);
			LuaParameters::Return(s21, 21, myFirstReturnArg);
			LuaParameters::Return(s22, 22, myFirstReturnArg);
			LuaParameters::Return(s23, 23, myFirstReturnArg);
			LuaParameters::Return(s24, 24, myFirstReturnArg);
			LuaParameters::Return(s25, 25, myFirstReturnArg);
			LuaParameters::Return(s26, 26, myFirstReturnArg);
			LuaParameters::Return(s27, 27, myFirstReturnArg);
			LuaParameters::Return(s28, 28, myFirstReturnArg);
			LuaParameters::Return(s29, 29, myFirstReturnArg);
			LuaParameters::Return(s30, 30, myFirstReturnArg);
			LuaParameters::Return(s31, 31, myFirstReturnArg);
			LuaParameters::Return(s32, 32, myFirstReturnArg);
			LuaParameters::Return(s33, 33, myFirstReturnArg);
			LuaParameters::Return(s34, 34, myFirstReturnArg);
			LuaParameters::Return(s35, 35, myFirstReturnArg);
			LuaParameters::Return(s36, 36, myFirstReturnArg);
			LuaParameters::Return(s37, 37, myFirstReturnArg);
			LuaParameters::Return(s38, 38, myFirstReturnArg);
			LuaParameters::Return(s39, 39, myFirstReturnArg);
			LuaParameters::Return(s40, 40, myFirstReturnArg);
			LuaParameters::Return(s41, 41, myFirstReturnArg);
			LuaParameters::Return(s42, 42, myFirstReturnArg);
			LuaParameters::Return(s43, 43, myFirstReturnArg);
			LuaParameters::Return(s44, 44, myFirstReturnArg);
			LuaParameters::Return(s45, 45, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40, typename P41, typename P42, typename P43, typename P44, typename P45 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&, P44&, P45&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&, P44&, P45&);
		typedef TYPELIST_45(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43, P44, P45) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall45< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43, P44, P45 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40, typename P41, typename P42, typename P43, typename P44, typename P45 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&, P44&, P45&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&, P44&, P45&);
		typedef TYPELIST_45(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43, P44, P45) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall45< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43, P44, P45 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall46

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40, typename P41, typename P42, typename P43, typename P44, typename P45, typename P46 >
	class LuaCall46 : public Registry::CallInterface
	{
	public:

		LuaCall46(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 46 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			P20 p20; P20* s20 = LuaParameters::Set(dispatchData->L, p20, 20, myFirstReturnArg); 
			P21 p21; P21* s21 = LuaParameters::Set(dispatchData->L, p21, 21, myFirstReturnArg); 
			P22 p22; P22* s22 = LuaParameters::Set(dispatchData->L, p22, 22, myFirstReturnArg); 
			P23 p23; P23* s23 = LuaParameters::Set(dispatchData->L, p23, 23, myFirstReturnArg); 
			P24 p24; P24* s24 = LuaParameters::Set(dispatchData->L, p24, 24, myFirstReturnArg); 
			P25 p25; P25* s25 = LuaParameters::Set(dispatchData->L, p25, 25, myFirstReturnArg); 
			P26 p26; P26* s26 = LuaParameters::Set(dispatchData->L, p26, 26, myFirstReturnArg); 
			P27 p27; P27* s27 = LuaParameters::Set(dispatchData->L, p27, 27, myFirstReturnArg); 
			P28 p28; P28* s28 = LuaParameters::Set(dispatchData->L, p28, 28, myFirstReturnArg); 
			P29 p29; P29* s29 = LuaParameters::Set(dispatchData->L, p29, 29, myFirstReturnArg); 
			P30 p30; P30* s30 = LuaParameters::Set(dispatchData->L, p30, 30, myFirstReturnArg); 
			P31 p31; P31* s31 = LuaParameters::Set(dispatchData->L, p31, 31, myFirstReturnArg); 
			P32 p32; P32* s32 = LuaParameters::Set(dispatchData->L, p32, 32, myFirstReturnArg); 
			P33 p33; P33* s33 = LuaParameters::Set(dispatchData->L, p33, 33, myFirstReturnArg); 
			P34 p34; P34* s34 = LuaParameters::Set(dispatchData->L, p34, 34, myFirstReturnArg); 
			P35 p35; P35* s35 = LuaParameters::Set(dispatchData->L, p35, 35, myFirstReturnArg); 
			P36 p36; P36* s36 = LuaParameters::Set(dispatchData->L, p36, 36, myFirstReturnArg); 
			P37 p37; P37* s37 = LuaParameters::Set(dispatchData->L, p37, 37, myFirstReturnArg); 
			P38 p38; P38* s38 = LuaParameters::Set(dispatchData->L, p38, 38, myFirstReturnArg); 
			P39 p39; P39* s39 = LuaParameters::Set(dispatchData->L, p39, 39, myFirstReturnArg); 
			P40 p40; P40* s40 = LuaParameters::Set(dispatchData->L, p40, 40, myFirstReturnArg); 
			P41 p41; P41* s41 = LuaParameters::Set(dispatchData->L, p41, 41, myFirstReturnArg); 
			P42 p42; P42* s42 = LuaParameters::Set(dispatchData->L, p42, 42, myFirstReturnArg); 
			P43 p43; P43* s43 = LuaParameters::Set(dispatchData->L, p43, 43, myFirstReturnArg); 
			P44 p44; P44* s44 = LuaParameters::Set(dispatchData->L, p44, 44, myFirstReturnArg); 
			P45 p45; P45* s45 = LuaParameters::Set(dispatchData->L, p45, 45, myFirstReturnArg); 
			P46 p46; P46* s46 = LuaParameters::Set(dispatchData->L, p46, 46, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19, *s20, *s21, *s22, *s23, *s24, *s25, *s26, *s27, *s28, *s29, *s30, *s31, *s32, *s33, *s34, *s35, *s36, *s37, *s38, *s39, *s40, *s41, *s42, *s43, *s44, *s45, *s46);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			LuaParameters::Return(s20, 20, myFirstReturnArg);
			LuaParameters::Return(s21, 21, myFirstReturnArg);
			LuaParameters::Return(s22, 22, myFirstReturnArg);
			LuaParameters::Return(s23, 23, myFirstReturnArg);
			LuaParameters::Return(s24, 24, myFirstReturnArg);
			LuaParameters::Return(s25, 25, myFirstReturnArg);
			LuaParameters::Return(s26, 26, myFirstReturnArg);
			LuaParameters::Return(s27, 27, myFirstReturnArg);
			LuaParameters::Return(s28, 28, myFirstReturnArg);
			LuaParameters::Return(s29, 29, myFirstReturnArg);
			LuaParameters::Return(s30, 30, myFirstReturnArg);
			LuaParameters::Return(s31, 31, myFirstReturnArg);
			LuaParameters::Return(s32, 32, myFirstReturnArg);
			LuaParameters::Return(s33, 33, myFirstReturnArg);
			LuaParameters::Return(s34, 34, myFirstReturnArg);
			LuaParameters::Return(s35, 35, myFirstReturnArg);
			LuaParameters::Return(s36, 36, myFirstReturnArg);
			LuaParameters::Return(s37, 37, myFirstReturnArg);
			LuaParameters::Return(s38, 38, myFirstReturnArg);
			LuaParameters::Return(s39, 39, myFirstReturnArg);
			LuaParameters::Return(s40, 40, myFirstReturnArg);
			LuaParameters::Return(s41, 41, myFirstReturnArg);
			LuaParameters::Return(s42, 42, myFirstReturnArg);
			LuaParameters::Return(s43, 43, myFirstReturnArg);
			LuaParameters::Return(s44, 44, myFirstReturnArg);
			LuaParameters::Return(s45, 45, myFirstReturnArg);
			LuaParameters::Return(s46, 46, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40, typename P41, typename P42, typename P43, typename P44, typename P45, typename P46 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&, P44&, P45&, P46&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&, P44&, P45&, P46&);
		typedef TYPELIST_46(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43, P44, P45, P46) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall46< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43, P44, P45, P46 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40, typename P41, typename P42, typename P43, typename P44, typename P45, typename P46 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&, P44&, P45&, P46&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&, P44&, P45&, P46&);
		typedef TYPELIST_46(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43, P44, P45, P46) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall46< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43, P44, P45, P46 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall47

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40, typename P41, typename P42, typename P43, typename P44, typename P45, typename P46, typename P47 >
	class LuaCall47 : public Registry::CallInterface
	{
	public:

		LuaCall47(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 47 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			P20 p20; P20* s20 = LuaParameters::Set(dispatchData->L, p20, 20, myFirstReturnArg); 
			P21 p21; P21* s21 = LuaParameters::Set(dispatchData->L, p21, 21, myFirstReturnArg); 
			P22 p22; P22* s22 = LuaParameters::Set(dispatchData->L, p22, 22, myFirstReturnArg); 
			P23 p23; P23* s23 = LuaParameters::Set(dispatchData->L, p23, 23, myFirstReturnArg); 
			P24 p24; P24* s24 = LuaParameters::Set(dispatchData->L, p24, 24, myFirstReturnArg); 
			P25 p25; P25* s25 = LuaParameters::Set(dispatchData->L, p25, 25, myFirstReturnArg); 
			P26 p26; P26* s26 = LuaParameters::Set(dispatchData->L, p26, 26, myFirstReturnArg); 
			P27 p27; P27* s27 = LuaParameters::Set(dispatchData->L, p27, 27, myFirstReturnArg); 
			P28 p28; P28* s28 = LuaParameters::Set(dispatchData->L, p28, 28, myFirstReturnArg); 
			P29 p29; P29* s29 = LuaParameters::Set(dispatchData->L, p29, 29, myFirstReturnArg); 
			P30 p30; P30* s30 = LuaParameters::Set(dispatchData->L, p30, 30, myFirstReturnArg); 
			P31 p31; P31* s31 = LuaParameters::Set(dispatchData->L, p31, 31, myFirstReturnArg); 
			P32 p32; P32* s32 = LuaParameters::Set(dispatchData->L, p32, 32, myFirstReturnArg); 
			P33 p33; P33* s33 = LuaParameters::Set(dispatchData->L, p33, 33, myFirstReturnArg); 
			P34 p34; P34* s34 = LuaParameters::Set(dispatchData->L, p34, 34, myFirstReturnArg); 
			P35 p35; P35* s35 = LuaParameters::Set(dispatchData->L, p35, 35, myFirstReturnArg); 
			P36 p36; P36* s36 = LuaParameters::Set(dispatchData->L, p36, 36, myFirstReturnArg); 
			P37 p37; P37* s37 = LuaParameters::Set(dispatchData->L, p37, 37, myFirstReturnArg); 
			P38 p38; P38* s38 = LuaParameters::Set(dispatchData->L, p38, 38, myFirstReturnArg); 
			P39 p39; P39* s39 = LuaParameters::Set(dispatchData->L, p39, 39, myFirstReturnArg); 
			P40 p40; P40* s40 = LuaParameters::Set(dispatchData->L, p40, 40, myFirstReturnArg); 
			P41 p41; P41* s41 = LuaParameters::Set(dispatchData->L, p41, 41, myFirstReturnArg); 
			P42 p42; P42* s42 = LuaParameters::Set(dispatchData->L, p42, 42, myFirstReturnArg); 
			P43 p43; P43* s43 = LuaParameters::Set(dispatchData->L, p43, 43, myFirstReturnArg); 
			P44 p44; P44* s44 = LuaParameters::Set(dispatchData->L, p44, 44, myFirstReturnArg); 
			P45 p45; P45* s45 = LuaParameters::Set(dispatchData->L, p45, 45, myFirstReturnArg); 
			P46 p46; P46* s46 = LuaParameters::Set(dispatchData->L, p46, 46, myFirstReturnArg); 
			P47 p47; P47* s47 = LuaParameters::Set(dispatchData->L, p47, 47, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19, *s20, *s21, *s22, *s23, *s24, *s25, *s26, *s27, *s28, *s29, *s30, *s31, *s32, *s33, *s34, *s35, *s36, *s37, *s38, *s39, *s40, *s41, *s42, *s43, *s44, *s45, *s46, *s47);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			LuaParameters::Return(s20, 20, myFirstReturnArg);
			LuaParameters::Return(s21, 21, myFirstReturnArg);
			LuaParameters::Return(s22, 22, myFirstReturnArg);
			LuaParameters::Return(s23, 23, myFirstReturnArg);
			LuaParameters::Return(s24, 24, myFirstReturnArg);
			LuaParameters::Return(s25, 25, myFirstReturnArg);
			LuaParameters::Return(s26, 26, myFirstReturnArg);
			LuaParameters::Return(s27, 27, myFirstReturnArg);
			LuaParameters::Return(s28, 28, myFirstReturnArg);
			LuaParameters::Return(s29, 29, myFirstReturnArg);
			LuaParameters::Return(s30, 30, myFirstReturnArg);
			LuaParameters::Return(s31, 31, myFirstReturnArg);
			LuaParameters::Return(s32, 32, myFirstReturnArg);
			LuaParameters::Return(s33, 33, myFirstReturnArg);
			LuaParameters::Return(s34, 34, myFirstReturnArg);
			LuaParameters::Return(s35, 35, myFirstReturnArg);
			LuaParameters::Return(s36, 36, myFirstReturnArg);
			LuaParameters::Return(s37, 37, myFirstReturnArg);
			LuaParameters::Return(s38, 38, myFirstReturnArg);
			LuaParameters::Return(s39, 39, myFirstReturnArg);
			LuaParameters::Return(s40, 40, myFirstReturnArg);
			LuaParameters::Return(s41, 41, myFirstReturnArg);
			LuaParameters::Return(s42, 42, myFirstReturnArg);
			LuaParameters::Return(s43, 43, myFirstReturnArg);
			LuaParameters::Return(s44, 44, myFirstReturnArg);
			LuaParameters::Return(s45, 45, myFirstReturnArg);
			LuaParameters::Return(s46, 46, myFirstReturnArg);
			LuaParameters::Return(s47, 47, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40, typename P41, typename P42, typename P43, typename P44, typename P45, typename P46, typename P47 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&, P44&, P45&, P46&, P47&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&, P44&, P45&, P46&, P47&);
		typedef TYPELIST_47(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43, P44, P45, P46, P47) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall47< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43, P44, P45, P46, P47 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40, typename P41, typename P42, typename P43, typename P44, typename P45, typename P46, typename P47 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&, P44&, P45&, P46&, P47&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&, P44&, P45&, P46&, P47&);
		typedef TYPELIST_47(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43, P44, P45, P46, P47) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall47< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43, P44, P45, P46, P47 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall48

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40, typename P41, typename P42, typename P43, typename P44, typename P45, typename P46, typename P47, typename P48 >
	class LuaCall48 : public Registry::CallInterface
	{
	public:

		LuaCall48(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 48 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			P20 p20; P20* s20 = LuaParameters::Set(dispatchData->L, p20, 20, myFirstReturnArg); 
			P21 p21; P21* s21 = LuaParameters::Set(dispatchData->L, p21, 21, myFirstReturnArg); 
			P22 p22; P22* s22 = LuaParameters::Set(dispatchData->L, p22, 22, myFirstReturnArg); 
			P23 p23; P23* s23 = LuaParameters::Set(dispatchData->L, p23, 23, myFirstReturnArg); 
			P24 p24; P24* s24 = LuaParameters::Set(dispatchData->L, p24, 24, myFirstReturnArg); 
			P25 p25; P25* s25 = LuaParameters::Set(dispatchData->L, p25, 25, myFirstReturnArg); 
			P26 p26; P26* s26 = LuaParameters::Set(dispatchData->L, p26, 26, myFirstReturnArg); 
			P27 p27; P27* s27 = LuaParameters::Set(dispatchData->L, p27, 27, myFirstReturnArg); 
			P28 p28; P28* s28 = LuaParameters::Set(dispatchData->L, p28, 28, myFirstReturnArg); 
			P29 p29; P29* s29 = LuaParameters::Set(dispatchData->L, p29, 29, myFirstReturnArg); 
			P30 p30; P30* s30 = LuaParameters::Set(dispatchData->L, p30, 30, myFirstReturnArg); 
			P31 p31; P31* s31 = LuaParameters::Set(dispatchData->L, p31, 31, myFirstReturnArg); 
			P32 p32; P32* s32 = LuaParameters::Set(dispatchData->L, p32, 32, myFirstReturnArg); 
			P33 p33; P33* s33 = LuaParameters::Set(dispatchData->L, p33, 33, myFirstReturnArg); 
			P34 p34; P34* s34 = LuaParameters::Set(dispatchData->L, p34, 34, myFirstReturnArg); 
			P35 p35; P35* s35 = LuaParameters::Set(dispatchData->L, p35, 35, myFirstReturnArg); 
			P36 p36; P36* s36 = LuaParameters::Set(dispatchData->L, p36, 36, myFirstReturnArg); 
			P37 p37; P37* s37 = LuaParameters::Set(dispatchData->L, p37, 37, myFirstReturnArg); 
			P38 p38; P38* s38 = LuaParameters::Set(dispatchData->L, p38, 38, myFirstReturnArg); 
			P39 p39; P39* s39 = LuaParameters::Set(dispatchData->L, p39, 39, myFirstReturnArg); 
			P40 p40; P40* s40 = LuaParameters::Set(dispatchData->L, p40, 40, myFirstReturnArg); 
			P41 p41; P41* s41 = LuaParameters::Set(dispatchData->L, p41, 41, myFirstReturnArg); 
			P42 p42; P42* s42 = LuaParameters::Set(dispatchData->L, p42, 42, myFirstReturnArg); 
			P43 p43; P43* s43 = LuaParameters::Set(dispatchData->L, p43, 43, myFirstReturnArg); 
			P44 p44; P44* s44 = LuaParameters::Set(dispatchData->L, p44, 44, myFirstReturnArg); 
			P45 p45; P45* s45 = LuaParameters::Set(dispatchData->L, p45, 45, myFirstReturnArg); 
			P46 p46; P46* s46 = LuaParameters::Set(dispatchData->L, p46, 46, myFirstReturnArg); 
			P47 p47; P47* s47 = LuaParameters::Set(dispatchData->L, p47, 47, myFirstReturnArg); 
			P48 p48; P48* s48 = LuaParameters::Set(dispatchData->L, p48, 48, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19, *s20, *s21, *s22, *s23, *s24, *s25, *s26, *s27, *s28, *s29, *s30, *s31, *s32, *s33, *s34, *s35, *s36, *s37, *s38, *s39, *s40, *s41, *s42, *s43, *s44, *s45, *s46, *s47, *s48);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			LuaParameters::Return(s20, 20, myFirstReturnArg);
			LuaParameters::Return(s21, 21, myFirstReturnArg);
			LuaParameters::Return(s22, 22, myFirstReturnArg);
			LuaParameters::Return(s23, 23, myFirstReturnArg);
			LuaParameters::Return(s24, 24, myFirstReturnArg);
			LuaParameters::Return(s25, 25, myFirstReturnArg);
			LuaParameters::Return(s26, 26, myFirstReturnArg);
			LuaParameters::Return(s27, 27, myFirstReturnArg);
			LuaParameters::Return(s28, 28, myFirstReturnArg);
			LuaParameters::Return(s29, 29, myFirstReturnArg);
			LuaParameters::Return(s30, 30, myFirstReturnArg);
			LuaParameters::Return(s31, 31, myFirstReturnArg);
			LuaParameters::Return(s32, 32, myFirstReturnArg);
			LuaParameters::Return(s33, 33, myFirstReturnArg);
			LuaParameters::Return(s34, 34, myFirstReturnArg);
			LuaParameters::Return(s35, 35, myFirstReturnArg);
			LuaParameters::Return(s36, 36, myFirstReturnArg);
			LuaParameters::Return(s37, 37, myFirstReturnArg);
			LuaParameters::Return(s38, 38, myFirstReturnArg);
			LuaParameters::Return(s39, 39, myFirstReturnArg);
			LuaParameters::Return(s40, 40, myFirstReturnArg);
			LuaParameters::Return(s41, 41, myFirstReturnArg);
			LuaParameters::Return(s42, 42, myFirstReturnArg);
			LuaParameters::Return(s43, 43, myFirstReturnArg);
			LuaParameters::Return(s44, 44, myFirstReturnArg);
			LuaParameters::Return(s45, 45, myFirstReturnArg);
			LuaParameters::Return(s46, 46, myFirstReturnArg);
			LuaParameters::Return(s47, 47, myFirstReturnArg);
			LuaParameters::Return(s48, 48, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40, typename P41, typename P42, typename P43, typename P44, typename P45, typename P46, typename P47, typename P48 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&, P44&, P45&, P46&, P47&, P48&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&, P44&, P45&, P46&, P47&, P48&);
		typedef TYPELIST_48(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43, P44, P45, P46, P47, P48) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall48< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43, P44, P45, P46, P47, P48 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40, typename P41, typename P42, typename P43, typename P44, typename P45, typename P46, typename P47, typename P48 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&, P44&, P45&, P46&, P47&, P48&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&, P44&, P45&, P46&, P47&, P48&);
		typedef TYPELIST_48(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43, P44, P45, P46, P47, P48) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall48< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43, P44, P45, P46, P47, P48 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall49

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40, typename P41, typename P42, typename P43, typename P44, typename P45, typename P46, typename P47, typename P48, typename P49 >
	class LuaCall49 : public Registry::CallInterface
	{
	public:

		LuaCall49(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 49 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			P20 p20; P20* s20 = LuaParameters::Set(dispatchData->L, p20, 20, myFirstReturnArg); 
			P21 p21; P21* s21 = LuaParameters::Set(dispatchData->L, p21, 21, myFirstReturnArg); 
			P22 p22; P22* s22 = LuaParameters::Set(dispatchData->L, p22, 22, myFirstReturnArg); 
			P23 p23; P23* s23 = LuaParameters::Set(dispatchData->L, p23, 23, myFirstReturnArg); 
			P24 p24; P24* s24 = LuaParameters::Set(dispatchData->L, p24, 24, myFirstReturnArg); 
			P25 p25; P25* s25 = LuaParameters::Set(dispatchData->L, p25, 25, myFirstReturnArg); 
			P26 p26; P26* s26 = LuaParameters::Set(dispatchData->L, p26, 26, myFirstReturnArg); 
			P27 p27; P27* s27 = LuaParameters::Set(dispatchData->L, p27, 27, myFirstReturnArg); 
			P28 p28; P28* s28 = LuaParameters::Set(dispatchData->L, p28, 28, myFirstReturnArg); 
			P29 p29; P29* s29 = LuaParameters::Set(dispatchData->L, p29, 29, myFirstReturnArg); 
			P30 p30; P30* s30 = LuaParameters::Set(dispatchData->L, p30, 30, myFirstReturnArg); 
			P31 p31; P31* s31 = LuaParameters::Set(dispatchData->L, p31, 31, myFirstReturnArg); 
			P32 p32; P32* s32 = LuaParameters::Set(dispatchData->L, p32, 32, myFirstReturnArg); 
			P33 p33; P33* s33 = LuaParameters::Set(dispatchData->L, p33, 33, myFirstReturnArg); 
			P34 p34; P34* s34 = LuaParameters::Set(dispatchData->L, p34, 34, myFirstReturnArg); 
			P35 p35; P35* s35 = LuaParameters::Set(dispatchData->L, p35, 35, myFirstReturnArg); 
			P36 p36; P36* s36 = LuaParameters::Set(dispatchData->L, p36, 36, myFirstReturnArg); 
			P37 p37; P37* s37 = LuaParameters::Set(dispatchData->L, p37, 37, myFirstReturnArg); 
			P38 p38; P38* s38 = LuaParameters::Set(dispatchData->L, p38, 38, myFirstReturnArg); 
			P39 p39; P39* s39 = LuaParameters::Set(dispatchData->L, p39, 39, myFirstReturnArg); 
			P40 p40; P40* s40 = LuaParameters::Set(dispatchData->L, p40, 40, myFirstReturnArg); 
			P41 p41; P41* s41 = LuaParameters::Set(dispatchData->L, p41, 41, myFirstReturnArg); 
			P42 p42; P42* s42 = LuaParameters::Set(dispatchData->L, p42, 42, myFirstReturnArg); 
			P43 p43; P43* s43 = LuaParameters::Set(dispatchData->L, p43, 43, myFirstReturnArg); 
			P44 p44; P44* s44 = LuaParameters::Set(dispatchData->L, p44, 44, myFirstReturnArg); 
			P45 p45; P45* s45 = LuaParameters::Set(dispatchData->L, p45, 45, myFirstReturnArg); 
			P46 p46; P46* s46 = LuaParameters::Set(dispatchData->L, p46, 46, myFirstReturnArg); 
			P47 p47; P47* s47 = LuaParameters::Set(dispatchData->L, p47, 47, myFirstReturnArg); 
			P48 p48; P48* s48 = LuaParameters::Set(dispatchData->L, p48, 48, myFirstReturnArg); 
			P49 p49; P49* s49 = LuaParameters::Set(dispatchData->L, p49, 49, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19, *s20, *s21, *s22, *s23, *s24, *s25, *s26, *s27, *s28, *s29, *s30, *s31, *s32, *s33, *s34, *s35, *s36, *s37, *s38, *s39, *s40, *s41, *s42, *s43, *s44, *s45, *s46, *s47, *s48, *s49);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			LuaParameters::Return(s20, 20, myFirstReturnArg);
			LuaParameters::Return(s21, 21, myFirstReturnArg);
			LuaParameters::Return(s22, 22, myFirstReturnArg);
			LuaParameters::Return(s23, 23, myFirstReturnArg);
			LuaParameters::Return(s24, 24, myFirstReturnArg);
			LuaParameters::Return(s25, 25, myFirstReturnArg);
			LuaParameters::Return(s26, 26, myFirstReturnArg);
			LuaParameters::Return(s27, 27, myFirstReturnArg);
			LuaParameters::Return(s28, 28, myFirstReturnArg);
			LuaParameters::Return(s29, 29, myFirstReturnArg);
			LuaParameters::Return(s30, 30, myFirstReturnArg);
			LuaParameters::Return(s31, 31, myFirstReturnArg);
			LuaParameters::Return(s32, 32, myFirstReturnArg);
			LuaParameters::Return(s33, 33, myFirstReturnArg);
			LuaParameters::Return(s34, 34, myFirstReturnArg);
			LuaParameters::Return(s35, 35, myFirstReturnArg);
			LuaParameters::Return(s36, 36, myFirstReturnArg);
			LuaParameters::Return(s37, 37, myFirstReturnArg);
			LuaParameters::Return(s38, 38, myFirstReturnArg);
			LuaParameters::Return(s39, 39, myFirstReturnArg);
			LuaParameters::Return(s40, 40, myFirstReturnArg);
			LuaParameters::Return(s41, 41, myFirstReturnArg);
			LuaParameters::Return(s42, 42, myFirstReturnArg);
			LuaParameters::Return(s43, 43, myFirstReturnArg);
			LuaParameters::Return(s44, 44, myFirstReturnArg);
			LuaParameters::Return(s45, 45, myFirstReturnArg);
			LuaParameters::Return(s46, 46, myFirstReturnArg);
			LuaParameters::Return(s47, 47, myFirstReturnArg);
			LuaParameters::Return(s48, 48, myFirstReturnArg);
			LuaParameters::Return(s49, 49, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40, typename P41, typename P42, typename P43, typename P44, typename P45, typename P46, typename P47, typename P48, typename P49 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&, P44&, P45&, P46&, P47&, P48&, P49&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&, P44&, P45&, P46&, P47&, P48&, P49&);
		typedef TYPELIST_49(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43, P44, P45, P46, P47, P48, P49) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall49< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43, P44, P45, P46, P47, P48, P49 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40, typename P41, typename P42, typename P43, typename P44, typename P45, typename P46, typename P47, typename P48, typename P49 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&, P44&, P45&, P46&, P47&, P48&, P49&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&, P44&, P45&, P46&, P47&, P48&, P49&);
		typedef TYPELIST_49(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43, P44, P45, P46, P47, P48, P49) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall49< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43, P44, P45, P46, P47, P48, P49 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





	//////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall50

	template<  typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40, typename P41, typename P42, typename P43, typename P44, typename P45, typename P46, typename P47, typename P48, typename P49, typename P50 >
	class LuaCall50 : public Registry::CallInterface
	{
	public:

		LuaCall50(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			dispatchData->numReturnValues = 50 - myFirstReturnArg + 1;
			P1 p1; P1* s1 = LuaParameters::Set(dispatchData->L, p1, 1, myFirstReturnArg); 
			P2 p2; P2* s2 = LuaParameters::Set(dispatchData->L, p2, 2, myFirstReturnArg); 
			P3 p3; P3* s3 = LuaParameters::Set(dispatchData->L, p3, 3, myFirstReturnArg); 
			P4 p4; P4* s4 = LuaParameters::Set(dispatchData->L, p4, 4, myFirstReturnArg); 
			P5 p5; P5* s5 = LuaParameters::Set(dispatchData->L, p5, 5, myFirstReturnArg); 
			P6 p6; P6* s6 = LuaParameters::Set(dispatchData->L, p6, 6, myFirstReturnArg); 
			P7 p7; P7* s7 = LuaParameters::Set(dispatchData->L, p7, 7, myFirstReturnArg); 
			P8 p8; P8* s8 = LuaParameters::Set(dispatchData->L, p8, 8, myFirstReturnArg); 
			P9 p9; P9* s9 = LuaParameters::Set(dispatchData->L, p9, 9, myFirstReturnArg); 
			P10 p10; P10* s10 = LuaParameters::Set(dispatchData->L, p10, 10, myFirstReturnArg); 
			P11 p11; P11* s11 = LuaParameters::Set(dispatchData->L, p11, 11, myFirstReturnArg); 
			P12 p12; P12* s12 = LuaParameters::Set(dispatchData->L, p12, 12, myFirstReturnArg); 
			P13 p13; P13* s13 = LuaParameters::Set(dispatchData->L, p13, 13, myFirstReturnArg); 
			P14 p14; P14* s14 = LuaParameters::Set(dispatchData->L, p14, 14, myFirstReturnArg); 
			P15 p15; P15* s15 = LuaParameters::Set(dispatchData->L, p15, 15, myFirstReturnArg); 
			P16 p16; P16* s16 = LuaParameters::Set(dispatchData->L, p16, 16, myFirstReturnArg); 
			P17 p17; P17* s17 = LuaParameters::Set(dispatchData->L, p17, 17, myFirstReturnArg); 
			P18 p18; P18* s18 = LuaParameters::Set(dispatchData->L, p18, 18, myFirstReturnArg); 
			P19 p19; P19* s19 = LuaParameters::Set(dispatchData->L, p19, 19, myFirstReturnArg); 
			P20 p20; P20* s20 = LuaParameters::Set(dispatchData->L, p20, 20, myFirstReturnArg); 
			P21 p21; P21* s21 = LuaParameters::Set(dispatchData->L, p21, 21, myFirstReturnArg); 
			P22 p22; P22* s22 = LuaParameters::Set(dispatchData->L, p22, 22, myFirstReturnArg); 
			P23 p23; P23* s23 = LuaParameters::Set(dispatchData->L, p23, 23, myFirstReturnArg); 
			P24 p24; P24* s24 = LuaParameters::Set(dispatchData->L, p24, 24, myFirstReturnArg); 
			P25 p25; P25* s25 = LuaParameters::Set(dispatchData->L, p25, 25, myFirstReturnArg); 
			P26 p26; P26* s26 = LuaParameters::Set(dispatchData->L, p26, 26, myFirstReturnArg); 
			P27 p27; P27* s27 = LuaParameters::Set(dispatchData->L, p27, 27, myFirstReturnArg); 
			P28 p28; P28* s28 = LuaParameters::Set(dispatchData->L, p28, 28, myFirstReturnArg); 
			P29 p29; P29* s29 = LuaParameters::Set(dispatchData->L, p29, 29, myFirstReturnArg); 
			P30 p30; P30* s30 = LuaParameters::Set(dispatchData->L, p30, 30, myFirstReturnArg); 
			P31 p31; P31* s31 = LuaParameters::Set(dispatchData->L, p31, 31, myFirstReturnArg); 
			P32 p32; P32* s32 = LuaParameters::Set(dispatchData->L, p32, 32, myFirstReturnArg); 
			P33 p33; P33* s33 = LuaParameters::Set(dispatchData->L, p33, 33, myFirstReturnArg); 
			P34 p34; P34* s34 = LuaParameters::Set(dispatchData->L, p34, 34, myFirstReturnArg); 
			P35 p35; P35* s35 = LuaParameters::Set(dispatchData->L, p35, 35, myFirstReturnArg); 
			P36 p36; P36* s36 = LuaParameters::Set(dispatchData->L, p36, 36, myFirstReturnArg); 
			P37 p37; P37* s37 = LuaParameters::Set(dispatchData->L, p37, 37, myFirstReturnArg); 
			P38 p38; P38* s38 = LuaParameters::Set(dispatchData->L, p38, 38, myFirstReturnArg); 
			P39 p39; P39* s39 = LuaParameters::Set(dispatchData->L, p39, 39, myFirstReturnArg); 
			P40 p40; P40* s40 = LuaParameters::Set(dispatchData->L, p40, 40, myFirstReturnArg); 
			P41 p41; P41* s41 = LuaParameters::Set(dispatchData->L, p41, 41, myFirstReturnArg); 
			P42 p42; P42* s42 = LuaParameters::Set(dispatchData->L, p42, 42, myFirstReturnArg); 
			P43 p43; P43* s43 = LuaParameters::Set(dispatchData->L, p43, 43, myFirstReturnArg); 
			P44 p44; P44* s44 = LuaParameters::Set(dispatchData->L, p44, 44, myFirstReturnArg); 
			P45 p45; P45* s45 = LuaParameters::Set(dispatchData->L, p45, 45, myFirstReturnArg); 
			P46 p46; P46* s46 = LuaParameters::Set(dispatchData->L, p46, 46, myFirstReturnArg); 
			P47 p47; P47* s47 = LuaParameters::Set(dispatchData->L, p47, 47, myFirstReturnArg); 
			P48 p48; P48* s48 = LuaParameters::Set(dispatchData->L, p48, 48, myFirstReturnArg); 
			P49 p49; P49* s49 = LuaParameters::Set(dispatchData->L, p49, 49, myFirstReturnArg); 
			P50 p50; P50* s50 = LuaParameters::Set(dispatchData->L, p50, 50, myFirstReturnArg); 
			
			ExecutionState rv = myFunction(*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s18, *s19, *s20, *s21, *s22, *s23, *s24, *s25, *s26, *s27, *s28, *s29, *s30, *s31, *s32, *s33, *s34, *s35, *s36, *s37, *s38, *s39, *s40, *s41, *s42, *s43, *s44, *s45, *s46, *s47, *s48, *s49, *s50);
			LuaParameters::Return(s1, 1, myFirstReturnArg);
			LuaParameters::Return(s2, 2, myFirstReturnArg);
			LuaParameters::Return(s3, 3, myFirstReturnArg);
			LuaParameters::Return(s4, 4, myFirstReturnArg);
			LuaParameters::Return(s5, 5, myFirstReturnArg);
			LuaParameters::Return(s6, 6, myFirstReturnArg);
			LuaParameters::Return(s7, 7, myFirstReturnArg);
			LuaParameters::Return(s8, 8, myFirstReturnArg);
			LuaParameters::Return(s9, 9, myFirstReturnArg);
			LuaParameters::Return(s10, 10, myFirstReturnArg);
			LuaParameters::Return(s11, 11, myFirstReturnArg);
			LuaParameters::Return(s12, 12, myFirstReturnArg);
			LuaParameters::Return(s13, 13, myFirstReturnArg);
			LuaParameters::Return(s14, 14, myFirstReturnArg);
			LuaParameters::Return(s15, 15, myFirstReturnArg);
			LuaParameters::Return(s16, 16, myFirstReturnArg);
			LuaParameters::Return(s17, 17, myFirstReturnArg);
			LuaParameters::Return(s18, 18, myFirstReturnArg);
			LuaParameters::Return(s19, 19, myFirstReturnArg);
			LuaParameters::Return(s20, 20, myFirstReturnArg);
			LuaParameters::Return(s21, 21, myFirstReturnArg);
			LuaParameters::Return(s22, 22, myFirstReturnArg);
			LuaParameters::Return(s23, 23, myFirstReturnArg);
			LuaParameters::Return(s24, 24, myFirstReturnArg);
			LuaParameters::Return(s25, 25, myFirstReturnArg);
			LuaParameters::Return(s26, 26, myFirstReturnArg);
			LuaParameters::Return(s27, 27, myFirstReturnArg);
			LuaParameters::Return(s28, 28, myFirstReturnArg);
			LuaParameters::Return(s29, 29, myFirstReturnArg);
			LuaParameters::Return(s30, 30, myFirstReturnArg);
			LuaParameters::Return(s31, 31, myFirstReturnArg);
			LuaParameters::Return(s32, 32, myFirstReturnArg);
			LuaParameters::Return(s33, 33, myFirstReturnArg);
			LuaParameters::Return(s34, 34, myFirstReturnArg);
			LuaParameters::Return(s35, 35, myFirstReturnArg);
			LuaParameters::Return(s36, 36, myFirstReturnArg);
			LuaParameters::Return(s37, 37, myFirstReturnArg);
			LuaParameters::Return(s38, 38, myFirstReturnArg);
			LuaParameters::Return(s39, 39, myFirstReturnArg);
			LuaParameters::Return(s40, 40, myFirstReturnArg);
			LuaParameters::Return(s41, 41, myFirstReturnArg);
			LuaParameters::Return(s42, 42, myFirstReturnArg);
			LuaParameters::Return(s43, 43, myFirstReturnArg);
			LuaParameters::Return(s44, 44, myFirstReturnArg);
			LuaParameters::Return(s45, 45, myFirstReturnArg);
			LuaParameters::Return(s46, 46, myFirstReturnArg);
			LuaParameters::Return(s47, 47, myFirstReturnArg);
			LuaParameters::Return(s48, 48, myFirstReturnArg);
			LuaParameters::Return(s49, 49, myFirstReturnArg);
			LuaParameters::Return(s50, 50, myFirstReturnArg);
			
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40, typename P41, typename P42, typename P43, typename P44, typename P45, typename P46, typename P47, typename P48, typename P49, typename P50 >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&, P44&, P45&, P46&, P47&, P48&, P49&, P50&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&, P44&, P45&, P46&, P47&, P48&, P49&, P50&);
		typedef TYPELIST_50(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43, P44, P45, P46, P47, P48, P49, P50) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall50< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43, P44, P45, P46, P47, P48, P49, P50 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11, typename P12, typename P13, typename P14, typename P15, typename P16, typename P17, typename P18, typename P19, typename P20, typename P21, typename P22, typename P23, typename P24, typename P25, typename P26, typename P27, typename P28, typename P29, typename P30, typename P31, typename P32, typename P33, typename P34, typename P35, typename P36, typename P37, typename P38, typename P39, typename P40, typename P41, typename P42, typename P43, typename P44, typename P45, typename P46, typename P47, typename P48, typename P49, typename P50 >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&, P44&, P45&, P46&, P47&, P48&, P49&, P50&), unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&, P10&, P11&, P12&, P13&, P14&, P15&, P16&, P17&, P18&, P19&, P20&, P21&, P22&, P23&, P24&, P25&, P26&, P27&, P28&, P29&, P30&, P31&, P32&, P33&, P34&, P35&, P36&, P37&, P38&, P39&, P40&, P41&, P42&, P43&, P44&, P45&, P46&, P47&, P48&, P49&, P50&);
		typedef TYPELIST_50(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43, P44, P45, P46, P47, P48, P49, P50) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef LuaCall50< Function, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40, P41, P42, P43, P44, P45, P46, P47, P48, P49, P50 > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}





} 

 #endif