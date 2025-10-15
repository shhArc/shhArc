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


#include "TextFile.h"

namespace shh
{

	// --------------------------------------------------------------------------						
	// Function:	BaseTextFile
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	BaseTextFile::BaseTextFile() :
		myStream(NULL) 
	{}
		

	// --------------------------------------------------------------------------						
	// Function:	~BaseTextFile
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	BaseTextFile::~BaseTextFile()
	{
		Close();
	}


	// --------------------------------------------------------------------------						
	// Function:	Close
	// Description:	closes strea,
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void BaseTextFile::Close()
	{
		if (myStream)
		{
			delete myStream;
			myStream = NULL;
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	GetPos
	// Description:	returns position in file
	// Arguments:	none
	// Returns:		positiom
	// --------------------------------------------------------------------------
	std::streamoff BaseTextFile::GetPos()
	{
		std::streamoff streampos = myStream->tellg();
		return streampos;
	}


	// --------------------------------------------------------------------------						
	// Function:	IOInterface
	// Description:	relative position move
	// Arguments:	amount to move, direction
	// Returns:		none
	// --------------------------------------------------------------------------
	void BaseTextFile::SeekOff(seekpos off, seekdir way)
	{
		std::streambuf* streamBuffer = myStream->rdbuf();
		pos_type streampos = streamBuffer->pubseekoff(off, way);
		return;
	}
	

	// --------------------------------------------------------------------------						
	// Function:	Eof
	// Description:	tests if end of file
	// Arguments:	none
	// Returns:		bool
	// --------------------------------------------------------------------------
	bool BaseTextFile::Eof()
	{ 
		return  myStream->eof(); 
	}

	// ITextFile ////////////////////////////////////////////////////////////////////////////////

	// --------------------------------------------------------------------------						
	// Function:	ITextFile
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	ITextFile::ITextFile()
	{}


	// --------------------------------------------------------------------------						
	// Function:	ITextFile
	// Description: constructor
	// Arguments:	filename, flags
	// Returns:		none
	// --------------------------------------------------------------------------
	ITextFile::ITextFile(const std::string& filename, IOInterface::Flags flags)
	{ 
		Open(filename,flags); 
	}


	// --------------------------------------------------------------------------						
	// Function:	Open
	// Description:	opens file
	// Arguments:	filename, flags
	// Returns:		none
	// --------------------------------------------------------------------------
	void ITextFile::Open(const std::string& filename, IOInterface::Flags flags)
	{
		myStream = new std::fstream(filename.c_str(), flags | std::ios_base::in);
	}


	// --------------------------------------------------------------------------						
	// Function:	Read
	// Description:	reads file into buffer
	// Arguments:	buffer, buffer size
	// Returns:		buffer size
	// --------------------------------------------------------------------------
	unsigned int ITextFile::Read(char* buffer, unsigned int count)
	{
		GetStream();
		return count;
	}


	// --------------------------------------------------------------------------						
	// Function:	Write
	// Description:	write file from buffer
	// Arguments:	buffer, buffer size
	// Returns:		buffer size
	// --------------------------------------------------------------------------
	unsigned int ITextFile::Write(const char* buffer, unsigned int count)
	{
		Exception::Throw("Attempting to write to an istream");
		return 0;
	}


	// --------------------------------------------------------------------------						
	// Function:	Peek
	// Description:	looks and next char but does not return
	// Arguments:	none
	// Returns:		char
	// --------------------------------------------------------------------------
	int ITextFile::Peek()
	{
		return GetStream().peek();
	}


	// --------------------------------------------------------------------------						
	// Function:	Get
	// Description:	gets next char and moves on
	// Arguments:	none
	// Returns:		char
	// --------------------------------------------------------------------------
	int ITextFile::Get()
	{
		return GetStream().get();
	}


	// --------------------------------------------------------------------------						
	// Function:	Read
	// Description:	reads into string stream
	// Arguments:	object to read into
	// Returns:		io stream interface
	// --------------------------------------------------------------------------
	IOInterface& ITextFile::Read(std::stringstream& object)
	{
		std::string str;
		GetStream() >> str;
		object << str;
		return *this;
	}


	// --------------------------------------------------------------------------						
	// Function:	Read
	// Description:	overloaded read functiom
	// Arguments:	object to read into
	// Returns:		io stream interface
	// --------------------------------------------------------------------------
	IOInterface& ITextFile::Read(std::string& object) { GetStream() >> object; return *this; }
	IOInterface& ITextFile::Read(char& object) { GetStream() >> object; return *this; }
	IOInterface& ITextFile::Read(unsigned char& object) { GetStream() >> object; return *this; }
	IOInterface& ITextFile::Read(short& object) { GetStream() >> object; return *this; };
	IOInterface& ITextFile::Read(unsigned short& object) { GetStream() >> object; return *this; }
	IOInterface& ITextFile::Read(int& object) { GetStream() >> object; return *this; }
	IOInterface& ITextFile::Read(unsigned int& object) { GetStream() >> object; return *this; }
	IOInterface& ITextFile::Read(long int& object) { GetStream() >> object; return *this; }
	IOInterface& ITextFile::Read(unsigned long int& object) { GetStream() >> object; return *this; }
	IOInterface& ITextFile::Read(__int64& object) { GetStream() >> object; return *this; }
	IOInterface& ITextFile::Read(float& object) { GetStream() >> object; return *this; }
	IOInterface& ITextFile::Read(double& object) { GetStream() >> object; return *this; }
	IOInterface& ITextFile::Read(bool& object) { GetStream() >> object; return *this; }

	
	// OTextFile ////////////////////////////////////////////////////////////////////


	// --------------------------------------------------------------------------						
	// Function:	OTextFile
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	OTextFile::OTextFile()
	{}

	
	// --------------------------------------------------------------------------						
	// Function:	OTextFile
	// Description:	constructor
	// Arguments:	file to open, flags
	// Returns:		none
	// --------------------------------------------------------------------------
	OTextFile::OTextFile(const std::string& filename, IOInterface::Flags flags)
	{ 
		Open(filename, flags); 
	}


	// --------------------------------------------------------------------------						
	// Function:	Open
	// Description:	open file
	// Arguments:	file to open, flags
	// Returns:		none
	// --------------------------------------------------------------------------
	void OTextFile::Open(const std::string& filename, IOInterface::Flags flags)
	{
		myStream = new std::fstream(filename.c_str(), flags | std::ios_base::out);
	}


	// --------------------------------------------------------------------------						
	// Function:	Read
	// Description:	catch error
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------	
	unsigned int OTextFile::Read(char* buffer, unsigned int count)
	{
		Exception::Throw("Attempting to read to an ostream");
		return 0;
	}
	

	// --------------------------------------------------------------------------						
	// Function:	Write
	// Description:	writes buffer to stream
	// Arguments:	beuffer, buffer size
	// Returns:		beuffer size
	// --------------------------------------------------------------------------
	unsigned int OTextFile::Write(const char* buffer, unsigned int count)
	{
		GetStream().write(reinterpret_cast<const char*>(buffer), count);
		return count;
	}
	

	// --------------------------------------------------------------------------						
	// Function:	Peek
	// Description:	error catch
	// Arguments:	none
	// Returns:		0
	// --------------------------------------------------------------------------
	int OTextFile::Peek()
	{
		Exception::Throw("Attempting to read to an ostream");
		return 0;
	}


	// --------------------------------------------------------------------------						
	// Function:	Put
	// Description:	writes char
	// Arguments:	char
	// Returns:		none
	// --------------------------------------------------------------------------
	void OTextFile::Put(char c)
	{
		GetStream().put(c);
	}


	// --------------------------------------------------------------------------						
	// Function:	Write
	// Description:	writes string stream object
	// Arguments:	object
	// Returns:		io stream interfaace
	// --------------------------------------------------------------------------
	IOInterface& OTextFile::Write(std::stringstream& object)
	{
		GetStream() << object.str();
		return *this;
	}

	// --------------------------------------------------------------------------						
	// Function:	Write
	// Description:	overloaded writes object
	// Arguments:	object
	// Returns:		io stream interfaace
	// --------------------------------------------------------------------------
	IOInterface& OTextFile::Write(const std::string& object) { GetStream() << object; return *this; }
	IOInterface& OTextFile::Write(const char* object) { GetStream() << object; return *this; }
	IOInterface& OTextFile::Write(const char object) { GetStream() << object; return *this; }
	IOInterface& OTextFile::Write(const unsigned char object) { GetStream() << object; return *this; }
	IOInterface& OTextFile::Write(const short object) { GetStream() << object; return *this; };
	IOInterface& OTextFile::Write(const unsigned short object) { GetStream() << object; return *this; }
	IOInterface& OTextFile::Write(const int object) { GetStream() << object; return *this; }
	IOInterface& OTextFile::Write(const unsigned int object) { GetStream() << object; return *this; };
	IOInterface& OTextFile::Write(const long int& object) { GetStream() << object; return *this; }
	IOInterface& OTextFile::Write(const unsigned long int& object) { GetStream() << object; return *this; }
	IOInterface& OTextFile::Write(const __int64& object) { GetStream() << object; return *this; }
	IOInterface& OTextFile::Write(const float& object) { GetStream() << object; return *this; }
	IOInterface& OTextFile::Write(const double& object) { GetStream() << object; return *this; }
	IOInterface& OTextFile::Write(const bool& object) { GetStream() << object; return *this; }


	// TextFile //////////////////////////////////////////////////////////////////////////
	

	// --------------------------------------------------------------------------						
	// Function:	TextFile
	// Description:	constructor
	// Arguments:	file to open, flags
	// Returns:		none
	// --------------------------------------------------------------------------
	TextFile::TextFile(const std::string& filename, IOInterface::Flags flags)
	{ 
		Open(filename, flags); 
	}


	// --------------------------------------------------------------------------						
	// Function:	Open
	// Description:	opens file
	// Arguments:	file to open, flags
	// Returns:		none
	// --------------------------------------------------------------------------
	void TextFile::Open(const std::string& filename, IOInterface::Flags flags)
	{
		myStream = new std::fstream(filename.c_str(), flags); {};
	}


	// --------------------------------------------------------------------------						
	// Function:	Read
	// Description:	read into buffer
	// Arguments:	buffer, amount to read
	// Returns:		count
	// --------------------------------------------------------------------------
	unsigned int TextFile::Read(char* buffer, unsigned int count)
	{
		return ITextFile::Read(buffer, count);
	}
	

	// --------------------------------------------------------------------------						
	// Function:	Write
	// Description:	writes buffer
	// Arguments:	buffer, amount to write
	// Returns:		num written
	// --------------------------------------------------------------------------
	unsigned int TextFile::Write(const char* buffer, unsigned int count)
	{
		return OTextFile::Write(buffer, count);
	}
	

	// --------------------------------------------------------------------------						
	// Function:	Peek
	// Description:	looks at next char but doent move on
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	int TextFile::Peek()
	{
		return ITextFile::Peek();
	}



}