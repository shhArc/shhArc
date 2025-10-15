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

#include "../Common/Exception.h"
#include "../Common/Debug.h"
#include "../Common/ThreadSafety.h"
#include "../Common/PreciseTime.h"
#include "Scheduler.h"
#include "VM.h"
#include "Process.h"
#include "SoftProcess.h"
#include <algorithm>


namespace shh {


	Scheduler::Array Scheduler::ourSchedulers;
	double Scheduler::ourMinDelay(0.0001);
	unsigned int Scheduler::ourMaxMessagesPerUpdate = 0;

	Mutex::Lock Scheduler::ourLock;
	Mutex* Scheduler::ourMutex = new Mutex;
	Mutex* Scheduler::ourCurrentVMMutex = new Mutex;

	GCPtr<Process> Scheduler::ourCurrentProcess;
	Scheduler::ProcessThreads Scheduler::ourProcessThreads;
	Mutex* Scheduler::ourCurrentProcessMutex = new Mutex;


	// --------------------------------------------------------------------------						
	// Function:	CompTimePriority
	// Description:	compares a time priority between two messages
	// Arguments:	message a, message b
	// Returns:		if a > b
	// --------------------------------------------------------------------------
	static bool CompTimePriority(const Scheduler::MessagePair& a, const Scheduler::MessagePair& b)
	{
		if (a.first.first == b.first.first)
			return a.first.second > b.first.second;
		else
			return a.first.first > b.first.first;
	}


	// --------------------------------------------------------------------------						
	// Function:	Main
	// Description:	Updates multithreaded workers until scheduler says stop
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Scheduler::Worker::Main()
	{
		Thread<Worker>* thread = reinterpret_cast<Thread<Worker>*>(this);
		while (thread->WaitForMoreWork())
		{
			double until = mySheduler->GetUpdateUntilTime();
			Message* msg = NULL;
			do
			{
				if (mySheduler->GetNextMessage(until, 0, msg))
					ExecutionState state = mySheduler->DispatchMessage(msg);

			} while (msg && !mySheduler->GetStopWork());
			mySheduler->DecrementActiveWorkers();
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	CreateScheduler
	// Description:	creates a scheduler
	// Arguments:	num workers thread, max time spent per update call, privileges
	// Returns:		scheduler
	// --------------------------------------------------------------------------
	GCPtr<Scheduler> Scheduler::CreateScheduler(unsigned int numWorkers, double maxTimePerUpdate, Privileges privileges)
	{
		GCPtr<Scheduler> ptr(new Scheduler(numWorkers, maxTimePerUpdate, privileges));
		return ptr;
	}


	// --------------------------------------------------------------------------						
	// Function:	Scheduler
	// Description:	constructor
	// Arguments:	num workers thread, max time spent per update call, privileges
	// Returns:		none
	// --------------------------------------------------------------------------
	Scheduler::Scheduler(unsigned int numWorkers, double maxTimePerUpdate, Privileges privileges) :
		Module(GCPtr<GCObject>(), maxTimePerUpdate),
		myPrivileges(privileges),
		myCurrentUpdateTime(0.0),
		myLastUpdateTime(0.0),
		myExecutePrivileges(BasicPrivilege),
		myStopWork(false),
		myActiveWorkers(0),
		myBusy(false)
	{
		myRequiresUpdate = true;
		myCurrentMessageQueue = &myActiveMessageQueue1;
		myNextMessageQueue = &myActiveMessageQueue2;

		GCPtr<Scheduler> me = GCPtr<Scheduler>(this);
		ourSchedulers.push_back(me);
		myAgentBaseProcess.resize(TOTAL_IMPLEMENTATIONS);
		mySchemaBaseProcess.resize(TOTAL_IMPLEMENTATIONS);
		myMasterBaseProcess.resize(TOTAL_IMPLEMENTATIONS);
		mySlaveBaseProcess.resize(TOTAL_IMPLEMENTATIONS);
		myBasicBaseProcess.resize(TOTAL_IMPLEMENTATIONS);

		myCurrentUpdater = myUpdaters.end();

#if MULTI_THREADED
		Worker *worker = new Worker(me);
		Thread<Worker>* thread = new Thread<Worker>();
		myWorkers.push_back(thread);
		thread->BeginThread(worker, &Worker::Main);
#endif
	}


	// --------------------------------------------------------------------------						
	// Function:	Scheduler
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Scheduler::~Scheduler()
	{
#if MULTI_THREADED
		Workers::iterator wit = myWorkers.begin();
		while (wit != myWorkers.end())
		{
			delete* wit;
			wit++;
		}
#endif
		VMs::iterator vit = myVMs.begin();
		if (vit != myVMs.end())
		{
			GCPtr<VM> vm(vit->second);
			vm.Destroy();
			vit = myVMs.begin();
		}

		Array::iterator it = std::find(ourSchedulers.begin(), ourSchedulers.end(), GCPtr<Scheduler>(this));
		DEBUG_ASSERT(it != ourSchedulers.end());
		ourSchedulers.erase(it);
	}


	// --------------------------------------------------------------------------						
	// Function:	Zap
	// Description:	destroys all my VMs
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Scheduler::Zap()
	{
		for (VMs::iterator vm = myVMs.begin(); vm != myVMs.end(); vm++)
			vm->second->Zap();

		myVMs.clear();
	}


	// --------------------------------------------------------------------------						
	// Function:	GetBaseProcess
	// Description:	gets base process that all processes that use this scheduler
	//				will be derived from
	// Arguments:	implementation of base process (Lua/Python), 
	//				privilege of process to get, process returned
	// Returns:		if got
	// --------------------------------------------------------------------------
	bool Scheduler::GetBaseProcess(Implementation i, Privileges privileges, GCPtr<Process>& p)
	{
		if (privileges & AgentPrivilege && myAgentBaseProcess[i].IsValid())
		{
			p = myAgentBaseProcess[i];
			return true;
		}
		else if (privileges & SchemaPrivilege && mySchemaBaseProcess[i].IsValid())
		{
			p = mySchemaBaseProcess[i];
			return true;
		}
		else if (privileges & MasterPrivilege && myMasterBaseProcess[i].IsValid())
		{
			p = myMasterBaseProcess[i];
			return true;
		}
		else if (privileges & SlavePrivilege && mySlaveBaseProcess[i].IsValid())
		{
			p = mySlaveBaseProcess[i];
			return true;
		}
		else if (privileges & BasicPrivilege && myBasicBaseProcess[i].IsValid())
		{
			p = myBasicBaseProcess[i];
			return true;
		}
		return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	SetBaseProcess
	// Description:	sets base process that all processes that use this scheduler
	//				will be derived from
	// Arguments:	implementation of base process (Lua/Python), 
	//				process to set
	// Returns:		none
	// --------------------------------------------------------------------------
	void Scheduler::SetBaseProcess(Implementation i, const GCPtr<Process>& p)
	{
		Privileges privileges = p->GetPrivileges();
		if (privileges & AgentPrivilege)
			myAgentBaseProcess[i] = p;
		else if (privileges & SchemaPrivilege)
			mySchemaBaseProcess[i] = p;
		else if(privileges & MasterPrivilege)
			myMasterBaseProcess[i] = p;
		else if (privileges & SlavePrivilege)
			mySlaveBaseProcess[i] = p;
		else if (privileges & BasicPrivilege)
			myBasicBaseProcess[i] = p;
	}


	// --------------------------------------------------------------------------						
	// Function:	AddVM
	// Description:	adds a vm to be managed by this scheduler
	// Arguments:	VM
	// Returns:		if added
	// --------------------------------------------------------------------------
	bool Scheduler::AddVM(const GCPtr<VM>& vm)
	{
		if (vm->GetScheduler().IsValid())
			return false;
		vm->SetScheduler(GCPtr<Scheduler>(this));
		myVMs[vm->GetId()] = vm.GetObject();
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	RemoveVM
	// Description:	removed a VM from being managed by this scheduler
	// Arguments:	VM
	// Returns:		if removed (may not belong)
	// --------------------------------------------------------------------------
	bool Scheduler::RemoveVM(const GCPtr<VM>& vm)
	{
		VMs::iterator it = myVMs.find(vm->GetId());
		if (it == myVMs.end())
			return false;

		myVMs.erase(it);
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetVM
	// Description:	get vm by id
	// Arguments:	id
	// Returns:		vm
	// --------------------------------------------------------------------------
	GCPtr<VM> Scheduler::GetVM(shhId id) const
	{
		VMs::const_iterator it = myVMs.find(id);
		if (it != myVMs.end())
			return GCPtr<VM>(it->second);

		return GCPtr<VM>();
	}


	// --------------------------------------------------------------------------						
	// Function:	AddUpdater
	// Description:	adds an updater to be updated in the update queue
	// Arguments:	updater
	// Returns:		if added
	// --------------------------------------------------------------------------
	bool Scheduler::AddUpdater(const GCPtr<Module>& updater)
	{
		UpdaterPair updaterPair(UpdaterPair(updater->GetPriority(), SubPriorityPair(updater->GetVM(), updater->GetSubPriority())));
		Updaters::iterator uit = myUpdaters.find(updaterPair);

		while (uit != myUpdaters.end() && uit->second != updater && updater->GetVM() == uit->second->GetVM())
			uit++;

		if (uit == myUpdaters.end() || uit->second != updater)
		{
			myUpdaters.insert(std::pair<UpdaterPair, GCPtr<Module>>(updaterPair, updater));
			updater->SetOwner(GCPtr<GCObject>(dynamic_cast<GCObject*>(this)));
			return true;
		}

		return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	RemoveUpdater
	// Description:	removed an updater from being updated in the update queue
	// Arguments:	updater
	// Returns:		if removed
	// --------------------------------------------------------------------------
	bool Scheduler::RemoveUpdater(const GCPtr<Module>& updater)
	{

		if (myCurrentUpdater != myUpdaters.end() && myCurrentUpdater->second == updater)
		{
			Updaters::iterator toDelete = myCurrentUpdater;
			myCurrentUpdater++;
			myUpdaters.erase(toDelete);
			updater->SetOwner(GCPtr<GCObject>());
			return true;
		}

		UpdaterPair updaterPair(UpdaterPair(updater->GetPriority(), SubPriorityPair(updater->GetVM(), updater->GetSubPriority())));
		Updaters::iterator uit = myUpdaters.find(updaterPair);
		if (uit != myUpdaters.end())
		{
			while (uit != myUpdaters.end() && uit->second != updater && updater->GetVM() == uit->second->GetVM())
				uit++;

			if (uit == myUpdaters.end())
				return false;

			if (uit == myCurrentUpdater)
				myCurrentUpdater++;

			updater->SetOwner(GCPtr<GCObject>());
			myUpdaters.erase(uit);
			if (myUpdaters.empty())
				myCurrentUpdater = myUpdaters.end();
			return true;
		}
		return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	Busy
	// Description:	returns of scheduler is currently updating
	// Arguments:	none
	// Returns:		bool
	// --------------------------------------------------------------------------
	bool Scheduler::Busy() const
	{
		return myBusy;
	}


	// --------------------------------------------------------------------------						
	// Function:	Update
	// Description:	update shceduler
	// Arguments:	time to update until, phase of update
	// Returns:		none
	// --------------------------------------------------------------------------
	bool Scheduler::Update(double until, unsigned int phase)
	{
		myBusy = true;
		if(until >= 0.0)	
			Module::Update(until, phase);

		double start = GetPreciseTime();
		myLastUpdateTime = myCurrentUpdateTime;

		// added pending messages up to the time until to the currently active message queue
		Lock();
		while (!myPendingMessageQueue.empty() && (myUpdateUntilTime < 0.0 || myPendingMessageQueue.top().first.first <= myUpdateUntilTime))
		{
			myCurrentUpdateTime = myPendingMessageQueue.top().first.first;
			Message* msg = (Message*)myPendingMessageQueue.top().second;

			if (msg->IsAlive())
			{
				msg->myTo->SetReady();
				myCurrentMessageQueue->push_back(myPendingMessageQueue.top());
			}
			myPendingMessageQueue.pop();
		}
		Unlock();
		

	
#if MULTI_THREADED
		// multithreaded set workers off until i timeout
		Workers::iterator wit = myWorkers.begin();
		while (wit != myWorkers.end())
		{
			LOCKINCREMENT(myActiveWorkers);
			(*wit)->SignalNewWork();
			wit++;
		}
		LOCKEXCHANGE(myStopWork, false);
		double now = GetPreciseTime();
		while (now - start <= myTimeOut && myActiveWorkers > 0)
		{
			SLEEP(0);
			now = GetPreciseTime();
		}
		LOCKEXCHANGE(myStopWork, true);
		while (myActiveWorkers > 0)
		{
			SLEEP(0);
		}
#else
		// single threaded just get the next message
		Message* msg = NULL;
		do { 
			double now = GetPreciseTime();
			if (now - start > myTimeOut)
				break;
			if (GetNextMessage(myUpdateUntilTime, 0, msg))
				ExecutionState state = DispatchMessage(msg);

		} while (msg);
#endif
		
		Lock();


		// added messages added to next up date to the current queue as current queue will be upadted on next call
#if PRESERVE_ACTIVE_MESSAGE_PRIORITY
		// this method preserves time and priority whilst not caring about a build up of unhandlable messages at begining of active queue
		myCurrentMessageQueue->insert(myCurrentMessageQueue->end(), myNextMessageQueue->begin(), myNextMessageQueue->end());
		myNextMessageQueue->clear();

#else
		// this method cares about not having build up of unhandlable mssages ay begining og active queue whilst not preserving time and priority 
		myNextMessageQueue->insert(myNextMessageQueue->end(), myCurrentMessageQueue->begin(), myCurrentMessageQueue->end());
		myCurrentMessageQueue->clear();
		ActiveMessageQueue* swap = myNextMessageQueue;
		myNextMessageQueue = myCurrentMessageQueue;
		myCurrentMessageQueue = swap;
#endif
		Unlock();

		myCurrentUpdateTime = until;
		myBusy = false;
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetStopWork
	// Description:	returned if stopped work flag set
	// Arguments:	none
	// Returns:		bool
	// --------------------------------------------------------------------------
	bool Scheduler::GetStopWork() const 
	{ 
		return myStopWork; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetNextMessage
	// Description:	get next message to dispatch
	// Arguments:	max time of message, phase of update, message got
	// Returns:		if got a message
	// --------------------------------------------------------------------------
	bool Scheduler::GetNextMessage(double until, unsigned int phase, Message *&msg)
	{
		LOCK_MUTEX(ourMutex);
		myUpdateUntilTime = until;

		if(until > 0.0)
		{

			// update processes with an update function first
			while(!myUpdaters.empty() && myCurrentUpdater != myUpdaters.end())
			{
				GCPtr<Module> updater = myCurrentUpdater->second;
				myCurrentUpdater++;
				if (updater->GetNextMessage(myUpdateUntilTime, phase, msg))
				{
					if (msg->myTo.IsValid())
					{
						ExecutionState state = msg->myTo->GetState();
						if (state == ExecutionBusy && msg->myState == ExecutionYielded && updater->GetImplementation() != Engine)
						{
							UNLOCK_MUTEX(ourMutex);
							return true;
						}
						else if (state == ExecutionReady && !msg->myTo->Busy())
						{
							// ready for a freah update
							// call the updaters hard update function
							if (updater->Update(myUpdateUntilTime, 0))
								updater->FlagUpdateCompleted();

							if (updater->GetImplementation() != Engine)
							{
								// send the soft update function to be handled
								UNLOCK_MUTEX(ourMutex);
								return true;
							}
						}

						if ((state == ExecutionReady && !msg->myTo->Busy()) ||
							((state == ExecutionBusy || state == ExecutionReceivingCallback) && msg->myTo->GetCurrentMessage() == msg))
						{
							// ready to recieve this messaage or continue processing it
							if (updater->GetImplementation() != Engine)
							{
								UNLOCK_MUTEX(ourMutex);
								return true;
							}
						}
					}
				}
			}	
		}

		Lock();

		if (myCurrentMessageQueue->empty())
		{
			if (myCurrentUpdater == myUpdaters.end())
				myCurrentUpdater = myUpdaters.begin();
			UNLOCK_MUTEX(ourMutex);
			msg = NULL;
			return false;
		}


		// get next message from the message queue
		ActiveMessageQueue::iterator next = myCurrentMessageQueue->begin();
		while (next != myCurrentMessageQueue->end())
		{
			msg = next->second;
			if (msg->IsAlive() && msg->myTo.IsValid())
			{
				ExecutionState state = msg->myTo.IsValid() ? msg->myTo->GetState() : ExecutionError;
				if (state != ExecutionError && msg->myFunctionName == SoftProcess::ourFinalizeMessage && !msg->myTo->CanFinalize())
				{
					// waiting to finalize but not ready to do so yet (may be destroying subprocesses
					next++;
				}
				else if ((state == ExecutionReady && !msg->myTo->Busy()) ||
					((state == ExecutionBusy || state == ExecutionReceivingCallback) && msg->myTo->GetCurrentMessage() == msg))
				{
					// ready to recieve this messaage or continue processing it
					myCurrentMessageQueue->erase(next);
					Unlock();
					UNLOCK_MUTEX(ourMutex);
					return true;
				}
				else if (state == ExecutionYielded || state == ExecutionTimedOut)
				{
					// handing another message and suspended this tick
					unsigned int d = (unsigned int)std::distance(myCurrentMessageQueue->begin(), next);
					myCurrentMessageQueue->erase(next);
					next = myCurrentMessageQueue->begin();
					std::advance(next, d);

					MessagePair active;
					active.first.first = msg->myScheduledTime;
					active.first.second = msg->myPriority;
					active.second = msg;
					msg->myTo->SetReady();
					myNextMessageQueue->push_back(active);
				}
				else
				{
					next++;
				}
			}
			else
			{
				unsigned int d = (unsigned int)std::distance(myCurrentMessageQueue->begin(), next);
				myCurrentMessageQueue->erase(next);
				if(msg->myFrom.IsValid())
				{
					if (msg->GetCallType() == Message::Synchronous)
					{
						// do synchronous callback now
						msg->myFrom->myState = ExecutionBusy;
						msg->myFrom->myCurrentMessage->myCallbackMessage = NULL;
						msg = msg->myFrom->myCurrentMessage;
						UNLOCK_MUTEX(ourMutex);
						Unlock();
						return true;
					}
				}
				next = myCurrentMessageQueue->begin();
				std::advance(next, d);
			}
			msg = NULL;
		}
		Unlock();


		if (myCurrentUpdater == myUpdaters.end())
			myCurrentUpdater = myUpdaters.begin();
		UNLOCK_MUTEX(ourMutex);
		msg = NULL;
		return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	DispatchMessage
	// Description:	sends or continues a message
	// Arguments:	message
	// Returns:		result of handling
	// --------------------------------------------------------------------------
	ExecutionState Scheduler::DispatchMessage(Message* msg)
	{
		ExecutionState state = ExecutionOk;
		
		GCPtr<Messenger> from = msg->myFrom;

		if (msg->myTo.IsValid())
		{
			// call message
			msg->myReceivedTime = myCurrentUpdateTime;
			Message::CallType ct = msg->GetCallType();
			state = msg->Call(ct != Message::Decoupled && ct != Message::UpdateMsg && ct != Message::TimerMsg);
		}
		LOCK_MUTEX(ourMutex);
		if (state == ExecutionYielded || state == ExecutionTimedOut) 
		{
			if (msg->myFunctionName == SoftProcess::ourUpdateMessage)
			{
				// yielded  a timer
				msg->myTo->myState = ExecutionBusy;
			}
			else
			{
				// yielded so added message to queue to be handled on next update
				msg->myTo->myCurrentMessage = msg;
				MessagePair mp;
				mp.first.first = msg->myScheduledTime;
				mp.first.second = msg->myPriority;
				mp.second = msg;
				msg->myTo->SetReady();
				myNextMessageQueue->push_back(mp);
			}
		}
		else if (state == ExecutionCompleted || state == ExecutionFailed || state == ExecutionError)
		{
			// end of message for one reason or another
			msg->myCompletedTime = myCurrentUpdateTime;
			if (msg->myTo.IsValid())
			{
				msg->myTo->myState = ExecutionReady;

				if (msg->myFunctionName == SoftProcess::ourBootMessage || msg->myFunctionName == SoftProcess::ourInitializeMessage)
				{
					// fist call to script compile complete (registered funcs and vars)
					GCPtr<Process> process;
					process.DynamicCast(msg->myTo);
					if (process.IsValid() && process->GetObject().IsValid())
							process->GetObject()->CompleteInitialization();
					else
						msg->myTo->CompleteInitialization();

					if (msg->myFunctionName == SoftProcess::ourBootMessage)
					{
						// normals script calls shhMain now
						msg->myFunctionName = SoftProcess::ourMainMessage;
						if (msg->SendMsg(0.0, 1))
						{
							state = ExecutionScheduled;
							msg->myState = state;
						}
					}
				}
				else if (msg->GetCallType() == Message::TimerMsg && state == ExecutionCompleted && msg->IsAlive())
				{
					// timer function completed
					if (msg->myRepeatTimer == 0)
					{
						// repeat timer on next update
						MessagePair mp;
						mp.first.first = myCurrentUpdateTime;
						mp.first.second = msg->myPriority;
						mp.second = msg;
						msg->myTo->SetReady();
						myNextMessageQueue->push_back(mp);
						state = ExecutionScheduled;
						msg->myState = state;
					}
					else
					{
						// repeat timer with delay
						MessagePair mp;
						mp.first.first = msg->myRepeatTimer + myCurrentUpdateTime;
						mp.first.second = msg->myPriority;
						mp.second = msg;
						Lock();
						myPendingMessageQueue.push(mp);
						Unlock();
						state = ExecutionScheduled;
						msg->myState = state;
					}
				}

				if (state != ExecutionScheduled)
				{
					bool destroy = msg->myDestroyOnCompletion;
					GCPtr<Messenger> toDestroy = msg->myTo;

					if ( (state == ExecutionCompleted || state == ExecutionFailed || state == ExecutionError) && msg->myFrom.IsValid() && msg->myTo.IsValid())
					{
						// finsied message call for one reason or another
						if (msg->GetCallType() == Message::Synchronous)
						{
							// add immediate callback
							if (msg->myFrom->myCurrentMessage != NULL)
							{
								msg->myTo->myState = ExecutionOk;
								msg->myFrom->myState = ExecutionReceivingCallback;
								if (state == ExecutionCompleted)
								{
									msg->myFrom->myCurrentMessage->myCallbackMessage = msg;
									state = ExecutionScheduled;
								}
								else
								{
									msg->myFrom->myCurrentMessage->myCallbackMessage = (Message*)-1;
								}
					
								msg->myTo->SetReady();
								
								// tell scheduler handling callback to start work
								Message* callback = msg->myFrom->myCurrentMessage;
								GCPtr<Scheduler> targetScheduler = msg->myFrom->GetScheduler();
								targetScheduler->ReceiveCallback(callback);
								if (targetScheduler.GetObject() == this)
								{
									Workers::iterator wit = myWorkers.begin();
									while (wit != myWorkers.end())
									{
										(*wit)->SignalNewWork();
										wit++;
									}
								}
							}
							else
							{
								state = ExecutionCompleted;
							}
						}

						if (state == ExecutionCompleted && msg->GetCallType() == Message::Asynchronous)
						{
							
							if (msg->InitiateCallback() >= 0)
							{
								// asynchronous callvack tell scheduler handling callback to start work
								msg->myTo->SetReady();
								state = ExecutionScheduled;
								GCPtr<Scheduler> targetScheduler = msg->myFrom->GetScheduler();
								if (targetScheduler.GetObject() == this)
								{
									Workers::iterator wit = myWorkers.begin();
									while (wit != myWorkers.end())
									{
										(*wit)->SignalNewWork();
										wit++;
									}
								}
							}
						}
					}

					if (destroy)
						toDestroy->Terminate(from);
				}

				if(state != ExecutionScheduled && msg->IsDeletable())
					delete msg;
					
			}
		}

		UNLOCK_MUTEX(ourMutex);
		return state;
	}


	// --------------------------------------------------------------------------						
	// Function:	DecrementActiveWorkers
	// Description:	sets number of active workers as one less
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Scheduler::DecrementActiveWorkers()
	{
		LOCKDECREMENT(myActiveWorkers);
	}


	// --------------------------------------------------------------------------						
	// Function:	SetTimer
	// Description:	sets a timer message
	// Arguments:	message
	// Returns:		if set
	// --------------------------------------------------------------------------
	bool Scheduler::SetTimer(Message* const msg)
	{	
		shhId id = msg->GetId();
		Timers::iterator it = myTimers.find(id);
		if (it == myTimers.end())
		{
			MessagePair mp;
			mp.first.first = myCurrentUpdateTime;
			mp.first.second = msg->myPriority;
			mp.second = msg;
			Lock();
			myPendingMessageQueue.push(mp);
			Unlock();
			myTimers[id] = msg;
			return true;
		}
		return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	StopTimer
	// Description:	stops a timer message
	// Arguments:	id of message, process calling stop
	// Returns:		if stopped
	// --------------------------------------------------------------------------
	bool Scheduler::StopTimer(shhId id, const GCPtr<Process>& requester)
	{
		Timers::iterator it = myTimers.find(id);
		if (it != myTimers.end())
		{
			Message* msg = it->second;
			if (msg->myTo == requester || msg->myFrom == requester)
			{
				msg->Cancel();
				myTimers.erase(it);
				return true;
			}
		}
		return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	RecieveMsg
	// Description:	recieve a sent message
	// Arguments:	message, receive time
	// Returns:		if logged
	// --------------------------------------------------------------------------
	bool Scheduler::RecieveMsg(Message* const msg, double recieveTime)
	{
		if(msg->GetCallType() == Message::TimerMsg)
			myTimers[msg->GetId()] = msg;

		if (recieveTime == 0.0)
		{
			// handle message immediately
			msg->myState = ExecutionScheduled;
			msg->myScheduledTime = myCurrentUpdateTime;
			MessagePair mp;
			mp.first.first = msg->myScheduledTime;
			mp.first.second = msg->myPriority;
			mp.second = msg;
			Lock();
			myCurrentMessageQueue->insert(myCurrentMessageQueue->end(), mp);
			Unlock();
		}
		else
		{
			// stop deadlock calling by using min delay
			double time = (recieveTime < GetCurrentUpdateTime() + GetMinDelay() ?
				GetCurrentUpdateTime() + GetMinDelay() : recieveTime);

			msg->myState = ExecutionScheduled;
			msg->myScheduledTime = time;
			MessagePair mp;
			mp.first.first = msg->myScheduledTime;
			mp.first.second = msg->myPriority;
			mp.second = msg;
			Lock();
			myPendingMessageQueue.push(mp);
			Unlock();
		}
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	ClearCurrentProcess
	// Description:	thread safe function for clearing currently active
	//				process for a thread
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Scheduler::ClearCurrentProcess()
	{
#if MULTI_THREADED
		LOCK_MUTEX(ourCurrentProcessMutex);
		ourProcessThreads.clear();
		UNLOCK_MUTEX(ourCurrentProcessMutex);
#else
		ourCurrentProcess.SetNull();
#endif
	}


	// --------------------------------------------------------------------------						
	// Function:	SetCurrentProcess
	// Description:	thread safe function for setting currently active
	//				process for a thread
	// Arguments:	process
	// Returns:		none
	// --------------------------------------------------------------------------
	void Scheduler::SetCurrentProcess(const GCPtr<Process>& p)
	{
#if MULTI_THREADED
		long threadId = GETTHREADID();
		LOCK_MUTEX(ourCurrentProcessMutex);
		ourProcessThreads[threadId] = p;
		UNLOCK_MUTEX(ourCurrentProcessMutex);
#else
		ourCurrentProcess = p;
#endif
	}


	// --------------------------------------------------------------------------						
	// Function:	GetCurrentProcess
	// Description:	thread safe function for getting currently active
	//				process for a thread
	// Arguments:	none
	// Returns:		process
	// --------------------------------------------------------------------------
	const GCPtr<Process> Scheduler::GetCurrentProcess()
	{
#if MULTI_THREADED
		long threadId = GETTHREADID();
		LOCK_MUTEX(ourCurrentProcessMutex);
		ProcessThreads::iterator it = ourProcessThreads.find(threadId);
		GCPtr<Process> p;
		if (it != ourProcessThreads.end())
			p = it->second;

		UNLOCK_MUTEX(ourCurrentProcessMutex);
		return p;
#else
		return ourCurrentProcess;
#endif
	}


	// --------------------------------------------------------------------------						
	// Function:	EraseThread
	// Description:	deletes worker thread that call this
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Scheduler::EraseThread()
	{
#if MULTI_THREADED

		long threadId = GETTHREADID();
		LOCK_MUTEX(ourCurrentProcessMutex);
		ProcessThreads::iterator pit = ourProcessThreads.find(threadId);
		if (pit != ourProcessThreads.end())
			ourProcessThreads.erase(pit);
		UNLOCK_MUTEX(ourCurrentProcessMutex);

#endif
	}


	// --------------------------------------------------------------------------						
	// Function:	ReceiveCallback
	// Description:	recieved a call back from another message
	// Arguments:	call back message
	// Returns:		none
	// --------------------------------------------------------------------------
	void Scheduler::ReceiveCallback(Message * const callback)
	{
		MessagePair mp;
		mp.first.first = myCurrentUpdateTime;
		mp.first.second = callback->myPriority;
		mp.second = callback;	
		Lock();
		myCurrentMessageQueue->push_back(mp);
		Unlock();
		Workers::iterator wit = myWorkers.begin();
		while (wit != myWorkers.end())
		{
			(*wit)->SignalNewWork();
			wit++;
		}
	}



	// --------------------------------------------------------------------------						
	// Function:	ClearAllMessages
	// Description:	clears all messages
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Scheduler::ClearAllMessages()
	{
		while (!myPendingMessageQueue.empty())
		{
			Message* msg = (Message*)myPendingMessageQueue.top().second;
			if (msg->IsDeletable())
				delete msg;
			myPendingMessageQueue.pop();
		}

		ActiveMessageQueue::iterator next = myNextMessageQueue->begin();
		while (next != myNextMessageQueue->end())
		{
			Message* msg = next->second;
			if (msg->IsDeletable())
				delete msg;

			next++;
		}
		myNextMessageQueue->clear();


		next = myCurrentMessageQueue->begin();
		while (next != myCurrentMessageQueue->end())
		{
			Message* msg = next->second;
			if (msg->IsDeletable())
				delete msg;

			next++;
		}
		myCurrentMessageQueue->clear();

	}

}
