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


#include "../VM/VM.h"
#include "../Arc/Api.h"
#include "LuaModule.h"

//! /library shh
//! Messaging and process functions aswell as variable copying, testing, debug tracing. 

namespace shh {

	Classifier LuaModule::ourTraceFilter;


	// **** Registration *******************************************************************************


	// --------------------------------------------------------------------------						
	// Function:	Register
	// Description:	registers functions and variables to be used by a process
	// Arguments:	alies of module and type, dictionary of configuration vars
	// Returns:		if sucessfull
	// --------------------------------------------------------------------------
	bool LuaModule::Register(const std::string& alias, const StringKeyDictionary& sd, Privileges privileges)
	{;
		GCPtr<Module> me(this);

		Api::OpenNamespace(alias);

		if (Api::GetImplementation() == Lua)
		{
			Api::LuaRegisterFunction("DeepCopy", DeepCopy);
			Api::LuaRegisterFunction("DeepCompare", DeepCompare);
			Api::LuaRegisterFunction("ErrorBox", ErrorBox);
			Api::LuaRegisterFunction("Trace", Trace);
		}
		Api::RegisterFunction("LogError", LogError, 2, me);
		Api::RegisterFunction("FilterTrace", FilterTrace, 2, me);

		Api::CloseNamespace();


		return Module::Register(alias, sd, privileges);
	}



	//! /namespace shh
	//! /function DeepCopy
	//! /param any_type data_to_clone
	//! /return cloned_type
	//! Used to make a clone copy my value of data_to_clone argument and returns it.
	int LuaModule::DeepCopy(lua_State* L)
	{

		Api::LuaCheckNumArguments(L, 1);
		const TValue* o = Api::LuaGetArg(L, 1);
		LuaHelperFunctions::DeepCopy(L, L, o);
		LuaSetStackValue(L, 1, LuaGetStackValue(L, 2));
		LuaDecStack(L);
		return 1;
	
	}



	//! /namespace shh
	//! /function DeepCompare
	//! /param any_type value1
	//! /param any_type value2
	//! /returns boolean
	//! Used to compare by value two paramaters. Returns true if equal false if not.
	int LuaModule::DeepCompare(lua_State* L)
	{
		Api::LuaCheckNumArguments(L, 2);

		const TValue* o1 = Api::LuaGetArg(L, 1);
		const TValue* o2 = Api::LuaGetArg(L, 2);

		if (LuaHelperFunctions::DeepCompare(L, o1, o2))
			LuaApiTemplate::Return(L, true);
		else
			LuaApiTemplate::Return(L, false);

		return 1;
	}



	//! /namespace shh
	//! /function LogError
	//! /param string message
	//! Function that tells Process when an error occurs.
	ExecutionState LuaModule::LogError(std::string& s)
	{
		if (s.empty())
			s = "(no message)";
		else if (s == "BAIL")
			return ExecutionOk;

		GCPtr<LuaProcess> process = Api::LuaGetCurrentProcess();
		if (process.IsValid())
			process->LogError(s);
		else
			LuaProcess::LogError(s);

		return ExecutionOk;
	}

	//! /namespace shh
	//! /function ErrorBox
	//! /param string format
	//! /param variable_arg optional_variable
	//! Used to print data to an error box. Use a % sign in the format string to designate where an argument is to be printed. 
	//! You may have any number of arguments following the format string. Each argument may be a number, string or system code.
	//! An standard engine error box is displayed.
	int LuaModule::ErrorBox(lua_State* L)
	{
		Api::LuaCheckNumArgumentsGreaterOrEqual(L, 1);

		std::string msg = Print(1);
		std::string t("Error");
		std::wstring title(t.begin(), t.end());
		std::wstring what(msg.begin(), msg.end());
		MessageBoxW(0, (LPCWSTR)what.c_str(), (LPCWSTR)title.c_str(), (MB_OK | MB_ICONERROR));

		return 0;

	}


	//! /namespace shh
	//! /function Trace
	//! /param string format
	//! /param variable_arg optional_variable
	//! Used to print data to the output stream. Use a % sign in the format string to designate where an argument is to be printed. 
	//! You may have any number of arguments following the format string. Each argument may be a number, string or system code.
	int LuaModule::Trace(lua_State* L)
	{
		Api::LuaCheckNumArgumentsGreaterOrEqual(L, 1);

		int arg = 1;
		shh::Classifier* classifiers = NULL;
		if (Api::LuaGetArgumentType(L, 1) != LUA_TSTRING)
		{
			LuaApiTemplate::GetArgument(L, 1, classifiers);
			arg = 2;
		}

		static const shh::Classifier empty;
		Trace(arg, classifiers == NULL ? empty : *classifiers);

		return 0;
	}


	//! /namespace shh
	//! /function FilterTrace
	//! /param classifier filter_attribute_lables
	//! Allows only traces with any of the given classifier labels to be output.
	ExecutionState LuaModule::FilterTrace(Classifier& classifiers)
	{
		ourTraceFilter = classifiers;
		return ExecutionOk;
	}



	// --------------------------------------------------------------------------						
	// Function:	Trace
	// Description:	trace worker function
	// Arguments:	argument to trace from, classifiers to filter by
	// Returns:		none
	// --------------------------------------------------------------------------
	void LuaModule::Trace(int arg, const Classifier& classifiers)
	{
		if (!ourTraceFilter.Empty())
		{
			if (classifiers.Empty())
				return;

			if (!ourTraceFilter.Match(classifiers, shh::Classifier::MATCH_ANY))
				return;

		}
		static const shh::Classifier basicTraceClassifiers = shh::Classifier() << "shhArc";

		std::string result;
		shh::Classifier noprintf;
		noprintf.Add("NO_PRINTF");
		if (classifiers.Match(noprintf, shh::Classifier::MATCH_ANY))
		{
			lua_State * L = Api::LuaGetCurrentLuaState();
			Api::LuaGetArgument(L, arg, result);
			if (classifiers.Empty())
			{
				OurTrace(basicTraceClassifiers, "%s", result.c_str());
			}
			else
			{
				shh::Classifier processTraceClassifiers = basicTraceClassifiers;
				processTraceClassifiers.Add(classifiers);
				OurTrace(processTraceClassifiers, "%s", result.c_str());
			}
			return;
		}
		else
		{
			result = Print(arg);
		}
		result += "\n";


		if (classifiers.Empty())
		{
			OurTrace(basicTraceClassifiers, result.c_str());
		}
		else
		{
			shh::Classifier processTraceClassifiers = basicTraceClassifiers;
			processTraceClassifiers.Add(classifiers);
			OurTrace(processTraceClassifiers, result.c_str());
		}

	}


	// --------------------------------------------------------------------------						
	// Function:	Print
	// Description:	prints an argument on the stack to a string
	// Arguments:	argument number
	// Returns:		string
	// --------------------------------------------------------------------------
	std::string LuaModule::Print(unsigned int arg)
	{

		lua_State* L = Api::LuaGetCurrentLuaState();

		std::string spec;
		Api::LuaGetArgument(L, arg, spec);

		std::string result;
		int pos = 0;
		arg++;
		while (!spec.empty())
		{
			pos = (int)spec.find("%");
			if (pos != -1)
			{
				//result += spec.substr(0, pos);
				std::string type = spec.substr(pos, 1);
				spec = spec.substr(pos + 1, spec.size());

				
				if (arg > Api::LuaGetNumArgs(L))
				{
					result += "**TOO FEW ARGS**";
					break;
				}
				else
				{
					TValue* value = LuaGetStackValue(L, arg);
					LuaHelperFunctions::Print(L, value, result);
					arg++;
				}
			}
			else
			{
				break;
			}
		}
		result += spec;
		return result;
	}
} // namespace shh