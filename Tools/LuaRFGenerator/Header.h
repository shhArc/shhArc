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


#ifndef SHH_LUA_TYPES

#define SHH_LUA_TYPES

typedef struct luaL_Reg LuaFunctionReg;
typedef lua_CFunction LuaCFunction;
typedef unsigned int LuaSize;
typedef int LuaTypeId;
typedef GCObject LuaGCObject;

namespace shh
{

	class BaseType;

	typedef struct LuaUserData
	{
		void* myShhType;
		void* myData;
		LuaTypeId myTypeId;
		BaseType* myType;
		static const void* ourMarker;
	} LuaUserData;
}
#endif


namespace shh
{


	////////////////////////////////////////////////////////////////////////////////////////////////
	// Lua C type function taking a lua_State as argument
	void LuaRegisterFunction(lua_State* L, const std::string name, LuaCFunction& fn, void* data, LuaTypeId dataType);
	void RegisterLuaFunction(const std::string& name, LuaCFunction fn, void* data = NULL, LuaTypeId dataType = 0);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// LuaCall0

	void RegisterLuaFunction(const std::string& functionName, ExecutionState(*f)(), unsigned int firstReturnArg, const GCPtr<ModuleInterface>& module);
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
			shh::BaseType* bt = shh::Registry::GetRegistry().GetType(dataType);
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
