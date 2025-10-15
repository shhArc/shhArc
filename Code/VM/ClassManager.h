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


#ifndef CLASSMANAGER_H
#define CLASSMANAGER_H



#include "../Common/SecureStl.h"
#include "../Common/Enums.h"
#include "../Common/Dictionary.h"
#include "../Config/GCPtr.h"
#include "Class.h"
#include <map>
#include <list>
#include <vector>


namespace shh {

	class Ovject;
	class Class;
	class ClassManager;
	class Environment;

	class ClassManager : public GCObject
	{
		friend class Object;
		friend class Class;
		friend GCPtr< ClassManager >;

	public:

		typedef bool (*ClassesSpecifier)(const std::string&, const std::string&, Registry::ClassSpecs&, bool, bool, const std::string&);



		typedef std::list<GCPtr<Class> > Classes;

		struct CompareImlementation {
			bool operator()(const Implementation a, const Implementation b) const { return (int)a < (int)b; }
		};
		typedef std::map < Implementation, Registry::ObjectInstantiator, CompareImlementation > Instatiators;

		virtual ~ClassManager();

		static GCPtr<ClassManager> CreateManager(Privileges privileges = BasicPrivilege, const std::string& typeName = "Class", GCPtr<Process> baseProcess = GCPtr<Process>(NULL));
		static GCPtr<ClassManager> CreateManager(const GCPtr<ClassManager>& other);

		void AddInstantiator(const Registry::ObjectInstantiator& instantiator);
		bool ScanClasses(Implementation implementation, const std::string& path, bool recurse, bool report, const std::string& classType);
		bool BuildHierachry(const GCPtr<VM>& vm);

		void FinalizeObjects();
		void DestroyClasses();
		bool DestroyClass(const std::string& className);
		bool AddClass(const GCPtr<Class>& cls);
		const GCPtr<Class>& GetClass(const std::string& className) const;

		GCPtr<Object> CreateObject(const std::string& className, const GCPtr<Environment>& env);
		void GetAllObjects(Class::Objects& objects) const;
		void GetObjectsOfClass(const std::string& className, Class::Objects& objects) const;
		void GetObjectsClassified(Classifier& classifier, Class::Objects& objects) const;

		const Privileges& GetPrivileges() const;
		virtual void AssureIntegrity(const GCPtr<Object>& object);

	protected:

	
		ClassManager(Privileges privileges, const std::string& typeName, GCPtr<Process> baseProcess);
		ClassManager(const GCPtr<ClassManager>& other);


		
		typedef std::multimap<std::string, std::string> ComponentConflicts;

		Privileges myPrivileges;
		GCPtr<Process> myBaseProcess;
		std::string myTypeName;
		std::string myCommentToken;
		Class::ClassMap myClasses;
		Instatiators myInstantiators;
		Registry::ClassSpecs myUnorderedSpecs;

		
	};


}



#endif // AGENTMANAGER_H
