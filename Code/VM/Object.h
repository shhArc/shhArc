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

#ifndef OBJECT_H
#define OBJECT_H


#include "../Common/SecureStl.h"
#include "../Common/Debug.h"
#include "../Common/Enums.h"
#include "../Config/GCPtr.h"
#include "../Arc/Module.h"
#include "Message.h"
#include <string>

namespace shh {


	class Process;
	class Object;
	class Class;
	class ClassManager;
	class Messenger;


	class Object : public Module
	{

		friend class ClassManager;
		friend class Class;
		template<class O, class OM> friend class ObjectModule;


	public:

		virtual ~Object();

		const GCPtr<ClassManager>& GetManager() const;
		const GCPtr<Class>& GetClass() const;
		const GCPtr<Process> &GetProcess() const;

		virtual std::string GetName() const;
		Classifier GetClassifiers() const;
		std::string ObjectString(Object* object) const;

		
		virtual void PushSelf(Implementation i);
		virtual bool CanFinalize() const;
		virtual bool Initialize(const GCPtr<GCObject>& owner, const std::string& id, const StringKeyDictionary& sd);
		virtual bool CompleteInitialization();
		virtual bool PostInitialization();
		virtual bool GetNextMessage(double until, unsigned int phase, Message*& msg);
		virtual	bool Update(double until, unsigned int phase);

		static void PushMessenger(Implementation i, const GCPtr<Messenger>& m);

	protected:

		static std::string ourTypeName;
		GCPtr<ClassManager> myClassManager;
		GCPtr<Class> myClass;
		GCPtr<Process> myProcess;
		Message myUpdateMessage;
		bool myFinalized;

		Object(const GCPtr<ClassManager>& manager, const GCPtr<Class>& objectClass, const GCPtr<Process> &process);
		virtual bool Finalize(GCObject*gc);
	
		
	};

}

#endif 
