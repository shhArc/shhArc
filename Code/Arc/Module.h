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

#ifndef MODULE_H
#define MODULE_H


#include "../Common/SecureStl.h"
#include "../Common/Enums.h"
#include "../Common/Dictionary.h"
#include "../Config/GCPtr.h"



#define SHHARC_API 

#define REGISTER_MODULE(MODULE) static GCPtr<Module> theRegistered##MODULE = Registry::GetRegistry().RegisterModule(GCPtr<Module>(new MODULE)); 
#define REGISTER_OBJECT_MODULE(OBJECT, MANAGER) static GCPtr<Module> theRegisteredObject##OBJECT = Registry::GetRegistry().RegisterModule(GCPtr<Module>(new ObjectModule<OBJECT, MANAGER>(#OBJECT))); 


namespace shh {

	class Message;
	class VM;
	class Process;
	class Module;
	
	class Module : public virtual GCObject
	{
	public:

		class Sorter
		{

		public:

			

			Sorter(const std::string& name) : myName(name), myId(++ourLastId) {}
			bool operator<(const Sorter& other) const { return myId < other.myId; }
			std::string myName;
			shhId myId;

		private:

			static shhId ourLastId;
		};

		typedef std::map< Sorter, GCPtr<Module> > Map;

		typedef enum 
		{
			IntegrityCehckOnInit = 0
		} Flags;
		
		Module();
		Module(const GCPtr<GCObject>& owner, double timeOut);
		~Module();

		virtual GCPtr<Module> Clone() { return GCPtr<Module>(new Module()); }

		virtual bool RegisterTypes(const std::string& alias, const StringKeyDictionary& sd);
		virtual bool Register(const std::string& alias, const StringKeyDictionary& sd, Privileges privileges);
		virtual bool Initialize(const GCPtr<GCObject>& owner, const std::string& id, const StringKeyDictionary& sd);
		virtual bool Finalize(GCObject* me);

		virtual bool Update(double until, unsigned int phase);
		virtual void FlagUpdateCompleted();
		virtual bool InitializeSubModule(const std::string& id, const StringKeyDictionary& sd);
		virtual bool FinalizeSubModule(const std::string& id);
		virtual bool UpdateSubModule(const std::string& id, double until, unsigned int phase);
		virtual bool GetNextMessage(double until, unsigned int phase, Message*& msg);

		virtual void SetActive(const GCPtr<GCObject>& object);
		virtual void SetAsInactive();
		virtual unsigned int GetNumPhasesPerUpdate() const;

		inline bool RequiresUpdate() const;
		inline bool SetOwner(const GCPtr<GCObject>& owner);
		inline const GCPtr<GCObject>& GetOwner() const;
		inline const GCPtr<GCObject>& GetVM() const;
		inline int GetPriority() const;
		inline int GetSubPriority()const;
		inline double GetUpdateUntilTime() const;
		inline Implementation GetImplementation() const;
		inline const std::string& GetId() const;

		virtual bool AssureIntegrity();
		virtual bool RequiresIntegrityCheckOnInit() const;
		virtual std::string GetName() const;
		static std::string StripName(const std::string& text);


		void SetFlag(int flagNo, bool state);
		bool GetFlag(int flagNo) const;


	

	protected:

		unsigned int myFlags;
		GCPtr<GCObject> myOwner;
		std::string myId;
		GCPtr<GCObject> myVM;
		int myPriority;
		int	mySubPriority;

		Implementation myImplementation;
		
		double myTimeOut;
		unsigned int myPhase;
		bool myRequiresUpdate;
		double myUpdateUntilTime;
		double myDelta;
		double myLastUpdateCallTime;
		bool myFullUpdate;

		Map mySubModules;

	};


	inline bool Module::RequiresUpdate() const 
	{ 
		return myRequiresUpdate; 
	}

	inline bool Module::SetOwner(const GCPtr<GCObject>& owner)
	{
		if (!myOwner.IsValid())
		{
			myOwner = owner;
			return true;
		}
		return false;
	}

	inline const GCPtr<GCObject>& Module::GetOwner() const
	{ 
		return myOwner; 
	}

	inline const GCPtr<GCObject>& Module::GetVM() const
	{ 
		return myVM; 
	}

	inline int Module::GetPriority()const
	{ 
		return myPriority; 
	}

	inline int Module::GetSubPriority()const
	{ 
		return mySubPriority; 
	}


	inline double Module::GetUpdateUntilTime() const
	{ 
		return myUpdateUntilTime; 
	}

	inline Implementation Module::GetImplementation() const
	{ 
		return myImplementation; 
	}


	inline const std::string& Module::GetId() const
	{ 
		return myId; 
	}

}

#endif