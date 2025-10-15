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
#pragma warning( disable : 4800 )
#endif


#include "../VM/VM.h"
#include "Whole.h"


namespace shh {


	// --------------------------------------------------------------------------						
	// Function:	CreateCollection
	// Description:	creates a collection
	// Arguments:	name, collection created 
	// Returns:		collection id
	// --------------------------------------------------------------------------
	unsigned int Whole::CreateCollection(const std::string collectionName, GCPtr<Collection> &collection)
	{ 
		GCPtr<Collection> c(new Collection);
		unsigned int id = myCollections.AddPart(collectionName, c);
		if (myCollections.AddPart(collectionName, c) != 0)
		{
			collection = c;
			return id;
		}
		c.Destroy();
		return 0;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetCollection
	// Description:	gets a collecion 
	// Arguments:	name, collection got
	// Returns:		if got
	// --------------------------------------------------------------------------
	bool Whole::GetCollection(const std::string& collectionName, GCPtr<Collection> &collection)
	{ 
		Collection::Part part;
		if (myCollections.GetPart(collectionName, part))
		{
			collection.StaticCast(part.myObject);
			return true;
		}

		return false; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetCollection
	// Description:	gets a collection
	// Arguments:	collection id, collection got
	// Returns:		if got
	// --------------------------------------------------------------------------
	bool Whole::GetCollection(unsigned int id, GCPtr<Collection>& collection)
	{
		Collection::Part part;
		if (myCollections.GetPart(id, part))
		{
			collection.StaticCast(part.myObject);
			return true;
		}

		return false;
	}



	// --------------------------------------------------------------------------						
	// Function:	AddPart
	// Description:	add part
	// Arguments:	collection name, part name, part
	// Returns:		id or -1 if collection doesnt exist
	// --------------------------------------------------------------------------
	unsigned int Whole::AddPart(const std::string& collectionName, const std::string &name, GCPtr<GCObject> object)
	{
		GCPtr<Collection> collection;
		if (GetCollection(collectionName, collection))
			return -1;

		return collection->AddPart(name, object);
	}


	// --------------------------------------------------------------------------						
	// Function:	AddPart
	// Description:	adds a part to a collection
	// Arguments:	collection id, part name, part
	// Returns:		part id
	// --------------------------------------------------------------------------
	unsigned int  Whole::AddPart(unsigned int collectionId, const std::string &partName, GCPtr<GCObject>& obj)
	{
		GCPtr<Collection> c;
		if (!GetCollection(collectionId, c))
			return 0;

		return c->AddPart(partName, obj);
	}


	// --------------------------------------------------------------------------						
	// Function:	GetPart
	// Description:	adds a part to a collection
	// Arguments:	collection name, part id, part got
	// Returns:		if got
	// --------------------------------------------------------------------------
	bool Whole::GetPart(const std::string& collectionName, unsigned int id, GCPtr<GCObject>& obj)
	{
		GCPtr<Collection> c;
		if (GetCollection(collectionName, c))
		{
			Collection::Part part;
			if (c->GetPart(id, part))
			{
				obj = part.myObject;
				return true;
			}
		}
		return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetPart
	// Description:	adds a part to a collection
	// Arguments:	collection name, part name, part got
	// Returns:		if got
	// --------------------------------------------------------------------------
	bool Whole::GetPart(const std::string &collectionName, std::string partName, GCPtr<GCObject>& obj)
	{
		GCPtr<Collection> c;
		if (GetCollection(collectionName, c))
		{
			Collection::Part part;
			if (c->GetPart(partName, part))
			{
				obj = part.myObject;
				return true;
			}
		}
		return false;
	}

	// --------------------------------------------------------------------------						
	// Function:	GetPart
	// Description:	adds a part to a collection
	// Arguments:	collection id, part id, part got
	// Returns:		if got
	// --------------------------------------------------------------------------
	bool Whole::GetPart(unsigned int collectionId, unsigned int id, GCPtr<GCObject>& obj)
	{
		GCPtr<Collection> c;
		if (GetCollection(collectionId, c))
		{
			Collection::Part part;
			if (c->GetPart(id, part))
			{
				obj = part.myObject;
				return true;
			}
		}
		return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetPart
	// Description:	adds a part to a collection
	// Arguments:	collection id, part name, part got
	// Returns:		if got
	// --------------------------------------------------------------------------
	bool Whole::GetPart(unsigned int collectionId, const std::string partName, GCPtr<GCObject>& obj)
	{
		GCPtr<Collection> c;
		if (GetCollection(collectionId, c))
		{
			Collection::Part part;
			if (c->GetPart(partName, part))
			{
				obj = part.myObject;
				return true;
			}
		}
		return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	DestroyCollection
	// Description:	destroys collection and all parts
	// Arguments:	collection name
	// Returns:		none
	// --------------------------------------------------------------------------
	void Whole::DestroyCollection(const std::string& collectionName)
	{
		myCollections.Destroy(collectionName);
	}


	// --------------------------------------------------------------------------						
	// Function:	DestroyCollection
	// Description:	destroys collection and all parts
	// Arguments:	collection di
	// Returns:		none
	// --------------------------------------------------------------------------
	void Whole::DestroyCollection(unsigned int collectionId)
	{
		myCollections.Destroy(collectionId);
	}


	// --------------------------------------------------------------------------						
	// Function:	GetActiveWhole
	// Description:	gets the whole of the process in the current thread
	// Arguments:	none
	// Returns:		whole
	// --------------------------------------------------------------------------
	Whole *Whole::GetActiveWhole()
	{
		GCPtr<Process> p = Scheduler::GetCurrentProcess();
		Whole* w = dynamic_cast<Whole*>(p.GetObject());
		if (w)
			return w;

		GCPtr<VM> vm = p->GetVM();
		w = dynamic_cast<Whole*>(vm.GetObject());
		if (w)
			return w;

		return NULL;
	}

}