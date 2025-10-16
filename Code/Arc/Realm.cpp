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


#include "Realm.h"
#include "God.h"

#include "Registry.h"
#include "../VM/Scheduler.h"
#include "../VM/VM.h"
#include "../VM/ClassManager.h"
#include "../Schema/Agent.h"
#include "../Schema/Node.h"
#include "../LuaProcess/LuaProcess.h"
#include "../File/FileSystem.h"
#include "../Common/Exception.h"
#include "../Modules/LuaModule.h"
#include "../Modules/ProcessModule.h"
#include "../Modules/ClassifierModule.h"
#include "../Modules/EnvironmentModule.h"
#include "../Modules/SystemModule.h"
#include "../Modules/ObjectModule.h"
#include "../Modules/WholeModule.h"
#include "../Modules/NodeAuxilaryModule.h"
#include "../File/IOVariant.h"
#include <algorithm>

//! /library shh
//! Messaging and process functions aswell as variable copying, testing, debug tracing. 



namespace shh
{
	REGISTER_MODULE(LuaModule)
	REGISTER_MODULE(ClassifierModule)
	REGISTER_MODULE(ProcessModule)
	REGISTER_MODULE(EnvironmentModule)
	REGISTER_MODULE(SystemModule)
	REGISTER_OBJECT_MODULE(Agent, ClassManager)
	REGISTER_MODULE(WholeModule)
	REGISTER_OBJECT_MODULE(Node, ClassManager)
	REGISTER_MODULE(NodeAuxilaryModule)
	
	//IMPLEMENT_ARCHIVE(Realm, "Realm")
	Realm::RealmMap Realm::ourRealmMap;
	GCPtr<Realm> Realm::ourActiveRealm;
	GCPtr<LuaProcess> Realm::ourLuaTypeBase;
	unsigned int  Realm::ourVersion = 0;


	// --------------------------------------------------------------------------						
	// Function:	Create
	// Description:	creates a template realm
	// Arguments:	realm name, config dict, realm to inherit fro,
	// Returns:		created realm
	// --------------------------------------------------------------------------
	GCPtr<Realm> Realm::Create(const ::std::string& name, const StringKeyDictionary& config, const GCPtr<Realm>& other)
	{
		GCPtr<Realm> realm(new Realm(name, Privileges(), ""));
		realm->Configure(config, other);
		Registry::GetRegistry().RegisterRealm(realm);
		ourRealmMap[realm->myName] = realm;
		return realm;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetRealm
	// Description:	gets a realm
	// Arguments:	name of realm
	// Returns:		realm
	// --------------------------------------------------------------------------
	const GCPtr<Realm>& Realm::GetRealm(const ::std::string& name)
	{
		return ourRealmMap.find(name)->second;
	}


	// --------------------------------------------------------------------------						
	// Function:	DestroyRealm
	// Description:	destorys a realm
	// Arguments:	name
	// Returns:		none
	// --------------------------------------------------------------------------
	void Realm::DestroyRealm(const ::std::string& name)
	{
		RealmMap::iterator it = ourRealmMap.find(name);
		if (it == ourRealmMap.end())
			return;

		Registry::GetRegistry().UnregisterRealm(it->second);
		it->second.Destroy();
		ourRealmMap.erase(it);
	}


	// --------------------------------------------------------------------------						
	// Function:	Realm
	// Description:	constructor
	// Arguments:	name of realm, privileges it has, unqiue id
	// Returns:		none
	// --------------------------------------------------------------------------
	Realm::Realm(const std::string& name, const Privileges& privileges, const std::string& id) :
		Environment()
	{
		myId = id;
		if(!name.empty())
			myName = name;
		myPaused = true;
		myPrivileges = privileges;
		myStartTime = -1;
	}


	// --------------------------------------------------------------------------						
	// Function:	Configure
	// Description:	configures realm
	// Arguments:	config dict, realm to inherit from
	// Returns:		none
	// --------------------------------------------------------------------------
	void Realm::Configure(const StringKeyDictionary & config, const GCPtr<Realm>&other) 
	{
		StringKeyDictionary languages;
		languages = GetMeta("languages", languages);

		StringKeyDictionary scheduler;
		scheduler = config.Get("scheduler", scheduler);
		myScheduler = Scheduler::CreateScheduler(0, 10000, myPrivileges);


		// set up scheduler and base processes
		GCPtr<LuaProcess> schemaBaseLua;
		GCPtr<LuaProcess> agentBaseLua;
		GCPtr<LuaProcess> masterBaseLua;
		GCPtr<LuaProcess> slaveBaseLua;
		GCPtr<LuaProcess> basicBaseLua;

		StringKeyDictionary locals;
		locals = config.Get("locals", locals);
		StringKeyDictionary updateDict;
		updateDict = config.Get("updater", updateDict);
		

		if (other.IsValid())
		{
			if (myName.empty())
				myName = other->myName;

			myBootPath = other->myBootPath;
			myUpdatePath = other->myUpdatePath;

			myUpdaterDict = other->myUpdaterDict;
			myUpdaterDict.Merge(updateDict);
			myLocalConfig = other->myLocalConfig;
			myLocalConfig.Merge(locals);

			myVM = GCPtr<VM>(new VM(other->myVM));


			StringKeyDictionary languages;
			languages = God::ourConfigFileDict.Get("languages", languages);
			for (StringKeyDictionary::VariablesConstIterator lit = languages.Begin(); lit != languages.End(); lit++)
			{
				std::string language = *lit->first;
				if (language == "lua")
				{

					GCPtr<Process> otherProcess;

					if (other->myScheduler->GetBaseProcess(Lua, SchemaPrivilege, otherProcess))
						schemaBaseLua = otherProcess->Clone();
					else
						schemaBaseLua = LuaProcess::Create(SchemaPrivilege);
					myScheduler->SetBaseProcess(Lua, schemaBaseLua);


					if (other->myScheduler->GetBaseProcess(Lua, AgentPrivilege, otherProcess))
						agentBaseLua = otherProcess->Clone();
					else
						agentBaseLua = LuaProcess::Create(AgentPrivilege);
					myScheduler->SetBaseProcess(Lua, agentBaseLua);


					if (other->myScheduler->GetBaseProcess(Lua, MasterPrivilege, otherProcess))
						masterBaseLua = otherProcess->Clone();
					else
						masterBaseLua = LuaProcess::Create(MasterPrivilege);
					myScheduler->SetBaseProcess(Lua, masterBaseLua);


					if (other->myScheduler->GetBaseProcess(Lua, SlavePrivilege, otherProcess))
						slaveBaseLua = otherProcess->Clone();
					else
						slaveBaseLua = LuaProcess::Create(SlavePrivilege);
					myScheduler->SetBaseProcess(Lua, slaveBaseLua);


					if (other->myScheduler->GetBaseProcess(Lua, BasicPrivilege, otherProcess))
						basicBaseLua = otherProcess->Clone();
					else
						basicBaseLua = LuaProcess::Create(BasicPrivilege);
					myScheduler->SetBaseProcess(Lua, basicBaseLua);

				}

			}

			for (ClassManagers::iterator cit = other->myClassManagers.begin(); cit != other->myClassManagers.end(); cit++)
			{
				myClassManagers[cit->first] = ClassManager::CreateManager(cit->second);
			}
		}
		else
		{
			myVM = GCPtr<VM>(new VM());

			myLocalConfig = locals;
			myUpdaterDict = updateDict;


			schemaBaseLua = LuaProcess::Create(SchemaPrivilege, Realm::ourLuaTypeBase);
			myScheduler->SetBaseProcess(Lua, schemaBaseLua);
			agentBaseLua = LuaProcess::Create(AgentPrivilege, Realm::ourLuaTypeBase);
			myScheduler->SetBaseProcess(Lua, agentBaseLua);
			masterBaseLua = LuaProcess::Create(MasterPrivilege, Realm::ourLuaTypeBase);
			myScheduler->SetBaseProcess(Lua, masterBaseLua);
			slaveBaseLua = LuaProcess::Create(SlavePrivilege, Realm::ourLuaTypeBase);
			myScheduler->SetBaseProcess(Lua, slaveBaseLua);
			basicBaseLua = LuaProcess::Create(BasicPrivilege, Realm::ourLuaTypeBase);
			myScheduler->SetBaseProcess(Lua, basicBaseLua);
		}

		ArrayKeyDictionary schemas;
		schemas = config.Get("schemas", schemas);


		for (ArrayKeyDictionary::VariablesConstIterator sit = schemas.Begin(); sit != schemas.End(); sit++)
		{
			StringKeyDictionary schema;
			if (sit->second->Get(schema))
			{
				std::string type;
				type = schema.Get("type", type);
				if (!type.empty())
				{
					GCPtr<ClassManager> om;
					if (!GetClassManager(type, om))
					{
						if (type == "Agent")
							myClassManagers[type] = ClassManager::CreateManager(myPrivileges, type, agentBaseLua);
						else
							myClassManagers[type] = ClassManager::CreateManager(myPrivileges, type, schemaBaseLua);
					}
				}
			}
		}


		for (StringKeyDictionary::VariablesConstIterator lit = languages.Begin(); lit != languages.End(); lit++)
		{
			std::string language = *lit->first;

			const Registry::NamedObjectInstantiators &oi = Registry::GetRegistry().GetObjectInstantiators(Lua);
			for (Registry::NamedObjectInstantiators::const_iterator oit = oi.begin(); oit != oi.end(); oit++)
			{
				GCPtr<ClassManager> om;
				if (GetClassManager(oit->first, om))
					om->AddInstantiator(oit->second);
			}
		}


		StringKeyDictionary engineClasses;
		engineClasses = config.Get("engine_classes", engineClasses);
		for (StringKeyDictionary::VariablesConstIterator eit = engineClasses.Begin(); eit != engineClasses.End(); eit++)
		{
			std::string typeName = *eit->first;
			GCPtr<ClassManager> cm;
			if (GetClassManager(typeName, cm))
			{
				ArrayKeyDictionary typeClasses;
				if (eit->second->Get(typeClasses))
				{
					for (ArrayKeyDictionary::VariablesConstIterator tit = typeClasses.Begin(); tit != typeClasses.End(); tit++)
					{
						std::string className;
						if (tit->second->Get(className))
						{
							GCPtr<Class> cls = Registry::GetRegistry().GetHardClass(typeName, className);
							if (cls.IsValid())
								cm->AddClass(cls);
						}
					}
				}
			}
		}
		myScheduler->AddVM(myVM);
		

		// set up Realm updater
		std::string updatefile = config.Get("updater", "default");
		std::string updatepath;
		FileSystem::GetVariable("UPDATERS", updatepath);
		
		typedef std::map<std::string, std::vector<std::string>> SchemaPaths;
		SchemaPaths schemaPaths;
		for (ArrayKeyDictionary::VariablesConstIterator sit = schemas.Begin(); sit != schemas.End(); sit++)
		{
			StringKeyDictionary schema;
			if (sit->second->Get(schema))
			{
				std::string type;
				type = schema.Get("type", type);
				if (!type.empty())
				{
					std::string upper = type;
					std::transform(upper.begin(), upper.end(), upper.begin(),
						[](unsigned char c) { return std::toupper(c); });

					SchemaPaths::iterator sp = schemaPaths.find(type);
					if (sp == schemaPaths.end())
					{
						schemaPaths[type] = std::vector<std::string>();
						sp = schemaPaths.find(type);
					}
					SoftProcess::Paths& pathVector = sp->second;

					ArrayKeyDictionary paths;
					paths = schema.Get("paths", paths);
					for (ArrayKeyDictionary::VariablesConstIterator pit = paths.Begin(); pit != paths.End(); pit++)
					{
						std::string subdir;
						if (pit->second->Get(subdir))
						{
							std::string path;
							FileSystem::GetVariable(upper, path);
							path += "/" + subdir;
							pathVector.push_back(path);
						}
					}
				}
			}
		}



		// get additional script paths

		StringKeyDictionary boot;
		boot = config.Get("boot", boot);

		ArrayKeyDictionary bootPaths;
		bootPaths = boot.Get("paths", bootPaths);
		if (myBootPath.empty())
		{
			std::string path;
			if(FileSystem::GetVariable("BOOT", path))
				myBootPath.push_back(path);
		}
		for (ArrayKeyDictionary::VariablesConstIterator bit = bootPaths.Begin(); bit != bootPaths.End(); bit++)
		{
			std::string subdir;
			if (bit->second->Get(subdir))
			{
				std::string path;
				if (FileSystem::GetVariable("BOOT", path))
				{
					path += "/" + subdir;
					myBootPath.push_back(path);
				}
			}
		}

		StringKeyDictionary update;
		update = config.Get("update", update);

		StringKeyDictionary updatePaths;
		updatePaths = update.Get("paths", updatePaths);
		if(myUpdatePath.empty())
		{
			std::string path;
			if (FileSystem::GetVariable("UPDATE", path))
				myUpdatePath.push_back(path);
		}

		for (StringKeyDictionary::VariablesConstIterator uit = updatePaths.Begin(); uit != updatePaths.End(); uit++)
		{
			std::string subdir;
			if (uit->second->Get(subdir))
			{
				std::string path;
				if (FileSystem::GetVariable("UPDATE", path))
				{
					path += "/" + subdir;
					myUpdatePath.push_back(path);
				}
			}
		}

		StringKeyDictionary script;
		script = config.Get("script", script);

		std::vector<std::string> scriptPathVector;
		ArrayKeyDictionary scriptPaths;
		scriptPaths = script.Get("paths", scriptPaths);
		for (ArrayKeyDictionary::VariablesConstIterator sit = scriptPaths.Begin(); sit != scriptPaths.End(); sit++)
		{
			std::string subdir;
			if (sit->second->Get(subdir))
			{
				std::string path;
				FileSystem::GetVariable("SCRIPT", path);
				path += "/" + subdir;
				scriptPathVector.push_back(path);
			}
		}




		//get up modules
		StringKeyDictionary modules;
		modules = config.Get("modules", modules);
		


		// add additional paths and modules to base processes
		for (StringKeyDictionary::VariablesConstIterator lit = languages.Begin(); lit != languages.End(); lit++)
		{
			
			std::string language = *lit->first;

			if (language == "lua")
			{
				for(int p = 0;  p != scriptPathVector.size(); p++)
					masterBaseLua->AddScriptPath(scriptPathVector[p] + "/Lua");
					
				for (int p = 0; p != scriptPathVector.size(); p++)
					slaveBaseLua->AddScriptPath(scriptPathVector[p] + "/Lua");
					
				for (int p = 0; p != scriptPathVector.size(); p++)
					basicBaseLua->AddScriptPath(scriptPathVector[p] + "/Lua");
					
					
				StringKeyDictionary reg;
				reg = config.Get("modules", reg);
				StringKeyDictionary::OrderedVariables::const_iterator it = reg.OrderedBegin();
				StringKeyDictionary::OrderedVariables::const_iterator end = reg.OrderedEnd();
				while (it != end)
				{
					const std::string moduleName = *it->first.myKey;
					StringKeyDictionary modSpec;
					if (it->second->Get(modSpec))
					{
						std::string alias;
						alias = modSpec.Get("alias", alias);
						StringKeyDictionary dict;
						dict = modSpec.Get("config", dict);
						ArrayKeyDictionary reg;
						reg = modSpec.Get("register", reg);
							
						for (ArrayKeyDictionary::VariablesConstIterator rit = reg.Begin(); rit != reg.End(); rit++)
						{
							for (StringKeyDictionary::VariablesConstIterator lit = languages.Begin(); lit != languages.End(); lit++)
							{
								std::string language = *lit->first;
								if (language == "lua")
								{
									std::string process;
									rit->second->Get(process);
									if(process ==  "node")
										Registry::GetRegistry().RegisterModuleInProcess(schemaBaseLua, moduleName, alias, dict);
									else if (process == "agent")
										Registry::GetRegistry().RegisterModuleInProcess(agentBaseLua, moduleName, alias, dict);
									else if (process == "master")
										Registry::GetRegistry().RegisterModuleInProcess(masterBaseLua, moduleName, alias, dict);
									else if (process == "slave")
										Registry::GetRegistry().RegisterModuleInProcess(slaveBaseLua, moduleName, alias, dict);
									else if (process == "basic")
										Registry::GetRegistry().RegisterModuleInProcess(basicBaseLua, moduleName, alias, dict);
								}
							}
						}
						it++;
					}
				}				
			}
		}
			
		for(ClassManagers::iterator cit = myClassManagers.begin(); cit != myClassManagers.end(); cit++)
		{
			std::string typeName = cit->first;
			GCPtr<ClassManager> om = cit->second;
			SchemaPaths::iterator pit = schemaPaths.find(typeName);
			if (pit != schemaPaths.end())
			{
				std::vector<std::string>& sp = pit->second;
				for (int p = 0; p != sp.size(); p++)
				{
					om->ScanClasses(Lua, sp[p], true, true, typeName);
				}
				om->BuildHierachry(myVM);
			}
		}
		
	}


	// --------------------------------------------------------------------------						
	// Function:	~Realm
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Realm::~Realm()
	{
		GCPtr<Realm> me(this);
		RealmMap::iterator it = ourRealmMap.find(myName);
		if (it != ourRealmMap.end())
			ourRealmMap.erase(it);
	}


	// --------------------------------------------------------------------------						
	// Function:	Initialize
	// Description:	initializes god of first creation
	// Arguments:	owning object, unique id of realm, dictionary of config vars
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool Realm::Initialize(const GCPtr<GCObject>& owner, const std::string& id, const StringKeyDictionary& sd)
	{
		try
		{
			bool ok = true;
			myTime = 0;
			myPaused = false;
			SetAsActiveRealm();	

			// run boot script;
			std::string bootScipt;
			bootScipt = sd.Get("boot_script", bootScipt);
			if (!bootScipt.empty())
			{
				
				GCPtr<Process> process = myVM->CreateMasterProcess(Lua);
				process->SetCurrentEnvironment(GCPtr<Realm>(this));

				GCPtr<SoftProcess> sp;
				sp.StaticCast(process);
				std::vector<std::string> paths;
				for (int b = 0; b != myBootPath.size(); b++)
					paths.push_back(myBootPath[b] + "/Lua");

				myVM->InitializeMasterProcess();
				if (sp->Execute(bootScipt, true, false, paths))
				{
					Message msg;
					msg.myFunctionName = "shhMain";
					msg.myTo = sp;
					VariantKeyDictionary *vd = new VariantKeyDictionary();
					for (StringKeyDictionary::VariablesConstIterator vit = sd.Begin(); vit != sd.End(); vit++)
					{
						NonVariant<std::string> s(*vit->first);
						vd->Set(s, *vit->second);
					}

					msg.AddArgument(vd);
					msg.Call(false, 0, false);
				}
				else
				{
					ok = false;
				}
				process.Destroy();
				
			}
			
			StringKeyDictionary subupdaters;
			subupdaters = myUpdaterDict.Get("subupdater", subupdaters);
			for (StringKeyDictionary::VariablesConstIterator it = subupdaters.Begin(); it != subupdaters.End(); it++)
			{
				StringKeyDictionary conf;
				if (it->second->Get(conf))
				{

					std::string name = conf.Get("module", "");
					if (!name.empty())
					{
						GCPtr<Module> module = Registry::GetRegistry().GetModule(name);
						if (module.IsValid())
						{
							std::string id = conf.Get("id", "");
							if (id.empty())
								id = name;

							mySubModules[id] = module->Clone();
						}
					}

				}
			}
			
			std::string updateScipt;
			updateScipt = sd.Get("update_script", updateScipt);
			if (!updateScipt.empty())
			{
				
				GCPtr<Process> process = myVM->CreateMasterProcess(Lua);
				process->SetCurrentEnvironment(GCPtr<Realm>(this));
				GCPtr<Process> oldProcess = myScheduler->GetCurrentProcess();
				myScheduler->SetCurrentProcess(process);
				
				GCPtr<Module> me(this);
				if (Scheduler::GetCurrentProcess()->GetImplementation() == Lua)
				{
					LuaApi::OpenNamespace("shh");
					RegisterLuaFunction("InitializeModule", InitializeModule, 3, me);
					RegisterLuaFunction("FinalizeModule", FinalizeModule, 2, me);
					RegisterLuaFunction("UpdateModule", UpdateModule, 4, me);
					RegisterLuaFunction("UpdateScheduler", UpdateScheduler, 3, me);
					LuaApi::CloseNamespace();
				}
				myScheduler->SetCurrentProcess(oldProcess);

				

				GCPtr<SoftProcess> sp;
				sp.StaticCast(process);
				std::vector<std::string> paths;
				for (int u = 0; u != myUpdatePath.size(); u++)
					paths.push_back(myUpdatePath[u] + "/Lua");

				myVM->InitializeMasterProcess();

				if (sp->Execute(updateScipt, true, false, paths))
				{
					Message msg;
					msg.myFunctionName = "shhInitialize";
					msg.myTo = sp;
					VariantKeyDictionary* vd = new VariantKeyDictionary();
					for (StringKeyDictionary::VariablesConstIterator vit = sd.Begin(); vit != sd.End(); vit++)
					{
						NonVariant<std::string> s(*vit->first);
						vd->Set(s, *vit->second);
					}

					msg.AddArgument(vd);
					msg.Call(false, 0, false);
					myUpdaterProcess = sp;
				}
				else
				{
					ok = false;
				}		
			}
			else
			{
				if (!Environment::Initialize(owner, id, sd))
					ok = false;
			}
			SetAsInactiveRealm();
			return ok;
		}
		catch (...)
		{
			return false;
		}

	}


	// --------------------------------------------------------------------------						
	// Function:	Update
	// Description:	updates
	// Arguments:	time to update until, update phase
	// Returns:		if successfull
	// --------------------------------------------------------------------------
	bool Realm::Update(double until, unsigned int phase)
	{
		if (myPaused)
			return false;

		bool ok = true;

		SetAsActiveRealm();
		if (myUpdaterProcess.IsValid())
		{
			Message msg;
			msg.myFunctionName = "shhUpdate";
			msg.myTo = myUpdaterProcess;
			msg.AddArgument(new double(until));
			msg.Call(false, 0, false);
		}
		else
		{
			if (!Environment::Update(until, phase))
				ok = false;
		}
		SetAsInactiveRealm();

		return ok;
	}


	// --------------------------------------------------------------------------						
	// Function:	Finalize
	// Description:	finalizes module before deletion
	// Arguments:	pointer to self (most derived class) needed for multiple
	//				inheritance
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool Realm::Finalize(GCObject* me)
	{
		if (myUpdaterProcess.IsValid())
		{
			Message msg;
			msg.myFunctionName = "shhFinalize";
			msg.myTo = myUpdaterProcess;
			msg.Call(false, 0, false);
			myUpdaterProcess.Destroy();
			return true;
		}
		else
		{
			return Module::Finalize(me);
		}
	}

	// --------------------------------------------------------------------------						
	// Function:	SetAsActiveRealm
	// Description:	makes active
	// Arguments:	none
	// Returns:		if sucessfull
	// --------------------------------------------------------------------------
	bool Realm::SetAsActiveRealm()
	{
		GCPtr<Realm> active = ourActiveRealm;

		if (this == active.GetObject())
		{
			return false;
		}
		else
		{
			ourActiveRealm = GCPtr<Realm>(this);
			for (Module::Map::iterator m = mySubModules.begin(); m != mySubModules.end(); m++)
				m->second->SetActive(ourActiveRealm);
			return true;
		}
		
	}


	// --------------------------------------------------------------------------						
	// Function:	SetAsInactiveRealm
	// Description:	makes inactive
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Realm::SetAsInactiveRealm()
	{
		for (Module::Map::iterator m = mySubModules.begin(); m != mySubModules.end(); m++)
			m->second->SetAsInactive();

		ourActiveRealm.SetNull();
	}


	// --------------------------------------------------------------------------						
	// Function:	GetActiveRealm
	// Description:	gets active realm
	// Arguments:	none
	// Returns:		realm
	// --------------------------------------------------------------------------
	const GCPtr<Realm>& Realm::GetActiveRealm()
	{ 
		return ourActiveRealm; 
	}


	// --------------------------------------------------------------------------						
	// Function:	IsActive
	// Description:	returns if this is active
	// Arguments:	none
	// Returns:		bool
	// --------------------------------------------------------------------------
	bool Realm::IsActive() const
	{
		return GetActiveRealm().GetObject() == this;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetName
	// Description:	returns name
	// Arguments:	none
	// Returns:		name
	// --------------------------------------------------------------------------
	std::string Realm::GetName() const
	{ 
		return myName;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetTime
	// Description:	returns current realm time
	// Arguments:	none
	// Returns:		time
	// --------------------------------------------------------------------------
	double Realm::GetTime()
	{
		return myTime;
	}


	// --------------------------------------------------------------------------						
	// Function:	Pause
	// Description:	pauses updates
	// Arguments:	pause state
	// Returns:		none
	// --------------------------------------------------------------------------
	void Realm::Pause(bool paused)
	{
		myPaused = paused;
		myLastUpdateTime = myTime;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetPaused
	// Description:	retuns pause state
	// Arguments:	none
	// Returns:		bool
	// --------------------------------------------------------------------------
	bool Realm::GetPaused()
	{
		return myPaused;
	}


	// --------------------------------------------------------------------------						
	// Function:	GertStepSize
	// Description:	returns update set size for this realm
	// Arguments:	none
	// Returns:		step size
	// --------------------------------------------------------------------------
	double Realm::GetStepSize()
	{
		return 1000.0;
	}


	// --------------------------------------------------------------------------						
	// Function:	Read
	// Description:	serialization in
	// Arguments:	io stream interface, version to log as
	// Returns:		none
	// --------------------------------------------------------------------------
	void Realm::Read(IOInterface& io, int version)
	{
		//INITIALIZE_SERIAL_IN(io);

		//SERIALIZE_IN(io, myLocalConfig)		
		//Realm::ReadStatic(io, version);

		//FINALIZE_SERIAL(io);
		
	}


	// --------------------------------------------------------------------------						
	// Function:	Write
	// Description:	serialization out
	// Arguments:	io stream interface, version to log as
	// Returns:		none
	// --------------------------------------------------------------------------
	void Realm::Write(IOInterface& io, int version) const
	{
		//INITIALIZE_SERIAL_OUT(io);

		//SERIALIZE_OUT(io, myLocalConfig);
		//Realm::WriteStatic(io, version);

		//FINALIZE_SERIAL(io);

	}


	// --------------------------------------------------------------------------						
	// Function:	CloseDown
	// Description:	static function to destory all realm templates
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Realm::CloseDown()
	{
		while (!ourRealmMap.empty())
		{
			RealmMap::iterator it = ourRealmMap.begin();
			GCPtr<Realm> r = it->second;
			r.Destroy();
		}
		ourRealmMap.clear();
		ourActiveRealm.SetNull();
	}


	//! /namespace shh
	//! /function InitializeModule
	//! /privilege Updater
	//! /param string module_id
	//! /param table config_dictionary
	//! /return bool
	//! Used by updater scripts to make a initialize a module.
	ExecutionState Realm::InitializeModule(std::string& id, VariantKeyDictionary &vd, bool& result)
	{
		StringKeyDictionary sd;
		for (VariantKeyDictionary::VariablesConstIterator vit = vd.Begin(); vit != vd.End(); vit++)
		{
			std::string s;
			if(vit->first->Get(s))
				sd.Set(s, *vit->second);
		}

		GCPtr<Environment> env = Scheduler::GetCurrentProcess()->GetCurrentEnvironment();
		result = env->InitializeSubModule(id, sd);
		return ExecutionOk;
	}


	//! /namespace shh
	//! /function FinalizeModule
	//! /privilege Updater
	//! /param string module_id
	//! /return bool 
	//! Used by updater scripts finalize a module.
	ExecutionState Realm::FinalizeModule(std::string& id, bool& result)
	{
		GCPtr<Environment> env = Scheduler::GetCurrentProcess()->GetCurrentEnvironment();
		result = env->FinalizeSubModule(id);
		return ExecutionOk;
	}


	//! /namespace shh
	//! /function UpdateModule
	//! /privilege Updater
	//! /param string module_id
	//! /param double time_to_update_until
	//! /param integer phase_of_update
	//! /return bool 
	//! Used by updater scripts update a module.
	ExecutionState Realm::UpdateModule(std::string &id, double &until, unsigned int &phase, bool &result)
	{
		GCPtr<Environment> env = Scheduler::GetCurrentProcess()->GetCurrentEnvironment();
		result = env->UpdateSubModule(id, until, phase);
		return ExecutionOk;
	}


	//! /namespace shh
	//! /function UpdateScheduler
	//! /privilege Updater
	//! /param double time_to_update_until
	//! /param integer phase_of_update
	//! /return bool
	//! Used by updater scripts update a scheduler.
	ExecutionState Realm::UpdateScheduler(double& until, unsigned int& phase, bool& result)
	{
		GCPtr<Process> process = Scheduler::GetCurrentProcess();
		GCPtr<Environment> env = process->GetCurrentEnvironment();
		GCPtr<Realm> realm;
		realm.DynamicCast(env);
		realm->myScheduler->Update(until, phase);
		Scheduler::SetCurrentProcess(process);
		result = true;
		return ExecutionOk;
	}

}
