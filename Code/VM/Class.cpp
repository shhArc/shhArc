///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 David K Bhowmik. All rights reserved.
// This file is part of shhArc.
//
// This Software is available under the MIT License with a No Modification clause
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

#include "../File/FileSystem.h"
#include "../File/TextFile.h"
#include "../File/JsonFile.h"
#include "../Common/Exception.h"
#include "Class.h"
#include "Object.h"
#include "ClassManager.h"
#include "SoftProcess.h"
#include <algorithm>



namespace shh {

	const std::string Class::ourMetaFileExtension = "meta";
	

	// --------------------------------------------------------------------------						
	// Function:	Class
	// Description:	constructor
	// Arguments:	class to clone from
	// Returns:		none
	// --------------------------------------------------------------------------
	Class::Class(const GCPtr<Class> &other) :
		mySpec(other->mySpec),
		myClassifiers(other->myClassifiers),
		myProcessConstructor(other->myProcessConstructor),
		myObjectConstructor(other->myObjectConstructor),
		myMeta(other->myMeta)
	{
		myProcess = myProcessConstructor(Privileges(), other->myProcess);
	}


	// --------------------------------------------------------------------------						
	// Function:	Class
	// Description:	constructor
	// Arguments:	manager, vm to compile class code with, spec of the class,
	//				base process to create class process from, construxtor 
	//				function to create new class process, constructor function to 
	//				create new class objects
	// Returns:		none
	// --------------------------------------------------------------------------
	Class::Class(const GCPtr<ClassManager>& manager, const GCPtr<VM>& vm, const Registry::ClassSpec& classSpec, const GCPtr<Process>& baseProcess, Registry::ProcessConstructor pc, Registry::ObjectConstructor oc) :
		mySpec(classSpec),
		myProcessConstructor(pc),
		myObjectConstructor(oc)
	{
		if (mySpec.myImplementation == Engine)
		{
			myClassifiers.Add(mySpec.myTypeName);
			myProcess = myProcessConstructor(manager->GetPrivileges(), baseProcess);
			myClassifiers.Add(GetName().c_str());
		}
		else
		{
			if (!mySpec.myParentName.empty())
			{
				// clone parent class
				myParent = manager->GetClass(mySpec.myParentName);
				myParent->myDerived[GetName()] = GCPtr<Class>(this);
				myClassifiers = myParent->GetClassifiers();
				myProcess = myProcessConstructor(manager->GetPrivileges(), myParent->myProcess);
				GCPtr<SoftProcess> sp;
				sp.DynamicCast(myProcess);
				sp->Overide(classSpec.myScript, mySpec.myOveridePrefix);				
			}
			else
			{
				// start with cloned base process
				myClassifiers.Add(mySpec.myTypeName);
				myProcess = myProcessConstructor(manager->GetPrivileges(), baseProcess);
			}
			myClassifiers.Add(GetName().c_str());


			GCPtr<SoftProcess> sp;
			sp.DynamicCast(myProcess);
			sp->AddScriptPath(mySpec.myPath);
			sp->SetVM(vm);

			// add new code to process
			sp->Execute(mySpec.myFilename, true, false);
			sp->ValidateFunctionNames(true);
			sp->SetVM(GCPtr<VM>());
		}

		std::string metaFile = mySpec.myPath + "/" + mySpec.myFilename.substr(0, mySpec.myFilename.rfind("."))+"."+ ourMetaFileExtension;
		if (FileSystem::IsValidFile(metaFile))
		{
			BinaryFile pathIn(metaFile, IOInterface::In);
			JsonFile::Read(pathIn, myMeta);
		}
	}

	
	// --------------------------------------------------------------------------						
	// Function:	Class
	// Description:	constructor
	// Arguments:	name of class, type of class (Agent/Mode), process to assign
	//				as this classes process, function to create new 
	//				class process, constructor function to create new class objects
	// Returns:		none
	// --------------------------------------------------------------------------
	Class::Class(const std::string &name, const std::string &typeName, const GCPtr<Process>& process, Registry::ProcessConstructor pc, Registry::ObjectConstructor oc) :
		myProcessConstructor(pc),
		myObjectConstructor(oc)
	{
		mySpec.myImplementation = Engine;
		mySpec.myClassName = name;
		mySpec.myTypeName = typeName;
		mySpec.myParentName = mySpec.myTypeName;
		mySpec.myInheritanceType = Registry::ClassSpec::Final;
		mySpec.myImplementation = Engine;

		myClassifiers.Add(mySpec.myTypeName);
		myClassifiers.Add(GetName().c_str());
		myProcess = process;
	}


	// --------------------------------------------------------------------------						
	// Function:	Class
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Class::~Class()
	{
		// remove derived thos from parent class
		if (myParent.IsValid())
		{
			ClassMap::iterator it = myParent->myDerived.find(mySpec.myClassName);
			if (it != myParent->myDerived.end())
				myParent->myDerived.erase(it);
		}

		// remove all managers that have objects of this class
		ManagerObjects::iterator mit = myObjects.begin();
		while(mit != myObjects.end())
		{
			RemoveManager(mit->first);
			mit = myObjects.begin();
		}
	}



	// --------------------------------------------------------------------------						
	// Function:	GetName
	// Description:	gets my class name
	// Arguments:	none
	// Returns:		name
	// --------------------------------------------------------------------------
	const std::string& Class::GetName() const
	{
		return mySpec.myClassName;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetTypeName
	// Description:	gets my type name (Agent/Node)
	// Arguments:	none
	// Returns:		name
	// --------------------------------------------------------------------------
	const std::string& Class::GetTypeName() const 
	{ 
		return mySpec.myTypeName;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetImplementation
	// Description:	gets my imlementation type (Lua/Python)
	// Arguments:	none
	// Returns:		implementation
	// --------------------------------------------------------------------------
	Implementation Class::GetImplementation() const 
	{ 
		return mySpec.myImplementation; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetClassifiers
	// Description:	get classifiers (eg Node/Agent and parent class names
	// Arguments:	none
	// Returns:		classifiers
	// --------------------------------------------------------------------------
	const Classifier& Class::GetClassifiers() const
	{
		return myClassifiers;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetObjects
	// Description:	get all objects created from this class and owned by the given
	//				manager
	// Arguments:	manager objects also belong to, objects retruned
	// Returns:		none
	// --------------------------------------------------------------------------
	void Class::GetObjects(const GCPtr<ClassManager>& manager, Class::Objects& objects) const
	{
		static Objects empty;
		ManagerObjects::const_iterator it = myObjects.find(manager);
		if (it == myObjects.end())
			objects.insert(objects.end(), it->second.begin(), it->second.end());

	}


	// --------------------------------------------------------------------------						
	// Function:	HasFunction
	// Description:	returns of class has a function of given name
	// Arguments:	function name
	// Returns:		if has it
	// --------------------------------------------------------------------------
	bool Class::HasFunction(const std::string& functionName) const
	{
		return myProcess->HasFunction(functionName);
	}


	// --------------------------------------------------------------------------						
	// Function:	GetMeta
	// Description:	returns meta dictionary of class 
	// Arguments:	none
	// Returns:		dictionary
	// --------------------------------------------------------------------------
	const StringKeyDictionary& Class::GetMeta() const
	{
		return myMeta;
	}


	// --------------------------------------------------------------------------						
	// Function:	AddManager
	// Description:	adds a manager for creating object of this type for
	// Arguments:	manager
	// Returns:		none
	// -------------------------------------------------------------------------
	void Class::AddManager(const GCPtr<ClassManager>& manager)
	{
		ManagerObjects::iterator it = myObjects.find(manager);
		if (it == myObjects.end())
			myObjects[manager] = Objects();
	}


	// --------------------------------------------------------------------------						
	// Function:	RemoveManager
	// Description:	removed a manager for creating object of this type for and
	//				destroys all objects created for that manager
	// Arguments:	manager
	// Returns:		none
	// --------------------------------------------------------------------------
	void Class::RemoveManager(const GCPtr<ClassManager>& manager)
	{
		ManagerObjects::iterator it = myObjects.find(manager);
		DEBUG_ASSERT(it != myObjects.end());

		if (it != myObjects.end())
		{
			Objects objects = it->second;
			while (!objects.empty())
			{
				GCPtr<Object> del = objects.back();
				del.Destroy(); // will add Finalize messages
				objects.pop_back();
			}
		}

	}


	// --------------------------------------------------------------------------						
	// Function:	FinalizeObjects
	// Description:	finalizes all objects e.g. calls script Finalize func etc
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Class::FinalizeObjects()
	{
		ManagerObjects managerObjects = myObjects;
		ManagerObjects::iterator it = managerObjects.begin();
		if (it != managerObjects.end())
		{
			Objects objects = it->second;
			while (!objects.empty())
			{
				GCPtr<Object> o = objects.back();
				o->Finalize(o.GetObject());
				objects.pop_back();
			}
			it++;
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	CreateObject
	// Description:	create an object of this class type for given manager
	// Arguments:	manager
	// Returns:		none
	// --------------------------------------------------------------------------
	GCPtr<Object> Class::CreateObject(const GCPtr<ClassManager>& manager)
	{
		GCPtr<Process> process = myProcess->Clone();
		GCPtr<Object> object(myObjectConstructor(manager, GCPtr<Class>(this), process));
		return object;
	}


	// --------------------------------------------------------------------------						
	// Function:	AddObject
	// Description:	adds an object to class
	// Arguments:	object
	// Returns:		none
	// --------------------------------------------------------------------------
	void Class::AddObject(const GCPtr<Object>& object)
	{
		// find manafer object is filed under
		const GCPtr<ClassManager>& manager = object->GetManager();
		ManagerObjects::iterator it = myObjects.find(manager);
		if (it == myObjects.end())
		{
			// add manager if not already logging for it
			AddManager(manager);
			it = myObjects.find(manager);
		}

		// file the object under manager
		Objects& objects = it->second;
		objects.push_back(object);
	}


	// --------------------------------------------------------------------------						
	// Function:	RemoveObject
	// Description:	removes object from class
	// Arguments:	object
	// Returns:		none
	// --------------------------------------------------------------------------
	void Class::RemoveObject(const GCPtr<Object>& object)
	{
		// find manager object is filed under
		ManagerObjects::iterator it = myObjects.find(object->GetManager());
		if (it == myObjects.end())
			return;

		Objects& objects = it->second;
		Objects::iterator oit = std::find(objects.begin(), objects.end(), object);
		
		// errase object for manager list it is filed under
		if (oit != objects.end())
			objects.erase(oit);

		// if no more objects for that manager then remove the manager from my list
		if (objects.empty())
			myObjects.erase(it);
			
	}

}
