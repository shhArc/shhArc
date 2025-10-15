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

#ifndef COLLECTION_H
#define COLLECTION_H

#include "../Common/SecureStl.h"
#include "../Config/GCPtr.h"
#include <map>
#include <string>

namespace shh {

	class Collection : public GCObject
	{
	public:

		class Part
		{
		public:
			Part() : myId(0) {}
			unsigned int myId;
			std::string myName;
			GCPtr<GCObject> myObject;
		};
		
		typedef typename std::map<unsigned int, Part > PartIdMap;
		typedef typename std::map<std::string, unsigned int > PartNameMap;
		typedef typename PartIdMap::iterator IdIterator;
		typedef typename PartIdMap::const_iterator ConstIdIterator;
		typedef typename PartNameMap::iterator NameIterator;
		typedef typename PartNameMap::const_iterator ConstNameIterator;


		Collection();
		Collection(const Collection& other);
		virtual ~Collection();

		const Collection& operator=(const Collection& other);


		inline unsigned int AddPart(std::string name, GCPtr<GCObject> object);
		inline bool GetPart(unsigned int id, Part& part);
		inline bool GetPart(std::string name, Part& part);
		inline bool RemovePart(unsigned int id);
		inline bool RemovePart(std::string name);
		inline bool Destroy(unsigned int id);
		inline bool Destroy(std::string name);


		inline int Size() const;
		inline IdIterator Begin();
		inline ConstIdIterator Begin() const;
		inline IdIterator End();
		inline ConstIdIterator End() const;

	
	private:

		unsigned int myNextId;
		PartIdMap myPartIds;
		PartNameMap myPartNames;

	};


	// --------------------------------------------------------------------------						
	// Function:	AddPart
	// Description:	add part
	// Arguments:	part name, part
	// Returns:		id
	// --------------------------------------------------------------------------
	inline unsigned int Collection::AddPart(std::string name, GCPtr<GCObject> object)
	{
		Part p;
		p.myId = myNextId;
		if (name.empty())
		{
			name = "__";
			name += std::to_string(p.myId);
		}
		else
		{
			NameIterator it = myPartNames.find(name);
			if (it != myPartNames.end())
				return 0;
		}

		myNextId++;
		p.myName = name;
		p.myObject = object;
		myPartIds[p.myId] = p;
		myPartNames[p.myName] = p.myId;
		return p.myId;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetPart
	// Description:	gets part
	// Arguments:	part id, part to return
	// Returns:		if got
	// --------------------------------------------------------------------------
	inline bool Collection::GetPart(unsigned int id, Part& part)
	{
		IdIterator it = myPartIds.find(id);
		if (it == myPartIds.end())
			return false;

		part = it->second;
		return part.myObject.IsValid();
	}


	// --------------------------------------------------------------------------						
	// Function:	GetPart
	// Description:	gets part
	// Arguments:	part name, part to return
	// Returns:		if got
	// --------------------------------------------------------------------------
	inline bool Collection::GetPart(std::string name, Part& part)
	{
		NameIterator it = myPartNames.find(name);
		if (it == myPartNames.end())
			return false;

		return GetPart(it->second, part);
	}


	// --------------------------------------------------------------------------						
	// Function:	RemovePart
	// Description:	removed part
	// Arguments:	id
	// Returns:		if done
	// --------------------------------------------------------------------------
	inline bool Collection::RemovePart(unsigned int id)
	{
		IdIterator it = myPartIds.find(id);
		if (it != myPartIds.end())
		{
			std::string name = it->second.myName;
			myPartIds.erase(it);
			NameIterator nit = myPartNames.find(name);
			myPartNames.erase(nit);
			return true;
		}
		return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	RemovePart
	// Description:	remove part
	// Arguments:	name of part
	// Returns:		if done
	// --------------------------------------------------------------------------
	inline bool Collection::RemovePart(std::string name)
	{
		NameIterator it = myPartNames.find(name);
		if (it == myPartNames.end())
			return false;

		return RemovePart(it->second);
	}

	inline bool Collection::Destroy(unsigned int id)
	{
		IdIterator it = myPartIds.find(id);
		if (it != myPartIds.end())
		{
			std::string name = it->second.myName;
			it->second.myObject.Destroy();
			myPartIds.erase(it);
			NameIterator nit = myPartNames.find(name);
			myPartNames.erase(nit);
			return true;
		}
		return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	Destroy
	// Description:	destroys part
	// Arguments:	part name
	// Returns:		if done
	// --------------------------------------------------------------------------
	inline bool Collection::Destroy(std::string name)
	{
		NameIterator it = myPartNames.find(name);
		if (it == myPartNames.end())
			return false;

		return Destroy(it->second);
	}


	// --------------------------------------------------------------------------						
	// Function:	Size
	// Description:	returns size of collection
	// Arguments:	none
	// Returns:		size
	// --------------------------------------------------------------------------
	inline int Collection::Size() const 
	{ 
		return  (int)myPartIds.size(); 
	}


	// --------------------------------------------------------------------------						
	// Function:	Begin
	// Description:	returns iterator to begin
	// Arguments:	none
	// Returns:		iterator
	// --------------------------------------------------------------------------
	inline Collection::IdIterator Collection::Begin()
	{ 
		return myPartIds.begin(); 
	}


	// --------------------------------------------------------------------------						
	// Function:	Begin
	// Description:	returns iterator to begin
	// Arguments:	none
	// Returns:		iterator
	// --------------------------------------------------------------------------
	inline Collection::ConstIdIterator Collection::Begin() const
	{ 
		return myPartIds.begin(); 
	}


	// --------------------------------------------------------------------------						
	// Function:	Begin
	// Description:	returns iterator to end
	// Arguments:	none
	// Returns:		iterator
	// --------------------------------------------------------------------------
	inline Collection::IdIterator Collection::End()
	{ 
		return myPartIds.end(); 
	}


	// --------------------------------------------------------------------------						
	// Function:	Begin
	// Description:	returns iterator to end
	// Arguments:	none
	// Returns:		iterator
	// --------------------------------------------------------------------------
	inline Collection::ConstIdIterator Collection::End() const
	{ 
		return myPartIds.end(); 
	}

}

#endif 
