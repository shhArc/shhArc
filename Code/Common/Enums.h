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

#ifndef ENUMS_H
#define ENUMS_H




namespace shh {

	typedef long long shhId;

	typedef enum
	{
		shhArc = 0,
		Engine ,
		Lua,
		Python,
		TOTAL_IMPLEMENTATIONS
	} Implementation;


	typedef enum
	{
		NoImps = 0,
		shhArcImp = 1, 
		EngineImp = 2,
		LuaImp = 4,
		PythonImp = 8,
		AllImps = 4294967295,
	} SupportedImps;
	
	inline SupportedImps operator|(const SupportedImps& r1, const SupportedImps& r2) { return (SupportedImps)((unsigned int)r1 | (unsigned int)r2); }
	inline SupportedImps operator&(const SupportedImps& r1, const SupportedImps& r2) { return (SupportedImps)((unsigned int)r1 & (unsigned int)r2); }
	inline SupportedImps operator~(const SupportedImps& r) { return (SupportedImps)~(unsigned int)r; }



	typedef enum
	{
		ExecutionOk,
		ExecutionReady,
		ExecutionBusy,
		ExecutionScheduled,
		ExecutionContinue,
		ExecutionCompleted,
		ExecutionTerminate,
		ExecutionYielded,
		ExecutionTimedOut,
		ExecutionAwaitingCallback,
		ExecutionReceivingCallback,
		ExecutionFailed,
		ExecutionError
	}	ExecutionState;


	typedef enum
	{
		NoPrivilege = 0,
		DebugPrivilege = 1,
		BasicPrivilege = 2,
		SlavePrivilege = 4,
		SchemaPrivilege = 8,
		MasterPrivilege = 16,
		AgentPrivilege = 32,
		WorldPrivilege = 64,
		GodPrivilege = 128,
		SystemPrivilege = 256,
		FirstFreePrivilege = 512,
	} Privileges;

	inline Privileges operator|(const Privileges& p1, const Privileges& p2) { return (Privileges)((unsigned int)p1 | (unsigned int)p2); }
	inline Privileges operator&(const Privileges& p1, const Privileges& p2) { return (Privileges)((unsigned int)p1 & (unsigned int)p2); }
	inline Privileges operator~(const Privileges& p) { return (Privileges)~(unsigned int)p; }


}
#endif
