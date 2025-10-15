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

#ifndef TEXTFILE_H
#define TEXTFILE_H

#ifdef _MSC_VER
#pragma warning( disable : 4250)
#endif

#include "../Common/SecureStl.h"
#include "../Common/Debug.h"
#include "../Common/Exception.h"
#include "IOInterface.h"
#include <ios>
#include <iostream>
#include <fstream>

namespace shh
{

	class BaseTextFile : public IOInterface
	{
		
	public:

		BaseTextFile();
		~BaseTextFile();
		void Close();

		virtual std::streamoff GetPos();
		virtual void SeekOff(seekpos off, seekdir way);
		virtual bool Eof();

	protected:

		std::iostream* myStream;

	};


	class ITextFile : public virtual BaseTextFile
	{
	public:

		ITextFile();
		ITextFile(const std::string& filename, IOInterface::Flags flags = None);

		void Open(const std::string& filename, IOInterface::Flags flags = None);

		inline std::istream& GetStream() { return *myStream; }

		virtual unsigned int Read(char* buffer, unsigned int count);
		virtual unsigned int Write(const char* buffer, unsigned int count);
		virtual int Peek();
		virtual int Get();

		virtual IOInterface& Read(std::stringstream& object);
		virtual IOInterface& Read(std::string& object);
		virtual IOInterface& Read(char& object);
		virtual IOInterface& Read(unsigned char& object);
		virtual IOInterface& Read(short& object);
		virtual IOInterface& Read(unsigned short& object);
		virtual IOInterface& Read(int& object);
		virtual IOInterface& Read(unsigned int& object);
		virtual IOInterface& Read(long int& object);
		virtual IOInterface& Read(unsigned long int& object);
		virtual IOInterface& Read(__int64& object);
		virtual IOInterface& Read(float& object);
		virtual IOInterface& Read(double& object);
		virtual IOInterface& Read(bool& object);

	};



	class OTextFile : public virtual BaseTextFile
	{
	public:

		OTextFile();
		OTextFile(const std::string& filename, IOInterface::Flags flags = None);
		void Open(const std::string& filename, IOInterface::Flags flags = None);

		inline std::ostream& GetStream() { return *myStream; };
		virtual unsigned int Read(char* buffer, unsigned int count);
		inline virtual unsigned int Write(const char* buffer, unsigned int count);
		virtual int Peek();
		virtual void Put(char c);

		virtual IOInterface& Write(std::stringstream& object);
		virtual IOInterface& Write(const std::string& object);
		virtual IOInterface& Write(const char* object);
		virtual IOInterface& Write(const char object);
		virtual IOInterface& Write(const unsigned char object);
		virtual IOInterface& Write(const short object);
		virtual IOInterface& Write(const unsigned short object);
		virtual IOInterface& Write(const int object);
		virtual IOInterface& Write(const unsigned int object);
		virtual IOInterface& Write(const long int& object);
		virtual IOInterface& Write(const unsigned long int& object);
		virtual IOInterface& Write(const __int64& object);
		virtual IOInterface& Write(const float& object);
		virtual IOInterface& Write(const double& object);
		virtual IOInterface& Write(const bool& object);

	};



	class TextFile : public OTextFile, public ITextFile
	{
	public:

		TextFile(const std::string& filename, IOInterface::Flags flags);
		void Open(const std::string& filename, IOInterface::Flags flags);

		inline std::iostream& GetStream() { return *myStream; };

		virtual unsigned int Read(char* buffer, unsigned int count);
		virtual unsigned int Write(const char* buffer, unsigned int count);
		virtual int Peek();

	};

}
#endif	// TextFFILE_H
