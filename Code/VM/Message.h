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

#ifndef MESSAGE_H
#define MESSAGE_H

#include "../Common/SecureStl.h"
#include "../Config/GCPtr.h"
#include "../Arc/Priority.h"
#include "../Arc/Type.h"
#include "../Config/MemoryDefines.h"
#include "Messenger.h"
#include <vector>

namespace shh {

	class Node;
	class BaseType;

	class Message
	{
		DECLARE_MEMORY_MANAGED(Message);

		friend class Scheduler;
		friend class LuaProcess;
		
	public:


		class Argument
		{
			friend class Message;
			friend class LuaProcess;

		public:

			Argument() : myValue(NULL), myType(NULL) { }
			Argument(const Argument& other) { *this = other; }
			Argument(void* value) : myValue(value), myType(NULL) {}

			inline void* GetValue() const { return myValue; }
			inline const BaseType* GetType() const { return myType;  }

			inline const Argument& operator=(const Argument& other)
			{
				myValue = other.myValue;
				myType = other.myType;
				return *this;
			}

		private:

			void* myValue;
			const BaseType* myType;
		};

		typedef std::vector<Argument> Arguments;


		typedef enum { BuildOk, BuildFailed, InvalidSendee, InvalidFunction, IncorrectArguments } BuildState;
		typedef enum { NotSet, Decoupled, Asynchronous, Synchronous, UpdateMsg, TimerMsg } CallType;

		inline Message(bool deletable = true);

		ExecutionState Call(bool needReturnValues = false, int numSenderArgsToIgnore = 0, bool isYieldable = true);
		BuildState Build(int messageHandlingArgs, int minArgumentsAllowed);
		bool SendMsg(double delay, int numSenderArgsToIgnore);
		int InitiateCallback();

		inline bool IsAlive() const;
		inline void Cancel();
		inline CallType GetCallType() const;
		inline bool SetCallType(CallType t);
		inline shhId GetId() const;

		inline bool IsDeletable() const;
		inline const Arguments& GetArguments() const;
		inline const Arguments& GetReturnValues() const;


		inline double GetScheduledTime() const;
		inline double GetReceivedTime() const;
		inline double GetCompletedTime() const;

		template<class T> inline void AddArgument(T* arg);
		void DeleteArguments();

		std::string myFunctionName;
		GCPtr<Messenger> myTo;
		GCPtr<Messenger> myFrom;
		int myPriority;
		ExecutionState myState;
		bool myDestroyOnCompletion;
		std::string myCallbackFunction;
		Message *myCallbackMessage;
		double myRepeatTimer;
		bool myDeletable;

	private:

		shhId myId;
		Arguments myArguments;
		Arguments myReturnValues;
		CallType myCallType;
		bool myBuilt;

		bool myCancelled;

		double myScheduledTime;
		double myReceivedTime;
		double myCompletedTime;

		static shhId ourLastId;
			

	};

	// --------------------------------------------------------------------------						
	// Function:	Message
	// Description:	constructor
	// Arguments:	whether object was malloced so can delete
	// Returns:		none
	// --------------------------------------------------------------------------
	inline Message::Message(bool deletable) :
		myId(0),
		myCallType(NotSet),
		myScheduledTime(0.0),
		myReceivedTime(0.0),
		myCompletedTime(0.0),
		myRepeatTimer(-1),
		myPriority(Priority::GetSlave()),
		myState(ExecutionOk),
		myBuilt(false),
		myDestroyOnCompletion(false),
		myCallbackMessage(NULL),
		myDeletable(deletable),
		myCancelled(false)
	{}


	// --------------------------------------------------------------------------						
	// Function:	IsAlive
	// Description:	test is message is still active
	// Arguments:	none
	// Returns:		nool
	// --------------------------------------------------------------------------
	inline bool Message::IsAlive() const
	{
		return !myCancelled || myTo.IsValid();
	}


	// --------------------------------------------------------------------------						
	// Function:	Cancel
	// Description:	cancels message
	// Arguments:	none
	// Returns:		none
	// -------------------------------------------------------------------------
	inline void Message::Cancel()
	{
		myCancelled = true;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetCallType
	// Description:	does what is says to the tine
	// Arguments:	none
	// Returns:		call type
	// --------------------------------------------------------------------------
	inline Message::CallType Message::GetCallType() const
	{
		return myCallType;
	}


	// --------------------------------------------------------------------------						
	// Function:	SetCallType
	// Description:	sets call type
	// Arguments:	call type
	// Returns:		if set
	// --------------------------------------------------------------------------
	inline bool Message::SetCallType(Message::CallType t)
	{
		if (myCallType == UpdateMsg || myCallType == TimerMsg || t == NotSet)
			return false;

		if (t == TimerMsg)
			myId = ++ourLastId;

		myCallType = t;
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetId
	// Description:	returns id
	// Arguments:	none
	// Returns:		returms id
	// --------------------------------------------------------------------------
	inline shhId Message::GetId() const
	{
		return myId;
	}


	// --------------------------------------------------------------------------						
	// Function:	IsDeletable
	// Description:	returns if deletable
	// Arguments:	none
	// Returns:		bool
	// --------------------------------------------------------------------------
	inline bool Message::IsDeletable() const
	{ 
		return myDeletable; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetArguments
	// Description:	returns argument
	// Arguments:	none
	// Returns:		arguments
	// --------------------------------------------------------------------------
	inline const Message::Arguments& Message::GetArguments() const
	{ 
		return myArguments; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetReturnValues
	// Description:	returns message return args
	// Arguments:	none
	// Returns:		return args
	// --------------------------------------------------------------------------
	inline const Message::Arguments& Message::GetReturnValues() const
	{ 
		return myReturnValues; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetScheduledTime
	// Description:	does what it says on the tin
	// Arguments:	none
	// Returns:		time
	// --------------------------------------------------------------------------
	inline double Message::GetScheduledTime() const
	{ 
		return myScheduledTime; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetReceivedTime
	// Description:	does what it says on the tin
	// Arguments:	none
	// Returns:		time
	// --------------------------------------------------------------------------
	inline double Message::GetReceivedTime() const
	{ 
		return myReceivedTime; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetCompletedTime
	// Description:	does what it says on the tin
	// Arguments:	none
	// Returns:		time
	// --------------------------------------------------------------------------
	inline double Message::GetCompletedTime() const
	{ 
		return myCompletedTime; 
	}


	// --------------------------------------------------------------------------						
	// Function:	AddArgument
	// Description:	does what it says on the tin
	// Arguments:	argument value
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class T> inline void Message::AddArgument(T* arg)
	{
		Argument argument;
		argument.myValue = arg;
		argument.myType = static_cast<BaseType*>(Type<T>::GetStatic());
		myArguments.push_back(argument);

		return;
	}


} 

#endif //MESSAGE_H
