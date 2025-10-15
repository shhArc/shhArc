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


#include "God.h"
#include "../Common/Exception.h"
#include "Registry.h"
#include "../VM/VM.h"
#include "../VM/Scheduler.h"
#include "../VM/ClassManager.h"
#include "../File/FileSystem.h"
#include "../File/ConfigurationFile.h"
#include "../File/JsonFile.h"
#include "../LuaProcess/LuaProcess.h"
#include "../LuaProcess/LuaLibrary.h"
#include "../Schema/Node.h"
#include "../Schema/Agent.h"
#include "../Config/MemoryDefines.h"

#ifdef _WIN64
#include <windows.h>
#endif

namespace shh {

	//IMPLEMENT_ARCHIVE(God, "God")



	std::string God::ourCommandLineArgs;
	std::string God::ourBootFileName;
	std::string God::ourConfigFileName;
	std::string God::ourGodRealm;
	StringKeyDictionary God::ourConfigFileDict;
	GCPtr<God> God::ourGod;
	unsigned int God::ourVersion = 0;


	// --------------------------------------------------------------------------						
	// Function:	GetGod
	// Description:	returns the God environment (there can be only one)
	// Arguments:	none
	// Returns:		god
	// --------------------------------------------------------------------------
	const GCPtr<God>& God::GetGod()
	{
		if (!ourGod.IsValid())
		{
			God::Boot();
			ourGod = GCPtr<God>(new God());
			ourGod->Configure(ourConfigFileDict, Realm::GetRealm(ourGodRealm));
			Environment::SetGlobalEnvironment(ourGod);
		}

		return ourGod;
	}


	// --------------------------------------------------------------------------						
	// Function:	DestroyGod
	// Description:	destroys the God environment (there can be only one)
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void God::DestroyGod()
	{
		if (!ourGod.IsValid())
			return;

		ourGod.Destroy();
	}


	// --------------------------------------------------------------------------						
	// Function:	God
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	God::God() :
		Realm("", GodPrivilege, "__GOD")
	{
	}


	// --------------------------------------------------------------------------						
	// Function:	~God
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	God::~God()
	{
	}


	// --------------------------------------------------------------------------						
	// Function:	Boot
	// Description:	initial static boot function called once at startup of shhArc
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void God::Boot()
	{

		// add instantiator for lua scripted nodes
		Registry::ObjectInstantiator luaNodeInstantiator;
		luaNodeInstantiator.myImplementation = Lua;
		luaNodeInstantiator.myClassesSpecifier = LuaProcess::ClassesSpecifier;
		luaNodeInstantiator.myProcessConstructor = LuaProcess::Create;
		luaNodeInstantiator.myObjectConstructor = shh::Node::Create;
		Registry::GetRegistry().RegisterObjectInstantiator("Node", luaNodeInstantiator);

		// add instantiator for hard engine nodes
		Registry::ObjectInstantiator engineNodeInstantiator;
		engineNodeInstantiator.myImplementation = Engine;
		engineNodeInstantiator.myClassesSpecifier = Process::ClassesSpecifier;
		engineNodeInstantiator.myProcessConstructor = Process::Create;
		engineNodeInstantiator.myObjectConstructor = Node::Create;
		Registry::GetRegistry().RegisterObjectInstantiator("Node", engineNodeInstantiator);

		// add instantiator for lua agents
		Registry::ObjectInstantiator luaAgentInstantiator;
		luaAgentInstantiator.myImplementation = Lua;
		luaAgentInstantiator.myClassesSpecifier = LuaProcess::ClassesSpecifier;
		luaAgentInstantiator.myProcessConstructor = LuaProcess::Create;
		luaAgentInstantiator.myObjectConstructor = Agent::Create;
		Registry::GetRegistry().RegisterObjectInstantiator("Agent", luaAgentInstantiator);

		// get config file name
		int switchPos = (int)ourCommandLineArgs.find("-config");
		if (switchPos < ourCommandLineArgs.size())
		{
			int startPos = (int)ourCommandLineArgs.find_first_of(" ", switchPos) + 1;
			int endPos = (int)ourCommandLineArgs.find_first_of(" ", startPos);
			ourConfigFileName = ourCommandLineArgs.substr(startPos, endPos - startPos);
		}
		else
		{
			ourConfigFileName = "default.cfg";
		}


		// get boot file name
		switchPos = (int)ourCommandLineArgs.find("-boot");
		if (switchPos < ourCommandLineArgs.size())
		{
			int startPos = (int)ourCommandLineArgs.find_first_of(" ", switchPos) + 1;
			int endPos = (int)ourCommandLineArgs.find_first_of(" ", startPos);
			ourBootFileName = ourCommandLineArgs.substr(startPos, endPos - startPos);
		}
		else
		{
			ourBootFileName = "default.lua";
		}	


		// get and set root directory
		std::string root;
		switchPos = (int)ourCommandLineArgs.find("-root");
		if (switchPos < ourCommandLineArgs.size())
		{
			int startPos = (int)ourCommandLineArgs.find_first_of(" ", switchPos) + 1;
			int endPos = (int)ourCommandLineArgs.find_first_of(" ", startPos);
			root = ourCommandLineArgs.substr(startPos, endPos - startPos);
		}
		else
		{
			FileSystem::GetWorkingDirectory(root);
		}
		FileSystem::SetVariable("ROOT", root);


		// set os folders
		std::string home;
		if (!FileSystem::GetHomeDirectory(home))
			home = "%ROOT%";
		FileSystem::SetVariable("HOME", home);

		std::string mydocuments;
		if (!FileSystem::GetMyDocumentsDirectory(mydocuments))
			mydocuments = home;
		FileSystem::SetVariable("MYDOCUMENTS", mydocuments);

		std::string desktop;
		if (!FileSystem::GetDesktopDirectory(desktop))
			desktop = home;
		FileSystem::SetVariable("DESKTOP", desktop);



		// set soft paths
		StringKeyDictionary pathsFile;
		IBinaryFile pathIn( root + "/paths.cfg", IOInterface::In);
		JsonFile::Read(pathIn, pathsFile);

		ArrayKeyDictionary paths;		
		paths = pathsFile.Get("paths", paths);
		ArrayKeyDictionary::OrderedVariables::const_iterator pit = paths.OrderedBegin();
		ArrayKeyDictionary::OrderedVariables::const_iterator pend = paths.OrderedEnd();
		while (pit != pend)
		{
			StringKeyDictionary path;
			path = paths.GetFromIt(pit, path);
			StringKeyDictionary::OrderedVariables::const_iterator it = path.OrderedBegin();
			StringKeyDictionary::OrderedVariables::const_iterator end = path.OrderedEnd();
			while (it != end)
			{
				std::string str;
				FileSystem::GetVariable(*it->first, str);
				FileSystem::SetVariable(*it->first, path.GetFromIt(it, str));
				it++;
			}
			pit++;
		}

		// set read paths
		ArrayKeyDictionary read;
		read = pathsFile.Get("read", read);
		pit = read.OrderedBegin();
		pend = read.OrderedEnd();
		while (pit != pend)
		{
			std::string str;
			pit->second->Get(str);
			if (!str.empty())
				FileSystem::AddReadPath(str);
			pit++;
		}

		// set write paths
		ArrayKeyDictionary write;
		write = pathsFile.Get("write", write);
		pit = write.OrderedBegin();
		pend = write.OrderedEnd();
		while (pit != pend)
		{
			std::string str;
			pit->second->Get(str); 
			if (!str.empty())
				FileSystem::AddWritePath(str);
			pit++;
		}


		// load config file
		std::string configDir;
		FileSystem::GetVariable("CONFIG", configDir);
		BinaryFile configIn(configDir + "/" + ourConfigFileName, IOInterface::In);
		JsonFile::Read(configIn, ourConfigFileDict);

		Environment::SetMetaVariables(ourConfigFileDict);

		StringKeyDictionary memory;
		memory = ourConfigFileDict.Get("memory", memory);
		CONFIGURE_MEMORYMANAGEMENT(memory);
		

		StringKeyDictionary priorities;
		priorities = God::ourConfigFileDict.Get("priorities", priorities);
		for (StringKeyDictionary::VariablesConstIterator pit = priorities.Begin(); pit != priorities.End(); pit++)
		{
			std::string id = *pit->first;
			long value;
			if (pit->second->Get(value))
				Priority::SetPriority(id, (int)value);
		}
		Priority::Freeze();

		StringKeyDictionary engine;
		engine = ourConfigFileDict.Get("engine", engine);
		Scheduler::SetMinDelay(engine.Get("messenge_min_delay", Scheduler::GetMinDelay()));
		Scheduler::ourMaxMessagesPerUpdate = (unsigned int)engine.Get("max_scheduler_messages", (long)Scheduler::ourMaxMessagesPerUpdate);
		Messenger::ourMaxMessagesPerUpdate = (unsigned int)engine.Get("max_messenger_messages", (long)Messenger::ourMaxMessagesPerUpdate);
	

		Privileges privileges;
		if (engine.ReadPrivileges("excluded_messengers", privileges))
			SoftProcess::ourExcludedSendMessengers = privileges;
		if (engine.ReadPrivileges("excluded_executers", privileges))
			SoftProcess::ourExcludedExecuters = privileges;




		StringKeyDictionary types;
		types = ourConfigFileDict.Get("types", types);
		static StringKeyDictionary typeDict;
		typeDict = types.Get("lua", typeDict);
		GCPtr<Scheduler> scheduler = Scheduler::CreateScheduler(0, 10000, BasicPrivilege);
		ourLuaTypeBase = LuaProcess::Create(BasicPrivilege);
		
		StringKeyDictionary languages;
		languages = ourConfigFileDict.Get("languages", languages);
		for (StringKeyDictionary::VariablesConstIterator lit = languages.Begin(); lit != languages.End(); lit++)
		{
			std::string language = *lit->first;
			if (language == "lua")
				Registry::GetRegistry().RegisterModuleTypes(ourLuaTypeBase, typeDict);
		}
		Registry::GetRegistry().SetCastableTypes();
		scheduler.Destroy();

		StringKeyDictionary libraries;
		libraries = ourConfigFileDict.Get("libraries", libraries);
		for (StringKeyDictionary::VariablesConstIterator lit = languages.Begin(); lit != languages.End(); lit++)
		{
			std::string language = *lit->first;
			if (language == "lua")
			{
				LuaClearLibs();
				ArrayKeyDictionary luaLib;
				luaLib = libraries.Get("lua", luaLib);
				for (ArrayKeyDictionary::VariablesConstIterator llit = luaLib.Begin(); llit != luaLib.End(); llit++)
				{
					std::string name;
					if (llit->second->Get(name))
					{
						if (name == "base")
							LuaAddLib(LUA_GNAME, luaopen_base);
						else if (name == "loadlib")
							LuaAddLib(LUA_LOADLIBNAME, luaopen_package);
						else if (name == "coroutine")
							LuaAddLib(LUA_COLIBNAME, luaopen_coroutine);
						else if (name == "table")
							LuaAddLib(LUA_TABLIBNAME, luaopen_table);
						else if (name == "io")
							LuaAddLib(LUA_IOLIBNAME, luaopen_io);
						else if (name == "os")
							LuaAddLib(LUA_OSLIBNAME, luaopen_os);
						else if (name == "string")
							LuaAddLib(LUA_STRLIBNAME, luaopen_string);
						else if (name == "math")
							LuaAddLib(LUA_MATHLIBNAME, luaopen_math);
						else if (name == "utf8")
							LuaAddLib(LUA_UTF8LIBNAME, luaopen_utf8);
					}
				}
			}
		}

		// read realms to clone from
		StringKeyDictionary realms;
		realms = ourConfigFileDict.Get("realms", realms);
		RELEASE_ASSERT(realms.Size() > 0);

		Realm::DictMap unorderedRealms;
		StringKeyDictionary::OrderedVariables::const_iterator it = realms.OrderedBegin();
		StringKeyDictionary::OrderedVariables::const_iterator end = realms.OrderedEnd();
		while (it != end)
		{
			std::string name = *it->first;
			StringKeyDictionary r;
			it->second->Get(r);
			unorderedRealms[name] = r;
			it++;
		}

		// build realms to clone from
		BuildHierachry(unorderedRealms);
	
	}


	// --------------------------------------------------------------------------						
	// Function:	BuildHierachry
	// Description:	builds inheritance hierachy of realms (called by Boot()
	// Arguments:	all read realms
	// Returns:		if successfull
	// --------------------------------------------------------------------------
	bool God::BuildHierachry(Realm::DictMap& unorderedRealms)
	{
		// get all classes that are not derived 
		Realm::DictMap::iterator it = unorderedRealms.begin();
		Realm::DictMap::iterator next = it;
		while (it != unorderedRealms.end())
		{
			next++;
			std::string parent;
			parent = it->second.Get("parent", parent);
			if (parent.empty())
			{
				Realm::Create(it->first, it->second, GCPtr<Realm>());
				unorderedRealms.erase(it);
			}
			it = next;

		}

		// build hierachy from bases

		it = unorderedRealms.begin();
		next = it;

		bool created = true;
		while (created && !unorderedRealms.empty())
		{
			created = false;
			next++;
			Realm::RealmMap::iterator exists = Realm::ourRealmMap.find(it->first);
			if (exists == Realm::ourRealmMap.end())
			{
				std::string parent;
				parent = it->second.Get("parent", parent);
				Realm::RealmMap::iterator found = Realm::ourRealmMap.find(parent);

				if (found != Realm::ourRealmMap.end())
				{
					Realm::Create(it->first, it->second, found->second);
					unorderedRealms.erase(it);
					created = true;
				}
			}
			it = next;
			if (it == unorderedRealms.end())
				it = unorderedRealms.begin();
		}

		if (!unorderedRealms.empty())
		{
			// there are some classes without a base class file
			std::string errorMessage = "Realms: Parent realms for the following could not be found: ";
			Realm::DictMap::iterator it = unorderedRealms.begin();
			while (it != unorderedRealms.end())
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
	// Function:	Initialize
	// Description:	initializes god of first creation
	// Arguments:	owning object, unique id of realm, dictionary of config vars
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool God::Initialize(const GCPtr<GCObject>& owner, const std::string& id, const StringKeyDictionary& sd)
	{
		try
		{
			myTime = 0;
			bool ok = Realm::Initialize(owner, id, sd);
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
	bool God::Update(double until, unsigned int phase)
	{
		if (myPaused)
			return false;

		SetAsActiveRealm();
		bool ok = Realm::Update(until, phase);
		if (phase == GetNumPhasesPerUpdate())
		{
			RealmMap worlds = myWorlds;
			for (RealmMap::iterator it = worlds.begin(); it != worlds.end(); it++)
			{
				if (it->second.IsValid())
				{
					unsigned int maxPhases = it->second->GetNumPhasesPerUpdate();
					for (unsigned int p = 0; p != maxPhases + 1; p++)
						if (!it->second->Update(it->second->GetTime() + it->second->GetStepSize(), p))
							ok = false;

					it->second->FlagUpdateCompleted();
				}
			}
			SetAsInactiveRealm();
		}
		return ok;
	}


	// --------------------------------------------------------------------------						
	// Function:	Update
	// Description:	master update function
	// Arguments:	time since last call
	// Returns:		if successfull
	// --------------------------------------------------------------------------
	bool God::Update(double timeSinceLastUpdate)
	{
		double now = myLastUpdateTime + timeSinceLastUpdate;
		if (myStartTime == -1)
		{
			//myObjectStartTime = myAgentManager->GetCurrentTime();
			myStartTime = 0;
			myPaceMaker = 0;
			myLastUpdateTime = -0.01;
			now = myStartTime;
		}

		double time = now - myStartTime;

		double maxUpdateTime = GetMeta("max_update_time", 0.02); // Chunk size of Realm updates, for example neurons are updated this often in simulation time.
		if (time - myLastUpdateTime > maxUpdateTime)
		{
			// miss ticks if over tick size
			time = myLastUpdateTime + maxUpdateTime;
			myStartTime += now - time;
		}


		try
		{
			int maxPhases = GetNumPhasesPerUpdate();
			for (int phase = 0; phase != maxPhases + 1; phase++)
				Update(now, phase);
		}
		catch (std::exception& e)
		{
			std::string errorMessage = "Exception caught in God agents: ";
			errorMessage += e.what();
			errorMessage += ".\n";
			ERROR_TRACE(errorMessage.c_str());
		}
		catch (...)
		{
			ERROR_TRACE("Unknown Error: Updating God agents.\n");
		}

		
		while (myPaceMaker < time)
		{
			for (RealmMap::iterator r = myWorlds.begin(); r != myWorlds.end(); r++)
			{
				double until = r->second->GetTime() + r->second->GetStepSize();
				int maxPhases = r->second->GetNumPhasesPerUpdate();
				for(int phase = 0; phase != maxPhases; phase++)
					r->second->Update(until, phase);
			}
			
			myPaceMaker += GetStepSize();
		}

		Lock();
		for(int w = 0; w != myWorldsToDestroy.size(); w++)
		{
			RealmMap::iterator it = myWorlds.find(myWorldsToDestroy[w]);
			if (it != myWorlds.end())
			{
				it->second.Destroy();
				myWorlds.erase(it);
			}
		}
		myWorldsToDestroy.clear();
		Unlock();
		

		SetAsActiveRealm();
		myLastUpdateTime = now;

		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetWorld
	// Description:	returns world of given name
	// Arguments:	name
	// Returns:		world
	// --------------------------------------------------------------------------
	const GCPtr<Realm> God::GetWorld(const ::std::string& name) 
	{
		Lock();
		RealmMap::const_iterator it = myWorlds.find(name);
		if (it != myWorlds.end())
		{
			Unlock();
			return it->second;
		}
		Unlock();
		return GCPtr<Realm>();
	}


	// --------------------------------------------------------------------------						
	// Function:	CreateWorld
	// Description:	creates a world of given name
	// Arguments:	name for world, dictionary of config vars, realm template to
	//				create world from
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool God::CreateWorld(const ::std::string& name, const StringKeyDictionary& config, const GCPtr<Realm>& other)
	{
		Lock();
		RealmMap::iterator it = myWorlds.find(name);
		if (it != myWorlds.end())
		{
			Unlock();
			return false;
		}
		GCPtr<Realm> world = GCPtr<Realm>(new Realm("", Privileges(), name));
		world->Configure(config, other);

		if (world.IsValid())
			myWorlds[name] = world;

		SetAsActiveRealm();
		Unlock();
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	DestroyWorld
	// Description:	destroys world of given name
	// Arguments:	world name
	// Returns:		if sucessful
	// --------------------------------------------------------------------------
	bool God::DestroyWorld(const ::std::string& name)
	{
		// need to queue for deletion after 
		Lock();
		RealmMap::iterator it = myWorlds.find(name);
		if (it != myWorlds.end())
		{
			Unlock();
			return false;
		}
		myWorldsToDestroy.push_back(name);
		Unlock();
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetWorlds
	// Description:	gets all worlds owned by God
	// Arguments:	none
	// Returns:		worlds
	// --------------------------------------------------------------------------
	const Realm::RealmMap& God::GetWorlds() const
	{ 
		return myWorlds; 
	}


	// --------------------------------------------------------------------------						
	// Function:	DestroyWorlds
	// Description:	destroys all worlds owned by God
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void God::DestroyWorlds()
	{
		while (!myWorlds.empty())
		{
			RealmMap::iterator it = myWorlds.begin();
			DestroyWorld(it->second->GetId());
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	Write
	// Description:	serialization out
	// Arguments:	io stream interface, version to log as
	// Returns:		none
	// --------------------------------------------------------------------------
	void God::Write(IOInterface& io, int version) const
	{
		//INITIALIZE_SERIAL_OUT(io);

		//SERIALIZE_OUT(io, ourMetaConfig);
		//Realm::WriteStatic(io, version);

		//FINALIZE_SERIAL(io);

	}


	// --------------------------------------------------------------------------						
	// Function:	Read
	// Description:	serialization in
	// Arguments:	io stream interface, version to log as
	// Returns:		none
	// --------------------------------------------------------------------------
	void God::Read(IOInterface& io, int version)
	{
		//INITIALIZE_SERIAL_IN(io);

		//SERIALIZE_IN(io, ourMetaConfig);
		//Realm::ReadStatic(io, version);

		//FINALIZE_SERIAL(io);
	}

}
