//////////////////////////////////////////////////////////////////////
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
// such a license from David K Bhowmik..
/////////////////////////////////////////////////////////////////////


#ifdef _MSC_VER
#pragma warning( disable : 4800 )
#endif


#include "../Common/SecureStl.h"
#include "../Arc/Api.h"
#include "../Schema/Whole.h"
#include "WholeModule.h"

//! /type Agent
//! Members functions of Agent type.

namespace shh {


	// --------------------------------------------------------------------------						
	// Function:	Register
	// Description:	registers functions and variables to be used by a process
	// Arguments:	alies of module and type, dictionary of configuration vars
	// Returns:		if sucessfull
	// --------------------------------------------------------------------------
	bool WholeModule::Register(const std::string& alias, const StringKeyDictionary& sd, Privileges privileges)
	{
		GCPtr<Module> me(this);

		Api::OpenNamespace(alias);
			
		Api::RegisterFunction("CreateCollection", CreateCollection, 2, me);
		Api::RegisterFunction("DestroyCollection", DestroyCollectionName, 2, me);
		Api::RegisterFunction("DestroyCollection", DestroyCollectionId, 2, me);
		Api::RegisterFunction("DestroyPart", DestroyPartNameName, 3, me);
		Api::RegisterFunction("DestroyPart", DestroyPartNameId, 3, me);
		Api::RegisterFunction("DestroyPart", DestroyPartIdName, 3, me);
		Api::RegisterFunction("DestroyPart", DestroyPartIdId, 3, me);	

		Api::CloseNamespace();

		return Module::Register(alias, sd, privileges);
	}


	//! /member Agent
	//! /function CreateCollection
	//! /privilege Agent
	//! /param string collection_name 
	//! /returns integer
	//! Creates a new collection to store parts in.
	ExecutionState WholeModule::CreateCollection(std::string& collectionName, unsigned int &id)
	{
		Whole* w = Whole::GetActiveWhole();
		if (!w)
			return ExecutionFailed;
		
		GCPtr<Collection> c;
		id = w->CreateCollection(collectionName, c);
		return ExecutionOk;
	}


	//! /member Agent
	//! /function DestroyCollection
	//! /privilege Agent
	//! /param string_or_integer collection_name_or_id
	//! Destroys a collection and all its parts.
	ExecutionState WholeModule::DestroyCollectionName(std::string &collectionName)
	{
		Whole* w = Whole::GetActiveWhole();
		if (!w)
			return ExecutionFailed;
		w->DestroyCollection(collectionName);
		return ExecutionOk;
	}



	ExecutionState WholeModule::DestroyCollectionId(unsigned int& collectionId)
	{
		Whole* w = Whole::GetActiveWhole();
		if (!w)
			return ExecutionFailed;
		w->DestroyCollection(collectionId);
		return ExecutionOk;
	}


	//! /member Agent
	//! /function DestoryPart
	//! /privilege Agent	
	//! /param string_or_integer collection_name_or_id 
	//! /param string_or_integer part_name_or_id
	//! Destroys a part.
	ExecutionState WholeModule::DestroyPartNameName(std::string& collectionName, std::string& partName)
	{
		Whole* w = Whole::GetActiveWhole();
		if (!w)
			return ExecutionFailed;
		w->DestroyPart(collectionName, partName);

		return ExecutionOk;
	}

	ExecutionState WholeModule::DestroyPartNameId(std::string& collectionName, unsigned int& partId)
	{
		Whole* w = Whole::GetActiveWhole();
		if (!w)
			return ExecutionFailed;
		w->DestroyPart(collectionName,partId);
		return ExecutionOk;
	}

	ExecutionState WholeModule::DestroyPartIdName(unsigned int& collectionId, std::string& partName)
	{
		Whole* w = Whole::GetActiveWhole();
		if (!w)
			return ExecutionFailed;
		w->DestroyPart(collectionId, partName);
		return ExecutionOk;
	}

	ExecutionState WholeModule::DestroyPartIdId(unsigned int& collectionId, unsigned int& partId)
	{
		Whole* w = Whole::GetActiveWhole();
		if (!w)
			return ExecutionFailed;
		w->DestroyPart(collectionId, partId);

		return ExecutionOk;
	}
}

