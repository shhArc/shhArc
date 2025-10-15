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

#include <vector>
#include "Archive.h"
#include "BinaryFile.h"

namespace shh
{


	// --------------------------------------------------------------------------						
	// Function:	Archive
	// Description:	constructor
	// Arguments:	whether to use object name tags in archive file, version
	// Returns:		none
	// --------------------------------------------------------------------------
	Archive::Archive(bool useTags, unsigned int version) :
		myIO(NULL),
		myNextPtrId(0),
		myCreatedIO(false)
	{
		myArchiveTags = useTags;
		myVersion = version;
	}


	// --------------------------------------------------------------------------						
	// Function:	Archive
	// Description:	constructor
	// Arguments:	io interface, 
	//				whether to use object name tags in archive file, version
	// Returns:		none
	// --------------------------------------------------------------------------
	Archive::Archive(IOInterface& io, bool useTags, unsigned int version) : 
		myIO(&io), 
		myNextPtrId(0), 
		myCreatedIO(false)
	{
		myVersion = version;
		myArchiveTags = useTags;
		io.myArchiveTags = myArchiveTags;
	}


	// --------------------------------------------------------------------------						
	// Function:	Open
	// Description:	opens file
	// Arguments:	file name, read write flags
	// Returns:		none
	// --------------------------------------------------------------------------
	void Archive::Open(const std::string& filename, IOInterface::Flags flags)
	{
		if (myIO && myCreatedIO)
			delete myIO;
		myCreatedIO = true;
		myIO = new BinaryFile(filename, flags);
		myIO->myArchiveTags = myArchiveTags;
		myIO->Open(filename, flags);
	}


	// --------------------------------------------------------------------------						
	// Function:	Close
	// Description:	close file
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Archive::Close()
	{
		if (myCreatedIO && myIO)
			delete myIO;
		myIO = NULL;
		myCreatedIO = false;
	}


	// --------------------------------------------------------------------------						
	// Function:	Peak
	// Description:	reads without moving forward in file
	// Arguments:	object
	// Returns:		none
	// --------------------------------------------------------------------------
	int Archive::Peek()
	{ 
		return 0; 
	}


	// --------------------------------------------------------------------------						
	// Function:	Eof
	// Description:	returns if at end of file
	// Arguments:	none
	// Returns:		bool
	// --------------------------------------------------------------------------
	bool Archive::Eof()
	{ 
		return myIO->Eof();
	}


	// --------------------------------------------------------------------------						
	// Function:	GetPos
	// Description:	returns position in file
	// Arguments:	none
	// Returns:		position
	// --------------------------------------------------------------------------
	std::streamoff  Archive::GetPos()
	{ 
		return myIO->GetPos(); 
	}
	

	// --------------------------------------------------------------------------						
	// Function:	SeekOff
	// Description:	moves file position relative to current
	// Arguments:	move amount, directiiom
	// Returns:		none
	// --------------------------------------------------------------------------
	void Archive::SeekOff(std::streamoff pos, seekdir way)
	{ 
		myIO->SeekOff(pos, way); 
	}


	// --------------------------------------------------------------------------						
	// Function:	Read
	// Description:	read whole archive
	// Arguments:	none
	// Returns:		list of objects constructed
	// --------------------------------------------------------------------------
	IOInterface::ConstructionList Archive::Read()
	{
		ConstructionList classes;
		myArchiveTags = true;
		myIO->myArchiveTags = myArchiveTags;
		while (!myIO->Eof())
			classes.push_back(InstanticateClass());

		return classes;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetInFuncs
	// Description:	returns map of input functions
	// Arguments:	none
	// Returns:		map
	// --------------------------------------------------------------------------
	Archive::InFuncs& Archive::GetInFuncs()
	{ 
		static InFuncs inFuncs; 
		return inFuncs; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetOutFuncs
	// Description:	returns map of output functions
	// Arguments:	none
	// Returns:		map
	// --------------------------------------------------------------------------
	Archive::OutFuncs& Archive::GetOutFuncs()
	{ 
		static OutFuncs outFuncs; 
		return outFuncs; 
	}
	

	// --------------------------------------------------------------------------						
	// Function:	GetConstructFuncs
	// Description:	returns map of constructor functions
	// Arguments:	none
	// Returns:		map
	// --------------------------------------------------------------------------
	Archive::ConstructFuncs& Archive::GetConstructFuncs()
	{ 
		static ConstructFuncs constructFuncs; 
		return constructFuncs; 
	}
	

	// --------------------------------------------------------------------------						
	// Function:	GetInFunc
	// Description:	returns input function for a class name
	// Arguments:	classname
	// Returns:		function
	// --------------------------------------------------------------------------
	Archive::InFunc& Archive::GetInFunc(const std::string className)
	{ 
		return GetInFuncs().find(className)->second; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetOutFunc
	// Description:	returns output function for a class name
	// Arguments:	classname
	// Returns:		function
	// --------------------------------------------------------------------------
	Archive::OutFunc& Archive::GetOutFunc(const std::string className)
	{ 
		return GetOutFuncs().find(className)->second; 
	}
	

	// --------------------------------------------------------------------------						
	// Function:	GetConstructFunc
	// Description:	returns constructor function for a class name
	// Arguments:	classname
	// Returns:		function
	// --------------------------------------------------------------------------
	Archive::ConstructFunc& Archive::GetConstructFunc(const std::string className)
	{ 
		return GetConstructFuncs().find(className)->second; 
	}


	// --------------------------------------------------------------------------						
	// Function:	InstanticateClass
	// Description:	read in class and create it
	// Arguments:	none
	// Returns:		pair of classname and object
	// --------------------------------------------------------------------------
	IOInterface::ConstructionPair Archive::InstanticateClass()
	{
		std::streamoff pos = GetPos();
		std::string classname;
		myIO->InitializeStreamIn(classname);
		myIO->FinalizeClassSerial();


		InFuncs::iterator in = GetInFuncs().find(classname);
		if (in == GetInFuncs().end())
			Exception::Throw("Achive class read function for %s not found", classname.c_str());

		myIO->SeekOff(pos, BaseBinaryFile::start);

		void* object = NULL;
		NoCreateClass::iterator it = myNoCreateClasses.find(classname);
		if (it != myNoCreateClasses.end())
			object = it->second;

		(*in->second)(*this, object);
		return ConstructionPair(classname, object);
	}


	// --------------------------------------------------------------------------						
	// Function:	InitializeStreamIn
	// Description: initializes the stream to read a class
	// Arguments:	class name
	// Returns:		none
	// --------------------------------------------------------------------------
	void Archive::InitializeStreamIn(std::string& classname)
	{ 
		myIO->InitializeStreamIn(classname); 
	}
	

	// --------------------------------------------------------------------------						
	// Function:	InitializeStreamOut
	// Description: initializes the stream to write a class
	// Arguments:	class name
	// Returns:		none
	// --------------------------------------------------------------------------
	void Archive::InitializeStreamOut(const std::string& classname)
	{ 
		myIO->InitializeStreamOut(classname); 
	}


	// --------------------------------------------------------------------------						
	// Function:	FinalizeClassSerial
	// Description:	end reading class
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Archive::FinalizeClassSerial()
	{ 
		myIO->FinalizeClassSerial(); 
	}


}