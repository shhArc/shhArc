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




#ifndef LUAHELPERFUNCTIONS_H
#define LUAHELPERFUNCTIONS_H

#include "../Config/GCPtr.h"
#include "../Arc/Registry.h"

#include "LuaWrapper.h"
#include <vector>
#include <string>

namespace shh
{
    class Process;
    class BaseType;

    class LuaHelperFunctions
    {
    public:

		typedef std::map<const void*, TValue> ValueMap;

        static void DeepCopy(lua_State* from, lua_State* to, const TValue* toClone, bool global = false, bool registry = false);
        static bool DeepCompare(lua_State* L, const TValue* o1, const TValue* o2);
        static void Print(lua_State* L, TValue* value, std::string& result, int indent = 0);
        static bool PushVariant(lua_State* to, const Variant* toClone);
		template<class V>
		static bool PopVariant(lua_State* from, const TValue* toClone, Variant& key, VariantKeyDictionary* dict, ValueMap& alreadyClonedMap);
		static bool PushDictionary(lua_State* to, const VariantKeyDictionary* dict);
        static bool PopDictionary(lua_State* from, const TValue* toClone, VariantKeyDictionary* dict);
        static void GetFunctionNames(lua_State* L, TValue const* table, std::vector<std::string>& names);
        static void OverideFunctions(lua_State* L, TValue const* table, const std::vector<std::string>& names, const std::string& overidePrefix);
        static bool ValidateFunctionNames(lua_State* L, bool allowClassOnlyFunctions,
            const std::string& inheritancePrefix,
            const std::string& messagePrefix,
            const std::string& timerPrefix,
            const std::string& systemPrefix,
            const std::string& staticPrefix,
            const std::string& initializePrefix,
            const std::string& finalizePrefix,
            const std::string& updatePrefix,
            std::string& errorMessage);

	private:

		static bool PopDictionary(lua_State* from, const TValue* toClone, VariantKeyDictionary* dict, ValueMap& alreadyClonedMap);

    };



	// --------------------------------------------------------------------------						
	// Function:	PopVariant	
	// Description:	pops a variant and copies into a dictionary
	// Arguments:	lua state, value popped, value key, dictionary to copy into,
	//				map of already cloned items (if multiple refs)
	// Returns:		is successful
	// --------------------------------------------------------------------------
	template<class V>
	bool LuaHelperFunctions::PopVariant(lua_State* from, const TValue* toClone, Variant& key, VariantKeyDictionary* dict, ValueMap& alreadyClonedMap)
	{
		LuaTypeId type = LuaGetTypeId(toClone);


		if (type == LUA_TNIL || type == LUA_TLIGHTUSERDATA || type == LUA_TFUNCTION || type == LUA_TTHREAD)
		{
			return false;
		}
		else if (type == LUA_TTABLE)
		{
			VariantKeyDictionary v;
			if (PopDictionary(from, toClone, &v, alreadyClonedMap))
				dict->Set(key, v);
			else
				return false;
		}
		else if (type == LUA_TBOOLEAN)
		{
			dict->Set(key, LuaGetBoolean(toClone));
		}
		else if (type == LUA_TNUMBER && LuaGetSubType(toClone) != LUA_VNUMFLT)
		{
			dict->Set(key, LuaGetNumber(toClone));
		}
		else if (type == LUA_TNUMBER && LuaGetSubType(toClone) != LUA_VNUMINT)
		{
			dict->Set(key, LuaGetInteger(toClone));
		}
		else if (type == LUA_TSTRING)
		{
			dict->Set(key, std::string(LuaGetString(toClone)));
		}
		else if (abs(type) <= (int)Registry::GetRegistry().GetLastTypeId() && type < 0)
		{
			void* v = LuaGetUserData(from, toClone);
			const BaseType* bt = LuaGetType(from, toClone);
			if (!bt->SetDictionary(&key, v, dict))
				return false;
		}
		else
		{
			return false;
		}

		return true;
	}
}


#endif