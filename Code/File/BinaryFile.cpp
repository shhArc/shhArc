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


#include "BinaryFile.h"

namespace shh
{

	// BaseBinaryFile /////////////////////////////////////////////////////////


	// --------------------------------------------------------------------------						
	// Function:	BaseBinaryFile
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	BaseBinaryFile::BaseBinaryFile() :
		myStream(NULL)
	{}


	// --------------------------------------------------------------------------						
	// Function:	:~BaseBinaryFile
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	BaseBinaryFile::~BaseBinaryFile()
	{
		Close();
	}


	// --------------------------------------------------------------------------						
	// Function:	Close
	// Description:	clotream
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void BaseBinaryFile::Close()
	{
		if (myStream)
		{
			delete myStream;
			myStream = NULL;
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	GetPos
	// Description:	returns current position in stream
	// Arguments:	none
	// Returns:		position
	// --------------------------------------------------------------------------
	std::streamoff BaseBinaryFile::GetPos()
	{
		std::streamoff streampos = myStream->tellg();
		return streampos;
	};


	// --------------------------------------------------------------------------						
	// Function:	Eof
	// Description:	returns if at end of file
	// Arguments:	none
	// Returns:		bool
	// --------------------------------------------------------------------------
	bool BaseBinaryFile::Eof()
	{
		return  myStream->eof();
	}



	// IBinaryFile ///////////////////////////////////////////////////////////////////


	// --------------------------------------------------------------------------						
	// Function:	IBinaryFile
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	IBinaryFile::IBinaryFile()
	{}


	// --------------------------------------------------------------------------						
	// Function:	IBinaryFile
	// Description:	constructor
	// Arguments:	filename, flags
	// Returns:		none
	// --------------------------------------------------------------------------
	IBinaryFile::IBinaryFile(const std::string& filename, IOInterface::Flags flags)
	{
		Open(filename, flags);
	}


	// --------------------------------------------------------------------------						
	// Function:	Open
	// Description:	opens file
	// Arguments:	filename, flags
	// Returns:		none
	// --------------------------------------------------------------------------
	void IBinaryFile::Open(const std::string& filename, IOInterface::Flags flags)
	{
		myStream = new std::fstream(filename.c_str(), flags | std::ios_base::in | std::ios_base::binary);
	}


	// --------------------------------------------------------------------------						
	// Function:	Read
	// Description:	reads number of chars into buffer
	// Arguments:	buffer,number of chars
	// Returns:		number of chars
	// --------------------------------------------------------------------------
	unsigned int IBinaryFile::Read(char* buffer, unsigned int count)
	{
		GetStream().read(reinterpret_cast<char*>(buffer), count);
		return count;
	}


	// --------------------------------------------------------------------------						
	// Function:	Write
	// Description:	writes number of chars from  buffer
	// Arguments:	buffer,number of chars
	// Returns:		number of chars
	// --------------------------------------------------------------------------
	unsigned int IBinaryFile::Write(const char* buffer, unsigned int count)
	{
		Exception::Throw("Attempting to write to an istream");
		return 0;
	}


	// --------------------------------------------------------------------------						
	// Function:	Peed
	// Description:	reads but does not move on
	// Arguments:	none
	// Returns:		value
	// --------------------------------------------------------------------------
	int IBinaryFile::Peek()
	{
		return GetStream().peek();
	}


	// OBinaryFile ////////////////////////////////////////////////////////////////////////

	// --------------------------------------------------------------------------						
	// Function:	OBinaryFile
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	OBinaryFile::OBinaryFile()
	{}


	// --------------------------------------------------------------------------						
	// Function:	OBinaryFile
	// Description:	constructor
	// Arguments:	filename, flags
	// Returns:		none
	// --------------------------------------------------------------------------
	OBinaryFile::OBinaryFile(const std::string& filename, IOInterface::Flags flags)
	{
		Open(filename, flags);
	}


	// --------------------------------------------------------------------------						
	// Function:	Open
	// Description:	opwens file
	// Arguments:	filename, flags
	// Returns:		none
	// --------------------------------------------------------------------------
	void OBinaryFile::Open(const std::string& filename, IOInterface::Flags flags)
	{
		myStream = new std::fstream(filename.c_str(), flags | std::ios_base::out | std::ios_base::binary);
	}


	// --------------------------------------------------------------------------						
	// Function:	Read
	// Description:	reads number of chars into buffer
	// Arguments:	buffer,number of chars
	// Returns:		number of chars
	// --------------------------------------------------------------------------
	unsigned int OBinaryFile::Read(char* buffer, unsigned int count)
	{
		Exception::Throw("Attempting to read to an ostream");
		return 0;
	}


	// --------------------------------------------------------------------------						
	// Function:	Peek
	// Description:	reads but does not move on
	// Arguments:	none
	// Returns:		value
	// --------------------------------------------------------------------------
	int OBinaryFile::Peek()
	{
		Exception::Throw("Attempting to read to an ostream");
		return 0;
	}

	// BinaryFile ////////////////////////////////////////////////////////////////////////////

	// --------------------------------------------------------------------------						
	// Function:	BinaryFile
	// Description:	constructor
	// Arguments:	filename, flags
	// Returns:		none
	// --------------------------------------------------------------------------
	BinaryFile::BinaryFile(const std::string& filename, IOInterface::Flags flags)
	{
		BinaryFile::Open(filename, flags);
	}


	// --------------------------------------------------------------------------						
	// Function:	Open
	// Description:	opwens file
	// Arguments:	filename, flags
	// Returns:		none
	// --------------------------------------------------------------------------
	void BinaryFile::Open(const std::string& filename, IOInterface::Flags flags)
	{
		myStream = new std::fstream(filename.c_str(), flags | std::fstream::binary); {};
	}


	// --------------------------------------------------------------------------						
	// Function:	Read
	// Description:	reads number of chars into buffer
	// Arguments:	buffer,number of chars
	// Returns:		number of chars
	// --------------------------------------------------------------------------
	unsigned int BinaryFile::Read(char* buffer, unsigned int count)
	{
		return IBinaryFile::Read(buffer, count);
	}


	// --------------------------------------------------------------------------						
	// Function:	Write
	// Description:	writes number of chars from  buffer
	// Arguments:	buffer,number of chars
	// Returns:		number of chars
	// --------------------------------------------------------------------------
	unsigned int BinaryFile::Write(const char* buffer, unsigned int count)
	{
		return OBinaryFile::Write(buffer, count);
	}


	// --------------------------------------------------------------------------						
	// Function:	Peek
	// Description:	reads but does not move on
	// Arguments:	none
	// Returns:		value
	// --------------------------------------------------------------------------
	int BinaryFile::Peek()
	{
		return IBinaryFile::Peek();
	}

}