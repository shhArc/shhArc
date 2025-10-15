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

#include "Object.h"
#include "Class.h"
#include "ClassManager.h"
#include "Process.h"
#include "Scheduler.h"
#include "VM.h"
#include "SoftProcess.h"
#include <inttypes.h>


namespace shh {

	std::string Object::ourTypeName = "Object";

	// --------------------------------------------------------------------------						
	// Function:	Object
	// Description:	constructor
	// Arguments:	manager this belongs to, class this belongs to, process 
	//				attached to this
	// Returns:		none
	// --------------------------------------------------------------------------
	Object::Object(const GCPtr<ClassManager>& manager, const GCPtr<Class>& objectClass, const GCPtr<Process>& process) :
		myClass(objectClass),
		myClassManager(manager),
		myProcess(process),
		myFinalized(false)
	{
		myImplementation = myClass->GetImplementation();
		myUpdateMessage.myDeletable = false;
		myUpdateMessage.myTo = myProcess;
		myUpdateMessage.myFunctionName = SoftProcess::ourUpdateMessage;
	}

	// --------------------------------------------------------------------------						
	// Function:	Object
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Object::~Object()
	{
		myClass->RemoveObject(GCPtr<Object>(this));
	}


	// --------------------------------------------------------------------------						
	// Function:	CompleteInitialization
	// Description:	call after initialization
	// Arguments:	none
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool Object::CompleteInitialization()
	{
		return myProcess->CompleteInitialization();
	}


	// --------------------------------------------------------------------------						
	// Function:	Initialize
	// Description:	call after construction
	// Arguments:	owner, id for this object instantiation, dict to use for vars
	//				when initializing
	// Returns:		if sucessful
	// --------------------------------------------------------------------------
	bool Object::Initialize(const GCPtr<GCObject>& owner, const std::string& id, const StringKeyDictionary& sd)
	{
		bool ok = Module::Initialize(owner, id, sd);
		myProcess->Initialize();
		const GCPtr<VM>& vm = Scheduler::GetCurrentProcess()->GetVM();
		vm->FlagUninitialized(myProcess);
		return ok;
	}


	// --------------------------------------------------------------------------						
	// Function:	PostInitialization
	// Description:	call after initialization
	// Arguments:	none
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool Object::PostInitialization()
	{
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	Finalize
	// Description:	call to destroy
	// Arguments:	pointer to this
	// Returns:		none
	// --------------------------------------------------------------------------
	bool Object::Finalize(GCObject*me)
	{	
		GCPtr<Object> object(dynamic_cast<Object*>(me));
		if (!myFinalized &&!object.IsDying())
		{
			Module::Finalize(me);

			bool deletedProcess = false;
			if (object->myProcess.IsValid())
				deletedProcess = myProcess->Terminate(GCPtr<Messenger>());
			else
				deletedProcess = true;

			if (deletedProcess)
			{
				if(object.IsValid() && object->GetClass().IsValid())
					object->GetClass()->RemoveObject(GCPtr<Object>(object));
				if (!object.IsDying())
					object.Destroy();
			}

			myFinalized = true;
			return deletedProcess;
		}
		return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetManager
	// Description:	gets manager that owns this
	// Arguments:	none
	// Returns:		manager
	// --------------------------------------------------------------------------
	const GCPtr<ClassManager>& Object::GetManager() const 
	{ 
		return myClassManager; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetClass
	// Description:	get class this belongs to
	// Arguments:	none
	// Returns:		class
	// --------------------------------------------------------------------------
	const GCPtr<Class>& Object::GetClass() const 
	{ 
		return myClass; 
	}

	// --------------------------------------------------------------------------						
	// Function:	GetProcess
	// Description:	get process attached to this object
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	const GCPtr<Process>& Object::GetProcess() const
	{
		return myProcess;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetClassifiers
	// Description:	gets classifiers this object has
	// Arguments:	none
	// Returns:		classifiers
	// --------------------------------------------------------------------------
	Classifier Object::GetClassifiers() const
	{
		return myClass->myClassifiers;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetName
	// Description:	get class name
	// Arguments:	none
	// Returns:		name
	// -------------------------------------------------------------------------
	std::string Object::GetName() const
	{
		return myClass->GetName();
	}

#ifdef _WIN64
#define snprintf _snprintf
#endif

	// --------------------------------------------------------------------------						
	// Function:	ObjectString
	// Description:	object
	// Arguments:	none
	// Returns:		returns a string desciptor and pointer
	// --------------------------------------------------------------------------
	std::string Object::ObjectString(Object* object) const
	{
		char temp[1024];
		if (object)
			snprintf(temp, 1023, "Object %s % " PRIu64, GetName().c_str(), (unsigned long long)object);
		else
			snprintf(temp, 1023, "Object null % " PRIu64, (unsigned long long)object);
		return std::string(temp);
	}


	// --------------------------------------------------------------------------						
	// Function:	PushSelf
	// Description:	virtual function to push this onto current process stack
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Object::PushSelf(Implementation i) 
	{
	}


	// --------------------------------------------------------------------------						
	// Function:	CanFinalize
	// Description:	returns if this can be finalized as may be busy destory members
	// Arguments:	none
	// Returns:		bool
	// --------------------------------------------------------------------------
	bool Object::CanFinalize() const 
	{ 
		return true; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetNextMessage
	// Description:	gets a message to execute (used for updaters)
	// Arguments:	message times up to which messages are handle, phase of message,
	//				message to return
	// Returns:		if sucessful
	// --------------------------------------------------------------------------
	bool Object::GetNextMessage(double until, unsigned int phase, Message*& msg)
	{
		msg = NULL;
		if (myProcess->GetVM()->IsInitialized())
		{
			if (myProcess->GetState() == ExecutionReady)
			{
				myRequiresUpdate = true;
				msg = &myUpdateMessage;
				return true;
			}
			else if (myProcess->GetState() == ExecutionBusy && myProcess->GetCurrentMessage() == &myUpdateMessage)
			{
				myRequiresUpdate = false;
				msg = &myUpdateMessage;
				return true;
			}
		}
		else if (myProcess->GetState() == ExecutionBusy && myProcess->GetCurrentMessage() == &myUpdateMessage)
		{
			myRequiresUpdate = false;
			msg = &myUpdateMessage;
			return true;
		}
		myRequiresUpdate = false;
		return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	Update
	// Description:	updates object 
	// Arguments:	update until time, update phase
	// Returns:		none
	// --------------------------------------------------------------------------
	bool Object::Update(double until, unsigned int phase)
	{
		if (Module::Update(until, phase))
		{
			myUpdateMessage.DeleteArguments();
			myUpdateMessage.AddArgument(new double(myDelta));
			return true;
		}
		return false;
	}
}

