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

#ifndef ENVIRONMENTMODULE_H
#define ENVIRONMENTMODULE_H

#include "../Common/SecureStl.h"
#include "../Arc/Module.h"


namespace shh {

	class Realm;

	class EnvironmentModule : public Module
	{
	public:

		virtual bool RegisterTypes(const std::string& alias, const StringKeyDictionary& sd);
		virtual bool Register(const std::string& alias, const StringKeyDictionary& sd, Privileges privileges);

		static ExecutionState CreateWorld(std::string& worldName, std::string& realmName, VariantKeyDictionary& vd, bool& result);
		static ExecutionState DestroyWorld(std::string& worldName, bool& result);
		static ExecutionState EnterWorld(std::string& worldName, bool& result);
		static ExecutionState ExitWorld();


		static ExecutionState GetLocalStr(std::string& key, std::string& defaultValue, std::string& value);
		static ExecutionState GetLocalNum(std::string& key, double& defaultValue, double& value);
		static ExecutionState GetGlobalStr(std::string& key, std::string& defaultValue, std::string& value);
		static ExecutionState GetGlobalNum(std::string& key, double& defaultValue, double& value);

		static ExecutionState SetLocalStr(std::string& key, std::string& value);
		static ExecutionState SetLocalNum(std::string& key, double& value);
		static ExecutionState SetGlobalStr(std::string& key, std::string& value);
		static ExecutionState SetGlobalNum(std::string& key, double& value);

		virtual std::string GetName() const { return GetNameStatic(); }
		static std::string GetNameStatic() { return StripName(std::string(typeid(EnvironmentModule).name())); }

	};

}
#endif
