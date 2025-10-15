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

#ifndef SOFTPROCESS_H
#define SOFTPROCESS_H


#include "../Common/SecureStl.h"
#include "../Config/GCPtr.h"
#include "Process.h"
#include "Class.h"
#include "VM.h"

namespace shh {


	class SoftProcess : public Process
	{
		friend class Class;

	public:

		enum TokenType { Token, BadToken, EndOfLine };

		typedef std::vector<std::string > ModuleNames;
		typedef std::vector<std::string> Paths;


		static const std::string ourInheritanceToken;
		static const std::string ourAbstractToken;
		static const std::string ourFinalToken;
		static const std::string ourNewLineToken;
		
		SoftProcess(Privileges privileges);
		virtual ~SoftProcess();

		virtual bool Initialize();
		virtual bool Terminate(const GCPtr<Messenger>& caller);



		virtual bool ValidateFunctionNames(bool allowClassOnlyFunctions);
		virtual bool Overide(const std::string& derivedCode, const std::string& overidePrefix);
		virtual bool Execute(const std::string& s, bool isFole, bool isYieldable = false, Paths paths = Paths());
		virtual void SetReady();
		virtual unsigned int GetTimeOut() const;
		bool IncTimeOut();
		bool TimedOut() const;

		bool RegisterModule(const std::string& name, const StringKeyDictionary& sd);
		void AddScriptPath(const std::string& path);
		const Paths& GetScriptPaths() const;

		static bool ClassesSpecifier(Implementation implementation, const std::string& typeName, const std::string& commentToken, const std::string& overideSperator, const std::string& scriptExtension, const std::string& encyptedScriptExtension, const std::string& path, Registry::ClassSpecs& unorderedSpecs, bool recurse, bool report, const std::string& classType);
		static bool ReadClassFile(const std::string& typeName, const std::string& commentToken, const std::string& filename, Registry::ClassSpec& classSpec, std::string& errorMessage, bool encrypted);
		static std::string LoadIntoString(const std::string& filename, bool encrypted);
		static TokenType GetToken(const std::string& script, int& pos, std::string& token, std::string& errorMessage);
		static bool ValidateToken(const std::string& token, std::string& errorMessage);

		static std::string ourBootMessage;
		static std::string ourMainMessage;
		static std::string ourInitializeMessage;
		static std::string ourFinalizeMessage;
		static std::string ourUpdateMessage;
		static std::string ourMessagePrefix;
		static std::string ourTimerPrefix;
		static std::string ourSystemPrefix;
		static std::string ourStaticPrefix;
		static Privileges ourExcludedSendMessengers;
		static Privileges ourExcludedExecuters;
		static Privileges ourInitializerMessengers;
		static Privileges ourFinalizerMessengers;
		static bool ourAllowMessegesWhenFinalising;
		


	protected:

	

		GCPtr<Object> myObject;
		ModuleNames myRegisteredModules;
		Paths myPaths;



		typedef std::map<long, GCPtr<Process> > CurrentProcesses;
		static GCPtr<Process> ourCurrentProcess;
		static CurrentProcesses ourProcessestThreads;

		unsigned int myTimeOut;
		unsigned int myTimeOutCount;
		unsigned int myMaxInstructions;
		unsigned int myInstructionCount;	// master counter to check for infinate loops
		unsigned int myMaxLockCount;

		
	};

}

#endif	// PROCESS_H
