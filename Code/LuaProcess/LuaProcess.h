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

#ifndef LUAPROCESS_H
#define LUAPROCESS_H

#include "../Config/MemoryDefines.h"
#include "../Common/SecureStl.h"
#include "../Config/GCPtr.h"
#include "../VM/Messenger.h"
#include "../VM/Scheduler.h"
#include "../VM/SoftProcess.h"
#include "../VM/ClassManager.h"
#include "../VM/VM.h"
#include "LuaWrapper.h"
#include <string>
#include <vector>
#include <map>
#include <list>

namespace shh {

	class MemoryLocator;

	class LuaProcess : public SoftProcess
	{

		DECLARE_MEMORY_MANAGED(LuaProcess);

		template<class T> friend class MemoryLocator::MemoryDestructor;
		friend class LuaApi;
		friend const TValue* luaH_getshortstr(Table* t, TString* key);
		friend void LuaThrowScriptError(const char* format, ...);
		friend class LuaTypeBase;
		friend Message::Argument;

	public:

		static const std::string ourCommentToken;
		static const std::string ourOverideSeperator;
		static const std::string ourScriptExtension;
		static const std::string ourEncyptedScriptExtension;


		static unsigned int ourTimeOut;

		static bool ClassesSpecifier(const std::string& typeName, const std::string& path, Registry::ClassSpecs& unorderedSpecs, bool recurse, bool report, const std::string& classType);
		static GCPtr<Process> Create(Privileges privileges = BasicPrivilege, const GCPtr<Process> spawnFrom = GCPtr<Process>());


		virtual GCPtr<Process> Clone();

		void SetGarbageCollectionGeneratiobnal(int pause, int stepmul, int stepsize);		
		void SetGarbageCollectionGeneratiobnal(int minormul, int majormul);
		static GCPtr<LuaProcess> GetCurrentLuaProcess();
		static lua_State* GetCurrentLuaState(bool throwError = true);
		lua_State* GetLuaState() { return myLuaState; }

		virtual void CollectAllGarbage();
		virtual bool ValidateFunctionNames(bool allowClassOnlyFunctions);
		virtual bool Overide(const std::string& derivedCode, const std::string& overidePrefix);
		virtual bool Execute(const std::string& s, bool isFile, bool isYieldable = false, Paths paths = Paths());

		void Clone(TValue* valueToClone, TValue* valueCloned);
		bool GetTable(const std::string& name, TValue& table);

		static void LogError(const std::string& error);
		virtual bool YieldProcess(ExecutionState state, int results);
		virtual int ContinueProcess();
		virtual bool Busy() const;
		virtual unsigned int GetTimeOut() const;
	
		void OpenNamespace(const std::string& fullNameSpace);
		void CloseNamespace();

		virtual bool HasFunction(const std::string& functionName) const;
		virtual void AssureIntegrity(bool processOnly = false);

	protected:

	
		static lua_State *ourMasterLuaState;
		static unsigned int ourNumLuaProcesses;

		LuaProcess(Privileges privileges, const GCPtr<LuaProcess> &spawnFrom);
		~LuaProcess();
		virtual const void* GetFunction(const std::string& functionName, int& argsExpected);
		virtual ExecutionState CallMessage(Message& msg, bool needReturnValues, bool isYieldable = true);
		virtual int InitiateCallback(Message& msg);

		


	private:

		lua_State* myLuaState;
		LuaGCObject* myInheritedFixedGCs;
		LuaGCObject* myInheritedAllGCs;
		bool myScriptError;

		std::vector<int> myResumeStates;

		unsigned int myDebugStackSize;
		CallInfo* myDebugCI;

		void UnwindCallStack();
		bool CallFunction(const std::string& functionName, int numArguments, bool returnsVal);
		virtual bool GetArgument(Message& msg, unsigned int arg);
		
	};

}// namespace shh
#endif

