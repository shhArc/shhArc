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

#ifndef OBJECTMODULE_H
#define OBJECTMODULE_H


#include "../Config/GCPtr.h"
#include "../Common/Enums.h"
#include "../Common/SecureStl.h"
#include "../Arc/Module.h"
#include "../Arc/Api.h"
#include "../Arc/Realm.h"
#include "../Arc/Type.h"
#include "../Arc/Type.inl"
#include "../VM/Scheduler.h"
#include <string>
#include <inttypes.h>


//! /type Object
//! Generic object functions. Token "Object" should be repkaced with Agent or Node variable names 


namespace shh {


	template<class O, class OM> class ObjectModule : public Module
	{
	public:

		ObjectModule(const std::string& typeName) 
		{ 
			TypeName(typeName);
		}
		bool RegisterTypes(const std::string& alias, const StringKeyDictionary& sd);
		bool Register(const std::string& alias, const StringKeyDictionary& sd, Privileges privileges);
		virtual std::string GetName() const { return GetNameStatic(); }
		static std::string GetNameStatic() { return StripName(std::string(typeid(ObjectModule<O, OM>).name())) + "Module"; }
		static std::string StripName(const std::string& text) 
		{ 
			std::string t = text.substr(0, text.rfind(","));
			return t.substr(t.rfind(" ") + 1, t.length()-t.rfind(" ") - 1);
		}

		static const std::string GetTypeName() 
		{
			return TypeName();
		}




		static std::string Stringer(void const* const data);

		static int TypeCheck(lua_State* L);
		static int Create(lua_State* L);
		static ExecutionState Destroy(GCPtr<O>& object, bool &destroyed);
		static ExecutionState IsValid(GCPtr<O>& object, bool& result);
		static ExecutionState This(GCPtr<O>& me);
		static ExecutionState ExpressSchema(std::string& file, std::string& metaFile, bool& result);



		static Privileges ourExcludedCreaters;
		static Privileges ourExcludedDestroyers;
		static bool ourAllowInterVM;

	

	private:

		static std::string ourAlias;

		static const std::string TypeName(const std::string& typeName = "")
		{
			static std::string localTypeName;
			if (!typeName.empty() && localTypeName.empty())
				localTypeName = typeName;
			return localTypeName;
		}
	};

	template<class O, class OM> Privileges ObjectModule<O, OM>::ourExcludedCreaters = BasicPrivilege | SlavePrivilege;
	template<class O, class OM> Privileges ObjectModule<O, OM>::ourExcludedDestroyers = BasicPrivilege | SlavePrivilege;
	template<class O, class OM> bool ObjectModule<O, OM>::ourAllowInterVM = true;
	template<class O, class OM> std::string ObjectModule<O,OM>::ourAlias = "Object";


	// --------------------------------------------------------------------------						
	// Function:	RegisterTypes
	// Description:	registers data types to be used by a process
	// Arguments:	alies of module and type, dictionary of configuration vars
	// Returns:		if sucessfull
	// --------------------------------------------------------------------------
	template<class O, class OM> bool ObjectModule<O, OM>::RegisterTypes(const std::string& alias, const StringKeyDictionary& sd)
	{
		ourAlias = alias;
		O::ourTypeName = alias;
		GCPtr<O>* example = NULL;
		int type = Api::LuaRegisterType(example, alias, Stringer, NULL, ourAllowInterVM, false);
		return Module::RegisterTypes(alias, sd);
	}


	// --------------------------------------------------------------------------						
	// Function:	Register
	// Description:	registers functions and variables to be used by a process
	// Arguments:	alies of module and type, dictionary of configuration vars
	// Returns:		if sucessfull
	// --------------------------------------------------------------------------
	template<class O, class OM> bool ObjectModule<O, OM>::Register(const std::string& alias, const StringKeyDictionary& sd, Privileges privileges)
	{
		GCPtr<Module> me(this);

		Implementation i = Scheduler::GetCurrentProcess()->GetImplementation();
		if (i == Lua)
		{
			GCPtr<O>* example = NULL;
			GCPtr<Module> me(this);

			Implementation i = Api::GetImplementation();

			Api::OpenNamespace("TypeCheck");
			if (Api::GetImplementation() == Lua)
				Api::LuaRegisterFunction(alias, TypeCheck);
			Api::CloseNamespace();

			if (!(privileges & ourExcludedCreaters))
			{
				if (Api::GetImplementation() == Lua)
					Api::LuaRegisterFunction(alias, Create);
			}

			Api::RegisterMemberFunction(example, "Destroy", Destroy, 2, me);
			Api::RegisterMemberFunction(example, "IsValid", IsValid, 2, me);
			
			Api::OpenNamespace("shh");
			Api::RegisterFunction(std::string("This") + alias, This, 1, me);
			Api::RegisterFunction("ExpressSchema", ExpressSchema, 3, me);
			Api::CloseNamespace();
		
		}

		return Module::Register(alias, sd, privileges);
	}


	// --------------------------------------------------------------------------						
	// Function:	Stringer
	// Description:	converts a value of type used by this module to a string
	//				for printing
	// Arguments:	void pointer to variable of type used by this module
	// Returns:		string
	// --------------------------------------------------------------------------
	template<class O, class OM> std::string ObjectModule<O, OM>::Stringer(void const* const data)
	{
		GCPtr<O>* o = (GCPtr<O>*)data;
		char temp[1024];
		if (o->IsValid())
			snprintf(temp, 1023, "%s %s % " PRIu64, ourAlias.c_str(), (*o)->Object::GetName().c_str(), (unsigned long long)o->GetObject());
		else
			snprintf(temp, 1023, "%s null % " PRIu64, ourAlias.c_str(), (unsigned long long)o->GetObject());
		return std::string(temp);
	}


	//! /namespace TypeCheck
	//! /function Object
	//! /param Object object
	//! /returns boolean
	//! Returns whether variable is an Object. Call using class name token.
	//! Agent or Node instead of tolen token Object, e.g: TypeCheck.Node(variable_name)
	template<class O, class OM> int ObjectModule<O, OM>::TypeCheck(lua_State* L)
	{
		Api::LuaCheckNumArguments(L, 1);
		bool ok = LuaTypeBase::GetArgumentType(L, 1) == LuaType<GCPtr<O>>::GetLuaId();
		LuaApiTemplate::Return(L, ok);
		return 1;
	}


	//! /member shhCONSTRUCTOR
	//! /function Object
	//! /param string class_name
	//! /param variable_arg optional_variable
	//! /returns object
	//! Creates an object of the given class. Call using class name token Agent or Node instead of token Object.
	template<class O, class OM> int ObjectModule<O, OM>::Create(lua_State* L)
	{

		bool ok = true;
		GCPtr<O> object;
		GCPtr<Class> oc;
		GCPtr<Process> cp = Scheduler::GetCurrentProcess();
		std::string name;
		Api::LuaGetArgument(L, 1, name);
		unsigned int argsToSkip = 1;

		if (cp->IsFinalizing() && !SoftProcess::ourAllowMessegesWhenFinalising)
		{
			ok = false;
			ERROR_TRACE("Process can not create %s when Finalizing.\n", ourAlias.c_str());
		}
		else
		{
			Api::LuaCheckNumArgumentsGreaterOrEqual(L, 1);

			if (cp->IsInitializing())
			{
				ok = false;
				ERROR_TRACE("Process can not create %s when Initializing.\n", ourAlias.c_str());
			}
			else
			{
				if (Scheduler::ourMaxMessagesPerUpdate != 0 && cp->GetNumMessagesSentThisUpdate() > Scheduler::ourMaxMessagesPerUpdate)
				{
					ok = false;
					ERROR_TRACE("Process has sent to many messages this update (limit &d)\n", Scheduler::ourMaxMessagesPerUpdate);
				}
				else
				{
					Privileges privileges = cp->GetPrivileges();
					if (privileges & ourExcludedCreaters)
					{
						ok = false;
						ERROR_TRACE("Trying to create %s but Process does not have authority.\n", ourAlias.c_str());
					}
					else
					{
						GCPtr<Environment> env = cp->GetCurrentEnvironment();
						if (!env.IsValid())
						{
							ok = false;
							ERROR_TRACE("Trying to create %s but Realm does not exist.\n", ourAlias.c_str());
						}
						else
						{
							GCPtr<OM> om;
							env->GetClassManager(GetTypeName(), om);
							if (!om.IsValid())
							{
								ok = false;
								ERROR_TRACE("Trying to create %s but %s Manager in Realm does not exist.\n", ourAlias.c_str(), ourAlias.c_str());
							}
							else
							{
								// must be in script dricetory or subdirectory
								oc = om->GetClass(name);
								if (!oc.IsValid())
								{
									ok = false;
									ERROR_TRACE("Trying to create %s %s but %s Class does not exist.\n", ourAlias.c_str(), name.c_str(), ourAlias.c_str());
								}
								else
								{
									GCPtr<Object> obj = om->CreateObject(name, env);

									object.DynamicCast(obj);

									if (!object.IsValid())
									{
										obj.Destroy();
										ok = false;
										RELEASE_TRACE("Error trying to create %s %s.\n", ourAlias.c_str(), name.c_str());
									}
									else
									{

										obj->Initialize(env->GetScheduler(), name, StringKeyDictionary());
										env->GetScheduler()->AddVM(obj->GetProcess()->GetVM());




										GCPtr<Schema> schema;
										schema.DynamicCast(obj);
										if (schema.IsValid() && schema->RequiresConfiguration())
										{
											VariantKeyDictionary variantConfig;
											Api::LuaGetArgument(L, 2, variantConfig);
											StringKeyDictionary config;
											ConvertVariantDictToStringDict(variantConfig, config);


											// configure schema interfaces and edges
											ok = schema->Configure(config);

											if (ok)
											{
												// build sub schemas
												VariantKeyDictionary variantSchemas;
												Api::LuaGetArgument(L, 3, variantSchemas);
												StringKeyDictionary schemas;
												ConvertVariantDictToStringDict(variantSchemas, schemas);
												for (StringKeyDictionary::VariablesConstIterator sit = schemas.Begin(); sit != schemas.End(); sit++)
												{
													StringKeyDictionary schemaDef;
													if (sit->second->Get(schemaDef))
													{
														std::string typeName;
														typeName = schemaDef.Get("type", typeName);
														std::string className;
														className = schemaDef.Get("class", className);
														GCPtr<Object> object = schema->Create(env, schema, typeName, className, schemaDef);
														if (!object.IsValid())
															RELEASE_TRACE("Failed to create subSchema %s %s.\n", typeName.c_str(), className.c_str());

													}
												}
											}

											if (ok)
												argsToSkip += 2;

										}
									}
								}
							}
						}
					}
				}
			}
		}

		if (ok)
		{
			Message* msg = new Message;
			msg->myFunctionName = SoftProcess::ourInitializeMessage;
			msg->myTo = object->Object::GetProcess();
			msg->myFrom = cp;
			msg->SetCallType(Message::Synchronous);
			msg->myDestroyOnCompletion = false;
			msg->myPriority = Priority::GetSystem();

			if (!msg->SendMsg(0.0, argsToSkip))
			{
				delete msg;
				if (oc->HasFunction(SoftProcess::ourInitializeMessage))
				{
					ok = false;
					RELEASE_TRACE("Trying to initialize %s %s but %s class does not have correct Initialize functio.\nn", ourAlias.c_str(), name.c_str(), ourAlias.c_str());
				}
				else
				{
					object->CompleteInitialization();
					LuaApiTemplate::Return(L, true);
					LuaApiTemplate::Return(L, object);
					Scheduler::SetCurrentProcess(cp);
					return 2;
				}
			}
			else
			{
				cp->YieldProcess(ExecutionAwaitingCallback, -1);
				Scheduler::SetCurrentProcess(cp);
				return 0;
			}
		}

		if(!ok)
		{
			object.Destroy();
			LuaApiTemplate::Return(L, false);
			LuaApiTemplate::Return(L, object);
			Scheduler::SetCurrentProcess(cp);
			return 2;
		}

		return 0;
	}


	//! /member Object
	//! /function Destroy
	//! /param Object object
	//! /returns boolean
	//! Destorys an object, returns if successfull.
	template<class O, class OM> ExecutionState ObjectModule<O, OM>::Destroy(GCPtr<O>& object, bool& destroyed)
	{
		destroyed = false;
		GCPtr<Process> cp = Scheduler::GetCurrentProcess();
		GCPtr<VM> vm = cp->GetVM();
		GCPtr<O> thisObject;
		thisObject.DynamicCast(vm);
		bool isSelf = thisObject == object;

		Privileges privileges = cp->GetPrivileges();

		if (!isSelf && privileges & ourExcludedDestroyers)
		{
			Api::LuaThrowScriptError("Trying to destroy %s but Process does not have authority");
		}
		else if (!isSelf && privileges & WorldPrivilege && object->Object::GetProcess()->GetPrivileges() & GodPrivilege)
		{
			Api::LuaThrowScriptError("Trying to destroy God %s but Process only has World privileges", ourAlias.c_str());
		}
		else
		{
			if(object->Object::GetProcess()->Terminate(cp))
			{
				object.Destroy();
				object.SetNull();
			}
			else
			{
				cp->YieldProcess(ExecutionAwaitingCallback, -1);
			}
			destroyed = true;
		}

		return ExecutionOk;
	}


	//! /member Object
	//! /function IsValid
	//! 
	//! /param Object object
	//! /returns boolean
	//! Returns true if an object is valid.
	template<class O, class OM> ExecutionState ObjectModule<O, OM>::IsValid(GCPtr<O>& object, bool& result)
	{
		result = object.IsValid();
		return ExecutionOk;
	}



	//! /member Object
	//! /function This
	//! /privilege Agent
	//! /privilege Node
	//! /returns object
	//! Returns object that this process belongs to.
	template<class O, class OM> ExecutionState ObjectModule<O, OM>::This(GCPtr<O>& me)
	{
		GCPtr<Process> cp = Scheduler::GetCurrentProcess();
		GCPtr<Object> object = cp->GetObject();
		me.DynamicCast(object);
		return ExecutionOk;
	}


	//! /member Object
	//! /function ExpressSchema
	//! /privilege Agent
	//! /privilege Node
	//! /param string schema_filename
	//! /param string meta_schema_filename
	//! /returns boolean
	//! Expresses a schema file, returns if successfull.
	template<class O, class OM> ExecutionState ObjectModule<O, OM>::ExpressSchema(std::string &file, std::string& metaFile, bool &result)
	{
		GCPtr<Process> cp = Scheduler::GetCurrentProcess();

		Privileges privileges = cp->GetPrivileges();
		if (privileges & ourExcludedCreaters)
			Api::LuaThrowScriptError("Trying to express Schema %s but Process does not have authority", file.c_str());

		GCPtr<Environment> env = cp->GetCurrentEnvironment();
		if (!env.IsValid())
			Api::LuaThrowScriptError("Trying to create Schema %s but Realm does not exist", file.c_str());

		
		GCPtr<Object> object = cp->GetObject();
		GCPtr<Schema> schema;
		schema.DynamicCast(object);
		if (!schema.IsValid())
		{
			result = false;
			RELEASE_TRACE("Trying to express Schema %s but process is not a schema.\n", file.c_str());
		}
		else
		{
			if (schema->IsExpressed())
			{
				RELEASE_TRACE("Trying to express Schema %s but Schema already expressed.\n", file.c_str());
			}
			else
			{
				result = Schema::LoadSchema(env, schema, file, metaFile);
				if (!result)
					RELEASE_TRACE("Error trying to express Schema %s.\n", file.c_str());

			}
		}

		return ExecutionOk;
	}
}
#endif // AGENTMODULE_H

