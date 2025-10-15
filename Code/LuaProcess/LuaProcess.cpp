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


#include "../Common/Debug.h"
#include "../Arc/Type.inl"
#include "../File/FileSystem.h"
#include "../File/FileSystem.h"
#include "../File/IOVariant.h"
#include "../VM/VM.h"
#include "LuaProcess.h"
#include "LuaWrapper.h"
#include "LuaHelperFunctions.h"
#include "LuaLibrary.h"
#include "LuaApi.h"
#include "LuaApiTemplate.h"
#include "LuaType.h"



#include <stdarg.h>
#include <fstream>

namespace shh {


	// LuaProcess /////////////////////////////////////////////////////////////////////

	IMPLEMENT_MEMORY_MANAGED(LuaProcess);
	
	// LuaProcess /////////////////////////////////////////////////////////////////////


	const std::string LuaProcess::ourCommentToken = "--";
	const std::string LuaProcess::ourOverideSeperator = "___";
	const std::string LuaProcess::ourScriptExtension("lua");
	const std::string LuaProcess::ourEncyptedScriptExtension("sls");

	unsigned int LuaProcess::ourTimeOut = 1000;
	lua_State *LuaProcess::ourMasterLuaState = NULL;
	unsigned int LuaProcess::ourNumLuaProcesses = 0;

	void ::luaS_resize(lua_State* L, int nsize);



	// --------------------------------------------------------------------------						
	// Function:	ClassesSpecifier
	// Description:	recusrses though folder loading class files reading the slass 
	//				specifiers
	// Arguments:	lua state, typename (Agent/Node), path, class specs to return
	//				whether to recurse, class name as appears in script
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool LuaProcess::ClassesSpecifier(const std::string& typeName, const std::string& path, Registry::ClassSpecs& unorderedSpecs, bool recurse, bool report, const std::string& classType)
	{
		return SoftProcess::ClassesSpecifier(Lua, typeName, ourCommentToken, ourOverideSeperator, ourScriptExtension, ourEncyptedScriptExtension, path, unorderedSpecs, recurse, report, classType);
	}


	// --------------------------------------------------------------------------						
	// Function:	Create
	// Description:	creates a LuaProcess
	// Arguments:	privileges of process, LuaProcess to derive from
	// Returns:		created process
	// --------------------------------------------------------------------------
	GCPtr<Process> LuaProcess::Create(Privileges privileges, const GCPtr<Process> spawnFrom)
	{ 
		GCPtr<LuaProcess> spawnFromLua;
		spawnFromLua.DynamicCast(spawnFrom);
		GCPtr<Process> oldProcess = Scheduler::GetCurrentProcess();
		GCPtr<LuaProcess> newProcess(new LuaProcess(privileges, spawnFromLua));
		Scheduler::SetCurrentProcess(oldProcess);
		return newProcess;
	}


	// --------------------------------------------------------------------------						
	// Function:	Clone
	// Description:	clones process
	// Arguments:	
	// Returns:		cloned process
	// --------------------------------------------------------------------------
	 GCPtr<Process> LuaProcess::Clone()
	{
		GCPtr<Process> oldProcess = Scheduler::GetCurrentProcess();
		GCPtr<Process> p;
		p.StaticCast(LuaProcess::Create(this->GetPrivileges(), GCPtr<LuaProcess>(this)));
		Scheduler::SetCurrentProcess(oldProcess);
		return p;
	}


	// --------------------------------------------------------------------------						
	// Function:	LuaProcess
	// Description:	constructor
	// Arguments:	privileges of process, process to clone
	// Returns:		none
	// --------------------------------------------------------------------------
	LuaProcess::LuaProcess(Privileges privileges, const GCPtr<LuaProcess> &spawnFrom) :
		SoftProcess(privileges)
	{
		myScriptError = false;
		Scheduler::SetCurrentProcess(GCPtr<Process>(this));

		myImplementation = Lua;

		lua_State* toClone = NULL;
		if (!spawnFrom.IsValid())
		{
			if (ourMasterLuaState == NULL)
			{
				
				myLuaState = luaL_newstate();
			
				// Need to initialize the global table with a value
				// else lua tries to reallocate a hash of size 0 
				// when it tries to resize for the first time
				LuaGetGlobalsStack(myLuaState);
				LuaInitTable(myLuaState, LuaGetStackValue(myLuaState, -1), 0, 1);
				LuaDecStack(myLuaState);
				LuaApiTemplate::RegisterVariable("shhVerson", 0.0);
			

				double *d = NULL;
				LuaApiTemplate::RegisterType(d, "double", NULL, NULL);
				float *f = NULL;
				LuaApiTemplate::RegisterType(f, "float", NULL, NULL);
				int *i = NULL;
				LuaApiTemplate::RegisterType(i, "int", NULL, NULL);
				unsigned int *u = NULL;
				LuaApiTemplate::RegisterType(u, "unsigned int", NULL, NULL);
				long* l = NULL;
				LuaApiTemplate::RegisterType(l, "long", NULL, NULL);
				unsigned long* ul = NULL;
				LuaApiTemplate::RegisterType(ul, "unsigned long", NULL, NULL);
				long long* ll = NULL;
				LuaApiTemplate::RegisterType(ll, "long long", NULL, NULL);
				unsigned long long* ull = NULL;
				LuaApiTemplate::RegisterType(ull, "unsigned long long", NULL, NULL);
				bool *b = NULL;
				LuaApiTemplate::RegisterType(b, "bool", NULL, NULL);
				std::string* s = NULL;
				LuaApiTemplate::RegisterType(s, "std::string", NULL, NULL);
		
				GCPtr<Registry::OverloadTable> *ot = NULL;
				LuaApiTemplate::RegisterType(ot, "shh::OverloadTable", NULL, NULL, false);
				
				VariantKeyDictionary* dict = NULL;
				LuaApiTemplate::RegisterType(dict, "shh::Dictionary", NULL, NULL);

				RegisterLuaLibs(myLuaState);
				
				ourMasterLuaState = myLuaState;
				myLuaState = NULL;

				
			}
			toClone = ourMasterLuaState;
		}
		else
		{
			toClone = spawnFrom->myLuaState;
			myRegisteredModules = spawnFrom->myRegisteredModules;
			myPaths = spawnFrom->myPaths;
		}

			
		myLuaState = luaL_newstate();

	

		//OverloadTable may not be cloned before cloning other types
		//in the registry that need it first member functions thier
		//so put it in 
		GCPtr<Registry::OverloadTable> *ot = NULL;
		LuaApiTemplate::RegisterType(ot, "shh::OverloadTable", NULL, NULL, false);

		// Need to reference metatable of inherited class
		TValue* registryToClone = LuaGetRegistry(toClone);
		LuaHelperFunctions::DeepCopy(toClone, myLuaState, registryToClone, false, true);
		LuaSetRegistry(myLuaState, LuaGetStackValue(myLuaState,-1));
		LuaDecStack(myLuaState);
		
		// need to deep copy global tables 
		LuaHelperFunctions::DeepCopy(toClone, myLuaState, LuaGetGlobalsValue(toClone), true, false);
		LuaSetGlobals(myLuaState);

		myInheritedFixedGCs = NULL;
		myInheritedAllGCs = NULL;
		
		ourNumLuaProcesses++;

		myDebugStackSize = LuaGetStackSize(myLuaState);
		myDebugCI = myLuaState->ci;
		SetReady();
		
	}


	// --------------------------------------------------------------------------						
	// Function:	~LuaProcess
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	LuaProcess::~LuaProcess()
	{
		lua_close(myLuaState);
		ourNumLuaProcesses--;
		ourMasterLuaState = NULL;
		if (ourNumLuaProcesses == 0 && ourMasterLuaState)
		{
			lua_close(ourMasterLuaState);
			ourMasterLuaState = NULL;
		}
	}
	

	// --------------------------------------------------------------------------						
	// Function:	SetGarbageCollectionIncremental
	// Description:	sets up lua garbage collection parameters 
	// Arguments:	memory in use pause time, stepmultiplier is how many elements 
	//				it marks or sweeps for each kilobyte of memory allocated, 
	//				stepsize of each incremental step
	// Returns:		none
	// --------------------------------------------------------------------------						
	void LuaProcess::SetGarbageCollectionGeneratiobnal(int pause, int stepmul, int stepsize)
	{
		lua_gc(myLuaState, LUA_GCINC, pause, stepmul, stepsize);
	}


	// --------------------------------------------------------------------------						
	// Function:	SetGarbageCollectionGenerational
	// Description:	sets up garbage collection parameters 
	// Arguments:	minormul controls the frequency of minor collections,
	//				majormul controls the frequency of major collections.
	// Returns:		none
	// --------------------------------------------------------------------------						
	void LuaProcess::SetGarbageCollectionGeneratiobnal(int minormul, int majormul)
	{
		lua_gc(myLuaState, LUA_GCGEN, minormul, majormul);

	}


	// --------------------------------------------------------------------------						
	// Function:	GetCurrentLuaProcess
	// Description:	returns the current process 
	// Arguments:	none
	// Returns:		lua process
	// --------------------------------------------------------------------------						
	GCPtr<LuaProcess>LuaProcess::GetCurrentLuaProcess()
	{
		GCPtr<LuaProcess> toProcess;
		toProcess.StaticCast(Scheduler::GetCurrentProcess());
		return toProcess;
	};


	// --------------------------------------------------------------------------						
	// Function:	GetCurrentLuaState
	// Description:	returns the lua state of the current process 
	// Arguments:	none
	// Returns:		lua state 
	// --------------------------------------------------------------------------							
	lua_State*LuaProcess::GetCurrentLuaState(bool throwError)	
	{ 
		GCPtr<LuaProcess> toProcess;
		toProcess.DynamicCast(Scheduler::GetCurrentProcess());
		if (toProcess.IsValid())
		{
			return toProcess->GetLuaState();
		}
		else if (throwError)
		{
			THROW("No active LuaProcess");
			return NULL;
		}
		else
		{
			return ourMasterLuaState;
		}
	};


	// --------------------------------------------------------------------------						
	// Function:	CollectAllGarbage
	// Description:	perform full garbage collection on this process
	// Arguments:	none
	// Returns:		lua process
	// --------------------------------------------------------------------------						
	void LuaProcess::CollectAllGarbage()
	{
		lua_gc(myLuaState, LUA_GCGEN);
		lua_gc(myLuaState, LUA_GCCOLLECT);
		lua_gc(myLuaState, LUA_GCINC);
	}


	// --------------------------------------------------------------------------						
	// Function:	ValidateFunctionNames
	// Description:	validates whether function names in process are allowed
	// Arguments:	allow class function prefixes
	// Returns:		if ok
	// --------------------------------------------------------------------------
	bool LuaProcess::ValidateFunctionNames(bool allowClassOnlyFunctions)
	{
		std::string errorMessage;
		if (!LuaHelperFunctions::ValidateFunctionNames(myLuaState, allowClassOnlyFunctions,
			LuaProcess::ourOverideSeperator,
			SoftProcess::ourMessagePrefix,
			SoftProcess::ourTimerPrefix,
			SoftProcess::ourSystemPrefix,
			SoftProcess::ourStaticPrefix,
			SoftProcess::ourInitializeMessage,
			SoftProcess::ourFinalizeMessage,
			SoftProcess::ourUpdateMessage,
			errorMessage))
		{
			errorMessage += ".\n";
			ERROR_TRACE(errorMessage.c_str());
			return false;
		}
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	Overide
	// Description:	overides virtual function in parent class
	// Arguments:	derived file to run, overide prefix 
	// Returns:		if sucessful
	// --------------------------------------------------------------------------
	bool LuaProcess::Overide(const std::string& derivedCode, const std::string& overidePrefix) 
	{
		GCPtr<Process> oldProcess = Scheduler::GetCurrentProcess();
		Scheduler::SetCurrentProcess(GCPtr<Process>(this));
	
		lua_State *temp = luaL_newstate();
		if (!luaL_loadstring(temp, derivedCode.c_str()))
		{
			int numResults = 0;
			int status = lua_resume(temp, NULL, 0, &numResults);
			if (status == LUA_ERRRUN)
			{
				lua_close(temp);
				RELEASE_ASSERT(false);
				return false;
			}
		}
		else
		{
			lua_close(temp);
			RELEASE_ASSERT(false);
			return false;
		}

		// rename dupicate functions in base file
		std::vector<std::string> newFunctionNames;
		LuaHelperFunctions::GetFunctionNames(temp, LuaGetGlobalsValue(temp), newFunctionNames);
		lua_close(temp);
		LuaHelperFunctions::OverideFunctions(myLuaState, LuaGetGlobalsValue(myLuaState), newFunctionNames, overidePrefix);

		Scheduler::SetCurrentProcess(oldProcess);

		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	Execute
	// Description:	executes a file or string of lua script code in this process
	// Arguments:	filename or string to run, whether string is a file, 
	//				whether to allow yielding and timeout, paths to search for 
	//				file (leave blank to use default paths of process)
	// Returns:		if was run without error
	// --------------------------------------------------------------------------
	bool LuaProcess::Execute(const std::string& s, bool isFile, bool isYieldable, Paths paths)
	{
		Lock();
		bool retVal = false;
		int stackSize = LuaGetStackSize(myLuaState);

		bool codeOk = true;
		std::string fullFileName;
		if (isFile)
		{
			if (paths.empty())
				paths = myPaths;

			// search for the file in all valid file paths for this process
			codeOk = false;
			for (Paths::iterator p = paths.begin(); p != paths.end(); p++)
			{
				if (isFile)
				{
					fullFileName = *p + "/" + s;
					int pos = (int)fullFileName.find(ourScriptExtension);
					if (pos == std::string::npos || pos != fullFileName.size() - ourScriptExtension.size())
						fullFileName += "."+ourScriptExtension;
				}

				if (FileSystem::IsValidFile(fullFileName))
				{
					codeOk = true;
					break;
				}

			}
		}

		if (codeOk)
		{
			GetVM()->SwapProcessIn(GCPtr<LuaProcess>(this));

			try
			{
				myDebugStackSize = LuaGetStackSize(myLuaState);
				myDebugCI = myLuaState->ci;

				bool yieldable = myYieldable;
				int timeOut = myTimeOut;
				myTimeOut = 0;

				// parse file
				int parsed;
				if (isFile)
					parsed = luaL_loadfile(myLuaState, fullFileName.c_str());
				else
					parsed = luaL_loadstring(myLuaState, s.c_str());


				if (isYieldable)
				{
					myYieldable = true;
					myTimeOut = timeOut;
				}
				else
				{
					myYieldable = false;
					myTimeOut = 0;
				}

				if (parsed == 0)
				{
					// execute code
					int numResults = 0;
					int status = lua_resume(myLuaState, NULL, 0, &numResults);
					if (status == LUA_ERRRUN)
						myScriptError = true;
				}

				myTimeOut = timeOut;
				myYieldable = yieldable;

				if (myScriptError)
				{
					UnwindCallStack();
					myState = ExecutionFailed;
					retVal = false;
				}
				else if (myState == ExecutionYielded || myState == ExecutionTimedOut || myState == ExecutionAwaitingCallback)
				{
					retVal = true;
				}
				else
				{
					DEBUG_ASSERT(LuaGetStackSize(myLuaState) == stackSize);
					retVal = true;
				}
			}
			catch (shh::Exception& e)
			{
				UnwindCallStack();
				myState = ExecutionError;
				ERROR_TRACE("Process: %x. Error executing script: %s.\n", this, e.what());
				retVal = false;
			}
			catch (...)
			{
				UnwindCallStack();
				myState = ExecutionError;
				if (isFile)
					ERROR_TRACE("LuaProcess::Unknown error from Process in file %s. Bailing message handler.\n", fullFileName.c_str());
				else
					ERROR_TRACE("LuaProcess::Unknown error from Process. Bailing message handler.\n");

				retVal = false;
			}

			GetVM()->SwapProcessOut();
		}
		else
		{
			return false;
		}

		if (myState == ExecutionCompleted)
			SetReady();
		
		Unlock();
		return retVal;
	}


	// --------------------------------------------------------------------------						
	// Function:	UnwindCallStack
	// Description:	resets call stack to how it was pre call
	// Arguments:	string of code to run, whether to timeoutnone
	// Returns:		none
	// --------------------------------------------------------------------------
	void LuaProcess::UnwindCallStack()
	{

		while (myLuaState->ci != myDebugCI)
			myLuaState->ci = myLuaState->ci->previous;

		while ((unsigned int)LuaGetStackSize(myLuaState) > myDebugStackSize)
			LuaDecStack(myLuaState);
		myScriptError = false;
	}


	// --------------------------------------------------------------------------						
	// Function:	Clone
	// Description:	Clones a data type into this process
	// Arguments:	value to clone, clone returned version
	// Returns:		none
	// --------------------------------------------------------------------------
	void LuaProcess::Clone(TValue *valueToClone, TValue* valueCloned)
	{
		LuaTypeId type = LuaGetTypeId(valueToClone);
		if (type == LUA_TTABLE || type < 0)
		{
			// copy table objects here cos we dont want references in messages
			LuaSetStackValue(myLuaState, 0, valueToClone);
			LuaIncStack(myLuaState);
			if (CallFunction("DeepCopy", 1, 1))
			{
				// get value returned
				*valueCloned = *LuaGetStackValue(myLuaState, -1);
				lua_pop(myLuaState, 1);
			}
		}
		else
		{
			// non reference value no or referencing cloning required
			*valueCloned = *valueToClone;
		}

	}


	// --------------------------------------------------------------------------						
	// Function:	GetTable
	// Description:	get table of name from global table
	// Arguments:	name ("" = global table), table got
	// Returns:		true if sucseeds
	// --------------------------------------------------------------------------						
	bool LuaProcess::GetTable(const std::string& name, TValue& table)
	{
		TValue* thisTable = LuaGetGlobalsValue(myLuaState);
		// if no name use global
		if (name.empty())
		{
			table = *thisTable;
			return true;
		}
		// see if table exists
		const TValue* value = LuaGetTableValue(LuaGetTable(thisTable), LuaNewString(myLuaState, name.c_str()));

		if(LuaGetTypeId(value) == LUA_TTABLE)
		{
			table = *value;
			return true;
		}

		return false;

	}


	// --------------------------------------------------------------------------						
	// Function:	LogError
	// Description:	function used to diplay script error messages
	// Arguments:	errormessage
	// Returns:		none
	// --------------------------------------------------------------------------
	void LuaProcess::LogError(const std::string& error)
	{
		std::string errorMessage = "shhArc Lua script error\n";
		errorMessage += " " + error;

		std::string errorExtra;
		LuaApi::GetErrorMessageDetails(errorExtra);
		errorMessage += errorExtra + ".\n";

		ERROR_TRACE(errorMessage.c_str());

		LuaApi::GetCurrentLuaProcess()->myScriptError = true;
	}


	// --------------------------------------------------------------------------						
	// Function:	YieldProcess
	// Description:	yields this process
	// Arguments:	state to set to, number of return values in function call 
	//				when yield
	// Returns:		if successfull
	// --------------------------------------------------------------------------						
	bool LuaProcess::YieldProcess(ExecutionState state, int results)
	{
		if (!myYieldable)
			return false;
		if (results >= 0)
			myResumeStates.push_back(results);
		
		if(myState != ExecutionYielded && myState != ExecutionTimedOut && myState != ExecutionAwaitingCallback)
			myState = state;

		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	ContinueProcess
	// Description:	continue process after yield
	// Arguments:	none
	// Returns:		number of return values needed on stack for resumed function
	// --------------------------------------------------------------------------						
	int LuaProcess::ContinueProcess() 
	{ 
		int rv = myResumeStates[myResumeStates.size()-1];
		myResumeStates.pop_back();
		if(myResumeStates.empty())
			myState = ExecutionOk; 
		return rv;
	}


	// --------------------------------------------------------------------------						
	// Function:	Busy
	// Description:	returns if VM process belongs to is busy
	// Arguments:	none
	// Returns:		bool
	// --------------------------------------------------------------------------
	bool LuaProcess::Busy() const
	{ 
		return GetVM()->Busy(); 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetTimeOut
	// Description:	returns time out allowed for process
	// Arguments:	none
	// Returns:		timeout
	// --------------------------------------------------------------------------
	unsigned int LuaProcess::GetTimeOut() const
	{ 
		return ourTimeOut; 
	}


	// --------------------------------------------------------------------------						
	// Function:	OpenNamespace
	// Description:	opens the namespace so functions can be registered to it.
	//				Creates namespace if it doesnt already exist.
	// Arguments:	namespace
	// Returns:		none
	// --------------------------------------------------------------------------						
	void LuaProcess::OpenNamespace(const std::string& fullNameSpace)
	{
		// keep globals on stack
		LuaGetGlobalsStack(myLuaState);

		// if no name register globally
		if (fullNameSpace.empty())
			return;

		int startPos = 0;
		while (startPos < fullNameSpace.size())
		{
			// allow sub namespaces given by dots
			int length = (int)fullNameSpace.find_first_of(".", startPos) - startPos;
			if (length < 0)
				length = (int)fullNameSpace.size() - startPos;

			std::string nameSpace = fullNameSpace.substr(startPos, length);
			startPos += length + 1;

			const TValue* value = LuaGetTableValue(LuaGetGlobalsTable(myLuaState), LuaNewString(myLuaState, nameSpace.c_str()));

			if(LuaGetTypeId(value) ==LUA_TNIL)
			{
				LuaGetGlobalsStack(myLuaState);
				lua_pushstring(myLuaState, nameSpace.c_str());
				StkId newTableName = LuaGetTopStack(myLuaState) - 1;

				// table doesnot exist so create it
				lua_newtable(myLuaState);			// on stack
				StkId newTable = LuaGetTopStack(myLuaState) - 1;

				lua_settable(myLuaState, LuaGetStackSize(myLuaState) - 2);
				lua_pop(myLuaState, 1);

				lua_getglobal(myLuaState, nameSpace.c_str());
				LuaSetGlobals(myLuaState);

			}
			else if(LuaGetTypeId(value) == LUA_TTABLE)
			{
				// table already exists so set it as the global table so we can
				// register functions to it
				LuaSetStackValue(myLuaState, 0, value);
				LuaIncStack(myLuaState);
				LuaSetGlobals(myLuaState);
			}
			else
			{
				// restore globals table before throwing
				LuaSetGlobals(myLuaState);
				LuaApi::ThrowScriptError("Error registering namespace %s, name already exists but not as a namespace.", nameSpace.c_str());
			}

		}
	}


	// --------------------------------------------------------------------------						
	// Function:	CloseNamespace
	// Description:	closes the namespace after functions can be registered to it
	//				returning to previous namespace
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------						
	void LuaProcess::CloseNamespace()
	{
		LuaSetGlobals(myLuaState);
	}


	// --------------------------------------------------------------------------						
	// Function:	HasFunction
	// Description:	returns of the process has a function of the given name
	// Arguments:	name
	// Returns:		bool
	// --------------------------------------------------------------------------						
	bool LuaProcess::HasFunction(const std::string& functionName) const
	{
		const TValue* function = LuaGetTableValue(LuaGetTable(LuaGetGlobalsValue(myLuaState)), LuaNewString(myLuaState, functionName.c_str()));
	
		if (LuaGetTypeId(function) != LUA_TFUNCTION || LuaGetFunctionType(function) == LUA_VCCL || LuaGetFunctionType(function) == LUA_VLCF)
			return false;

		return true;

	}


	// --------------------------------------------------------------------------						
	// Function:	AssureIntegrity
	// Description:	validates integrity of process and its objects
	// Arguments:	whether to only integrity check process not objects
	// Returns:		none
	// --------------------------------------------------------------------------
	void LuaProcess::AssureIntegrity(bool processOnly) 
	{
	}

	
	// --------------------------------------------------------------------------						
	// Function:	GetFunction
	// Description:	gets a function of a given name from the LuaProcess
	//				and checks it has the expected number of arguments
	// Arguments:	funciton name, expected numner of args
	// Returns:		pointer to function
	// --------------------------------------------------------------------------
	const void* LuaProcess::GetFunction(const std::string& functionName, int& argsExpected)
	{
		const TValue* function = LuaGetTableValue(LuaGetTable(LuaGetGlobalsValue(myLuaState)), LuaNewString(myLuaState, functionName.c_str()));
		if(LuaGetTypeId(function)!= LUA_TFUNCTION || LuaGetFunctionType(function) == LUA_VCCL) //|| LuaGetFunctionType(function) == LUA_VLCF)
			return NULL;

		// check arguments
		LClosure* cl = LuaGetLClosure(function);
		const Proto* const tf = cl->p;
		if (LuaGetFunctionType(function) == LUA_VCCL || LuaGetFunctionType(function) == LUA_VLCF)
			argsExpected = -1;
		else
			argsExpected = tf->numparams;

		return function;
	}


	// --------------------------------------------------------------------------						
	// Function:	CallMessage
	// Description:	calls a function in the process specified by the msg package
	// Arguments:	message package, where to allow return values, if call can
	//				yield or timeout
	// Returns:		if it ran ok
	// --------------------------------------------------------------------------
	ExecutionState LuaProcess::CallMessage(Message& msg, bool needReturnValues, bool isYieldable)
	{
		if (msg.myTo.GetObject() != this)
		{
			msg.myState = ExecutionFailed;
			return msg.myState;
		}

		Lock();
		GetVM()->SwapProcessIn(msg.myTo);
		
		bool yieldable = myYieldable;
		myYieldable = isYieldable;
		int timeOut = myTimeOut;
		if (!myYieldable)
			myTimeOut = 0;

		try
		{
			if (myState == ExecutionBusy || myState == ExecutionReceivingCallback)
			{
				// resume yielded function call
				if (!msg.myCallbackMessage)
					myState = ExecutionContinue;
				
				int numResults = 0;
				int status = lua_resume(myLuaState, NULL, 0, &numResults);


				if (status == LUA_ERRRUN)
					myScriptError = true;
				else if (myState == ExecutionOk)
					myState = ExecutionCompleted;
			}
			else
			{
				// frash function call
				unsigned int argsExpected = 0;
				const TValue* function = NULL;
				if (msg.myFunctionName != SoftProcess::ourBootMessage)
				{
					function = LuaGetTableValue(LuaGetTable(LuaGetGlobalsValue(myLuaState)), LuaNewString(myLuaState, msg.myFunctionName.c_str()));
					if (LuaGetTypeId(function) != LUA_TFUNCTION || LuaGetFunctionType(function) == LUA_VCCL || LuaGetFunctionType(function) == LUA_VLCF)
					{
						ERROR_TRACE("LuaProcess::Attempt to call %s when it is not a function, from LuaProcess: %x, Bailing message handler.\n", msg.myFunctionName.c_str(), msg.myTo.GetObject());
						msg.myState = ExecutionFailed;
						Unlock();
						return msg.myState;
					}

					LClosure* cl = LuaGetLClosure(function);
					const Proto* const tf = cl->p;
					argsExpected = tf->numparams;
				}

				myDebugStackSize = LuaGetStackSize(myLuaState);
				myDebugCI = myLuaState->ci;

				// add function to stack
				LuaSetStackValue(myLuaState, 0, function);
				LuaIncStack(myLuaState);

				if (argsExpected != msg.myArguments.size())
				{
					myScriptError = true;
				}
				else
				{
					// add message args to stack
					EnsureFreeStack(myLuaState, (int)msg.myArguments.size());
					for (unsigned int i = 0; i < msg.myArguments.size(); i++)
					{
						if (i < argsExpected)
							msg.myArguments[i].GetType()->Push(Lua, msg.myArguments[i].GetValue());
						
						if(msg.myArguments[i].GetType()->IsIntegralType())
							delete msg.myArguments[i].GetValue();
					}
					int numArgs = (int)msg.myArguments.size();
					msg.myArguments.clear();

					myState = ExecutionOk;

					// call the function
					int numResults = 0;
					int status = lua_resume(myLuaState, NULL, numArgs, &numResults);

					if (status == LUA_ERRRUN)
						myScriptError = true;
					else if (myState == ExecutionOk)
						myState = ExecutionCompleted;
	
					DEBUG_ASSERT(myScriptError != true);
				}
			}
		
			
			if (myScriptError)
			{
				UnwindCallStack();
				myState = ExecutionFailed;
			}
			else if (myState == ExecutionCompleted)
			{
				int numRetVals = LuaGetCallStackSize(myLuaState) - myDebugStackSize;
				if (needReturnValues)
				{
					// add return args to calling message package so can be passed back out
					msg.myReturnValues.clear();
					int v = 0;
					DEBUG_ASSERT(v >= 0);
					while (v != numRetVals)
					{
						msg.myReturnValues.push_back(Message::Argument());
						v++;
					}
					while (v--)
					{
						if (LuaGetStackTypeId(myLuaState, -1) < 0)
						{
							const BaseType* type = reinterpret_cast<const BaseType*>(LuaGetStackType(myLuaState, -1));
							void* object = type->Clone(LuaGetStackUserData(myLuaState, -1));

							msg.myReturnValues[v].myValue = object;
							msg.myReturnValues[v].myType = type;
							LuaDecStack(myLuaState);
						}
						else if (LuaGetStackTypeId(myLuaState, -1) == LUA_TBOOLEAN)
						{
							BaseType* type = Type<bool>::GetStatic();
							bool b = lua_toboolean(myLuaState, -1);
							void* object = type->Clone(&b);

							msg.myReturnValues[v].myValue = object;
							msg.myReturnValues[v].myType = type;
							lua_pop(myLuaState, 1);
						}
						else if (LuaGetStackTypeId(myLuaState, -1) == LUA_TNUMBER)
						{
							BaseType* type = Type<double>::GetStatic();
							double n = lua_tonumber(myLuaState, -1);
							void* object = type->Clone(&n);

							msg.myReturnValues[v].myValue = object;
							msg.myReturnValues[v].myType = type;
							lua_pop(myLuaState, 1);
						}
						else if (LuaGetStackTypeId(myLuaState, -1) == LUA_TSTRING)
						{
							BaseType* type = Type<std::string>::GetStatic();
							std::string s = lua_tostring(myLuaState, -1);
							void* object = type->Clone(&s);

							msg.myReturnValues[v].myValue = object;
							msg.myReturnValues[v].myType = type;
							lua_pop(myLuaState, 1);
						}
						else if (LuaGetStackTypeId(myLuaState, -1) == LUA_TTABLE)
						{
							TValue* value = LuaGetStackValue(myLuaState, -1);
							VariantKeyDictionary* dict = new VariantKeyDictionary;
							LuaHelperFunctions::PopDictionary(myLuaState, value, dict);
							BaseType* type = Type<VariantKeyDictionary>::GetStatic();
							msg.myReturnValues[v].myValue = dict;
							msg.myReturnValues[v].myType = type;
							lua_pop(myLuaState, 1);
						}
						else
						{
							lua_pop(myLuaState, 1);
						}
					}
				}
				else
				{
					while (LuaGetTopStack(myLuaState) > LuaGetCodeBase(myLuaState) + myDebugStackSize + 1)
						LuaDecStack(myLuaState);
				}

				DEBUG_ASSERT(LuaGetStackSize(myLuaState) == myDebugStackSize);
				myDebugStackSize = LuaGetStackSize(myLuaState);
			}
		}
		catch (shh::Exception& e)
		{
			myState = ExecutionError;
			UnwindCallStack();
			Unlock();
			LuaApi::ThrowScriptError("LuaProcess caught error\n%s\nBailing message handler.", e.what());

			ERROR_TRACE("LuaProcess: %x. Error executing script: %s.\n", msg.myTo.GetObject(), e.what());
		}
		catch (...)
		{
			myState = ExecutionError;
			UnwindCallStack();
			Unlock();
			LuaApi::ThrowScriptError("LuaProcess caught error\nUnknown exception type\nBailing message handler.");
			ERROR_TRACE("LuaProcess::Unknown error in function %s, from LuaProcess: %x, Bailing message handler.\n", msg.myFunctionName.c_str(), msg.myTo.GetObject());
		}

		myYieldable = yieldable;
		myTimeOut = timeOut;
		
		GetVM()->SwapProcessOut();
		msg.myState = myState;
		if(myState == ExecutionCompleted)
			SetReady();
		Unlock();
		return msg.myState;
	}


	// --------------------------------------------------------------------------						
	// Function:	InitiateCallback
	// Description:	sets up return values after message call
	// Arguments:	message package
	// Returns:		number of return values to send to call back
	// --------------------------------------------------------------------------
	int LuaProcess::InitiateCallback(Message& msg)
	{
		if (msg.myFrom.GetObject() != this)
			return -1;
	
		// need a Process mutex here cos may be handling message on another threads
		Lock();

		EnsureFreeStack(myLuaState, (int)msg.myReturnValues.size());
		int i;
		GCPtr<Process> oldProcess = Scheduler::GetCurrentProcess();
		Scheduler::SetCurrentProcess(GCPtr<Process>(this));
		
		for (i = 0; i < msg.myReturnValues.size(); i++)
			msg.myReturnValues[i].myType->Push(Lua, msg.myReturnValues[i].myValue);

		Scheduler::SetCurrentProcess(oldProcess);
		msg.myReturnValues.clear();

		Unlock();

		return i;
	}


	// --------------------------------------------------------------------------						
	// Function:	CallFunction
	// Description:	calls a funciton of the given name, expects numArguments
	//				to be on the stack already. Fails if args are wrong or
	//				funciton doesnt exist.Note: expects process swaped in already
	// Arguments:	funciton to call, no args on stack, if returns a value
	// Returns:		true if was called
	// --------------------------------------------------------------------------
	bool LuaProcess::CallFunction(const std::string& functionName, int numArguments, bool returnsVal)
	{

		Message::Argument rv;
		StkId oldStackTop = LuaGetTopStack(myLuaState);
		StkId oldCodeBasebase = LuaGetCodeBase(myLuaState);
		int argsExpected;
		const TValue* func = static_cast<const TValue*>(GetFunction(functionName, argsExpected));
		if (!func)
			return false;

		Lock();
		LuaSetStackValue(myLuaState, 0, func);
		LuaIncStack(myLuaState);

		int top = LuaGetStackSize(myLuaState);
		lua_insert(myLuaState, top - numArguments);
		lua_call(myLuaState, numArguments, (returnsVal ? 1 : 0));

		if (myScriptError)
		{
			// reset so we dont think we have an error on next call
			myScriptError = false;
		}

		LuaGetTopStack(myLuaState) = oldStackTop;
		LuaGetCodeBase(myLuaState) = oldCodeBasebase;
		Unlock();
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetArgument
	// Description:	adds argument of given stack position to the massage package
	// Arguments:	message to add to, argument number tp add
	// Returns:		if successfull
	// --------------------------------------------------------------------------						
	bool LuaProcess::GetArgument(Message& msg, unsigned int arg)
	{

		if (LuaGetCallStackSize(myLuaState) < arg)
			return false;

		// get the registered Type of the arg
		TValue* valueToClone = LuaGetStackValue(myLuaState, arg);
		LuaTypeId typeId = LuaGetTypeId(valueToClone);
		const BaseType* type = NULL;
		if (typeId < 0)
		{
			type = LuaGetType(myLuaState, valueToClone);
		}
		else
		{
			if (typeId == LUA_TSTRING)
			{
				type = Registry::GetRegistry().GetType(LuaType<std::string>::GetTypeId());
			}
			else if (typeId == LUA_TBOOLEAN)
			{
				type = Registry::GetRegistry().GetType(LuaType<bool>::GetTypeId());
			}
			else if (typeId == LUA_TNUMBER)
			{
				if (LuaGetSubType(valueToClone) == LUA_VNUMFLT)
				{
					type = Registry::GetRegistry().GetType(LuaType<double>::GetTypeId());
				}
				else
				{
					type = Registry::GetRegistry().GetType(LuaType<long long>::GetTypeId());
				}
			}
			else if (typeId == LUA_TTABLE)
			{
				type = Registry::GetRegistry().GetType(LuaType<VariantKeyDictionary>::GetTypeId());
			}
			else
			{
				RELEASE_ASSERT(false);
			}

		}

		if(myVM != msg.myTo->GetVM())
		{
			if (type)
			{
				Implementation i = msg.myTo->GetImplementation();
				if(!type->CanSendBetween(i))
					return false;
			}
		}


		// clone the TValue
		TValue valueCloned;
		if (typeId == LUA_TTABLE)
			valueCloned = *valueToClone;
		else
			Clone(valueToClone, &valueCloned);
		
		
		// clone the actual data
		Message::Argument argCloned;
		void* data;
		std::string s;
		bool b;
		double f;
		int i;
		if (typeId < 0)
		{
			data = LuaGetUserData(myLuaState, &valueCloned);
		}
		else if (typeId == LUA_TTABLE)
		{
			VariantKeyDictionary* dict = new VariantKeyDictionary;
			LuaHelperFunctions::PopDictionary(myLuaState, &valueCloned, dict);
			data = dict;
		}
		else
		{
			LuaSetStackValue(myLuaState, 0, &valueCloned);
			if (typeId == LUA_TSTRING)
			{
				s = lua_tostring(myLuaState, 0);
				data = &s;
			}
			else if (typeId == LUA_TBOOLEAN)
			{
				b = lua_toboolean(myLuaState, 0);
				data = &b;
			}
			else if (typeId == LUA_TNUMBER)
			{
				if (LuaGetSubType(&valueCloned) == LUA_VNUMFLT)
				{
					f = lua_tonumber(myLuaState, 0);
					data = &f;
				}
				else
				{
					i = (int)lua_tointeger(myLuaState, 0);
					data = &i;
				}
			}
			else
			{
				RELEASE_ASSERT(false);
				data = NULL;
			}

		}
		argCloned.myType = type;
		argCloned.myValue = argCloned.myType->Clone(data);
		msg.myArguments.push_back(argCloned);

		return true;
	}






} // namespace shh
