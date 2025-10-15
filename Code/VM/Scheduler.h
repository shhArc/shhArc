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


#ifndef SCHEDULER_H
#define SCHEDULER_H


#include "../Common/SecureStl.h"
#include "../Config/GCPtr.h"
#include "../Common/Thread.h"
#include "../Common/Mutex.h"
#include "../Arc/Module.h"
#include "../Arc/Module.h"
#include "Message.h"
#include <MemoryManager.h>
#include <vector>
#include <map>
#include <queue>

namespace shh {

	class Process;

	class Scheduler : public Module
	{
		friend class VM;
		friend class Message;
		
	public:

		typedef std::vector< GCPtr<Scheduler> > Array;
		typedef std::map<shhId, VM* > VMs;
		typedef std::vector< GCPtr<Process> > Processes;
		typedef std::pair<double, int> TimePriorityPair;
		typedef std::pair<TimePriorityPair, Message*> MessagePair;

		Privileges myExecutePrivileges;
		static unsigned int ourMaxMessagesPerUpdate;

		static GCPtr<Scheduler> CreateScheduler(unsigned int numWorkers, double maxTimePerUpdate, Privileges privileges = BasicPrivilege);


		virtual ~Scheduler();
		void Zap();

		bool GetBaseProcess(Implementation i, Privileges privileges, GCPtr<Process>& p);
		void SetBaseProcess(Implementation i, const GCPtr<Process>& p);
		
		bool AddVM(const GCPtr<VM>& vm);
		bool RemoveVM(const GCPtr<VM> &vm);
		GCPtr<VM> GetVM(shhId id) const;


		bool AddUpdater(const GCPtr<Module>& updater);
		bool RemoveUpdater(const GCPtr<Module>& updater);
		

		virtual bool Busy() const;
	
		bool Update(double until, unsigned int phase);
		bool GetStopWork() const;
		

		static void SetCurrentProcess(const GCPtr<Process>& p);
		static const GCPtr<Process> GetCurrentProcess();
		static void ClearCurrentProcess();

		static void EraseThread();



		inline static double GetMinDelay();
		inline static void SetMinDelay(double d);
		inline double GetCurrentUpdateTime() const;
		inline double GetLastUpdateTime() const;
		inline void Lock();
		inline void Unlock();



		bool SetTimer(Message* const msg);
		bool StopTimer(shhId id, const GCPtr<Process> &requester);

		void ClearAllMessages();

	protected:

		Scheduler(unsigned int numWorkers, double maxTimePerUpdate, Privileges privileges);
		void DecrementActiveWorkers();
		virtual bool GetNextMessage(double until, unsigned int phase, Message*& msg);
		ExecutionState DispatchMessage(Message* msg);
		bool RecieveMsg(Message* const msg, double recieveTime);
		void ReceiveCallback(Message* const callback);
		




		class MessagePairCompare
		{
		public:
			inline bool operator() (const MessagePair &a, const MessagePair&b) const
			{
				if (a.first == b.first)
					return a.second < b.second;
				else
					return a.first < b.first;
					
				return true;
			}
		};

		class Worker
		{
		public:

			Worker(const GCPtr<Scheduler>& sheduler) : mySheduler(sheduler) {}
			void Main();

		private:

			GCPtr<Scheduler> mySheduler;
		};

		friend Worker;

		typedef std::vector<Thread<Worker>*> Workers;
		typedef std::multimap<std::string, std::string> ComponentConflicts;


		typedef std::pair< GCPtr<GCObject> ,int> SubPriorityPair;
		typedef std::pair<int, SubPriorityPair> UpdaterPair;

		class UpdaterCompare
		{
		public:

			inline bool operator() (const UpdaterPair& a, const UpdaterPair& b) const
			{
				if (a.first == b.first)
				{
					if (a.second.first.GetObject() == b.second.first.GetObject())
						return a.second < b.second;
					else
						return a.second.second < b.second.second;
				}
				else
				{
					return a.first < b.first;
				}
			}
		};

		typedef std::multimap<UpdaterPair, GCPtr<Module>, UpdaterCompare> Updaters;


		typedef std::priority_queue<MessagePair, std::vector<MessagePair>, MessagePairCompare > PendingMessageQueue;
		typedef std::vector<MessagePair> ActiveMessageQueue;
		typedef std::map<shhId, Message*> Timers;
		


		Processes myAgentBaseProcess;
		Processes mySchemaBaseProcess;
		Processes myMasterBaseProcess;
		Processes mySlaveBaseProcess;
		Processes myBasicBaseProcess;

		static Array ourSchedulers;
		static double ourMinDelay;
		double myCurrentUpdateTime;
		double myLastUpdateTime;
		VMs myVMs;

		Privileges myPrivileges;
		bool myBusy;

		static Mutex::Lock ourLock;
		static Mutex* ourMutex;
		static Mutex* ourCurrentVMMutex;
		static Mutex* ourCurrentProcessMutex;
		Workers myWorkers;
		bool myStopWork;
		unsigned int myActiveWorkers;
		Mutex myMutex;

		PendingMessageQueue myPendingMessageQueue;
		ActiveMessageQueue myActiveMessageQueue1;
		ActiveMessageQueue myActiveMessageQueue2;
		ActiveMessageQueue *myCurrentMessageQueue;
		ActiveMessageQueue *myNextMessageQueue;
		Timers myTimers;


		Updaters myUpdaters;
		Updaters::iterator myCurrentUpdater;


		typedef std::map<long, GCPtr<Process> > ProcessThreads;
		static GCPtr<Process> ourCurrentProcess;
		static ProcessThreads ourProcessThreads;

	};


	// --------------------------------------------------------------------------						
	// Function:	GetMinDelay
	// Description:	gets minumum delay allowed when sending messages
	// Arguments:	none
	// Returns:		time
	// --------------------------------------------------------------------------
	inline double Scheduler::GetMinDelay() 
	{ 
		return ourMinDelay; 
	}


	// --------------------------------------------------------------------------						
	// Function:	SetMinDelay
	// Description:	sets minumum delay allowed when sending messages
	// Arguments:	none
	// Returns:		time
	// --------------------------------------------------------------------------
	inline void Scheduler::SetMinDelay(double d) 
	{ 
		if (d > 0) 
			ourMinDelay = d; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetCurrentUpdateTime
	// Description:	return time for currently running update
	// Arguments:	none
	// Returns:		time
	// --------------------------------------------------------------------------
	inline double Scheduler::GetCurrentUpdateTime() const
	{ 
		return myCurrentUpdateTime; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetLastUpdateTime
	// Description:	return last time update was called
	// Arguments:	none
	// Returns:		time
	// --------------------------------------------------------------------------
	inline double Scheduler::GetLastUpdateTime() const
	{ 
		return myLastUpdateTime; 
	}


	// --------------------------------------------------------------------------						
	// Function:	Lock
	// Description:	thread unlocks scheduler
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	inline void Scheduler::Lock() 
	{ 
		myMutex.LockMutex(); 
	}


	// --------------------------------------------------------------------------						
	// Function:	Unlock
	// Description:	thread unlocks scheduler
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	inline void Scheduler::Unlock() 
	{ 
		myMutex.UnlockMutex(); 
	}


}



#endif // Scheduler_H
