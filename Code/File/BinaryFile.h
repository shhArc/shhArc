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

#ifndef BINARYFILE_H
#define BINARYFILE_H

#include "../Common/SecureStl.h"
#include "../Common/Debug.h"
#include "../Common/Exception.h"
#include "IOInterface.h"
#include <ios>
#include <iostream>
#include <fstream>

namespace shh
{

	class BaseBinaryFile : public IOInterface
	{
		
	public:

		BaseBinaryFile();
		~BaseBinaryFile();
		void Close();

		virtual std::streamoff GetPos();
		inline void SeekOff(seekpos off, seekdir way);
		virtual bool Eof();

	protected:

		std::iostream* myStream;

	};


	class IBinaryFile : public virtual BaseBinaryFile
	{
	public:

		IBinaryFile();
		IBinaryFile(const std::string& filename, IOInterface::Flags flags = None);
		void Open(const std::string& filename, IOInterface::Flags flags = None);

		inline std::istream& GetStream();
		inline const std::istream& GetStream() const;
		virtual unsigned int Read(char* buffer, unsigned int count);
		virtual unsigned int Write(const char* buffer, unsigned int count);

		virtual int Peek();

	};



	class OBinaryFile : public virtual BaseBinaryFile
	{
	public:

		OBinaryFile();
		OBinaryFile(const std::string& filename, IOInterface::Flags flags = IOInterface::None);
		void Open(const std::string& filename, IOInterface::Flags flags = None);

		inline std::ostream& GetStream();
		inline const std::ostream& GetStream() const;
		virtual unsigned int Read(char* buffer, unsigned int count);
		inline virtual unsigned int Write(const char* buffer, unsigned int count);
		virtual int Peek();

	};



	class BinaryFile : public OBinaryFile, public IBinaryFile
	{
	public:

		BinaryFile(const std::string& filename, IOInterface::Flags flags);
		void Open(const std::string& filename, IOInterface::Flags flags);

		virtual unsigned int Read(char* buffer, unsigned int count);
		virtual unsigned int Write(const char* buffer, unsigned int count);
		virtual int Peek();

	};


	// BaseBinaryFile Inlines //////////////////////////////////////////////////////////

	// --------------------------------------------------------------------------						
	// Function:	SeekOff
	// Description:	moves to relative position in file
	// Arguments:	amount to move, direction
	// Returns:		none
	// --------------------------------------------------------------------------
	inline void BaseBinaryFile::SeekOff(seekpos off, seekdir way)
	{
		std::streambuf* streamBuffer = myStream->rdbuf();
		pos_type streampos = streamBuffer->pubseekoff(off, way);
		return;
	};

	// IBinaryFile Inlines ////////////////////////////////////////////////////////////

	// --------------------------------------------------------------------------						
	// Function:	GetStream
	// Description:	returns stream
	// Arguments:	none
	// Returns:		stream
	// --------------------------------------------------------------------------
	inline std::istream& IBinaryFile::GetStream() 
	{ 
		return *myStream; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetStream
	// Description:	returns stream
	// Arguments:	none
	// Returns:		stream
	// --------------------------------------------------------------------------
	inline const std::istream& IBinaryFile::GetStream() const
	{
		return *myStream;
	}

	// OBinaryFile Inlines ////////////////////////////////////////////////////////////

	// --------------------------------------------------------------------------						
	// Function:	GetStream
	// Description:	returns stream
	// Arguments:	none
	// Returns:		stream
	// --------------------------------------------------------------------------
	inline std::ostream& OBinaryFile::GetStream() 
	{ 
		return *myStream; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetStream
	// Description:	returns stream
	// Arguments:	none
	// Returns:		stream
	// --------------------------------------------------------------------------
	inline const std::ostream& OBinaryFile::GetStream() const
	{
		return *myStream;
	}

	// --------------------------------------------------------------------------						
	// Function:	Write
	// Description:	writes chars to tream
	// Arguments:	pointer to chars, number of chars
	// Returns:		number of chars
	// --------------------------------------------------------------------------
	inline unsigned int OBinaryFile::Write(const char* buffer, unsigned int count)
	{
		GetStream().write(reinterpret_cast<const char*>(buffer), count);
		return count;
	}

}
#endif	// BINARYFFILE_H
