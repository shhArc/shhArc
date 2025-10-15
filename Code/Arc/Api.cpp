/////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////



#include "Api.h"

namespace shh {

	// --------------------------------------------------------------------------						
	// Function:	CloseDown
	// Description:	closes down the engine
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Api::CloseDown()
	{
		God::GetGod()->DestroyWorlds();
		God::DestroyGod();
		Realm::CloseDown();
	}


	// --------------------------------------------------------------------------						
	// Function:	FinalizeGodObjects
	// Description:	finalizes all god objects
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Api::FinalizeGodObjects()
	{
		God::GetGod()->FinalizeObjects();
	}


	// --------------------------------------------------------------------------						
	// Function:	FinalizeWorldObjects
	// Description:	finalizes all objects belonging to a world
	// Arguments:	world name
	// Returns:		none
	// --------------------------------------------------------------------------
	void Api::FinalizeWorldObjects(const std::string &name)
	{
		GCPtr<Realm> world = God::GetGod()->GetWorld(name);
		if(world.IsValid())
			world->FinalizeObjects();
	}


	// --------------------------------------------------------------------------						
	// Function:	CreateWorld
	// Description:	creates a world
	// Arguments:	world name, config dict, name of realm to use as template
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool Api::CreateWorld(const ::std::string& name, const StringKeyDictionary& config, const std::string &templateRealm)
	{
		GCPtr<Realm> realm = Realm::GetRealm(templateRealm);
		if (!realm.IsValid())
			return false;

		return God::GetGod()->CreateWorld(name, config, realm);
	}


	// --------------------------------------------------------------------------						
	// Function:	DestroyWorlds
	// Description:	destroys a world
	// Arguments:	world name
	// Returns:		none
	// --------------------------------------------------------------------------
	void Api::DestroyWorld(const std::string& name)
	{
		God::GetGod()->DestroyWorld(name);
	}


	// --------------------------------------------------------------------------						
	// Function:	DestroyWorlds
	// Description:	destorys all worlds
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Api::DestroyWorlds()
	{
		God::GetGod()->DestroyWorlds();
	}


	// --------------------------------------------------------------------------						
	// Function:	CreateGod
	// Description:	create god
	// Arguments:	name of god, realm to use as template
	// Returns:		none
	// --------------------------------------------------------------------------
	void Api::CreateGod(const std::string& name, const std::string& realmTemplate)
	{
		God::ourGodRealm = realmTemplate;
		GCPtr<God> god = God::GetGod();

		StringKeyDictionary sd;
		sd.Set("boot_script", God::ourBootFileName.c_str());
		sd.Set("update_script", God::ourUpdateFileName.c_str());
		god->Initialize(god, name, sd);
	}


	// --------------------------------------------------------------------------						
	// Function:	DestroyGod
	// Description:	destroys god
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Api::DestroyGod()
	{
		God::DestroyGod();
	}


	// --------------------------------------------------------------------------						
	// Function:	CreateClass
	// Description:	creatre a new class 
	// Arguments:	name of class, type of class (Agent/Mode), process to assign
	//				as this classes process, function to create new 
	//				class process, constructor function to create new class objects
	// Returns:		class
	// --------------------------------------------------------------------------
	GCPtr<Class> Api::CreateClass(const std::string& name, const std::string& typeName, const GCPtr<Process>& process, Registry::ProcessConstructor pc, Registry::ObjectConstructor oc)
	{
		GCPtr<Class> cls(new Class(name, typeName, process, pc, oc));
		return cls;
	}


	// --------------------------------------------------------------------------						
	// Function:	NewMessage
	// Description:	creates a new message package
	// Arguments:	message name, reciever, whether the message package can
	//				be deleted
	// Returns:		message
	// --------------------------------------------------------------------------
	Message* Api::NewMessage(const std::string& name, const GCPtr<Messenger>& to, bool deletable)
	{
		Message* msg = new Message(deletable);
		msg->myFunctionName = name;
		msg->myTo = to;
		return msg;
	}


	// --------------------------------------------------------------------------						
	// Function:	AddPart
	// Description:	add part
	// Arguments:	whole (e.g Agent) ,collection name, part name, part
	// Returns:		id or -1 if collection doesnt exist
	// --------------------------------------------------------------------------
	unsigned int Api::AddPart(const GCPtr<Whole>& w, std::string& collectionName, std::string name, GCPtr<GCObject> object)
	{
		return w->AddPart(collectionName, name, object);
	}


	// --------------------------------------------------------------------------						
	// Function:	AddPart
	// Description:	add part
	// Arguments:	whole (e.g Agent) ,collection id, part name, part
	// Returns:		id or -1 if collection doesnt exist
	// --------------------------------------------------------------------------
	unsigned int Api::AddPart(const GCPtr<Whole>& w, unsigned int collectionId, std::string name, GCPtr<GCObject> object)
	{
		return w->AddPart(collectionId, name, object);
	}



	// --------------------------------------------------------------------------						
	// Function:	GetPart
	// Description:	get part
	// Arguments:	whole (e.g Agent) ,collection name, part id, part
	// Returns:		if got
	// --------------------------------------------------------------------------
	bool Api::GetPart(const GCPtr<Whole>& w, std::string& collectionName, unsigned int id, GCPtr<GCObject>& obj)
	{
		return w->GetPart(collectionName, id, obj);

	}


	// --------------------------------------------------------------------------						
	// Function:	GetPart
	// Description:	get part
	// Arguments:	whole (e.g Agent) ,collection name, part name, part
	// Returns:		if got
	// --------------------------------------------------------------------------
	bool Api::GetPart(const GCPtr<Whole>& w, std::string& collectionName, std::string name, GCPtr<GCObject>& obj)
	{
		return w->GetPart(collectionName, name, obj);
	}


	// --------------------------------------------------------------------------						
	// Function:	GetPart
	// Description:	get part
	// Arguments:	whole (e.g Agent) ,collection id, part id, part
	// Returns:		if got
	// --------------------------------------------------------------------------
	bool Api::GetPart(const GCPtr<Whole>& w, unsigned int collectionId, unsigned int id, GCPtr<GCObject>& obj)
	{
		return w->GetPart(collectionId, id, obj);
	}


	// --------------------------------------------------------------------------						
	// Function:	GetPart
	// Description:	get part
	// Arguments:	collection id, part name, part
	// Returns:		if got
	// --------------------------------------------------------------------------
	bool Api::GetPart(const GCPtr<Whole>& w, unsigned int collectionId, std::string name, GCPtr<GCObject>& obj)
	{
		return w->GetPart(collectionId, name, obj);
	}


	// --------------------------------------------------------------------------						
	// Function:	CreateCollection
	// Description:	Creates a new collection to store parts in.
	// Arguments:	whole (e.g Agent) , collection_name
	// Returns:		id
	// --------------------------------------------------------------------------
	unsigned int Api::CreateCollection(const GCPtr<Whole> &w, std::string& collectionName)
	{
		GCPtr<Collection> c;
		return w->CreateCollection(collectionName, c);	
	}


	// --------------------------------------------------------------------------						
	// Function:	DestroyCollection
	// Description:	Destroys a collection and all its parts.
	// Arguments:	whole (e.g Agent) , collection_name
	// Returns:		none
	// --------------------------------------------------------------------------
	void Api::DestroyCollection(const GCPtr<Whole>& w, std::string& collectionName)
	{
		w->DestroyCollection(collectionName);
	}


	// --------------------------------------------------------------------------						
	// Function:	DestroyCollection
	// Description:	Destroys a collection and all its parts.
	// Arguments:	whole (e.g Agent) , collection_id
	// Returns:		none
	// --------------------------------------------------------------------------
	void Api::DestroyCollection(const GCPtr<Whole>& w, unsigned int& collectionId)
	{
		w->DestroyCollection(collectionId);
	}


	// --------------------------------------------------------------------------						
	// Function:	DestoryPart
	// Description:	Destroys a part.
	// Arguments:	whole (e.g Agent) , collection name, part name
	// Returns:		none
	// --------------------------------------------------------------------------
	void Api::DestroyPart(const GCPtr<Whole>& w, std::string& collectionName, std::string& partName)
	{
		GCPtr<Collection> c;
		w->GetCollection(collectionName, c);
		if (c.IsValid())
			c->Destroy(partName);
	}


	// --------------------------------------------------------------------------						
	// Function:	DestoryPart
	// Description:	Destroys a part.
	// Arguments:	whole (e.g Agent) , collection name, part Id
	// Returns:		none
	// --------------------------------------------------------------------------
	void Api::DestroyPart(const GCPtr<Whole>& w, std::string& collectionName, unsigned int& partId)
	{
		GCPtr<Collection> c;
		w->GetCollection(collectionName, c);
		if (c.IsValid())
			c->Destroy(partId);
	}


	// --------------------------------------------------------------------------						
	// Function:	DestoryPart
	// Description:	Destroys a part.
	// Arguments:	whole (e.g Agent) , collection id, part name
	// Returns:		none
	// --------------------------------------------------------------------------
	void Api::DestroyPart(const GCPtr<Whole>& w, unsigned int& collectionId, std::string& partName)
	{
		GCPtr<Collection> c;
		w->GetCollection(collectionId, c);
		if (c.IsValid())
			c->Destroy(partName);
	}


	// --------------------------------------------------------------------------						
	// Function:	DestoryPart
	// Description:	Destroys a part.
	// Arguments:	whole (e.g Agent) , collection name, part id
	// Returns:		none
	// --------------------------------------------------------------------------
	void Api::DestroyPart(const GCPtr<Whole>& w, unsigned int& collectionId, unsigned int& partId)
	{
		GCPtr<Collection> c;
		w->GetCollection(collectionId, c);
		if (c.IsValid())
			c->Destroy(partId);
	}


	// --------------------------------------------------------------------------						
	// Function:	LuaThrowScriptError
	// Description:	throws a lua error
	// Arguments:	format, data to print
	// Returns:		none
	// --------------------------------------------------------------------------
	void Api::LuaThrowScriptError(const char* format, ...)
	{
		va_list ap;
		va_start(ap, format);
		LuaApi::ThrowScriptError(format, ap);
		va_end(ap);
	}

} // namespace shh

