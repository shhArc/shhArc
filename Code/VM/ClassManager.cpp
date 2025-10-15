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
#include "ClassManager.h"
#include "Class.h"
#include "Object.h"
#include "Process.h"
#include "VM.h"




namespace shh {


	// --------------------------------------------------------------------------						
	// Function:	CreateManager
	// Description:	constructor
	// Arguments:	privileges of manager, manager type (Agent/Node), process
	//				that all class processes have as thier base
	// Returns:		manager
	// --------------------------------------------------------------------------
	GCPtr<ClassManager> ClassManager::CreateManager(Privileges privileges, const std::string & typeName, GCPtr<Process> baseProcess)
	{
		GCPtr<ClassManager> ptr(new ClassManager(privileges, typeName, baseProcess));
		return ptr;
	}


	// --------------------------------------------------------------------------						
	// Function:	CreateManager
	// Description:	constructor
	// Arguments:	manager to clone
	// Returns:		manager
	// -------------------------------------------------------------------------
	GCPtr<ClassManager> ClassManager::CreateManager(const GCPtr<ClassManager>& other)
	{
		GCPtr<ClassManager> ptr(new ClassManager(other));
		return ptr;
	}


	// --------------------------------------------------------------------------						
	// Function:	ClassManager
	// Description:	constructor
	// Arguments:	privileges of manager, type (Agent/Node), base process all
	//				classes will use as thier base
	// Returns:		none
	// --------------------------------------------------------------------------
	ClassManager::ClassManager(Privileges privileges, const std::string& typeName, GCPtr<Process> baseProcess) :
		myPrivileges(privileges),
		myTypeName(typeName),
		myBaseProcess(baseProcess)
	{}


	// --------------------------------------------------------------------------						
	// Function:	Class
	// Description:	constructor
	// Arguments:	manager to clone from
	// Returns:		none
	// --------------------------------------------------------------------------
	ClassManager::ClassManager(const GCPtr<ClassManager>& other) :
		myPrivileges(other->myPrivileges),
		myTypeName(other->myTypeName),
		myCommentToken(other->myCommentToken),
		myInstantiators(other->myInstantiators),
		myUnorderedSpecs(other->myUnorderedSpecs)
	{

		// clone base process
		myBaseProcess = other->myBaseProcess->Clone();

		// clone classes
		Class::ClassMap::const_iterator it = other->myClasses.begin();
		while (it != other->myClasses.end())
		{
			GCPtr<Class> cls(new Class(it->second));
			myClasses[it->first] = cls;
			it++;
		}

		// remap class hierachy for my cloned classes
		it = other->myClasses.begin();
		while (it != other->myClasses.end())
		{
			GCPtr<Class> cls = myClasses.find(it->second->mySpec.myClassName)->second;
			
			if (!it->second->mySpec.myParentName.empty())
			{
				Class::ClassMap::iterator parent = myClasses.find(it->second->mySpec.myParentName);
				if(parent != myClasses.end())
					cls->myParent = parent->second;
			}
			Class::ClassMap::const_iterator dit = it->second->myDerived.begin();
			while (dit != it->second->myDerived.end())
			{
				GCPtr<Class> derived = myClasses.find(dit->second->GetName())->second;
				cls->myDerived[dit->second->GetName()] =derived;
				dit++;
			}
			it++;
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	~ClassManager
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	ClassManager::~ClassManager()
	{
		DestroyClasses();
	}


	// --------------------------------------------------------------------------						
	// Function:	AddInstantiator
	// Description:	add the function that creates object of the type (Agent/Node)
	//				this manager managers
	// Arguments:	instantiator func	
	// Returns:		none
	// --------------------------------------------------------------------------
	void ClassManager::AddInstantiator(const Registry::ObjectInstantiator& instantiator)
	{
		myInstantiators[instantiator.myImplementation] = instantiator;
	}


	// --------------------------------------------------------------------------						
	// Function:	ScanClasses
	// Description:	cans classes in a folder an puts them in a list not
	//				yet sorted by inheritance hierachy
	// Arguments:	implementation (Lua/Python), path to scan, whether to recurse
	//				whether to report uncreateable classes, type of class to 
	//				create (Agent/Node)
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool ClassManager::ScanClasses(Implementation implementation, const std::string& path, bool recurse, bool report, const std::string& classType)
	{
		Instatiators::iterator it = myInstantiators.find(implementation);
		if (it == myInstantiators.end())
			return false;

		Registry::ClassSpecs specs;
		if(!it->second.myClassesSpecifier(myTypeName, path, specs, recurse, report, classType))
			return false;

		for (Registry::ClassSpecs::iterator sit = specs.begin(); sit != specs.end(); sit++)
		{
			Registry::ClassSpecs::iterator exists = myUnorderedSpecs.find(sit->first);
			if (exists != myUnorderedSpecs.end())
				RELEASE_TRACE("%s: %s already exists.\n", classType.c_str(), sit->first);
			else
				myUnorderedSpecs[sit->first] = sit->second;
		}

		it->second.myPaths.push_back(path);
		return true;
	}



	// --------------------------------------------------------------------------						
	// Function:	BuildHierachry
	// Description:	builds the classes hierachically after all paths scanned
	// Arguments:	vm to use to compile class code
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool ClassManager::BuildHierachry(const GCPtr<VM>& vm)
	{

		// get all classes that are not derived 

		GCPtr<ClassManager> manager(this);

		Registry::ClassSpecs::iterator it = myUnorderedSpecs.begin();
		Registry::ClassSpecs::iterator next = it;
		while (it != myUnorderedSpecs.end())
		{
			next++;
			GCPtr<Class> exists = GetClass(it->first);
			if (exists.IsValid())
			{
				ERROR_TRACE("%s: %s already exists.\n", myTypeName.c_str(), it->first.c_str());
			}
			else if (it->second.myParentName.empty())
			{
				it->second.myTypeName = myTypeName;
				Registry::ObjectInstantiator inst = myInstantiators.find(it->second.myImplementation)->second;
				GCPtr<Class> cls(new Class(manager, vm, it->second, myBaseProcess, inst.myProcessConstructor, inst.myObjectConstructor));
				myClasses[it->first] = cls;
				myUnorderedSpecs.erase(it);
			}
			it = next;

		}

		// build inheritance hierachy from bases
		bool created = true;
		it = myUnorderedSpecs.begin();
		next = it;
		while (created && !myUnorderedSpecs.empty())
		{
			next++;
			created = false;
			GCPtr<Class> exists = GetClass(it->first);
			if (!exists.IsValid())
			{

				Implementation implementation = it->second.myImplementation;
				std::string parentName = it->second.myParentName;
				GCPtr<Class> parent = GetClass(parentName);
				if (parent.IsValid())
				{
					if (parent->GetImplementation() != implementation)
					{
						ERROR_TRACE("%s: %s and super %s have different implementations.\n", myTypeName.c_str(), it->first.c_str(), parentName.c_str());
					}
					else if (it->second.myInheritanceType == Registry::ClassSpec::Final)
					{
						ERROR_TRACE("%s: %s has super % s that is Final.\n", myTypeName.c_str(), it->first.c_str(), parentName.c_str());
					}
					else
					{
						it->second.myTypeName = myTypeName;
						if (it->second.myImplementation != parent->mySpec.myImplementation)
						{
							RELEASE_TRACE("%s: %s cant inherit from %s due to differing implementations.\n", myTypeName.c_str(), it->first.c_str(), parentName.c_str());
						}
						else
						{
							Registry::ObjectInstantiator inst = myInstantiators.find(it->second.myImplementation)->second;
							GCPtr<Class> cls(new Class(manager, vm, it->second, myBaseProcess, inst.myProcessConstructor, inst.myObjectConstructor));
							myClasses[it->first] = cls;
							myUnorderedSpecs.erase(it);
							created = true;
						}

					}


				}
			}
			it = next;
			if (it == myUnorderedSpecs.end())
				it = myUnorderedSpecs.begin();
		}


		if (!myUnorderedSpecs.empty())
		{
			// report there are some classes without a base class file
			std::string errorMessage = "ClassManager: Base classes for the following " + myTypeName + " could not be found: ";
			Registry::ClassSpecs::iterator it = myUnorderedSpecs.begin();
			while (it != myUnorderedSpecs.end())
			{
				errorMessage += it->first + ", ";
				it++;
			}
			errorMessage += ".\n";
			ERROR_TRACE(errorMessage.c_str());
			return false;
		}
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	FinalizeObjects
	// Description:	funalize all objects of all classes managed by this
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void ClassManager::FinalizeObjects()
	{
		Class::ClassMap::iterator cit = myClasses.begin();
		while (cit != myClasses.end())
		{
			cit->second->FinalizeObjects();
			cit++;
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	DestroyClasses
	// Description:	destroy all classes and their objects managed by this
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void ClassManager::DestroyClasses()
	{
		while (!myClasses.empty())
		{
			Class::ClassMap::iterator it = myClasses.begin();
			DestroyClass(it->first);
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	DestroyClass
	// Description:	destroy a class and its objects managed by this
	// Arguments:	name of class
	// Returns:		none
	// --------------------------------------------------------------------------
	bool ClassManager::DestroyClass(const std::string& className)
	{

		Class::ClassMap::iterator cit = myClasses.find(className);
		if (cit != myClasses.end())
		{
			Class::ClassMap derived = cit->second->myDerived;
			Class::ClassMap::iterator cls = derived.begin();

			while (cls != derived.end())
			{
				if (cls->second.IsValid())
					derived.insert(cls->second->myDerived.begin(), cls->second->myDerived.begin());

				cls++;
			}

			// swap order so we delete most derived first
			while (!derived.empty())
			{
				GCPtr<Class> mostDerived = derived.begin()->second;
				derived.erase(derived.begin());
				if (mostDerived.IsValid())
				{
					Class::ClassMap::iterator dit = myClasses.find(mostDerived->GetName());
					mostDerived.Destroy();
					myClasses.erase(dit);
				}
			}
			GCPtr<Class> clsPtr = cit->second;
			myClasses.erase(cit);
			clsPtr.SetNull();

			return true;
		}

		return false;
	}



	// --------------------------------------------------------------------------						
	// Function:	AddClass
	// Description:	adds a class to be managed by this
	// Arguments:	class
	// Returns:		none
	// --------------------------------------------------------------------------
	bool ClassManager::AddClass(const GCPtr<Class>& cls)
	{
		Class::ClassMap::iterator it = myClasses.find(cls->GetName());
		if (it != myClasses.end())
		{
			RELEASE_TRACE("%s class %s already exist.\n", cls->GetTypeName().c_str(), cls->GetName().c_str());
			return false;
		}

		myClasses[cls->GetName()] = cls;
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetClass
	// Description:	get class of name
	// Arguments:	name
	// Returns:		class
	// --------------------------------------------------------------------------
	const GCPtr<Class>& ClassManager::GetClass(const std::string& className) const
	{

		Class::ClassMap::const_iterator it = myClasses.find(className);
		if (it != myClasses.end())
			return it->second;

		const static GCPtr<Class> nullPtr(NULL);
		return nullPtr;
	}


	// --------------------------------------------------------------------------						
	// Function:	CreateObject
	// Description:	creates object of class name
	// Arguments:	class name, environment object exists in
	// Returns:		oject
	// --------------------------------------------------------------------------
	GCPtr<Object> ClassManager::CreateObject(const std::string& className, const GCPtr<Environment> &env)
	{
		
		const GCPtr<Class> cls = GetClass(className);
		if (cls.IsValid())
		{
			GCPtr<Object> object = cls->CreateObject(GCPtr<ClassManager>(this));
			cls->AddObject(object);
			object->GetProcess()->SetObject(object);
			object->GetProcess()->SetCurrentEnvironment(env);
			object->GetProcess()->SetHomeEnvironment(env);
			return object;
		}
		const static GCPtr<Object> nullPtr(NULL);
		return nullPtr;
	}
	

	// --------------------------------------------------------------------------						
	// Function:	GetAllObjects
	// Description:	gets all objects from all classes managed by this
	// Arguments:	object to return
	// Returns:		none
	// --------------------------------------------------------------------------
	void ClassManager::GetAllObjects(Class::Objects& objects) const
	{
		GCPtr<ClassManager> manager((ClassManager*)this);
		Class::ClassMap::const_iterator it = myClasses.begin();
		while(it != myClasses.end())
			it->second->GetObjects(manager, objects);
	}


	// --------------------------------------------------------------------------						
	// Function:	GetObjectsOfClass
	// Description:	get all objects of a class
	// Arguments:	class name, object to return
	// Returns:		none
	// --------------------------------------------------------------------------
	void ClassManager::GetObjectsOfClass(const std::string& className, Class::Objects& objects) const
	{
		GCPtr<ClassManager> manager((ClassManager*)this);
		Class::ClassMap::const_iterator it = myClasses.find(className);
		if (it != myClasses.end())
			it->second->GetObjects(manager, objects);

		return;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetObjectsClassified
	// Description:	get all objects which matcn a classifer
	// Arguments:	classifier, object to return
	// Returns:		none
	// --------------------------------------------------------------------------
	void ClassManager::GetObjectsClassified(Classifier& classifier, Class::Objects& objects) const
	{
		GCPtr<ClassManager> manager((ClassManager*)this);
		for (Class::ClassMap::const_iterator cit = myClasses.begin(); cit != myClasses.end(); cit++)
		{
			if (cit->second->GetClassifiers() == classifier)
				cit->second->GetObjects(manager, objects);
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	GetPrivileges
	// Description:	does what it say on the tin
	// Arguments:	none
	// Returns:		privileges
	// --------------------------------------------------------------------------
	const Privileges& ClassManager::GetPrivileges() const 
	{ 
		return myPrivileges; 
	}


	// --------------------------------------------------------------------------						
	// Function:	AssureIntegrity
	// Description:	checks the object os ok
	// Arguments:	object
	// Returns:		none
	// --------------------------------------------------------------------------
	void ClassManager::AssureIntegrity(const GCPtr<Object>& object) 
	{
	}

}