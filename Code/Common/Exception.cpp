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

#include "Exception.h"
#include <string>
#include <stdio.h>
#include <stdarg.h>
#define _vsnprintf vsnprintf

namespace shh {

	// --------------------------------------------------------------------------						
	// Function:	Exception
	// Description:	constructor
	// Arguments:	execption description
	// Returns:		none
	// --------------------------------------------------------------------------
	Exception::Exception(const std::string& description) :
		myDescription(description) {}


	// --------------------------------------------------------------------------						
	// Function:	Exception
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Exception::Exception() {};


	// --------------------------------------------------------------------------						
	// Function:	~Exception
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Exception::~Exception() {}


	// --------------------------------------------------------------------------						
	// Function:	what
	// Description:	returns exception desription as ascii string
	// Arguments:	none
	// Returns:		string
	// --------------------------------------------------------------------------
	const char* Exception::what() const throw()
	{
		return myDescription.c_str();
	}


	// --------------------------------------------------------------------------						
	// Function:	What
	// Description:	returns exception desription as wide string
	// Arguments:	none
	// Returns:		string
	// --------------------------------------------------------------------------
	const wchar_t* Exception::What() const throw()
	{
		static std::wstring wide(myDescription.begin(), myDescription.end());
		return wide.c_str();
	}


	// --------------------------------------------------------------------------						
	// Function:	Throw
	// Description:	throw exception
	// Arguments:	valiable prinf style arg list
	// Returns:		none
	// --------------------------------------------------------------------------
	void Exception::Throw(const char* format, ...)
	{
		// Variable argument list support.
		va_list ap;
		va_start(ap, format);

		const int bufferSize = 1024;
		char trace[bufferSize];
		_vsnprintf(trace, bufferSize, format, ap);

		// End variable argument list support.
		va_end(ap);

		throw Exception(trace);
	}


} 
