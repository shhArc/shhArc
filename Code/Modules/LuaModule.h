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

#ifndef LUAMODULE_H
#define LUAMODULE_H


#include "../Common/SecureStl.h"
#include "../Common/Classifier.h"
#include "../Common/Dictionary.h"
#include "../Arc/Module.h"




namespace shh {

	class LuaModule : public Module
	{
	public:

		virtual bool Register(const std::string& alias, const StringKeyDictionary& sd, Privileges privileges);
		virtual std::string GetName() const { return GetNameStatic(); }
		static std::string GetNameStatic() { return StripName(std::string(typeid(LuaModule).name())); }


		static int DeepCopy(lua_State* L);
		static int DeepCompare(lua_State* L);
		static ExecutionState LogError(std::string& s);
		static int ErrorBox(lua_State* L);
		static int Trace(lua_State* L);
		static ExecutionState FilterTrace(Classifier& classifiers);
		static void Trace(int arg, const Classifier& classifiers);
		static std::string Print(unsigned int arg);
	
	private:

		static Classifier ourTraceFilter;
	
	};
} // namespace shh
#endif