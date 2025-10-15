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

#ifndef PRIORITY_H
#define PRIORITY_H

#include "../Common/SecureStl.h"
#include "../Common/Debug.h"
#include "../Common/Enums.h"
#include "../Config/GCPtr.h"
#include <string>
#include <algorithm>
#include <vector>


namespace shh {

	class Priority
	{
	public:

		static const int ourBadPriority;


		Priority(int value);
		inline bool operator=(const Priority& other);
		inline bool operator==(const Priority& other) const;
		inline bool operator<(const Priority& other) const;
		inline bool operator>(const Priority& other) const;

		static int GetPriority(const std::string& id);
		static bool SetPriority(const std::string& id, int value);
		static void Freeze();


		static inline int GetshhArc();
		static inline int GetEngine();
		static inline int GetSystem();
		static inline int GetAgent();
		static inline int GetMaster();
		static inline int GetSlave();
		static inline int GetSchema();
		static inline int GetExecute();

	protected:

		int myPriority;

	private:

		typedef std::map<std::string, int> Priorities;
		static bool ourPrioritiesFrozem;
		static Priorities ourPriorities;

		static int ourshhArc;
		static int ourEngine;
		static int ourSystem;
		static int ourAgent;
		static int ourMaster;
		static int ourSlave;
		static int ourSchema;
		static int ourExecute;

		static Priorities InitPriorities();

		
		
	};


	inline bool Priority::operator=(const Priority& other) { myPriority = other.myPriority; }
	inline bool Priority::operator==(const Priority& other) const { return myPriority == other.myPriority; }
	inline bool Priority::operator<(const Priority& other) const { return myPriority < other.myPriority; }
	inline bool Priority::operator>(const Priority& other) const { return myPriority > other.myPriority; }
	inline int Priority::GetshhArc() { return ourshhArc; }
	inline int Priority::GetEngine() { return ourEngine; }
	inline int Priority::GetSystem() { return ourSystem; }
	inline int Priority::GetAgent() { return ourAgent; }
	inline int Priority::GetMaster() { return ourMaster; }
	inline int Priority::GetSlave() { return ourSlave; }
	inline int Priority::GetSchema() { return ourSchema; }
	inline int Priority::GetExecute() { return ourExecute; }

}
#endif
