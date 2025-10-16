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
#pragma warning(disable:4786 4503)
#endif

#include "GCPtrBase.h"

namespace shh {


	//IMPLEMENT_MEMORY_MANAGED(GCInfo)



	// GCInfo ////////////////////////////////////////////////////////////////////////


	// --------------------------------------------------------------------------						
	// Function:	GCInfo
	// Description:	constructor
	// Arguments:	object pointed to, ref count, if object is valud
	// Returns:		none
	// --------------------------------------------------------------------------
	GCInfo::GCInfo(GCObjectBase* object, bool memoryManaged, int count, bool valid) :
		MemoryInfo(object, memoryManaged, count, valid)
	{ 
		LOCKEXCHANGEGCOBJECT(myGCObject, object); 
	}
	


	// --------------------------------------------------------------------------						
	// Function:	GCInfo
	// Description:	constructor
	// Arguments:	object pointed to, ref count, if object is valud
	// Returns:		none
	// --------------------------------------------------------------------------
	GCInfo::GCInfo(void* object, bool memoryManaged, int count, bool valid) :
		MemoryInfo(object, memoryManaged, count, valid)
	{ 
		myGCObject = NULL; 
	}
		


	// --------------------------------------------------------------------------						
	// Function:	GCInfo
	// Description:	constructor
	// Arguments:	memoryklocator of object pointed to, memory offset of object
	//				ref count, if object is valud
	// Returns:		none
	// --------------------------------------------------------------------------
	GCInfo::GCInfo(MemoryLocator& l, MemoryOffset offset, bool memoryManaged, int count, bool valid) :
		MemoryInfo(&l, offset, memoryManaged, count, valid)
	{
		GCObjectBase* obj = reinterpret_cast<GCObjectBase*>(myObject);
		LOCKEXCHANGEGCOBJECT(myGCObject, obj);
	}
	

	// --------------------------------------------------------------------------						
	// Function:	~GCInfo
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	GCInfo::~GCInfo()
	{
		if (IsValid() && myGCObject != NULL)
		{
			GCInfo* null = NULL;
			LOCKEXCHANGEGCOBJECT(myGCObject->myGCInfo, null);
		}
	}

		

	// --------------------------------------------------------------------------						
	// Function:	CreateNull
	// Description:	create null gcinfo
	// Arguments:	none
	// Returns:		gcinfo
	// --------------------------------------------------------------------------
	GCInfo* GCInfo::CreateNull()
	{
		static GCInfo null((void*)NULL, false, 1, false);
		return &null;
	}



	// --------------------------------------------------------------------------						
	// Function:	RegisterMoveAll
	// Description:	move memory location of object (batch process)
	// Arguments:	delta, direction
	// Returns:		none
	// --------------------------------------------------------------------------
	void GCInfo::RegisterMoveAll(MemorySize offsetDelta, MemoryLocator::Direction offsetDir)
	{
		MemoryInfo::RegisterMoveAll(offsetDelta, offsetDir);
		if (myGCObject)
		{
			GCObjectBase* obj = reinterpret_cast<GCObjectBase*>(myObject);
			LOCKEXCHANGEGCOBJECT(myGCObject, obj);
		}
	}



	// --------------------------------------------------------------------------						
	// Function:	RegisterMoveSingle
	// Description:	move memory location of object
	// Arguments:	memory locator of object, offset
	// Returns:		none
	// --------------------------------------------------------------------------
	void GCInfo::RegisterMoveSingle(MemoryLocator* locator, MemoryOffset& offset)
	{
		MemoryInfo::RegisterMoveSingle(locator, offset);
		if (myGCObject)
		{
			GCObjectBase* obj = reinterpret_cast<GCObjectBase*>(myObject);
			LOCKEXCHANGEGCOBJECT(myGCObject, obj);
		}
	}
	


	// GCObjectBase //////////////////////////////////////////////////////////////////////


	// --------------------------------------------------------------------------						
	// Function:	GCObjectBase
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	GCObjectBase::GCObjectBase() : myCollectable(1), myGCInfo(0), myMemoryStart(this)
	{

	}



	// --------------------------------------------------------------------------						
	// Function:	~GCObjectBase
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	GCObjectBase::~GCObjectBase()
	{
		VoidGCInfo();
	}


	// --------------------------------------------------------------------------						
	// Function:	Finalize
	// Description:	destructor
	// Arguments:	object to finalize
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool GCObjectBase::Finalize(GCObjectInterface<GCObjectBase> *gc)
	{
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	VoidGCInfo
	// Description:	clear gc info
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void GCObjectBase::VoidGCInfo()
	{
		if (myGCInfo != NULL)
		{
			myGCInfo->SetValid(false);
			GCInfo* null = NULL;
			LOCKEXCHANGEGCOBJECT(myGCInfo, null);
		}
	}



	// --------------------------------------------------------------------------						
	// Function:	Init
	// Description:	initialize GCObjectBase
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void GCObjectBase::Init()
	{
		GCInfo* null = NULL;
		LOCKEXCHANGEGCOBJECT(myGCInfo, null);
		int f = 0;
		LOCKEXCHANGEGCOBJECT(myCollectable, f);
	}


	// --------------------------------------------------------------------------						
	// Function:	IsDying
	// Description:	return if object is being delete
	// Arguments:	none
	// Returns:		bool
	// --------------------------------------------------------------------------
	bool GCObjectBase::IsDying() const
	{
		return myGCInfo && myGCInfo->IsDying();
	}


	// --------------------------------------------------------------------------						
	// Function:	IsDeleteable
	// Description:	returns if deltable object
	// Arguments:	none
	// Returns:		returns if deltable object
	// --------------------------------------------------------------------------
	bool GCObjectBase::IsDeleteable() const { return (bool)myCollectable; }


	// --------------------------------------------------------------------------						
	// Function:	MovePointers
	// Description:	set object passed in to point at the object this points to
	// Arguments:	object to set
	// Returns:		none
	// --------------------------------------------------------------------------
	void GCObjectBase::MovePointers(GCObjectBase* object)
	{
		GCInfo* info = myGCInfo;
		LOCKEXCHANGEGCOBJECT(object->myGCInfo, info);
		if (object->myGCInfo != NULL)
			LOCKEXCHANGEGCOBJECT(object->myGCInfo->myObject, object);

		GCInfo* null = NULL;
		LOCKEXCHANGEGCOBJECT(myGCInfo, null);
	}



	// --------------------------------------------------------------------------						
	// Function:	GetGCInfo
	// Description:	get GCObjects info
	// Arguments:	if a memory managed object
	// Returns:		gcinfo
	// --------------------------------------------------------------------------
	GCInfo* GCObjectBase::GetGCInfo(bool memoryManaged)
	{
		if (myGCInfo)
		{
			return myGCInfo;
		}
		else if (!memoryManaged)
		{
			GCInfo *info = new GCInfo(this, false);
			LOCKEXCHANGEGCOBJECT(myGCInfo, info);
			return myGCInfo;
		}
		else
		{
			if (MemoryLocator::IsVariableSizeData((char*)myMemoryStart))
			{
				MemoryLocator::VariableHeader& header = MemoryLocator::GetVariableHeader((char*)myMemoryStart);
				GCInfo* info = new GCInfo(*header.myLocator, header.myLocator->AddressOffset(this), true);
				LOCKEXCHANGEGCOBJECT(myGCInfo, info);
				header.myMemoryInfo = myGCInfo;
			}
			else
			{
				MemoryLocator::FixedHeader& header = MemoryLocator::GetFixedHeader((char*)myMemoryStart);
				GCInfo* info = new GCInfo(*header.myLocator, header.myLocator->AddressOffset(this), true);
				LOCKEXCHANGEGCOBJECT(myGCInfo, info);
				header.myMemoryInfo = myGCInfo;
			}
			return myGCInfo;
		}
	}

	// --------------------------------------------------------------------------						
	// Function:	SetGCMemoryStart
	// Description:	sets the memory allocation point for this object as
	//				with multiple inheritace GCObject may not me at the start
	//				of the objects memory and this is needed for MemoryLocators
	// Arguments:	memory allocation point of object
	// Returns:		none
	// --------------------------------------------------------------------------
	void GCObjectBase::SetGCMemoryStart(void* mem)
	{
		myMemoryStart = mem;
	}


	// --------------------------------------------------------------------------						
	// Function:	NullInfo
	// Description:	nulls the GCInfo for this object
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void GCObjectBase::NullInfo()
	{
		GCInfo* null = NULL;
		LOCKEXCHANGEGCOBJECT(myGCInfo, null);	// else will delete gcinfo and undeletable ofject will still have it as gcinfo
	}
}
