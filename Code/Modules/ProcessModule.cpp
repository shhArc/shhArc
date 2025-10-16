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



#include "../Arc/Api.h"
#include "../VM/VM.h"
#include "../Arc/Type.inl"
#include "../Schema/Agent.h"
#include "../Schema/Node.h"
#include "../File/IOVariant.h"
#include "ProcessModule.h"

//! /library shh
//! Messaging and process functions aswell as variable copying, testing, debug tracing. 

namespace shh {


	// --------------------------------------------------------------------------						
	// Function:	Register
	// Description:	registers functions and variables to be used by a process
	// Arguments:	alies of module and type, dictionary of configuration vars
	// Returns:		if sucessfull
	// --------------------------------------------------------------------------
	bool ProcessModule::Register(const std::string& alias, const StringKeyDictionary& sd, Privileges privileges)
	{

		GCPtr<Module> me(this);

		Api::OpenNamespace(alias);

		if (Api::GetImplementation() == Lua)
			LuaApiTemplate::RegisterVariable("__Lua", (lua_Integer)Lua);


		if (!(privileges & SoftProcess::ourExcludedExecuters))
		{
			if (Api::GetImplementation() == Lua)
			{
				Api::LuaRegisterFunction("ExecuteFile", ExecuteFile);
				Api::LuaRegisterFunction("ExecuteString", ExecuteString);
			}
		}
		if (!(privileges & SoftProcess::ourExcludedSendMessengers))
		{
			if (Api::GetImplementation() == Lua)
			{
				Api::LuaRegisterFunction("SendMsg", SendMsg);
				Api::LuaRegisterFunction("SetTimer", SetTimer);
			}
			Api::RegisterFunction("StopTimer", StopTimer, 2, me);
		}

		Api::RegisterFunction("YieldProcess", YieldProcess, 1, me);
		Api::RegisterFunction("GetMsgScheduledTime", GetMsgScheduledTime, 1, me);
		Api::RegisterFunction("GetMsgRecievedTime", GetMsgRecievedTime, 1, me);
		Api::RegisterFunction("GetMsgDelta", GetMsgDelta, 1, me);

		Api::CloseNamespace();
	
		
		return Module::Register(alias, sd, privileges);
	}



	//! /namespace shh
	//! /function ExecuteFile
	//! /privilege Agent
	//! /privilege Script
	//! /param string filename
	//! /param variable_args optional_variables
	//! Executes a script file of the name given in a new temporarily spawned new Process.
	//! The file must be in the scripts directory. Also optionally calls function Main() within 
	//! script which may accept and return args if it exists. Number of arguments accepted is arbitary.
	int ProcessModule::ExecuteFile(lua_State* L)
	{
	
		GCPtr<Process> cp = Scheduler::GetCurrentProcess();

		if (cp->IsFinalizing() && !SoftProcess::ourAllowMessegesWhenFinalising)
			Api::LuaThrowScriptError("Process can not call Execute when Finalizing.");

		Api::LuaCheckNumArgumentsGreaterOrEqual(L, 2);


		if (Scheduler::ourMaxMessagesPerUpdate != 0 && cp->GetNumMessagesSentThisUpdate() > Scheduler::ourMaxMessagesPerUpdate)
			Api::LuaThrowScriptError("Process has sent to many messages this update (limit &d)", Scheduler::ourMaxMessagesPerUpdate);

		if (cp->IsInitializing())
			return ExecutionError;

		lua_Integer i;
		Api::LuaGetArgument(L, 1, i);

		Implementation implementation;
		if (i == (int)Lua)
			implementation = Lua;
		else
			Api::LuaThrowScriptError("implementation %d does not exist", i);

		std::string file;
		Api::LuaGetArgument(L, 2, file);
		
		// must be in script dricetory or subdirectory
		bool notInPath = file.find("\\") == 0 || file.find("/") == 0 || file.find(":") != -1 || file.rfind("..") != -1;
		if (notInPath)
			Api::LuaThrowScriptError("Invalid script path %s: \nScripts must be within script path or subdirectory.\nDrive \\ and .. not permitted.", file.c_str());
		

		int posSlash = (int)file.rfind("\\");
		int posBackSlash = (int)file.rfind("/");
		int startPos = posSlash > posBackSlash ? posSlash : (posBackSlash != -1 ? posBackSlash : 0);
		std::string fileName = file.substr(startPos, file.length()-startPos);

		Execute(cp, implementation, fileName, true);
		return 0;

	}



	//! /namespace shh
	//! /function ExecuteString
	//! /privilege Agent
	//! /privilege Script
	//! /param string code
	//! /param variable_args optional_variables
	//! The file must be in the scripts directory. Also optionally calls function Main() within 
	//! script which may accept and returns args if it exists. Number of arguments accepted is arbitary.
	int ProcessModule::ExecuteString(lua_State* L)
	{
		GCPtr<Process> cp = Scheduler::GetCurrentProcess();

		if (cp->IsFinalizing() && !SoftProcess::ourAllowMessegesWhenFinalising)
			Api::LuaThrowScriptError("Process can not call Execute when Finalizing.");

		Api::LuaCheckNumArgumentsGreaterOrEqual(L, 2);
		
		lua_Integer i;
		Api::LuaGetArgument(L, 1, i);
		
		Implementation implementation;
		if (i == (int)Lua)
			implementation = Lua;
		else
			Api::LuaThrowScriptError("implementation %d does not exist", i);

		
		std::string s;
		Api::LuaGetArgument(L, 2, s);
	
	
		if (Scheduler::ourMaxMessagesPerUpdate != 0 && cp->GetNumMessagesSentThisUpdate() > Scheduler::ourMaxMessagesPerUpdate)
			Api::LuaThrowScriptError("Process has sent to many messages this update (limit %d)", Scheduler::ourMaxMessagesPerUpdate);

		if (cp->IsInitializing())
			return ExecutionError;

		if (Scheduler::ourMaxMessagesPerUpdate != 0 && cp->GetNumMessagesSentThisUpdate() > Scheduler::ourMaxMessagesPerUpdate)
			Api::LuaThrowScriptError("Process has sent to many messages this update (limit &d)", Scheduler::ourMaxMessagesPerUpdate);

		Execute(cp, implementation, s, false);
		return 0;
	}



	// --------------------------------------------------------------------------						
	// Function:	Execute
	// Description:	executes a file of string
	// Arguments:	calling process, language implementations, code string or 
	//				filename, whether the given string is a code string or 
	//				filename
	// Returns:		if sucessfull
	// --------------------------------------------------------------------------
	bool ProcessModule::Execute(const GCPtr<Process> &cp, Implementation implementation, std::string & s, bool isFile)
	{
		GCPtr<Scheduler> scheduler = cp->GetVM()->GetScheduler();
		Privileges privileges = cp->GetVM()->GetScheduler()->myExecutePrivileges;

		GCPtr<SoftProcess> process;
		if (privileges & MasterPrivilege || privileges & AgentPrivilege)
		{
			GCPtr<VM> vm(new VM());
			scheduler->AddVM(vm);
			process = vm->CreateMasterProcess(implementation, privileges);
		}
		else
		{
			process = cp->GetVM()->CreateSlaveProcess(implementation, privileges);
		}

		process->Initialize();
		if (process->Execute(s, isFile))
		{
			ExecutionState state = process->GetState();
			if (state == ExecutionYielded || state == ExecutionTimedOut || state == ExecutionAwaitingCallback)
				process->CompleteInitialization();

			Message* msg = new Message;
			msg->myFunctionName = process->IsInitializing() ? SoftProcess::ourBootMessage : SoftProcess::ourMainMessage;

			msg->myTo = process;
			msg->myFrom =LuaProcess::GetCurrentLuaProcess();
			msg->SetCallType(Message::Synchronous);
			msg->myDestroyOnCompletion = true;
			msg->myPriority = Priority::GetExecute();
			if (!process->IsInitializing())
				process->SetReady();

			if (!msg->SendMsg(0.0, 2))
			{
				process.Destroy();
				delete msg;
			}
			else
			{
				cp->YieldProcess(ExecutionAwaitingCallback, -1);
			}
			return true;	
		}

		return false;
	}


	//! /namespace shh
	//! /function SendMsg
	//! /privilege Agent
	//! /privilege Script
	//! /param Agent_Schema receiver
	//! /param double delay
	//! /param string callback_function_name
	//! /param string function_name_to_call
	//! /returns boolean if_successfull
	//! /returns optional_returned_arguments
	//! Sends a message to an Agent/Schema. Optional additonal arguments may be sent to the agent.
	int ProcessModule::SendMsg(lua_State* L)
	{
		GCPtr<Process> cp = Scheduler::GetCurrentProcess();

		if (cp->IsFinalizing() && !SoftProcess::ourAllowMessegesWhenFinalising)
			Api::LuaThrowScriptError("Process can not send message when Finalizing.");
	
	
		if (Scheduler::ourMaxMessagesPerUpdate != 0 && cp->GetNumMessagesSentThisUpdate() > Scheduler::ourMaxMessagesPerUpdate)
			Api::LuaThrowScriptError("Process has sent to many messages this update (limit &d)", Scheduler::ourMaxMessagesPerUpdate);

		if (cp->IsInitializing())
		{
			LuaApiTemplate::Return(L, false);
			return 1;
		}

		GCPtr<LuaProcess> from = cp;
		GCPtr<Process> to;
		GCPtr<Scheduler> scheduler = Scheduler::GetCurrentProcess()->GetCurrentEnvironment()->GetScheduler();
	
		if (Api::LuaGetArgumentType(L, 1) == LuaType<shhId>::GetLuaId())
		{
			shhId id;
			Api::LuaGetArgument(L, 1, id);
			GCPtr<VM> vm = scheduler->GetVM(id);
			if (!vm.IsValid())
				Api::LuaThrowScriptError("SendMsg reciever VM %d does not exist", id);

			to = vm->GetMasterProcess();
			if (!to.IsValid())
				Api::LuaThrowScriptError("SendMsg reciever VM %d does not have a master process", id);
		}
		else
		{
			GCPtr<Object> o;
			if (Api::LuaGetArgumentType(L, 1) == LuaType<GCPtr<Agent>>::GetLuaId())
			{
				GCPtr<Agent> a;
				LuaApiTemplate::GetArgument(L, 1, a);
				if (a.IsValid())
					o.StaticCast(a);
			}
			else if (Api::LuaGetArgumentType(L, 1) == LuaType<GCPtr<Node>>::GetLuaId())
			{
				GCPtr<Node> n;
				LuaApiTemplate::GetArgument(L, 1, n);
				if (n.IsValid())
					o.StaticCast(n);
			}
				
			if (!o.IsValid())
				Api::LuaThrowScriptError("Not a valid Object for SendMsg");

			to = o->GetProcess();
			
		}

		if (to->IsInitializing())
		{
			LuaApiTemplate::Return(L, false);
			return 1;
		}

		double delay;
		Api::LuaGetArgument(L, 2, delay);

		std::string callbackFunction;
		Api::LuaGetArgument(L, 3, callbackFunction);

		std::string name;
		Api::LuaGetArgument(L, 4, name);

		Message* msg = new Message;
		msg->myFunctionName = SoftProcess::ourMessagePrefix+name;
		msg->myTo = to;
		msg->myFrom = from;
		msg->SetCallType(callbackFunction.empty() ? Message::Decoupled : Message::Asynchronous);
		msg->myCallbackFunction = SoftProcess::ourMessagePrefix + callbackFunction;
		msg->myDestroyOnCompletion = false;

		Privileges privileges = to->GetPrivileges();
		if(privileges & Privileges::AgentPrivilege )
			msg->myPriority = Priority::GetAgent();
		else if (privileges & Privileges::MasterPrivilege)
			msg->myPriority = Priority::GetMaster();
		else if (privileges & Privileges::SchemaPrivilege)
			msg->myPriority = Priority::GetSchema();
		else
			msg->myPriority = Priority::GetSlave();

		if (!msg->SendMsg(delay, 4))
		{
			delete msg;
			LuaApiTemplate::Return(L, false);
		}
		else
		{
			LuaApiTemplate::Return(L, true);
		}
		return 1;
			
	}


	//! /namespace shh
	//! /function SetTimer
	//! /privilege Agent
	//! /privilege Script
	//! /param Agent_Schema receiver
	//! /param double message_delay_intervals
	//! /param string message_name 
	//! /returns boolean 
	//! /returns integer
	//! Sends a timer message to a process. Returns if successful and the timer id.
	int ProcessModule::SetTimer(lua_State* L)
	{
		GCPtr<Process> cp = Scheduler::GetCurrentProcess();

		if (cp->IsFinalizing() && !SoftProcess::ourAllowMessegesWhenFinalising)
			Api::LuaThrowScriptError("Process can not set timer when Finalizing.");

		Api::LuaCheckNumArguments(L, 3);

		

		if (Scheduler::ourMaxMessagesPerUpdate != 0 && cp->GetNumMessagesSentThisUpdate() > Scheduler::ourMaxMessagesPerUpdate)
			Api::LuaThrowScriptError("Process has sent to many messages this update (limit &d)", Scheduler::ourMaxMessagesPerUpdate);

		if (cp->IsInitializing())
			return ExecutionError;

		GCPtr<LuaProcess> from = cp;
		GCPtr<Process> to;

		GCPtr<Scheduler> scheduler = cp->GetVM()->GetScheduler();
		if (Api::LuaGetArgumentType(L, 1) == LuaType<shhId>::GetLuaId())
		{
			shhId id;
			Api::LuaGetArgument(L, 1, id);
			GCPtr<VM> vm = scheduler->GetVM(id);
			if (!vm.IsValid())
				Api::LuaThrowScriptError("SendMsg reciever VM %d does not exist", id);

			to = vm->GetMasterProcess();
			if (!to.IsValid())
				Api::LuaThrowScriptError("SendMsg reciever VM %d does not have a master process", id);
		}
		else
		{
			GCPtr<Messenger> m;
			LuaApiTemplate::GetArgument(L, 1, m);
			if (!m.IsValid())
				Api::LuaThrowScriptError("Not a valid Messenger");

			GCPtr<VM> vm;
			vm.DynamicCast(m);
			if (vm.IsValid())
				to = vm->GetMasterProcess();
			else
				to.StaticCast(m);
		}


		if (to->IsInitializing())
		{
			LuaApiTemplate::Return(L, false);
			return 1;
		}

		double delay;
		Api::LuaGetArgument(L, 2, delay);

		std::string name;
		Api::LuaGetArgument(L, 3, name);

		Message* msg = new Message;
		msg->myFunctionName = SoftProcess::ourTimerPrefix+name;
		msg->myTo = to;
		msg->myFrom = from;
		msg->SetCallType(Message::TimerMsg);
		msg->myRepeatTimer = delay;
		msg->myDestroyOnCompletion = false;

		Privileges privileges = to->GetPrivileges();
		if (privileges & Privileges::AgentPrivilege)
			msg->myPriority = Priority::GetAgent();
		else if (privileges & Privileges::MasterPrivilege)
			msg->myPriority = Priority::GetMaster();
		else if (privileges & Privileges::SchemaPrivilege)
			msg->myPriority = Priority::GetSchema();
		else
			msg->myPriority = Priority::GetSlave();

		if (msg->Build(3, 0) != Message::BuildOk)
		{
			delete msg;
			LuaApiTemplate::Return(L, false);
			return 1;
		}
		else
		{
			bool ok = scheduler->SetTimer(msg);
			LuaApiTemplate::Return(L, ok);
			LuaApiTemplate::Return(L,(long)msg->GetId());
			return 2;
		}
		

	}
	


	//! /namespace shh
	//! /function StopTimer
	//! /privilege Agent
	//! /privilege Script
	//! /param integer timer_id 
	//! /returns boolean
	//! Stops a timer message running on an Agent/Schema.
	ExecutionState ProcessModule::StopTimer(shhId& id, bool &result)
	{
		GCPtr<Process> cp = Scheduler::GetCurrentProcess();
		GCPtr<Scheduler> scheduler = cp->GetVM()->GetScheduler();
		result = scheduler->StopTimer(id, cp);
		return ExecutionOk;
	}

	
	//! /namespace shh
	//! /function YieldProcess
	//! Yields the currently active Process.
	ExecutionState ProcessModule::YieldProcess()
	{
		GCPtr<Process> cp = Scheduler::GetCurrentProcess();
		cp->YieldProcess(ExecutionYielded, -1);
		return ExecutionOk;
	}


	//! /namespace shh
	//! /function GetMsgScheduledTime
	//! /returns double
	//! Returns the time the current message was scheduled for.
	ExecutionState ProcessModule::GetMsgScheduledTime(double &time)
	{
		GCPtr<Process> cp = Scheduler::GetCurrentProcess();
		Message *msg = cp->GetCurrentMessage();
		time = 0.0;
		if (msg)
			time = msg->GetScheduledTime();

		return ExecutionOk;
	}


	//! /namespace shh
	//! /function GetMsgRecievedTime
	//! /returns double
	//! Returns the time the current message was received.
	ExecutionState ProcessModule::GetMsgRecievedTime(double &time)
	{
		GCPtr<Process> cp = Scheduler::GetCurrentProcess();
		Message* msg = cp->GetCurrentMessage();
		time = 0.0;
		if (msg)
			time = msg->GetReceivedTime();

		return ExecutionOk;
	}


	//! /namespace shh
	//! /function GetMsgDelta
	//! /returns double
	//! Returns the time delta of current message (used for timers and update messages).
	ExecutionState ProcessModule::GetMsgDelta(double &time)
	{
		GCPtr<Process> cp = Scheduler::GetCurrentProcess();
		Message* msg = cp->GetCurrentMessage();
		time = 0.0;
		if (msg)
		{
			if (msg->GetCallType() != Message::TimerMsg || msg->GetCompletedTime() == 0.0)
				time = 0.0;
			else
				time = msg->GetReceivedTime() - msg->GetCompletedTime();
		}
		return ExecutionOk;
	}

} // namespace shh