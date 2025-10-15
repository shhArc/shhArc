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


#ifndef REALM_H
#define REALM_H


#include "../Common/SecureStl.h"
#include "../Config/GCPtr.h"
#include "../Arc/Environment.h"
#include "../Arc/Module.h"
#include "../File/Archive.h"



namespace shh {

	class LuaProcess;
	class SoftProcess;

	class Realm : public Environment
	{
		//DECLARE_ARCHIVE(Realm)

		friend class God;

	public:

		typedef std::map<std::string, GCPtr<Realm>> RealmMap;
		typedef std::vector<GCPtr<Realm>> Realms;
		typedef std::map<std::string, StringKeyDictionary> DictMap;


		static GCPtr<Realm> Create(const ::std::string& name, const StringKeyDictionary& config, const GCPtr<Realm>& other);
		static const GCPtr<Realm>& GetRealm(const ::std::string& name);
		static void DestroyRealm(const ::std::string& name);
			
		virtual bool Initialize(const GCPtr<GCObject>& owner, const std::string& id, const StringKeyDictionary& sd);
		virtual bool Update(double until, unsigned int phase);
		virtual bool Finalize(GCObject* me);

		bool SetAsActiveRealm();
		void SetAsInactiveRealm();
		static const GCPtr<Realm>& GetActiveRealm();
		bool IsActive() const;
		virtual std::string GetName() const;

		double GetTime();
		void Pause(bool paused);
		bool GetPaused();
		virtual double GetStepSize();
		


		
		void Read(IOInterface& io, int version);
		void Write(IOInterface& io, int version) const;

		static void CloseDown();

		static unsigned int ourVersion;

	protected:

		static GCPtr<LuaProcess> ourLuaTypeBase;

		std::string myName;
		Privileges myPrivileges;

		bool myPaused;
		double myTime;
		double myStartTime;
		double myLastUpdateTime;
		double myObjectStartTime;
		
		std::vector<std::string> myBootPath;
		std::vector<std::string> myUpdatePath;

		GCPtr<SoftProcess> myUpdaterProcess;
		StringKeyDictionary myUpdaterDict;

	
		Realm();
		Realm(const std::string& name, const Privileges& privileges, const std::string& id);
		virtual ~Realm();

		void Configure(const StringKeyDictionary& config, const GCPtr<Realm>& other);
		
	private:
		
		static RealmMap ourRealmMap;
		static GCPtr<Realm> ourActiveRealm;
		
		static ExecutionState InitializeModule(std::string& id, VariantKeyDictionary& vd, bool& result);
		static ExecutionState FinalizeModule(std::string& id, bool& result);
		static ExecutionState UpdateModule(std::string& id, double& until, unsigned int& phase, bool& result);
		static ExecutionState UpdateScheduler(double& until, unsigned int& phase, bool& result);

		
	};



}

#endif // Realm_H
