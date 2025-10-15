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
#pragma warning(disable:4786 4503)
#endif

#include "../Common/Debug.h"
#include "Messenger.h"
#include "Object.h"
#include "Process.h"
#include "VM.h"



namespace shh
{
	unsigned int Messenger::ourMaxMessagesPerUpdate = 0;
	shhId Messenger::ourLastId = 0;

	// --------------------------------------------------------------------------						
	// Function:	Messenger
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Messenger::Messenger() :
		myId(++ourLastId), 
		myCurrentMessage(NULL), 
		myNumMessagesSentThisUpdate(0), 
		myInitializing(false), 
		myInitialized(false), 
		myFinalizing(false), 
		myFinalized(false) 
	{}



	// --------------------------------------------------------------------------						
	// Function:	Messenger
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Messenger::~Messenger() 
	{}


	// --------------------------------------------------------------------------						
	// Function:	CompleteInitialization
	// Description:	call when initialization is complete
	// Arguments:	none
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool Messenger::CompleteInitialization()
	{
		myInitializing = false;
		myInitialized = true;
		return myInitialized;
	}


	// --------------------------------------------------------------------------						
	// Function:	PostInitialization
	// Description:	call after intialization
	// Arguments:	none
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool Messenger::PostInitialization() 
	{ 
		return true; 
	}


	// --------------------------------------------------------------------------						
	// Function:	CanFinalize
	// Description:	returns if able to finalize at present time
	// Arguments:	none
	// Returns:		returns if able to finalize at present time
	// --------------------------------------------------------------------------
	bool Messenger::CanFinalize() const 
	{ 
		return true; 
	}


	// --------------------------------------------------------------------------						
	// Function:	Finalize
	// Description:	finalizes
	// Arguments:	pointer to self
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool Messenger::Finalize(GCObject* gc)
	{ 
		return Terminate(GCPtr<Messenger>());
	}


	// --------------------------------------------------------------------------						
	// Function:	Terminate
	// Description:	finalizes and destroys
	// Arguments:	caller
	// Returns:		none
	// --------------------------------------------------------------------------
	bool Messenger::Terminate(const GCPtr<Messenger>& caller)
	{
		myFinalizing = true;
		return CompleteFinalization();
	}


	// --------------------------------------------------------------------------						
	// Function:	CompleteFinalization
	// Description:	final stage of finalizaatiom destroys this
	// Arguments:	none
	// Returns:		of successful
	// --------------------------------------------------------------------------
	bool Messenger::CompleteFinalization()
	{
		if (myFinalized)
			return true;
		myFinalizing = true;
		myFinalized = true;
		GCPtr<Messenger> me(this);
		me.Destroy();
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetArgument
	// Description:	gets arguments from messenger derived process
	// Arguments:	msg to add arg to, arg number on stack
	// Returns:		none
	// --------------------------------------------------------------------------
	bool Messenger::GetArgument(Message& msg, unsigned int arg) 
	{ 
		return false; 
	}


	// --------------------------------------------------------------------------						
	// Function:	SetReady
	// Description:	flags as ready to receive messages
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Messenger::SetReady() 
	{ 
		myState = ExecutionReady; 
		myNumMessagesSentThisUpdate = 0; 
	}


	// --------------------------------------------------------------------------						
	// Function:	SetCurrentMessage
	// Description:	set the currently handling message
	// Arguments:	message
	// Returns:		if set
	// --------------------------------------------------------------------------
	bool Messenger::SetCurrentMessage(Message* msg)
	{
		if (myState != ExecutionOk || myState != ExecutionReady || myState != ExecutionCompleted)
		{
			myState = ExecutionBusy;
			myCurrentMessage = msg;
			return true;
		}
		return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	PushSelf
	// Description:	pushes self as an object on to current process 
	//				stack
	// Arguments:	implemnation language pushing to
	// Returns:		none
	// --------------------------------------------------------------------------
	void Messenger::PushSelf(Implementation i) 
	{
		GCPtr<Messenger> me(this);
		GCPtr<Object> object;
		object.DynamicCast(me);
		if (object.IsValid())
		{
			object->PushSelf(i);
		}
		else
		{
			GCPtr<Process> process;
			process.DynamicCast(me);
			if (process.IsValid())
			{
				GCPtr<VM> vm = process->GetVM();
				object.DynamicCast(vm);
				if (object.IsValid())
				{
					object->PushSelf(i);
				}
			}


		}
	}


}
