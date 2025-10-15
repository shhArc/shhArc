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

#include "Collection.h"


namespace shh {


	
	// --------------------------------------------------------------------------						
	// Function:	Collection
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Collection::Collection() : 
		myNextId(1) 
	{}


	// --------------------------------------------------------------------------						
	// Function:	Collection
	// Description:	constructor
	// Arguments:	other to clone from
	// Returns:		none
	// --------------------------------------------------------------------------
	Collection::Collection(const Collection& other) 
	{ 
		*this = other; 
	}


	// --------------------------------------------------------------------------						
	// Function:	Collection
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Collection::~Collection()
	{
		while (!myPartIds.empty())
		{
			IdIterator it = myPartIds.begin();
			if (it->second.myObject.IsValid())
			{
				GCPtr<GCObject> o = it->second.myObject;	// else iterator fucks
				o.Destroy();
			}
			myPartIds.erase(it);
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	=
	// Description:	assignment operator
	// Arguments:	other
	// Returns:		this
	// --------------------------------------------------------------------------
	const Collection& Collection::operator=(const Collection& other)
	{
		myPartIds = other.myPartIds;
		return *this;
	}

}
