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

#include "../Common/Mutex.h"
#include "../Common/ThreadSafety.h"
#include "../Common/Debug.h"
#include "../Common/Exception.h"
#include "../File/BinaryFile.h"
#include "../File/FileSystem.h"
#include "../Arc/Registry.h"
#include "SoftProcess.h"


namespace shh {

	const std::string SoftProcess::ourInheritanceToken = "Specializes";
	const std::string SoftProcess::ourAbstractToken = "Abstract";
	const std::string SoftProcess::ourFinalToken = "Final";
	const std::string SoftProcess::ourNewLineToken = "\n";

	std::string SoftProcess::ourBootMessage = "__BOOT";
	std::string SoftProcess::ourMainMessage = "shhMain";
	std::string SoftProcess::ourInitializeMessage = "shhInitialize";
	std::string SoftProcess::ourFinalizeMessage = "shhFinalize";
	std::string SoftProcess::ourUpdateMessage = "shhUpdate";
	std::string SoftProcess::ourMessagePrefix = "shhMessage";
	std::string SoftProcess::ourTimerPrefix = "shhTimer";
	std::string SoftProcess::ourSystemPrefix = "shhSystem";
	std::string SoftProcess::ourStaticPrefix = "shhStatic";


	Privileges SoftProcess::ourExcludedSendMessengers = BasicPrivilege | SchemaPrivilege;
	Privileges SoftProcess::ourExcludedExecuters = BasicPrivilege | SchemaPrivilege;
	Privileges SoftProcess::ourInitializerMessengers = AgentPrivilege | SchemaPrivilege;
	Privileges SoftProcess::ourFinalizerMessengers = AgentPrivilege | SchemaPrivilege;

	bool SoftProcess::ourAllowMessegesWhenFinalising = true;


	// --------------------------------------------------------------------------						
	// Function:	SoftProcess
	// Description:	constructor
	// Arguments:	privileges this process has
	// Returns:		none
	// --------------------------------------------------------------------------
	SoftProcess::SoftProcess(Privileges privileges) :
		Process(privileges),
		myTimeOut(1000),
		myTimeOutCount(0),
		myMaxInstructions(-1),
		myInstructionCount(0),
		myMaxLockCount(-1)
	{	}


	// --------------------------------------------------------------------------						
	// Function:	Process
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	SoftProcess::~SoftProcess()
	{
	}


	// --------------------------------------------------------------------------						
	// Function:	Initialize
	// Description:	initializes process
	// Arguments:	none
	// Returns:		if sucessful
	// --------------------------------------------------------------------------
	bool SoftProcess::Initialize()
	{
		if (GetPrivileges() & ourInitializerMessengers)
		{
			myInitializing = true;
			Message* msg = new Message();
			msg->myTo = GCPtr<Messenger>(dynamic_cast<Messenger*>(this));
			msg->myFunctionName = SoftProcess::ourInitializeMessage;
			if (msg->Build(0, 0) == Message::BuildOk)
			{
				msg->SendMsg(0.0, 0);
				return false;
			}
		}
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	Terminate
	// Description:	finalizes and destroys
	// Arguments:	caller
	// Returns:		none
	// --------------------------------------------------------------------------
	bool SoftProcess::Terminate(const GCPtr<Messenger> &caller)
	{
		// note: process for classes have vm nulled after building
		if (!myFinalizing && !IsDying() && myVM.IsValid())
		{
			Process::Terminate(caller);
			myFinalizing = true;
			Message* msg = new Message;
			msg->myFunctionName = ourFinalizeMessage;
			msg->myTo = GCPtr<Process>(this);
			msg->myFrom = caller;
			msg->SetCallType(Message::Synchronous);
			msg->myDestroyOnCompletion = true;
			msg->myPriority = Priority::GetSystem();
			if (msg->SendMsg(0.0, 1))
				return false;

			delete msg;
		}
		return CompleteFinalization();
	}


	// --------------------------------------------------------------------------						
	// Function:	ValidateFunctionNames
	// Description:	validates whether function names in process are allowed
	// Arguments:	allow class function prefixes
	// Returns:		if ok
	// --------------------------------------------------------------------------
	bool SoftProcess::ValidateFunctionNames(bool allowClassOnlyFunctions)
	{ 
		return true; 
	}


	// --------------------------------------------------------------------------						
	// Function:	Overide
	// Description:	overides virtual function in parent class
	// Arguments:	derived file to run, overide prefix 
	// Returns:		if sucessful
	// --------------------------------------------------------------------------
	bool SoftProcess::Overide(const std::string& derivedCode, const std::string& overidePrefix) 
	{ 
		return true; 
	}


	// --------------------------------------------------------------------------						
	// Function:	Execute
	// Description:	executes a file or sting of lua script code in this process
	// Arguments:	filename or string to run, whether string is a file, 
	//				whether to allow yielding and timeout, paths to search for 
	//				file (leave blank to use default paths of process)
	// Returns:		if was run without error
	// --------------------------------------------------------------------------
	bool SoftProcess::Execute(const std::string& s, bool isFile, bool isYieldable, Paths paths)
	{
		return false; 
	}


	// --------------------------------------------------------------------------						
	// Function:	SetReady
	// Description:	flag process as ready to recieve messages
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void SoftProcess::SetReady()
	{
		myNumMessagesSentThisUpdate = 0;
		myTimeOutCount = 0;
		myTimeOut = GetTimeOut();
		if (myState == ExecutionBusy || myState == ExecutionYielded || myState == ExecutionTimedOut)
		{
			myState = ExecutionBusy;
		}
		else if (myState != ExecutionAwaitingCallback)
		{
			myCurrentMessage = NULL;
			myState = ExecutionReady;
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	GetTimeOut
	// Description:	returns how long process has run for
	// Arguments:	none
	// Returns:		time out
	// --------------------------------------------------------------------------
	unsigned int SoftProcess::GetTimeOut() const 
	{ 
		return 0; 
	}


	// --------------------------------------------------------------------------						
	// Function:	IncTimeOut
	// Description:	increments how long process has run for
	// Arguments:	none
	// Returns:		time out
	// --------------------------------------------------------------------------
	bool SoftProcess::IncTimeOut()
	{
#if MULTI_THREADED 
		return (myTimeOut != 0 && ++myTimeOutCount >= myTimeOut);
#else
		return false;
#endif
	}


	// --------------------------------------------------------------------------						
	// Function:	TimedOut
	// Description:	returns if process has timed out
	// Arguments:	none
	// Returns:		bool
	// --------------------------------------------------------------------------
	bool SoftProcess::TimedOut() const
	{
#if MULTI_THREADED 
		return (myTimeOut != 0 && myTimeOutCount >= myTimeOut);
#else
		return false;
#endif
	}


	// --------------------------------------------------------------------------						
	// Function:	RegisterModule
	// Description:	Registers handler setup function with the Process. The funciton
	//				may register new data types and/or new script functions
	//				with the Process. The functions is run by the Process to do that.
	//				** All registrations are duplicated in spawned Processs **
	// Arguments:	module name, dictionary of config vars for module setup
	// Returns:		if could be registered 
	// --------------------------------------------------------------------------
	bool SoftProcess::RegisterModule(const std::string& name, const StringKeyDictionary& sd)
	{
		ModuleNames::iterator it = std::find(myRegisteredModules.begin(), myRegisteredModules.end(), name);
		if (it == myRegisteredModules.end())
			return false;

		myRegisteredModules.push_back(name);
		Registry::GetRegistry().RegisterModuleInProcess(GCPtr<Process>(this), name, "", sd);

		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	AddScriptPath
	// Description:	adds disk search path to look for script files
	// Arguments:	path string
	// Returns:		none
	// --------------------------------------------------------------------------
	void SoftProcess::AddScriptPath(const std::string& path)
	{
		if (std::find(myPaths.begin(), myPaths.end(), path) == myPaths.end())
			myPaths.push_back(path);
	}


	// --------------------------------------------------------------------------						
	// Function:	GetScriptPaths
	// Description:	return paths can search to run scripts
	// Arguments:	none
	// Returns:		paths
	// --------------------------------------------------------------------------
	const SoftProcess::Paths& SoftProcess::GetScriptPaths() const
	{ 
		return myPaths; 
	}


	// --------------------------------------------------------------------------						
	// Function:	ClassesSpecifier
	// Description:	recusrses though folder loading class files reading the slass 
	//				specifiers
	// Arguments:	implementation (Lua/Python), typename (Agent/Node), comment
	//				prefix token for class spec string, seperator token prefixing 
	//				class name with parent name, separator to prefix overided 
	//				function names with, script file name extension to 
	//				scan, encypted script file name extension to scan, path to 
	//				scan for filesclass specs to return, whether to recurse, 
	//				whether to report error, class name as appears in script
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool SoftProcess::ClassesSpecifier(Implementation implementation, 
		const std::string& typeName, const std::string& commentToken, 
		const std::string& overideSperator, const std::string& scriptExtension,
		const std::string& encyptedScriptExtension, const std::string& path, 
		Registry::ClassSpecs& unorderedSpecs, bool recurse, bool report, 
		const std::string& classType)
	{
		std::vector<std::string> subDirs;
		std::string filter = "*" + scriptExtension;
		FileSystem::GetDirectoryContents(path, "*", FileSystem::GetDirectories, subDirs);

		for (int s = 0; s != subDirs.size(); s++)
		{
			//if (report)
			//	myProgressReport(myManagerType + " scanning for "+ classType, ((float)s) / (float)subDirs.size());

			std::string fullPath = path + "/" + subDirs[s];

			std::vector<std::string> files;
			std::string filter = "*." + scriptExtension;
			FileSystem::GetDirectoryContents(fullPath, filter, FileSystem::GetFiles, files);

			std::vector<std::string> encFiles;
			filter = "*" + encyptedScriptExtension;
			FileSystem::GetDirectoryContents(fullPath, filter, FileSystem::GetFiles, encFiles);

			for (int f = 0; f < encFiles.size(); f++)
			{
				std::string name = encFiles[f].substr(0, encFiles[f].length() - 4);
				std::string unEncFile = name + scriptExtension;
				if (std::find(files.begin(), files.end(), unEncFile) == files.end())
					files.push_back(encFiles[f]);
			}

			// load class files and set up inheritance specs
			for (int f = 0; f != files.size(); f++)
			{

				std::string errorMessage;
				Registry::ClassSpec classSpec;
				classSpec.myImplementation = implementation;
				classSpec.myPath = fullPath + "/";
				classSpec.myFilename = files[f];

				std::string fullFile = fullPath + "/" + files[f];
				std::string ext = fullFile.substr(fullFile.length() - 4, fullFile.length());
				bool enc = (ext == encyptedScriptExtension);
				if (!ReadClassFile(typeName, commentToken, fullFile, classSpec, errorMessage, enc))
					ERROR_TRACE("%s in %s class file: %s.\n", errorMessage.c_str(), classType.c_str(), fullFile.c_str());


				if (classSpec.myClassName != files[f].substr(0, files[f].size() - 4))
				{
					ERROR_TRACE("%s class name: %s does not match class file name: %s. Skipping.\n", classType.c_str(), classSpec.myClassName.c_str(), files[f].c_str());
					continue;
				}

				if (!classSpec.myParentName.empty())
					classSpec.myOveridePrefix = classSpec.myParentName+overideSperator;

				Registry::ClassSpecs::iterator exists = unorderedSpecs.find(classSpec.myClassName);
				if (exists != unorderedSpecs.end())
					RELEASE_TRACE("%s: %s already exists.\n", classType.c_str(), classSpec.myClassName.c_str());
				else
					unorderedSpecs[classSpec.myClassName]= classSpec;
			}

			if (files.empty() && recurse)
				ClassesSpecifier(implementation, typeName, commentToken, overideSperator, scriptExtension, encyptedScriptExtension, fullPath, unorderedSpecs, false, report, classType);



		}
		return true;

	}


	// --------------------------------------------------------------------------						
	// Function:	ReadClassFile
	// Description:	reads agent class file from disk and creates the class spec
	// Arguments:	typename (Agent/Node), comment prefix token for class spec 
	//				string, file name, spec returned, error message returned 
	//				returned if not read ok, if file is encrypted
	// Returns:		true if read ok
	// --------------------------------------------------------------------------
	bool SoftProcess::ReadClassFile(const std::string& typeName, const std::string& commentToken, const std::string& filename, Registry::ClassSpec& classSpec, std::string& errorMessage, bool encrypted)
	{

		std::string script = LoadIntoString(filename, encrypted);
		classSpec.myScript = script;

		int pos = 0;
		while (pos < script.length())
		{
			char c = script[pos];
			if (c == 13)
				script = script.substr(0, pos) + script.substr(pos + 1, script.length() - pos);
			else
				pos++;
		}




		pos = 0;
		if (classSpec.myScript.substr(pos, commentToken.length()) != commentToken)
		{
			errorMessage += "No class definition.";
			return false;
		}

		pos+= (int)commentToken.length();	// skip #


		// get class name
		if (GetToken(script, pos, classSpec.myClassName, errorMessage) != Token)
			return false;

		// get inheritance token
		std::string token;
		if (GetToken(script, pos, token, errorMessage) != Token)
			return false;

		if (token != ourInheritanceToken)
		{
			errorMessage += "Bad token :" + token + ". ";
			errorMessage += ourInheritanceToken + " expected.";
			return false;
		}


		// get parent class name
		if (GetToken(script, pos, classSpec.myParentName, errorMessage) != Token)
			return false;

		if (classSpec.myParentName == typeName)
			classSpec.myParentName.clear();

		// get inheritance type class name
		TokenType tokType = GetToken(script, pos, token, errorMessage);


		if (tokType == EndOfLine)
		{
			classSpec.myInheritanceType = Registry::ClassSpec::Inheritable;
		}
		else
		{
			// should be inheritance type
			if (token == ourAbstractToken)
			{
				classSpec.myInheritanceType = Registry::ClassSpec::Abstract;
			}
			else if (token == ourFinalToken)
			{
				classSpec.myInheritanceType = Registry::ClassSpec::Final;
			}
			else
			{
				errorMessage += "Bad token :" + token + ". Optional inheritance qualifier expected.";
				return false;
			}

			tokType = GetToken(script, pos, token, errorMessage);
		}


		// should have embeded toke or end of line at this point

		if (tokType == EndOfLine)
		{
			return true;
		}
		else
		{
			errorMessage += "End of line expected.";
			return false;
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	LoadIntoString
	// Description:	loads a file into a string
	// Arguments:	file name, if file is encrypted
	// Returns:		string
	// --------------------------------------------------------------------------
	std::string SoftProcess::LoadIntoString(const std::string& filename, bool encrypted)
	{
		//if (encrypted)
		//{
		//	theEncrypted = true;
		//	theBuffer.LoadFile(filename);
		//	return theDecryptedBuffer;
		//}
		//else
		{
			//theEncrypted = false;
			BinaryFile file(filename, IOInterface::In);
			std::string data;
			int length;
			if (FileSystem::GetFileSize(filename, length))
			{
				data.resize(length);
				file.Read((char*)&data[0], length);
			}
			return data;
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	GetToken
	// Description:	given string and a position to start from
	//				get the next token
	// Arguments:	script string, pos, token to return, errormessage returned 
	//				if not read ok
	// Returns:		token type
	// --------------------------------------------------------------------------
	SoftProcess::TokenType SoftProcess::GetToken(const std::string& script, int& pos, std::string& token, std::string& errorMessage)
	{

		// skip whilte space
		while (script[pos] == ' ')
			pos++;


		if (script[pos] == ourNewLineToken[0])
			return EndOfLine;

		// get parent class name
		int end1 = (int)script.find(ourNewLineToken, pos);
		int end2 = (int)script.find(" ", pos);
		int end3 = (int)script.find(",", pos);

		int end;
		if ((end1 < end2 || end2 == -1) && (end1 < end3 || end3 == -1))
			end = end1;
		else if (end2 < end3 || end3 == -1)
			end = end2;
		else
			end = end3;


		token = script.substr(pos, end - pos);
		if (token[token.length() - 1] == '\r')
			token = token.substr(0, token.length() - 1);

		if (!ValidateToken(token, errorMessage))
			return BadToken;


		pos = end;

		return Token;

	}


	// --------------------------------------------------------------------------						
	// Function:	ValidateToken
	// Description:	checks a token contains the right alpha numeric chars
	// Arguments:	token, errormessage returned if not ok
	// Returns:		true if ok
	// --------------------------------------------------------------------------
	bool SoftProcess::ValidateToken(const std::string& token, std::string& errorMessage)
	{
		// must begin with a capital letter
		std::string firstChar = token.substr(0, 1);
		const std::string allowedFirstChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
		if (allowedFirstChars.find(firstChar, 0) == -1)
		{
			errorMessage += "Invalid token: " + token + ". Must begin with a letter.";
			return false;
		}

		// all others must be alpha numeric
		int c = 0;
		while (c != token.size())
		{
			std::string charToCheck = token.substr(c, 1);
			const std::string allowedChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890";
			if (allowedChars.find(charToCheck, 0) == -1)
			{
				errorMessage += "Invalid token: " + token + ". Must contain alpha numeric characters only.";
				return false;
			}
			c++;
		}

		return true;
	}
}
