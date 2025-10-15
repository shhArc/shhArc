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
#include "SoftProcess.h"
#include "Scheduler.h"


namespace shh
{
	IMPLEMENT_MEMORY_MANAGED(Message);

	shhId Message::ourLastId = 0;



	// --------------------------------------------------------------------------						
	// Function:	Call
	// Description:	calls a function specified by the message package
	// Arguments:	needReturnValues, numSenderArgsToIgnore from calling script, 
	//				whether call can time out or yield
	// Returns:		if it ran ok
	// --------------------------------------------------------------------------
	ExecutionState Message::Call(bool needReturnValues, int numSenderArgsToIgnore, bool isYieldable)
	{
		ExecutionState retVal = ExecutionFailed;
		if (myTo.IsValid())
		{
			if (myTo->GetCurrentMessage() == this)
			{
				// resume update
				retVal = myTo->CallMessage(*this, needReturnValues, isYieldable);
			}
			else if (myBuilt || Build(numSenderArgsToIgnore, -1) == BuildOk)
			{
				// new call
				myTo->myCurrentMessage = this;
				retVal = myTo->CallMessage(*this, needReturnValues, isYieldable);
			}
		}

		return retVal;
	}


	// --------------------------------------------------------------------------						
	// Function:	Build
	// Description:	builds a message package 
	//				must be set myTo first as also the message name
	//				Build also adds args from the from messenger to the args list
	// Arguments:	number of args in sending Messenger to ignore 
	//				min number of args the funciton is allowed to accept (no args 
	//				trying to be sent can be more than those accepted, extra are 
	//				ignored) 
	//				minArgumentsAllowed == n -> n is positive 
	//				minArgumentsAllowed == -2 -> the number of sent does not 
	//				need to match those recieved by function
	//				minArgumentsAllowed = -1 -> the number of sent args must be
	//				exactly the same as no recieved. 
	// Returns:		InvalidSendee, InvalidFunction, IncorrectArguments or BuildOk
	// --------------------------------------------------------------------------
	Message::BuildState Message::Build(int numSenderArgsToIgnore, int minArgumentsAllowed)
	{
		if (!myTo.IsValid())
		{
			myState = ExecutionFailed;
			return InvalidSendee;
		}


		if (myFunctionName == SoftProcess::ourBootMessage)
		{
			// if first execute of a script then yield ok as it doesnt call
			// a specific function it is registering code and vars
			myTo->myCurrentMessage = this;
			myState = ExecutionYielded;
			myBuilt = true;
			int i = 1;
			while (myFrom->GetArgument(*this, numSenderArgsToIgnore + i))
			{
				i++;
			}

			return BuildOk;
		}


		// set function of given name
		int argsPrePackaged = (int)myArguments.size();
		int argsExpectedByFunction;
		const void *function = myTo->GetFunction(myFunctionName, argsExpectedByFunction);
		if (!function)
			return InvalidFunction;
		

		// if updater or timer then package args
		if (myFrom.IsValid() && (myCallType != UpdateMsg && myCallType != TimerMsg))
		{
			for (int i = 1; i < argsExpectedByFunction - argsPrePackaged + 1; i++)
				if (!myFrom->GetArgument(*this, numSenderArgsToIgnore + i))
					break;
		}
	
		// check correct number of args
		if ((minArgumentsAllowed != -2) &&
			((minArgumentsAllowed == -1 && argsExpectedByFunction != myArguments.size()) ||	// expects exact number
				(minArgumentsAllowed > argsExpectedByFunction || argsExpectedByFunction > myArguments.size())))
		{
			myState = ExecutionFailed;
			return IncorrectArguments;
		}

		myState = ExecutionOk;
		myBuilt = true;

		return BuildOk;
	}



	// --------------------------------------------------------------------------						
	// Function:	SendMsg
	// Description:	sends this message
	// Arguments:	time delay before message gets received, number of args in 
	//				the sender script to ignore before adding new send args, 
	// Returns:		none
	// --------------------------------------------------------------------------
	bool Message::SendMsg(double delay, int numSenderArgsToIgnore)
	{
		if (myTo.IsValid() && (Messenger::ourMaxMessagesPerUpdate == 0 || myTo->GetNumMessagesSentThisUpdate() <= Messenger::ourMaxMessagesPerUpdate))
		{
			
			DEBUG_ASSERT(myTo->GetScheduler().IsValid());

			// build message if not done already
			if (myFunctionName == SoftProcess::ourBootMessage || myBuilt || Build(numSenderArgsToIgnore, -1) == BuildOk)
			{
				DEBUG_TRACE("\nMessage: %llx sent %s to %llx at %f to arive in %f.\n",
					(myFrom.IsValid() ? myFrom.GetObject() : 0),
					myFunctionName.c_str(),
					myTo.GetObject(),
					myTo->GetScheduler()->GetCurrentUpdateTime(),
					delay);


				// send message to scheduler
				if (myTo->GetScheduler()->RecieveMsg(this, delay))
				{
					if (myFunctionName == SoftProcess::ourBootMessage)
						myTo->myCurrentMessage = this;

					myTo->IncMessagesSentThisUpdate();
					
					if (myCallType == Message::Synchronous && myFrom.IsValid())
						myFrom->myState = ExecutionAwaitingCallback;

					return true;
				}
			}
		}
		DEBUG_TRACE("\nError: Message could not be posted.\n");

		return false;

	}


	// --------------------------------------------------------------------------						
	// Function:	InitiateCallback
	// Description: sends a callback from this message
	// Arguments:	none
	// Returns:		number of return args to add in call back
	// --------------------------------------------------------------------------
	int Message::InitiateCallback()
	{
		int rv = -1;
		if (myCallType == Synchronous)
		{
			// synchrionous happens uimmediately
			if (myFrom.IsValid())
			{
				bool* b = new bool(true);
				Type<bool>::GetStatic()->Push(myFrom->GetImplementation(), b);
				rv = 1;
				if (myFunctionName == SoftProcess::ourInitializeMessage)
				{
					myTo->PushSelf(Lua);
					rv++;
				}
				rv += myFrom->InitiateCallback(*this);;
			}
			else
			{
				bool* b = new bool(false);
				Type<bool>::GetStatic()->Push(myFrom->GetImplementation(), b);
				rv = 1;
			}
			delete this;			
		}
		else if (myCallType == Asynchronous)
		{
			// asynchronous is queued
			GCPtr<Messenger> tmp = myTo;
			myTo = myFrom;
			myFrom = tmp;
			myCallType = Decoupled;
			myFunctionName = myCallbackFunction;
			myCallbackFunction.clear();
			myArguments = myReturnValues;
			myReturnValues.clear();
			if (SendMsg(0.0, 0))
				return (int)myArguments.size();
		}
		return rv;
	}


	// --------------------------------------------------------------------------						
	// Function:	DeleteArguments
	// Description:	delete arguments and deletes memory
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Message::DeleteArguments()
	{
		for (int i = 0; i != myArguments.size(); i++)
			myArguments[i].myType->Destroy(myArguments[i].myValue);

		myArguments.clear();

		for (int i = 0; i != myReturnValues.size(); i++)
			myReturnValues[i].myType->Destroy(myReturnValues[i].myValue);

		myReturnValues.clear();
	}



}
