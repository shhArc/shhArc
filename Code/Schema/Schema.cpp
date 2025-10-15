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

#include "../Common/Debug.h"
#include "../VM/ClassManager.h"
#include "../VM/Scheduler.h"
#include "../VM/SoftProcess.h"
#include "../File/FileSystem.h"
#include "../File/JsonFile.h"
#include "Schema.h"
#include "Agent.h"

namespace shh
{
	const std::string Schema::ourSchemaFileExtension = "schema";



	// --------------------------------------------------------------------------						
	// Function:	LoadSchema
	// Description:	loads schema from a file definition
	// Arguments:	environment schema belongs to, parent schema, file name,
	//				name of meta schema file
	// Returns:		if sucessfull
	// --------------------------------------------------------------------------
	bool Schema::LoadSchema(const GCPtr<Environment>& env, const GCPtr<Schema>& parent, const std::string& file, const std::string& metaFile)
	{
		std::string folder;
		FileSystem::GetVariable("SCHEMA", folder);
		std::string fullPath = folder + "/" + file + "." + ourSchemaFileExtension;
		StringKeyDictionary spec;
		BinaryFile schemaIn(fullPath, IOInterface::In);
		if (!JsonFile::Read(schemaIn, spec))
			return false;

		FileSystem::GetVariable("META", folder);
		std::string metaPath = folder + "/" + metaFile + "." + Class::ourMetaFileExtension;
		StringKeyDictionary meta;
		BinaryFile metaIn(metaPath, IOInterface::In);
		if (!JsonFile::Read(metaIn, meta))
			return false;

		if (!ValidateSchema(env, spec, meta))
			return false;

		return ExpressSchema(env, parent, spec);
	}


	// --------------------------------------------------------------------------						
	// Function:	ValidateSchema
	// Description:	expresses schema from a file definition
	// Arguments:	environment schema belongs to, schema definition dictionary
	//				meta schema
	// Returns:		if sucessfull
	// --------------------------------------------------------------------------
	bool Schema::ValidateSchema(const GCPtr<Environment>& env, const StringKeyDictionary& spec, const StringKeyDictionary& meta)
	{
		ArrayKeyDictionary schemas;
		schemas = spec.Get("schemas", schemas);

		typedef std::map<std::string, int> Counters;
		typedef std::map<std::string, Counters> Types;

		Types types;

		for (ArrayKeyDictionary::VariablesConstIterator sit = schemas.Begin(); sit != schemas.End(); sit++)
		{
			StringKeyDictionary schemaDef;
			if (sit->second->Get(schemaDef))
			{
				std::string typeName;
				typeName = schemaDef.Get("type", typeName);
				std::string className;
				className = schemaDef.Get("class", className);

				Types::iterator tit = types.find(typeName);
				if (tit == types.end())
				{
					Counters counters;
					counters[className] = 1;
					types[typeName] = counters;
				}
				else
				{
					Counters::iterator cit = tit->second.find(className);
					if (cit == tit->second.end())
						tit->second[className] = 1;
					else
						cit->second++;
				}
			}
		}


		StringKeyDictionary metaSchemas;
		metaSchemas = meta.Get("schemas", metaSchemas);

		for (StringKeyDictionary::VariablesConstIterator mit = metaSchemas.Begin(); mit != metaSchemas.End(); mit++)
		{
			std::string typeName = *mit->first;
			StringKeyDictionary classes;
			if (!mit->second->Get(classes))
				return false;	// error in meta file
			
			for (StringKeyDictionary::VariablesConstIterator cit = classes.Begin(); cit != classes.End(); cit++)
			{
				std::string className = *cit->first;
				StringKeyDictionary classConstraint;
				if (!cit->second->Get(classConstraint))
					return false; 	// error in meta file

				long min = classConstraint.Get("min", (long) -1);
				long max = classConstraint.Get("max", (long) -1);
				if(min == -1.0 || max == -1.0)
					return false; 	// error in meta file

				// check if has any of this type
				Types::iterator tit = types.find(typeName);
				if (tit == types.end() && min == 0)
					continue;

				// check if has any of this class
				Counters::iterator it = tit->second.find(className);
				if (it == tit->second.end() && min == 0)
					continue;

				int count = it->second;

				// check in range
				if (count < min || (count > max && max != 0))
					return false; 

				tit->second.erase(it);
				if (tit->second.empty())
					types.erase(tit);
			}
		}

		if (!types.empty())
			return false;	// unwanted subSchemas in schema


		StringKeyDictionary params;
		params = spec.Get("parameters", params);
		StringKeyDictionary metaParams;
		metaParams = meta.Get("parameters", metaParams);

		for (StringKeyDictionary::VariablesConstIterator pit = metaParams.Begin(); pit != metaParams.End(); pit++)
		{
			std::string paramName = *pit->first;
			StringKeyDictionary paramConstraint;
			if (!pit->second->Get(paramConstraint))
				return false; 	// error in meta file

			if (!params.Exists(paramName))
				return false; // parameter not present

			std::string type = paramConstraint.Get("type", "");
			if (type == "float")
			{
				if (!params.IsType<double>(paramName))
					return false;

				double min = paramConstraint.Get("min", (double)0.0);
				double max = paramConstraint.Get("max", (double)0.0);

				double p = params.Get(paramName, (double)0.0);
				if (p < min || p > max)
					return false;
			}
			else if (type == "integer")
			{
				if (!params.IsType<long>(paramName))
					return false;

				long min = paramConstraint.Get("min", (long)0);
				long max = paramConstraint.Get("max", (long)0);

				long p = params.Get(paramName, (long)0);
				if (p < min || p > max)
					return false;
			}
			else if (type == "boolean")
			{
				if (!params.IsType<bool>(paramName))
					return false;
			}
			else if (type == "string")
			{
				if (!params.IsType<std::string>(paramName))
					return false;
			}
			else if (type == "object")
			{
				if (!params.IsType<StringKeyDictionary>(paramName))
					return false;
			}
			else if (type == "array")
			{
				if (!params.IsType<ArrayKeyDictionary>(paramName))
					return false;
			}
			else if (type == "null")
			{
				if (!params.IsType<Variant::NullType>(paramName))
					return false;
			}
			else
			{
				return false; // unkown parameter in meta file
			}
					
		}

		for (ArrayKeyDictionary::VariablesConstIterator sit = schemas.Begin(); sit != schemas.End(); sit++)
		{
			StringKeyDictionary schemaDef;
			if (sit->second->Get(schemaDef))
			{
				std::string typeName;
				typeName = schemaDef.Get("type", typeName);
				std::string className;
				className = schemaDef.Get("class", className);

				GCPtr<ClassManager> cm;
				env->GetClassManager(typeName, cm);
				if (!cm.IsValid())
					return false;

				GCPtr<Class> cls = cm->GetClass(className);
				if (!cls.IsValid())
					return false;

				const StringKeyDictionary& metaDef = cls->GetMeta();

				if (!ValidateSchema(env, schemaDef, metaDef))
					return false;
			}
		}
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	ExpressSchema
	// Description:	expresses schema from a file definition
	// Arguments:	environment schema belongs to, parent schema, schema 
	//				definition dictionary
	// Returns:		if sucessfull
	// --------------------------------------------------------------------------
	bool Schema::ExpressSchema(const GCPtr<Environment>&env, const GCPtr<Schema>&parent, const StringKeyDictionary &spec)
	{
		ArrayKeyDictionary schemas;
		schemas = spec.Get("schemas", schemas);
		for (ArrayKeyDictionary::VariablesConstIterator sit = schemas.Begin(); sit != schemas.End(); sit++)
		{
			StringKeyDictionary schemaDef;
			if (sit->second->Get(schemaDef))
			{
				std::string typeName;
				typeName = schemaDef.Get("type", typeName);
				std::string className;
				className = schemaDef.Get("class", className);

				GCPtr<Object> object = Create(env, parent, typeName, className, schemaDef);
			}
		}
		parent->myExpressed = true;
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	Create
	// Description:	creates a derived schema type of a particular class 
	// Arguments:	environment schema belongs to, parent schema, derived schema 
	//				type name, class name of the schema, schema config specification
	// Returns:		created schema
	// --------------------------------------------------------------------------
	GCPtr<Object> Schema::Create(const GCPtr<Environment>& env, const GCPtr<Schema>& parent, const std::string& typeName, const std::string& className, const StringKeyDictionary& spec)
	{
		bool ok = true;

		StringKeyDictionary stringParameters;
		stringParameters = spec.Get("parameters", stringParameters);
		

	
		GCPtr<Object> object;
		GCPtr<ClassManager> om;
		GCPtr<Class> oc;
		env->GetClassManager(typeName, om);
		if (!om.IsValid())
		{
			ok = false;
			ERROR_TRACE("Trying to create %s but %s Manager in Realm does not exist.\n", className.c_str(), typeName.c_str());
		}
		else
		{
			oc = om->GetClass(className);
			if (!oc.IsValid())
			{
				ok = false;
				ERROR_TRACE("Trying to create %s %s but %s Class does not exist.\n", typeName.c_str(), className.c_str(), typeName.c_str());
			}
			else
			{
				object = om->CreateObject(className, env);
					
				if (!object.IsValid())
				{
					ok = false;
					ERROR_TRACE("Error trying to create %s %s.\n", typeName.c_str(), className.c_str());
				}
				else
				{
					GCPtr<Schema> schema;
					schema.DynamicCast(object);
					if (schema.IsValid())
					{
						schema->myParent = parent;
						parent->AddSchema(schema);
					}
					
					schema->myName = spec.Get("name", "__NONAME");

					// configure schema interfaces and edges
					StringKeyDictionary config;
					config = spec.Get("configuration", config);
					ok = schema->Configure(config);

					object->Initialize(env->GetScheduler(), className, stringParameters);
					env->GetScheduler()->AddVM(object->GetProcess()->GetVM());




					if (ok)
					{
						// build sub schemas
						ArrayKeyDictionary schemas;
						schemas = spec.Get("schemas", schemas);
						for (ArrayKeyDictionary::VariablesConstIterator sit = schemas.Begin(); sit != schemas.End(); sit++)
						{
							StringKeyDictionary schemaDef;
							if (sit->second->Get(schemaDef))
							{
								std::string typeName;
								typeName = schemaDef.Get("type", typeName);
								std::string className;
								className = schemaDef.Get("class", className);

								GCPtr<Object> object = Create(env, schema, typeName, className, schemaDef);
								if (!object.IsValid())
									RELEASE_TRACE("Failed to create subSchema %s %s.\n", typeName.c_str(), className.c_str());
								
							}
						}

						schema->myExpressed = true;
					}
					
				}
			}
		}

		if(ok)
		{		
			// call initialize message
			Message* msg = new Message;
			msg->myFunctionName = SoftProcess::ourInitializeMessage;
			msg->myTo = object->GetProcess();
			msg->SetCallType(Message::Decoupled);
			msg->myDestroyOnCompletion = false;
			msg->myPriority = Priority::GetSystem();

			
			VariantKeyDictionary *parameters = new VariantKeyDictionary();
			ConvertStringDictToVarientDict(stringParameters, *parameters);	
			msg->AddArgument(parameters);

			bool ok = true;
			if (!msg->SendMsg(0.0, 1))
			{
				delete msg;

				if (oc->HasFunction(SoftProcess::ourInitializeMessage))
				{
					ok = false;
					object.Destroy();
					ERROR_TRACE("Trying to initialize %s %s but %s class does not have correct Initialize function.\n", typeName.c_str(), className.c_str(), typeName.c_str());
				}
				else
				{
					object->CompleteInitialization();
				}

			}
		}
		else
		{
			object.Destroy();
		}
		return object;
	}


	// --------------------------------------------------------------------------						
	// Function:	Schema
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Schema::Schema() :
		myExpressed(false)
	{ 
	}


	// --------------------------------------------------------------------------						
	// Function:	~Schema
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Schema::~Schema()
	{
		if (myParent.IsValid())
			myParent->RemoveSchema(GCPtr<Schema>(this));
		DestorySchemas();
	}


	// --------------------------------------------------------------------------						
	// Function:	RequiresConfiguration
	// Description:	flag whether with schema requires configuration on creation
	// Arguments:	none
	// Returns:		flag
	// --------------------------------------------------------------------------
	bool Schema::RequiresConfiguration() const
	{ 
		return false; 
	}


	// --------------------------------------------------------------------------						
	// Function:	Configure
	// Description:	virtual function to configure derived schema objects
	// Arguments:	dictinary of configuration parameters
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool Schema::Configure(const StringKeyDictionary& conf) 
	{ 
		return true; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetParent
	// Description:	returns parent schema
	// Arguments:	none
	// Returns:		parent schema
	// --------------------------------------------------------------------------
	const GCPtr<Schema>& Schema::GetParent() const 
	{ 
		return myParent; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetSchemas
	// Description:	returns sub schemas owned by this schema
	// Arguments:	none
	// Returns:		schemas
	// --------------------------------------------------------------------------
	const Schema::Schemas& Schema::GetSchemas() const 
	{ 
		return mySchemas; 
	}


	// --------------------------------------------------------------------------						
	// Function:	AddSchema
	// Description:	adds a schema as sub schema owned by this schema
	// Arguments:	schema
	// Returns:		none
	// --------------------------------------------------------------------------
	void Schema::AddSchema(const GCPtr<Schema>& s)
	{
		s->myParent = GCPtr<Schema>(this);
		mySchemas.push_back(s);
		myExpressed = true;
	}


	// --------------------------------------------------------------------------						
	// Function:	RemoveSchema
	// Description:	removes a schema as sub schema owned by this schema
	// Arguments:	schema
	// Returns:		none
	// --------------------------------------------------------------------------
	void Schema::RemoveSchema(const GCPtr<Schema>& s)
	{
		Schemas::iterator it = std::find(mySchemas.begin(), mySchemas.end(), s);
		if(it != mySchemas.end())
			mySchemas.erase(it);
	}


	// --------------------------------------------------------------------------						
	// Function:	DestorySchemas
	// Description:	destorys all owned sub schemas
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Schema::DestorySchemas()
	{
		Schemas schemas = mySchemas;
		for (Schemas::iterator it = schemas.begin(); it != schemas.end(); it++)
			it->Destroy();

		mySchemas.clear();
	}


	// --------------------------------------------------------------------------						
	// Function:	GetName
	// Description:	returns name
	// Arguments:	none
	// Returns:		name
	// --------------------------------------------------------------------------
	const std::string& Schema::GetName() const
	{
		return myName;
	}


	// --------------------------------------------------------------------------						
	// Function:	IsExpressed
	// Description:	returns if this schema has been expressed this flag is so
	//				you dont try to express more than one schema file in a schema
	// Arguments:	none
	// Returns:		if expressed
	// --------------------------------------------------------------------------
	bool Schema::IsExpressed() const 
	{ 
		return myExpressed; 
	}
}