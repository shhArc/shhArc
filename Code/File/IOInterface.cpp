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

#include "IOInterface.h"
#include "IODictionary.h"

namespace shh {

	// --------------------------------------------------------------------------						
	// Function:	IOInterface
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	IOInterface::IOInterface() : 
		myArchiveTags(false), 
		myVersion(0) 
	{}


	// --------------------------------------------------------------------------						
	// Function:	~IOInterface
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	IOInterface::~IOInterface() 
	{}


	// --------------------------------------------------------------------------						
	// Function:	Read
	// Description:	error catcher
	// Arguments:	buffer, beuffer size
	// Returns:		none
	// --------------------------------------------------------------------------
	unsigned int IOInterface::Read(char* buffer, unsigned int count) 
	{ 
		Exception::Throw("Attempting to read to an ostream when not implemented");
		return -1; 
	}


	// --------------------------------------------------------------------------						
	// Function:	Write
	// Description:	error catcher
	// Arguments:	buffer, beuffer size
	// Returns:		none
	// --------------------------------------------------------------------------
	unsigned int IOInterface::Write(const char* buffer, unsigned int count) 
	{ 
		Exception::Throw("Attempting to write to a istream when not implemented"); 
		return -1; 
	}


	// --------------------------------------------------------------------------						
	// Function:	Read
	// Description:	overloaded stream read
	// Arguments:	object
	// Returns:		io interface
	// --------------------------------------------------------------------------
	IOInterface& IOInterface::Read(std::string& object)
	{
		unsigned int size;
		Read(reinterpret_cast<char*>(&size), sizeof(size));
		object.resize(size);
		Read(reinterpret_cast<char*>(&object[0]), size);
		//object[size] = '\0';
		return *this;
	}
	IOInterface& IOInterface::Read(std::stringstream& object)
	{
		StrStreamBuf* buf = reinterpret_cast<StrStreamBuf*>(object.rdbuf());
		Read(buf->IPtr(), (unsigned int)(buf->IEnd() - buf->IBegin()));
		return *this;
	}
	IOInterface& IOInterface::Read(char& object)
	{
		Read(reinterpret_cast<char*>(&object), sizeof(object));
		return *this;
	}
	IOInterface& IOInterface::Read(unsigned char& object)
	{
		Read(reinterpret_cast<char*>(&object), sizeof(object));
		return *this;
	}
	IOInterface& IOInterface::Read(short& object)
	{
		Read(reinterpret_cast<char*>(&object), sizeof(object));
		return *this;
	}
	IOInterface& IOInterface::Read(unsigned short& object)
	{
		Read(reinterpret_cast<char*>(&object), sizeof(object));
		return *this;
	}
	IOInterface& IOInterface::Read(int& object)
	{
		Read(reinterpret_cast<char*>(&object), sizeof(object));
		return *this;
	}
	IOInterface& IOInterface::Read(unsigned int& object)
	{
		Read(reinterpret_cast<char*>(&object), sizeof(object));
		return *this;
	}
	IOInterface& IOInterface::Read(long int& object)
	{
		Read(reinterpret_cast<char*>(&object), sizeof(object));
		return *this;
	}
	IOInterface& IOInterface::Read(unsigned long int& object)
	{
		Read(reinterpret_cast<char*>(&object), sizeof(object));
		return *this;
	}
	IOInterface& IOInterface::Read(__int64& object)
	{
		Read(reinterpret_cast<char*>(&object), sizeof(object));
		return *this;
	}
	IOInterface& IOInterface::Read(float& object)
	{
		Read(reinterpret_cast<char*>(&object), sizeof(object));
		return *this;
	}
	IOInterface& IOInterface::Read(double& object)
	{
		Read(reinterpret_cast<char*>(&object), sizeof(object));
		return *this;
	}
	IOInterface& IOInterface::Read(bool& object)
	{
		Read(reinterpret_cast<char*>(&object), sizeof(object));
		return *this;
	}


	// --------------------------------------------------------------------------						
	// Function:	Write
	// Description:	overloaded stream write
	// Arguments:	object
	// Returns:		io interface
	// --------------------------------------------------------------------------	
	IOInterface& IOInterface::Write(const std::string& object)
	{
		unsigned int size = (unsigned int)object.size();
		Write(reinterpret_cast<const char*>(&size), sizeof(size));
		Write(reinterpret_cast<const char*>(object.c_str()), (unsigned int)object.size());
		return *this;
	}
	IOInterface& IOInterface::Write(const std::stringstream& object)
	{
		StrStreamBuf* buf = reinterpret_cast<StrStreamBuf*>(object.rdbuf());
		Write(buf->OPtr(), (unsigned int)(buf->OEnd() - buf->OBegin()));
		return *this;
	}
	IOInterface& IOInterface::Write(const char* object)
	{
		const char* start = object;
		while (*object != 0)
		{
			object++;
		}
		Write(start, (unsigned int)(object - start));
		return *this;
	}
	IOInterface& IOInterface::Write(const char object)
	{
		Write(reinterpret_cast<const char*>(&object), sizeof(object));
		return *this;
	}
	IOInterface& IOInterface::Write(const unsigned char object)
	{
		Write(reinterpret_cast<const char*>(&object), sizeof(object));
		return *this;
	}

	IOInterface& IOInterface::Write(const short object)
	{
		Write(reinterpret_cast<const char*>(&object), sizeof(object));
		return *this;
	}
	IOInterface& IOInterface::Write(const unsigned short object)
	{
		Write(reinterpret_cast<const char*>(&object), sizeof(object));
		return *this;
	}
	IOInterface& IOInterface::Write(const int object)
	{
		Write(reinterpret_cast<const  char*>(&object), sizeof(object));
		return *this;
	}
	IOInterface& IOInterface::Write(const unsigned int object)
	{
		Write(reinterpret_cast<const  char*>(&object), sizeof(object));
		return *this;
	}
	IOInterface& IOInterface::Write(const long int object)
	{
		Write(reinterpret_cast<const  char*>(&object), sizeof(object));
		return *this;
	}
	IOInterface& IOInterface::Write(const unsigned long int object)
	{
		Write(reinterpret_cast<const  char*>(&object), sizeof(object));
		return *this;
	}
	IOInterface& IOInterface::Write(const __int64 object)
	{
		Write(reinterpret_cast<const  char*>(&object), sizeof(object));
		return *this;
	}
	IOInterface& IOInterface::Write(const float object)
	{
		Write(reinterpret_cast<const  char*>(&object), sizeof(object));
		return *this;
	}
	IOInterface& IOInterface::Write(const double object)
	{
		Write(reinterpret_cast<const  char*>(&object), sizeof(object));
		return *this;
	}
	IOInterface& IOInterface::Write(const bool object)
	{
		Write(reinterpret_cast<const  char*>(&object), sizeof(object));
		return *this;
	}


	// --------------------------------------------------------------------------						
	// Function:	InstanticateClass
	// Description:	error catcher
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	IOInterface::ConstructionPair IOInterface::InstanticateClass() 
	{ 
		Exception::Throw("Attempting to instantiate when not implemented");
		return ConstructionPair(); 
	}


	// --------------------------------------------------------------------------						
	// Function:	InitializeStreamIn
	// Description:	inital call when reading an object
	// Arguments:	class name returned
	// Returns:		none
	// --------------------------------------------------------------------------
	void IOInterface::InitializeStreamIn(std::string& classname) 
	{ 
		Read(classname); 
	}


	// --------------------------------------------------------------------------						
	// Function:	InitializeStreamOut
	// Description:	inital call when reading an object
	// Arguments:	class name to write
	// Returns:		none
	// --------------------------------------------------------------------------
	void IOInterface::InitializeStreamOut(const std::string& classname) 
	{ 
		Write(classname); 
	}


	// --------------------------------------------------------------------------						
	// Function:	InitializeStreamOut
	// Description:	inital call when reading an object
	// Arguments:	class name to write
	// Returns:		none
	// --------------------------------------------------------------------------
	void IOInterface::InitializeStreamOut(const char classname[]) 
	{ 
		InitializeStreamOut(std::string(classname)); 
	}


	// --------------------------------------------------------------------------						
	// Function:	FinalizeClassSerial
	// Description:	call after writing an object
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void IOInterface::FinalizeClassSerial() 
	{}


	// --------------------------------------------------------------------------						
	// Function:	SetVarname
	// Description:	sets current var name before object io
	// Arguments:	name
	// Returns:		none
	// --------------------------------------------------------------------------
	void IOInterface::SetVarname(const std::string& name) 
	{ 
		myVarname = name; 
	}


	// --------------------------------------------------------------------------						
	// Function:	ClearVarname
	// Description:	clears current varname after objecrt io
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void IOInterface::ClearVarname(const std::string& name) 
	{ 
		myVarname.clear(); 
	}


	// --------------------------------------------------------------------------						
	// Function:	peek
	// Description:	looks at char in stream but doesnt move on
	// Arguments:	none
	// Returns:		chat
	// --------------------------------------------------------------------------
	int IOInterface::peek() 
	{ 
		return Peek(); 
	}


	// --------------------------------------------------------------------------						
	// Function:	eof
	// Description:	tests if end of file
	// Arguments:	none
	// Returns:		bool
	// --------------------------------------------------------------------------
	bool IOInterface::eof() 
	{ 
		return Eof(); 
	}


	// --------------------------------------------------------------------------						
	// Function:	get
	// Description:	gext char from stream
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	int IOInterface::get() 
	{
		unsigned char c; 
		Read(reinterpret_cast<char*>(&c), 1); 
		return c; 
	}


	// --------------------------------------------------------------------------						
	// Function:	()
	// Description:	tests if more to read
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	IOInterface::operator void* ()
	{ 
		return Peek() != -1 ? (void*)1 : (void*)0; 
	}


	// IO Interface Streaming functions //////////////////////////////////////////


	// --------------------------------------------------------------------------						
	// Function:	>>
	// Description:	overloaded stream in
	// Arguments:	io stream insterface, object
	// Returns:		io interface
	// --------------------------------------------------------------------------
	IOInterface& operator>>(IOInterface& io, std::stringstream& object)
	{
		io.Read(object);
		return io;
	}
	IOInterface& operator>>(IOInterface& io, std::string& object)
	{
		io.Read(object);
		return io;
	}
	IOInterface& operator>>(IOInterface& io, char& object)
	{
		io.Read(object);
		return io;
	}
	IOInterface& operator>>(IOInterface& io, unsigned char& object)
	{
		io.Read(object);
		return io;
	}
	IOInterface& operator>>(IOInterface& io, short& object)
	{
		io.Read(object);
		return io;
	}
	IOInterface& operator>>(IOInterface& io, unsigned short& object)
	{
		io.Read(object);
		return io;
	}
	IOInterface& operator>>(IOInterface& io, int& object)
	{
		io.Read(object);
		return io;
	}
	IOInterface& operator>>(IOInterface& io, __int64& object)
	{
		io.Read(object);
		return io;
	}
	IOInterface& operator>>(IOInterface& io, unsigned int& object)
	{
		io.Read(object);
		return io;
	}
	IOInterface& operator>>(IOInterface& io, long int& object)
	{
		io.Read(object);
		return io;
	}
	IOInterface& operator>>(IOInterface& io, unsigned long int& object)
	{
		io.Read(object);
		return io;
	}
	IOInterface& operator>>(IOInterface& io, float& object)
	{
		io.Read(object);
		return io;
	}
	IOInterface& operator>>(IOInterface& io, double& object)
	{
		io.Read(object);
		return io;
	}
	IOInterface& operator>>(IOInterface& io, bool& object)
	{
		io.Read(object);
		return io;
	}

	// --------------------------------------------------------------------------						
	// Function:	<<
	// Description:	overloaded stream out
	// Arguments:	io stream insterface, object
	// Returns:		io interface
	// --------------------------------------------------------------------------
	IOInterface& operator<<(IOInterface& io, const std::stringstream& object)
	{
		io.Write(object);
		return io;
	}
	IOInterface& operator<<(IOInterface& io, const std::string& object)
	{
		io.Write(object);
		return io;
	}
	IOInterface& operator<<(IOInterface& io, const char* object)
	{
		io.Write(object);
		return io;
	}
	IOInterface& operator<<(IOInterface& io, const char object)
	{
		io.Write(object);
		return io;
	}
	IOInterface& operator<<(IOInterface& io, const unsigned char object)
	{
		io.Write(object);
		return io;
	}
	IOInterface& operator<<(IOInterface& io, const short object)
	{
		io.Write(object);
		return io;
	}
	IOInterface& operator<<(IOInterface& io, const unsigned short object)
	{
		io.Write(object);
		return io;
	}
	IOInterface& operator<<(IOInterface& io, const int object)
	{
		io.Write(object);
		return io;
	}
	IOInterface& operator<<(IOInterface& io, const unsigned int object)
	{
		io.Write(object);
		return io;
	}
	IOInterface& operator<<(IOInterface& io, const long int object)
	{
		io.Write(object);
		return io;
	}
	IOInterface& operator<<(IOInterface& io, const unsigned long int object)
	{
		io.Write(object);
		return io;
	}
	IOInterface& operator<<(IOInterface& io, const __int64 object)
	{
		io.Write(object);
		return io;
	}
	IOInterface& operator<<(IOInterface& io, const float object)
	{
		io.Write(object);
		return io;
	}
	IOInterface& operator<<(IOInterface& io, const double object)
	{
		io.Write(object);
		return io;
	}
	IOInterface& operator<<(IOInterface& io, const bool object)
	{
		io.Write(object);
		return io;
	}

}// namespace shh