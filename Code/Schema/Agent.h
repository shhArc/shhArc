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

#ifndef AGENT_H
#define AGENT_H


#include "../Config/MemoryDefines.h"
#include "../Common/SecureStl.h"
#include "../Common/Debug.h"
#include "../Common/Classifier.h"
#include "../VM/VM.h"
#include "../VM/Object.h"
#include "Schema.h"
#include "Whole.h"

#include <vector>
#include <queue>

namespace shh {



	class Agent;
	class ClassManager;
	class Message;
#



	class Agent : public VM, public Object, public Schema, public Whole
	{
		DECLARE_MEMORY_MANAGED(Agent);
		friend class ClassManager;

	public:

		static GCPtr<Object> Create(const GCPtr<ClassManager>& classManager, const GCPtr<Class>& objectClass, const GCPtr<Process>& process);


		virtual ~Agent();
	
		inline void SetNeedsComponentIntegrityCheck();
		inline const Classifier& GetTraceClassifier() const;

		virtual bool AddSlaveProcess(const GCPtr<Process>& slave);
		virtual bool RemoveProcess(const GCPtr<Process>& p);
		virtual bool Initialize(const GCPtr<GCObject>& owner, const std::string& id, const StringKeyDictionary& sd);
		virtual void PushSelf(Implementation i);

		virtual bool CanFinalize() const;
	

		static GCPtr<Agent> GetActiveAgent();

	protected:


		Agent(const GCPtr<ClassManager> &classManager, const GCPtr<Class>& objectClass, const GCPtr<Process>& process);
		virtual bool Finalize(GCObject*me);
		virtual void AssureIntegrity(bool vmOnly = false);

	private:

		bool myNeedsComponentIntegrityCheck;
		Classifier myTraceClassifier;

		
	};

	// --------------------------------------------------------------------------						
	// Function:	SetNeedsComponentIntegrityCheck
	// Description:	does what it says on the tin
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	inline void Agent::SetNeedsComponentIntegrityCheck() 
	{ 
		myNeedsComponentIntegrityCheck = true; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetTraceClassifier
	// Description:	does what it says on the tin
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	inline const Classifier& Agent::GetTraceClassifier() const 
	{ 
		return myTraceClassifier; 
	}

}

#endif //AGENT_H
