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
#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H


#include "../Common/SecureStl.h"
#include "../Common/Mutex.h"
#include "../Common/Dictionary.h"
#include "../Arc/Module.h"
#include "../Config/GCPtr.h"
#include "../Config/Config.h"


namespace shh {

	class VM;

	class ClassManager;
	class Scheduler;

	class Environment : public Module
	{

	public:


		Environment();
		~Environment();

		virtual bool Initialize(const GCPtr<GCObject>& owner, const std::string& id, const StringKeyDictionary& sd);
		virtual bool Finalize(GCObject* me);
		virtual bool Update(double until, unsigned int phase);


		bool GetClassManager(const std::string& name, GCPtr<ClassManager>& manager) const;
		bool SetClassManager(const std::string& name, const GCPtr<ClassManager>& manager);

		const GCPtr<Scheduler>& GetScheduler() const;
		const GCPtr<VM>& GetVM() const;




		static void SetMetaVariables(const StringKeyDictionary& d);
		static void SetGlobalEnvironment(const GCPtr<Environment>& e);


		template<class T> static const T GetMeta(const std::string& variableKey, const T& defaultValue);
		template<class T> static bool SetMeta(const std::string& variableKey, const T& value);
		std::string static GetMeta(const std::string& variableKey, const char defaultValue[]);
		bool static SetMeta(const std::string& variableKey, const char value[]);


		template<class T> static const T GetGlobal(const std::string& variableKey, const T& defaultValue, bool checkMeta = true);
		template<class T> static bool SetGlobal(const std::string& variableKey, const T& value);
		std::string static GetGlobal(const std::string& variableKey, const char defaultValue[], bool checkMeta = true);
		bool static SetGlobal(const std::string& variableKey, const char value[]);

		template<class T> const T GetLocal(const std::string& variableKey, const T& defaultValue) const;
		template<class T> bool SetLocal(const std::string& variableKey, const T& value);
		std::string GetLocal(const std::string& variableKey, const char defaultValue[]) const;
		bool SetLocal(const std::string& variableKey, const char value[]);

		void IncEntryCount();
		void DecEntryCount();

		void FinalizeObjects();

	protected:

		typedef std::map<GCPtr<Environment>, StringKeyDictionary> Locals;
		typedef std::map<std::string, GCPtr<ClassManager>> ClassManagers;
	

		static StringKeyDictionary ourMetaConfig;
		static Mutex ourMutex;
		static GCPtr<Environment> ourGlobalEnvironment;
			
		mutable Mutex myMutex;
		StringKeyDictionary myLocalConfig;
		
		GCPtr<Scheduler> myScheduler;
		GCPtr<VM> myVM;
		ClassManagers myClassManagers;
		unsigned int myEntryCount;

		
	};



	// --------------------------------------------------------------------------						
	// Function:	GetMeta
	// Description:	gets a meta variable of keu
	// Arguments:	key, default value of not found
	// Returns:		variable
	// --------------------------------------------------------------------------
	template<class T> const T Environment::GetMeta(const std::string& variableKey, const T& defaultValue)
	{
#if MULTI_THREADED
		ourMutex.LockMutex();
#endif
		T value = Dictionary<std::string>::GetConfig(variableKey, defaultValue, ourMetaConfig);
#if MULTI_THREADED
		ourMutex.UnlockMutex();
#endif
		return value;
	}


	// --------------------------------------------------------------------------						
	// Function:	SetMeta
	// Description:	sets a meta variable of keu
	// Arguments:	key, value
	// Returns:		if successful
	// --------------------------------------------------------------------------
	template<class T> bool Environment::SetMeta(const std::string& variableKey, const T& value)
	{
#if MULTI_THREADED
		ourMutex.LockMutex();
#endif
		bool ok = Dictionary<std::string>::SetConfig(variableKey, value, ourMetaConfig);
#if MULTI_THREADED
		ourMutex.UnlockMutex();
#endif
		return ok;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetGlobal
	// Description:	gets a global variable of keu
	// Arguments:	key, default value of not found
	// Returns:		variable
	// --------------------------------------------------------------------------
	template<class T> static const T Environment::GetGlobal(const std::string& variableKey, const T& defaultValue, bool checkMeta)
	{
		T value = defaultValue;
		if (checkMeta)
			value = GetMeta(variableKey, value);

		value = ourGlobalEnvironment->GetLocal(variableKey, value);

		return value;
	}


	// --------------------------------------------------------------------------						
	// Function:	SetGlobal
	// Description:	sets a global variable of keu
	// Arguments:	key, value
	// Returns:		if successful
	// --------------------------------------------------------------------------
	template<class T> static bool Environment::SetGlobal(const std::string& variableKey, const T& value)
	{
		return ourGlobalEnvironment->SetLocal(variableKey, value);
	}


	// --------------------------------------------------------------------------						
	// Function:	GetLocal
	// Description:	gets a local variable of keu
	// Arguments:	key, default value of not found
	// Returns:		variable
	// --------------------------------------------------------------------------
	template<class T> const T Environment::GetLocal(const std::string& variableKey, const T& defaultValue) const
	{
#if MULTI_THREADED
		myMutex.LockMutex();
#endif
		T value = Dictionary<std::string>::GetConfig(variableKey, defaultValue, myLocalConfig);
#if MULTI_THREADED
		myMutex.UnlockMutex();
#endif
		return value;
	}


	// --------------------------------------------------------------------------						
	// Function:	SetLocal
	// Description:	sets a local variable of keu
	// Arguments:	key, value
	// Returns:		if successful
	// --------------------------------------------------------------------------
	template<class T> bool Environment::SetLocal(const std::string& variableKey, const T& value)
	{
#if MULTI_THREADED
		myMutex.LockMutex();
#endif
		bool ok = Dictionary<std::string>::SetConfig(variableKey, value, myLocalConfig);
#if MULTI_THREADED
		myMutex.UnlockMutex();
#endif
		return ok;
	}



} // namespace shh
#endif
