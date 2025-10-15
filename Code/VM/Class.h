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

#ifndef CLASS_H
#define CLASS_H


#include "../Common/SecureStl.h"
#include "../Config/GCPtr.h"
#include "../Arc/Registry.h"
#include "../Common/Classifier.h"	
#include "../Common/Enums.h"
#include "Object.h"
#include <string>
#include <list>
#include <map>


namespace shh {


	class ClassManager;
	class Process;
	class VM;

	class Class : public GCObject
	{
		friend GCPtr< Class >;
		friend class ClassManager;
		friend class Object;
		friend class Agent;

	public:

		typedef std::list<GCPtr<Class> > Classes;
		typedef std::map<std::string, GCPtr<Class> > ClassMap;

	
		typedef std::list< GCPtr<Object> > Objects;
		typedef std::map<GCPtr<ClassManager>, Objects> ManagerObjects;

		Class(const std::string& name, const std::string& typeName, const GCPtr<Process>& process, Registry::ProcessConstructor pc, Registry::ObjectConstructor oc);
		~Class();

		const std::string& GetName() const;
		const std::string& GetTypeName() const;
		Implementation GetImplementation() const;
		const Classifier& GetClassifiers() const;
		void GetObjects(const GCPtr<ClassManager>& manager, Class::Objects& objects) const;

		bool HasFunction(const std::string& functionName) const;

		const StringKeyDictionary& GetMeta() const;

		static const std::string ourMetaFileExtension;
	
	protected:

		Class(const GCPtr<ClassManager>& manager, const GCPtr<VM> &vm, const Registry::ClassSpec& classSpec, const GCPtr<Process> &baseProcess, Registry::ProcessConstructor pc, Registry::ObjectConstructor oc);
		Class(const GCPtr<Class>& other);

		void AddManager(const GCPtr<ClassManager>& manager);
		void RemoveManager(const GCPtr<ClassManager>& manager);
		void FinalizeObjects();

		GCPtr<Object> CreateObject(const GCPtr<ClassManager>& manager);
		void AddObject(const GCPtr<Object>& object);
		void RemoveObject(const GCPtr<Object>& object);



		ManagerObjects myObjects;
		GCPtr<Class> myParent;
		GCPtr<Process> myProcess;
		Registry::ClassSpec mySpec;
		Classifier myClassifiers;
		Registry::ProcessConstructor myProcessConstructor;
		Registry::ObjectConstructor myObjectConstructor;
		ClassMap myDerived; 
		StringKeyDictionary myMeta;

		

	};


}



#endif //AGENTCLASS_H
