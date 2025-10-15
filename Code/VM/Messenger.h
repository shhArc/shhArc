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

#ifndef MESSENGER_H
#define MESSENGER_H


#include "../Common/SecureStl.h"
#include "../Config/GCPtr.h"
#include "../Arc/Module.h"
#include <vector>

namespace shh {

	class Message;
	class VM;

	class Messenger : public GCObject
	{
		friend class Message;
		friend class VM;
		friend class Scheduler;
		template<class O, class OM> friend class ObjectModule;

		
	public:
	
		Messenger();
		virtual ~Messenger();

		virtual bool CompleteInitialization();
		virtual bool PostInitialization();
		virtual bool CanFinalize() const;
		virtual bool Finalize(GCObject* gc);
		virtual bool Terminate(const GCPtr<Messenger>& caller);
		virtual bool CompleteFinalization();
		virtual bool Busy() const = 0;


		inline Implementation GetImplementation() const;
		inline ExecutionState GetState() const;

		virtual bool GetArgument(Message& msg, unsigned int arg);
		virtual void SetReady();
		bool SetCurrentMessage(Message* msg);

		inline Message* GetCurrentMessage() const;
		inline shhId GetId() const;
		inline unsigned int GetNumMessagesSentThisUpdate() const;
		inline void IncMessagesSentThisUpdate();

		virtual const GCPtr<VM> &GetVM() const = 0;
		virtual const GCPtr<Scheduler>& GetScheduler() const = 0;

		inline bool IsInitializing() const;
		inline bool IsInitialized() const;
		inline bool IsFinalizing() const;
		inline bool IsFinalized() const;

		
		virtual void PushSelf(Implementation i);

		static unsigned int ourMaxMessagesPerUpdate;


	protected:

		Implementation myImplementation;
		ExecutionState myState;
		shhId myId;
		unsigned int myNumMessagesSentThisUpdate;
		Message* myCurrentMessage;

		bool myInitializing;
		bool myInitialized;
		bool myFinalizing;
		bool myFinalized;

		virtual void SetVM(const GCPtr<VM>& vm) = 0;
		virtual const void* GetFunction(const std::string& functionName, int& argsExpected) { return NULL; };
		virtual ExecutionState CallMessage(Message& msg, bool needReturnValues, bool isYieldable = true) { return ExecutionFailed; }
		virtual int InitiateCallback(Message& msg) { return 0; }

	private:

		static shhId ourLastId;
	};



	// --------------------------------------------------------------------------						
	// Function:	GetImplementation
	// Description:	does what it says on the tin
	// Arguments:	none
	// Returns:		does what it says on the tin
	// --------------------------------------------------------------------------
	inline Implementation Messenger::GetImplementation() const 
	{ 
		return myImplementation; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetState
	// Description:	does what it says on the tin
	// Arguments:	none
	// Returns:		does what it says on the tin
	// --------------------------------------------------------------------------
	inline ExecutionState Messenger::GetState() const 
	{ 
		return myState; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetCurrentMessage
	// Description:	does what it says on the tin
	// Arguments:	none
	// Returns:		does what it says on the tin
	// --------------------------------------------------------------------------
	inline Message* Messenger::GetCurrentMessage() const
	{ 
		return myCurrentMessage; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetId
	// Description:	does what it says on the tin
	// Arguments:	none
	// Returns:		does what it says on the tin
	// --------------------------------------------------------------------------
	inline shhId Messenger::GetId() const
	{ 
		return myId; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetNumMessagesSentThisUpdate
	// Description:	does what it says on the tin
	// Arguments:	none
	// Returns:		does what it says on the tin
	// --------------------------------------------------------------------------
	inline unsigned int Messenger::GetNumMessagesSentThisUpdate() const 
	{ 
		return myNumMessagesSentThisUpdate; 
	
	}


	// --------------------------------------------------------------------------						
	// Function:	IncMessagesSentThisUpdate
	// Description:	does what it says on the tin
	// Arguments:	none
	// Returns:		does what it says on the tin
	// --------------------------------------------------------------------------
	inline void Messenger::IncMessagesSentThisUpdate() 
	{ 
		myNumMessagesSentThisUpdate++; 
	}


	// --------------------------------------------------------------------------						
	// Function:	IsInitializing
	// Description:	does what it says on the tin
	// Arguments:	none
	// Returns:		does what it says on the tin
	// --------------------------------------------------------------------------
	inline bool Messenger::IsInitializing() const
	{ 
		return myInitializing; 
	}


	// --------------------------------------------------------------------------						
	// Function:	IsInitialized
	// Description:	does what it says on the tin
	// Arguments:	none
	// Returns:		does what it says on the tin
	// --------------------------------------------------------------------------
	inline bool Messenger::IsInitialized() const
	{ 
		return myInitialized; 
	}


	// --------------------------------------------------------------------------						
	// Function:	IsFinalizing
	// Description:	does what it says on the tin
	// Arguments:	none
	// Returns:		does what it says on the tin
	// --------------------------------------------------------------------------
	inline bool Messenger::IsFinalizing() const
	{ 
		return myFinalizing; 
	}


	// --------------------------------------------------------------------------						
	// Function:	IsFinalized
	// Description:	does what it says on the tin
	// Arguments:	none
	// Returns:		does what it says on the tin
	// --------------------------------------------------------------------------
	inline bool Messenger::IsFinalized() const
	{ 
		return myFinalized; 
	}

} 

#endif //MESSENGER_H
