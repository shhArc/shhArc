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
#pragma warning( disable : 4786 4503 )
#endif

#include "Registry.h"
#include "Realm.h"
#include "../Arc/Module.h"
#include "../Common/Exception.h"
#include "../VM/Process.h"
#include "../VM/VM.h"
#include "../VM/Scheduler.h"
#include "../VM/Class.h"
#include "../LuaProcess/LuaType.h"
#include "../File/IOVariant.h"
#include <algorithm>
#include <functional>

namespace shh {



	struct CompareSubTypes : public std::unary_function<int, bool>
	{
		explicit CompareSubTypes(const Registry::OverloadTable::ArgumentTypes &args, const Registry::OverloadTable::SharedTypes& shared) : 
			mySearchFor(args),
			mySharedTypes(shared) 
		{}
		bool operator() (const Registry::OverloadTable::FunctionCallPair &pair)
		{
			const Registry::OverloadTable::ArgumentTypes &arg2 = pair.first;
			if (mySearchFor.size() != arg2.size())
				return false;

			for (int a = 0; a != mySearchFor.size(); a++)
			{
				if (mySearchFor[a] != arg2[a])
				{
					Registry::OverloadTable::SharedTypes::iterator sit = mySharedTypes.find(mySearchFor[a]);
					if (sit == mySharedTypes.end())
						return false;

					Registry::OverloadTable::ArgumentTypes::iterator ait = std::find(sit->second.begin(), sit->second.begin(), mySearchFor[a]);
					if (ait == sit->second.end())
						return false;
				}
			}
			return true;
		}
		const Registry::OverloadTable::ArgumentTypes& mySearchFor;
		Registry::OverloadTable::SharedTypes mySharedTypes;
	};

	// OverloadTable ////////////////////////////////////////////////////


	// --------------------------------------------------------------------------						
	// Function:	OverloadTable
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Registry::OverloadTable::OverloadTable() 
	{}


	// --------------------------------------------------------------------------						
	// Function:	OverloadTable
	// Description:	constructor
	// Arguments:	function name overload table if for
	// Returns:		none
	// --------------------------------------------------------------------------
	Registry::OverloadTable::OverloadTable(const std::string& funcName) : 
		myFunctionName(funcName) 
	{}

	
	// --------------------------------------------------------------------------						
	// Function:	~OverloadTable
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Registry::OverloadTable::~OverloadTable() 
	{}


	// --------------------------------------------------------------------------						
	// Function:	Call
	// Description:	calls a function
	// Arguments:	implementations of caller, argument types being sent,
	//				types that asre interchangable, dispatch data (contains 
	//				lua state)
	// Returns:		execution state
	// --------------------------------------------------------------------------	ExecutionState Registry::OverloadTable::Call(Implementation i, const Registry::OverloadTable::ArgumentTypes& args, const SharedTypes& sharedTypes, void* dispatchData)
	ExecutionState Registry::OverloadTable::Call(Implementation i, const Registry::OverloadTable::ArgumentTypes& args, const SharedTypes& sharedTypes, void* dispatchData)
	{
		//Args = GetTypeIds(L)
		FunctionCalls::iterator bc = myFunctionCalls.find(args);
		if (bc != myFunctionCalls.end())
		{
			return bc->second->Call(dispatchData);
		}
		else
		{
			CompareSubTypes comp(args, sharedTypes);
			bc = std::find_if(myFunctionCalls.begin(), myFunctionCalls.end(), comp);
			if (bc != myFunctionCalls.end())
			{
				return bc->second->Call(dispatchData);
			}
			else
			{
				std::string msg;
				if (i == Lua)
					msg += "Lua";
				else if (i == Python)
					msg += "Python";
			
				msg += " function" + myFunctionName + "(";
				for (int a = 0; a != args.size(); a++)
				{
					if (a > 0)
						msg += ", ";
					if (args[a] == 0)
						msg += "nil";
					else
						msg += Registry::GetRegistry().GetTypeName(args[a]);
				}
				msg += ")";
				Registry::DispatchData* d = static_cast<Registry::DispatchData*>(dispatchData);
				d->myMessage = msg;
				return ExecutionFailed;
			}
			
		}
		return ExecutionOk;
	}

	// Registry ////////////////////////////////////////////////////

	GCPtr<Registry> *Registry::ourRegistry = NULL;
	

	// --------------------------------------------------------------------------						
	// Function:	Registry
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Registry::Registry() :
		myNextTypeId(BASE_TYPEID)
	{
		myShhToImpId.resize(TOTAL_IMPLEMENTATIONS);
		myImpToShhId.resize(TOTAL_IMPLEMENTATIONS);
		myObjectInstantiators.resize(TOTAL_IMPLEMENTATIONS);
	}


	// --------------------------------------------------------------------------						
	// Function:	~Registry
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Registry::~Registry()
	{

	}


	// --------------------------------------------------------------------------						
	// Function:	GetRegistry
	// Description:	gets the registry (there can be only one)
	// Arguments:	none
	// Returns:		registry
	// --------------------------------------------------------------------------
	Registry& Registry::GetRegistry()
	{
		if (ourRegistry == NULL)
			ourRegistry = new GCPtr<Registry>();

		if(!ourRegistry->IsValid())
			*ourRegistry = GCPtr<Registry>(new Registry());

		return **ourRegistry;
	}


	// --------------------------------------------------------------------------						
	// Function:	SetRegistry
	// Description:	sets the registry (there can be only one)
	// Arguments:	registry
	// Returns:		none
	// --------------------------------------------------------------------------
	bool Registry::SetRegistry(const GCPtr<Registry> &r)
	{
		if (ourRegistry == NULL)
			ourRegistry = new GCPtr<Registry>();

		if(ourRegistry->IsValid())
			return false;

		*ourRegistry = r;
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	CloseRegistry
	// Description:	destorys registry
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Registry::CloseRegistry()
	{
		if (ourRegistry->IsValid())
			ourRegistry->Destroy();
	}


	// --------------------------------------------------------------------------						
	// Function:	GetEqualsFunction
	// Description:	returns the data comparision function for the data type
	//				of the given type. This funciton is used to compare equivalence
	//				of user data types.
	// Arguments:	data type
	// Returns:		equals funciton
	// --------------------------------------------------------------------------						
	EqualsFunction const Registry::GetEqualsFunction(int typeId) const
	{
		typeId = typeId < 0 ? -typeId : typeId;
		DEBUG_ASSERT(myEqualsFunctions.size() >= typeId && typeId < myNextTypeId);
		return myEqualsFunctions[typeId];
	}

	// --------------------------------------------------------------------------						
	// Function:	GetStringFunction
	// Description:	returns the string conversion function for the data type
	// Arguments:	data type id
	// Returns:		string funciton
	// --------------------------------------------------------------------------						
	StringFunction const Registry::GetStringFunction(int typeId) const
	{
		typeId = typeId < 0 ? -typeId : typeId;
		DEBUG_ASSERT(myStringFunctions.size() >= typeId && typeId < myNextTypeId);
		return myStringFunctions[typeId];
	}

	// --------------------------------------------------------------------------						
	// Function:	GetValueFunction
	// Description:	returns the value conversion function for the data type
	// Arguments:	data type id
	// Returns:		value funciton
	// --------------------------------------------------------------------------						
	ValueFunction const Registry::GetValueFunction(int typeId) const
	{
		typeId = typeId < 0 ? -typeId : typeId;
		DEBUG_ASSERT(myValueFunctions.size() >= typeId && typeId < myNextTypeId);
		return myValueFunctions[typeId];
	}


	// --------------------------------------------------------------------------						
	// Function:	RegisterType
	// Description:	registeers a data type
	// Arguments:	type, id of implementation language, 
	//				language being registered for
	// Returns:		none
	// --------------------------------------------------------------------------
	void Registry::RegisterType(const BaseType *type, int impId, Implementation imp)
	{
		int typeId = type->GetId();

		if (myTypes.size() < typeId+1)
			myTypes.resize(typeId + 1);

		if (myShhToImpId[0].size() < typeId+1)
		{
			for (int i = 0; i != TOTAL_IMPLEMENTATIONS; i++)
				myShhToImpId[i].resize(typeId + 1);
		}

		if (myImpToShhId[0].size() < impId+1)
		{
			for (int i = 0; i != TOTAL_IMPLEMENTATIONS; i++)
				myImpToShhId[i].resize(impId + 1);
		}

		RELEASE_ASSERT(myTypes[typeId] == NULL);
		if (!myTypes[typeId] != NULL)
		{
			myTypes[typeId] = type->GetStaticVirtual();
			myTypeNames[type->GetName()] = type->GetId();
			myShhToImpId[imp][typeId] = impId;
			myImpToShhId[imp][impId] = typeId;
		}

		
	}


	// --------------------------------------------------------------------------						
	// Function:	GetType
	// Description:	gets registered type of id
	// Arguments:	id
	// Returns:		type
	// --------------------------------------------------------------------------
	BaseType const * const Registry::GetType(int typeId) const
	{
		typeId = typeId < 0 ? -typeId : typeId;
		DEBUG_ASSERT(myTypes.size() >= typeId && typeId < myNextTypeId);
		return myTypes[typeId];
	}


	// --------------------------------------------------------------------------						
	// Function:	GetTypeName
	// Description:	gets name of a tpye
	// Arguments:	id
	// Returns:		name
	// --------------------------------------------------------------------------
	const std::string &Registry::GetTypeName(int typeId) const
	{
		typeId = typeId < 0 ? -typeId : typeId;
		DEBUG_ASSERT(myTypes.size() >= typeId && typeId < myNextTypeId);
		return myTypes[typeId]->GetName();
	}


	// --------------------------------------------------------------------------						
	// Function:	GetTypeId
	// Description:	get type id for type name
	// Arguments:	name
	// Returns:		id
	// --------------------------------------------------------------------------
	int Registry::GetTypeId(const std::string name) const
	{
		TypeNames::const_iterator it = myTypeNames.find(name);
		if (it == myTypeNames.end())
			return 0;

		return it->second;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetLastTypeId
	// Description:	get last registered type id
	// Arguments:	none
	// Returns:		id
	// --------------------------------------------------------------------------
	unsigned int Registry::GetLastTypeId() const
	{
		return myNextTypeId - 1;
	}


	// --------------------------------------------------------------------------						
	// Function:	SetCastableTypes
	// Description:	called to set types that can be casted for each language
	//				e.g. doube/float
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Registry::SetCastableTypes()
	{
		LuaTypeBase::SetCastableTypes();
	}


	// --------------------------------------------------------------------------						
	// Function:	RegisterFunctions
	// Description:	Called when a particualr data type is registerd. This registers
	//				with the base class a fucntions to handle the datatype
	// Arguments:	type id, functions
	// Returns:		none
	// --------------------------------------------------------------------------						
	void Registry::RegisterFunctions(int typeId, EqualsFunction e, StringFunction s, ValueFunction v)
	{
		typeId = typeId < 0 ? -typeId : typeId;

		if (myEqualsFunctions.size() < typeId + 1)
			myEqualsFunctions.resize(typeId + 1);
		if (myStringFunctions.size() < typeId + 1)
			myStringFunctions.resize(typeId + 1);
		if (myValueFunctions.size() < typeId + 1)
			myValueFunctions.resize(typeId + 1);

		if (e)
			myEqualsFunctions[typeId] = e;

		if (s)
			myStringFunctions[typeId] = s;

		if (v)
			myValueFunctions[typeId] = v;
	}


	// --------------------------------------------------------------------------						
	// Function:	RegisterRealm
	// Description:	registers a realm template
	// Arguments:	realm
	// Returns:		realm
	// --------------------------------------------------------------------------
	GCPtr<Realm> Registry::RegisterRealm(const GCPtr<Realm>& m)
	{
		std::string moduleName = m->GetName();

		Modules::iterator it = myRealms.find(moduleName);
		if (it != myRealms.end())
		{
			std::string error = "Realm " + m->GetName() + " already registered";
			THROW(error.c_str());
		}

		myRealms[moduleName] = m;
		myModuleFunctions[moduleName] = FunctionOverloadTables();
		return m;
	}


	// --------------------------------------------------------------------------						
	// Function:	UnregisterRealm
	// Description:	unregisters a realm
	// Arguments:	realm
	// Returns:		none
	// --------------------------------------------------------------------------
	void Registry::UnregisterRealm(const GCPtr<Realm>& m)
	{
		std::string moduleName = m->GetName();

		Modules::iterator it = myRealms.find(moduleName);
		if (it != myRealms.end())
			myRealms.erase(it);

		ModuleFunctions::iterator f = myModuleFunctions.find(moduleName);
		if (f != myModuleFunctions.end())
			myModuleFunctions.erase(f);
	}


	// --------------------------------------------------------------------------						
	// Function:	GetModule
	// Description:	gets a module of a given name
	// Arguments:	name
	// Returns:		module
	// --------------------------------------------------------------------------
	GCPtr<Module> Registry::GetModule(const std::string& name) const
	{
		Modules::const_iterator it = myModules.find(name);
		GCPtr<Module> m;
		if (it != myModules.end())
			m = it->second;

		return m;
	}


	// --------------------------------------------------------------------------						
	// Function:	RegisterModule
	// Description:	registers a module
	// Arguments:	module
	// Returns:		module
	// --------------------------------------------------------------------------
	GCPtr<Module> Registry::RegisterModule(const GCPtr<Module>& m)
	{
		std::string moduleName = m->GetName();

		Modules::iterator it = myModules.find(moduleName);
		if (it != myModules.end())
		{
			std::string error = "Module " + m->GetName() + " already registered";
			THROW(error.c_str());
		}

		myModules[moduleName] = m;
		myModuleFunctions[moduleName] = FunctionOverloadTables();
		return m;
	}


	// --------------------------------------------------------------------------						
	// Function:	RegisterModuleInProcess
	// Description:	registers a module with a process
	// Arguments:	process, module name, alias of module in process, dictionary
	//				if config vars
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool Registry::RegisterModuleInProcess(const GCPtr<Process>& process, const std::string& moduleName, const std::string& alias, const StringKeyDictionary& sd)
	{
		Privileges privileges = process->GetPrivileges();
		const GCPtr<Process> previous = Scheduler::GetCurrentProcess();
		Scheduler::SetCurrentProcess(process);
		Modules::iterator it = myModules.find(moduleName);
		if (it == myModules.end())
		{
			Scheduler::SetCurrentProcess(previous);
			std::string error = "Attempting to register module " + moduleName + "s with Process but module not exist";
			THROW(error.c_str());
			return false;
		}
		bool ok = it->second->Register(alias, sd, privileges);
		Scheduler::SetCurrentProcess(previous);
		return ok;
	}


	// --------------------------------------------------------------------------						
	// Function:	AddModuleFunction
	// Description:	registers a C function as belonging to a module
	// Arguments:	module, function name, function args types, caller interface
	//				for function
	// Returns:		overload table for function
	// --------------------------------------------------------------------------
	GCPtr<Registry::OverloadTable> Registry::AddModuleFunction(const GCPtr<Module>& module, const std::string& functionName, Registry::OverloadTable::ArgumentTypes& argTypes, const GCPtr<Registry::CallInterface>& call)
	{
		std::string moduleName = module->GetName();
		Modules::iterator it = myModules.find(moduleName);
		if (it == myModules.end() || it->second != module)
		{
			it = myRealms.find(moduleName);
			if (it == myRealms.end())
				return GCPtr<OverloadTable>();
		}
		ModuleFunctions::iterator mit = myModuleFunctions.find(module->GetName());
		if (mit == myModuleFunctions.end())
			return GCPtr<OverloadTable>();

		GCPtr< OverloadTable > ot;
		FunctionOverloadTables::iterator fit = mit->second.find(functionName);
		if (fit == mit->second.end())
		{
			ot = GCPtr<OverloadTable>(new OverloadTable(functionName));
			mit->second[functionName] = ot;
		}
		else
		{
			ot = fit->second;
		}


		OverloadTable::FunctionCalls::iterator cit = ot->myFunctionCalls.find(argTypes);
		if (cit == ot->myFunctionCalls.end())
		{
			ot->myFunctionCalls[argTypes] = call;
		}
		else
		{
			if (cit->second->GetFunction() != call->GetFunction())
				return GCPtr<OverloadTable>();
		}


		return ot;
	}


	// --------------------------------------------------------------------------						
	// Function:	RegisterModuleTypes
	// Description:	register types for modules in list with a process
	// Arguments:	process, module list
	// Returns:		if successfull
	// --------------------------------------------------------------------------
	bool Registry::RegisterModuleTypes(const GCPtr<Process>& process, const StringKeyDictionary &moduleList)
	{
		const GCPtr<Process> previous = Scheduler::GetCurrentProcess();
		Scheduler::SetCurrentProcess(process);
		StringKeyDictionary::OrderedVariables::const_iterator dit = moduleList.OrderedBegin();
		StringKeyDictionary::OrderedVariables::const_iterator dend = moduleList.OrderedEnd();
		while(dit != dend)
		{
			std::string moduleName = *dit->first;
			Modules::iterator it = myModules.find(moduleName);
			if (it == myModules.end())
			{
				std::string error = "Attempting to register types in module " + moduleName + " but module does not exist";
				THROW(error.c_str());
				return false;
			}

			StringKeyDictionary modSpec;
			dit->second->Get(modSpec);
			std::string alias;
			alias = modSpec.Get("alias", alias);
			StringKeyDictionary dict;
			dict = modSpec.Get("config", dict);

			it->second->RegisterTypes(alias, dict);
			dit++;
		}
		Scheduler::SetCurrentProcess(previous);
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	RegisterObjectInstantiator
	// Description:	register C functions for creating object of type
	// Arguments:	type name, instantiation struct contain function pointer 
	// Returns:		if successfull
	// --------------------------------------------------------------------------
	bool Registry::RegisterObjectInstantiator(const std::string& typeName, const Registry::ObjectInstantiator& i)
	{

		ObjectInstantiatorsMap &map = myObjectInstantiators[i.myImplementation];

		NamedObjectInstantiators::iterator it = map.find(typeName);
		if (it != map.end())
			return false;

		map[typeName] = i;
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	RegisterHardClass
	// Description:	register a hard object class
	// Arguments:	class
	// Returns:		if successfull
	// --------------------------------------------------------------------------
	bool Registry::RegisterHardClass(const GCPtr<Class>& cls)
	{
		const std::string &typeName = cls->GetTypeName();
		const std::string &className = cls->GetName();

		TypeHardClasses::iterator tit = myHardClasses.find(typeName);
		if (tit == myHardClasses.end())
		{
			myHardClasses[typeName] = HardClasses();
			tit = myHardClasses.find(typeName);
		}

		HardClasses::iterator hit = tit->second.find(className);
		if (hit != tit->second.end())
			return false;

		tit->second[className] = cls;
		
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetHardClass
	// Description:	get a hard object class
	// Arguments:	type name, class name
	// Returns:		class
	// --------------------------------------------------------------------------
	GCPtr<Class> Registry::GetHardClass(const std::string& typeName, const std::string& className) const
	{
		
		TypeHardClasses::const_iterator tit = myHardClasses.find(typeName);
		if (tit == myHardClasses.end())
			return GCPtr<Class>();
		

		HardClasses::const_iterator hit = tit->second.find(className);
		if (hit == tit->second.end())
			return GCPtr<Class>();
		
		return hit->second;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetObjectInstantiators
	// Description:	get all object instantiators functionsfor an implemtation 
	//				language
	// Arguments:	implementation
	// Returns:		instantiator
	// --------------------------------------------------------------------------
	const Registry::NamedObjectInstantiators& Registry::GetObjectInstantiators(Implementation i) const
	{
		return myObjectInstantiators[i];
	}
	

	// --------------------------------------------------------------------------						
	// Function:	GetNewTypeId
	// Description: gerts a new type id
	// Arguments:	none
	// Returns:		id
	// --------------------------------------------------------------------------
	int Registry::GetNewTypeId()
	{
		return myNextTypeId++;
	}

	
}