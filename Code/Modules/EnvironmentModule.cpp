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

#include "../Arc/Api.h"
#include "../Arc/Realm.h"
#include "../Arc/God.h"
#include "../Common/Exception.h"
#include "../File/IOVariant.h"
#include "../Schema/Schema.h"
#include "EnvironmentModule.h"
#include "../Arc/Api.h"

//! /type Environment
//! Controls manipulation of worlds and related variables.

namespace shh {



	// --------------------------------------------------------------------------						
	// Function:	RegisterTypes
	// Description:	registers data types to be used by a process
	// Arguments:	alies of module and type, dictionary of configuration vars
	// Returns:		if sucessfull
	// --------------------------------------------------------------------------
	bool EnvironmentModule::RegisterTypes(const std::string& alias, const StringKeyDictionary& sd)
	{
		return Module::RegisterTypes(alias, sd);
	}


	// --------------------------------------------------------------------------						
	// Function:	Register
	// Description:	registers functions and variables to be used by a process
	// Arguments:	alies of module and type, dictionary of configuration vars
	// Returns:		if sucessfull
	// --------------------------------------------------------------------------
	bool EnvironmentModule::Register(const std::string& alias, const StringKeyDictionary& sd, Privileges privileges)
	{

		GCPtr<Module> me(this);

		Api::OpenNamespace(alias);

		if (privileges & GodPrivilege)
		{
			Api::RegisterFunction("CreateWorld", CreateWorld, 4, me);
			Api::RegisterFunction("DestroyWorld", DestroyWorld, 2, me);
			Api::RegisterFunction("EnterWorld", EnterWorld, 2, me);
			Api::RegisterFunction("ExitWorld", ExitWorld, 1, me);
			Api::RegisterFunction("SetGlobal", SetGlobalStr, 3, me);
			Api::RegisterFunction("SetGlobal", SetGlobalNum, 3, me);
		}

		Api::RegisterFunction("GetLocal", GetLocalStr, 3, me);
		Api::RegisterFunction("GetLocal", GetLocalNum, 3, me);
		Api::RegisterFunction("GetGlobal", GetGlobalStr, 3, me);
		Api::RegisterFunction("GetGlobal", GetGlobalNum, 3, me);
		Api::RegisterFunction("SetLocal", SetLocalStr, 3, me);
		Api::RegisterFunction("SetLocal", SetLocalNum, 3, me);
		Api::RegisterFunction("GetObjects", GetObjects, 2, me);

		Api::CloseNamespace();

		return Module::Register(alias, sd, privileges);
	}


	//! /namespace Environment
	//! /function CreateWorld
	//! /privilege God
	//! /param string world_name
	//! /param string name_of_template_realm
	//! /returns boolean
	//! Creates a new world from a template Realm to create from, returns bool if successfull.
	ExecutionState EnvironmentModule::CreateWorld(std::string& worldName, std::string &realmName, VariantKeyDictionary &vd, bool &result)
	{
		result = false;
		GCPtr<Realm> realm = Realm::GetRealm(realmName);
		if (realm.IsValid())
		{
			StringKeyDictionary sd;
			for (VariantKeyDictionary::VariablesConstIterator vit = vd.Begin(); vit != vd.End(); vit++)
			{
				std::string s;
				if (vit->first->Get(s))
					sd.Set(s, *vit->second);
			}
			result = God::GetGod()->CreateWorld(worldName, sd, realm);
		}
		return ExecutionOk;
	}
	

	//! /namespace Environment
	//! /function DestroyWorld
	//! /privilege God
	//! /param string world_name
	//! /returns boolean
	//! Destorys a world. Returns bool if successfull.
	ExecutionState EnvironmentModule::DestroyWorld(std::string& worldName, bool& result)
	{
		result = God::GetGod()->DestroyWorld(worldName);
		return ExecutionOk;
	}


	//! /namespace Environment
	//! /function EnterWorld
	//! /privilege God
	//! /param string world_name
	//! /returns boolean
	//! Enters the world for God agents and scripts to operate in. Returns bool if successfull.
	ExecutionState EnvironmentModule::EnterWorld(std::string& worldName, bool &result)
	{
		result = false;
		GCPtr<Realm> world = God::GetGod()->GetWorld(worldName);
		if (world.IsValid())
		{
			world->IncEntryCount();
			GCPtr<Process> p = Scheduler::GetCurrentProcess();
			p->SetCurrentEnvironment(world);
			result = true;
		}
		return ExecutionOk;
	}



	//! /namespace Environment
	//! /function ExitWorld
	//! /privilege God
	//! Exists the currently entered world the God agent or script is currently in.
	ExecutionState EnvironmentModule::ExitWorld()
	{
		GCPtr<Environment> env = Scheduler::GetCurrentProcess()->GetCurrentEnvironment();
		if (God::GetGod() != env)
			env->DecEntryCount();
		GCPtr<Process> p = Scheduler::GetCurrentProcess();
		p->SetCurrentEnvironment(God::GetGod());
		return ExecutionOk;
	}


	//! /namespace Environment
	//! /function GetLocal
	//! /param string variable_name
	//! /param any_type default
	//! /returns any_type
	//! Returns a local (Realm) variable of a given name. Returns default if variable doesnt exist.
	ExecutionState EnvironmentModule::GetLocalStr(std::string& key, std::string& defaultValue, std::string& value)
	{
		GCPtr<Environment> env = Scheduler::GetCurrentProcess()->GetCurrentEnvironment();
		value = env->GetLocal(key, defaultValue);
		return ExecutionOk;
	}

	ExecutionState EnvironmentModule::GetLocalNum(std::string& key, double& defaultValue, double& value)
	{
		GCPtr<Environment> env = Scheduler::GetCurrentProcess()->GetCurrentEnvironment();
		value = env->GetLocal(key, defaultValue);
		return ExecutionOk;
	}

	//! /namespace Environment
	//! /function GetGlobal
	//! /param string variable_name
	//! /param any_type default
	//! /returns any_type
	//! Returns a global variable of a given name. Return default if variable doesnt exist.
	ExecutionState EnvironmentModule::GetGlobalStr(std::string& key, std::string& defaultValue, std::string& value)
	{
		value = Environment::GetGlobal(key, defaultValue);
		return ExecutionOk;
	}

	ExecutionState EnvironmentModule::GetGlobalNum(std::string& key, double& defaultValue, double& value)
	{
		value = Environment::GetGlobal(key, defaultValue);
		return ExecutionOk;
	}


	//! /namespace Environment
	//! /function SetLocal
	//! /param string variable_name
	//! /param any_type value_to_set
	//! Sets a local (Realm) variable of a given name.
	ExecutionState EnvironmentModule::SetLocalStr(std::string& key, std::string& value)
	{
		GCPtr<Environment> env = Scheduler::GetCurrentProcess()->GetCurrentEnvironment();
		value = env->SetLocal(key, value);
		return ExecutionOk;
	}

	ExecutionState EnvironmentModule::SetLocalNum(std::string& key, double& value)
	{
		GCPtr<Environment> env = Scheduler::GetCurrentProcess()->GetCurrentEnvironment();
		value = env->SetLocal(key, value);
		return ExecutionOk;
	}


	//! /namespace Environment
	//! /function SetGlobal
	//! /privilege God
	//! /param string variable_name
	//! /param any_type value_to_set
	//! Sets a global variable of a given name.
	ExecutionState EnvironmentModule::SetGlobalStr(std::string& key, std::string& value)
	{
		value = Environment::SetGlobal(key, value);
		return ExecutionOk;
	}

	ExecutionState EnvironmentModule::SetGlobalNum(std::string& key, double& value)
	{
		value = Environment::GetGlobal(key, value);
		return ExecutionOk;
	}


	//! /namespace Environment
	//! /function GetObjects
	//! /privilege All
	//! /param string object_type
	//! /returns table objects
	//! Gets all object of a particular type from the environment/agent
	ExecutionState EnvironmentModule::GetObjects(std::string& type, VariantKeyDictionary& dict)
	{
		GCPtr<ClassManager> cm;
		if (Api::GetCurrentEnvironment()->GetClassManager(type, cm))
		{
			if (cm->GetPrivileges() && AgentPrivilege)
			{
				Class::Objects objects;
				cm->GetAllObjects(objects);
				Class::Objects::iterator it = objects.begin();
				int i = 1;
				while (it != objects.end())
				{
					dict.Set(NonVariant<int>(i), *it);
					i++;
					it++;
				}
			}
			if (cm->GetPrivileges() && SchemaPrivilege)
			{

				unsigned int uid = Schema::GetTypeCode(type);
				if(uid == 0)
					return ExecutionOk;

				GCPtr<Schema> schema;
				GCPtr<Object> object = Api::GetCurrentProcess()->GetObject();
				schema.DynamicCast(object);
				if (schema.IsValid())
				{
					Schema::Schemas schemas;
					Schema::Schemas toProcess = schema->GetSchemas();
					for (Schema::Schemas::iterator it = toProcess.begin(); it != toProcess.end(); it++)
					{
						if ((*it)->GetTypeCode() == uid)
						{
							schemas.push_back(*it);
							Schema::Schemas children = (*it)->GetSubSchemas("type");
							schemas.insert(schemas.end(), children.begin(), children.end());
						}
					}
					Schema::Schemas::iterator it = schemas.begin();
					int i = 1;
					GCPtr<Object> rv;
					while (it != schemas.end())
					{
						rv.DynamicCast(*it);
						if (rv.IsValid())
						{
							dict.Set(NonVariant<int>(i), rv);
							i++;
						}
						it++;
					}
				}

			}
			return ExecutionOk;
		}
	}
}