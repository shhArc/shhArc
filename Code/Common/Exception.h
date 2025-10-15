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

#ifndef EXCEPTION_H
#define EXCEPTION_H

// See Debug.h for notes on usage



#include "SecureStl.h"
#include "Debug.h"
#include <string>
#include <exception>

namespace shh
{

	class Exception : public std::exception
	{
	public:

		virtual ~Exception();

		virtual const char* what() const throw();
		const wchar_t* What() const throw();

		static void Throw(const char* format, ...);

	protected:

		Exception(const std::string& description);
		Exception();

		mutable std::string myDescription;
	};

#define THROW(S) \
{\
	char buffer[1024];\
	_snprintf_s(buffer, 1023, "Exception raised in file '%s', line %d: %s",\
		__FILE__, __LINE__, S);\
		shh::Exception::Throw(buffer);\
}


}	

#endif // EXCEPTION_H
