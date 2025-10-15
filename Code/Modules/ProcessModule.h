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

#ifndef PROCESSMODULE_H
#define PROCESSMODULE_H

#include "../Common/SecureStl.h"
#include "../Arc/Module.h"


namespace shh {

	class Process;
	
	class ProcessModule : public Module
	{
	public:

	
		virtual bool Register(const std::string& alias, const StringKeyDictionary& sd, Privileges privileges);
		virtual std::string GetName() const { return GetNameStatic(); }
		static std::string GetNameStatic() { return StripName(std::string(typeid(ProcessModule).name())); }

		static int ExecuteFile(lua_State* L);
		static int ExecuteString(lua_State* L);
		static int SendMsg(lua_State* L);
		static int SetTimer(lua_State* L);
		static ExecutionState StopTimer(shhId& id, bool& result);
		static ExecutionState YieldProcess();
		static ExecutionState GetMsgScheduledTime(double &time);
		static ExecutionState GetMsgRecievedTime(double &time);
		static ExecutionState GetMsgDelta(double &time);
		
	
	private:

		
		static bool Execute(const GCPtr<Process>& lp, Implementation implementation, std::string& s, bool isFile);

	};
} // namespace shh
#endif