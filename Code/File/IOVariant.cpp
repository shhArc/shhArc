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
#include "IOVariant.h"

#ifdef __GNUG__
#include <cxxabi.h>
#include <memory>
#endif


namespace shh {


#ifdef __GNUG__

	std::string Demangle(const char* name) {
		int status = -4;
		std::unique_ptr<char, void(*)(void*)> res{
			abi::__cxa_demangle(name, NULL, NULL, &status),
			std::free
		};
		return (status == 0) ? res.get() : name;
	}
#else
	std::string Demangle(const char* name) {
		return name;
	}
#endif

	std::string ExtractClassName(const char* typeString)
	{
		const std::string full_type = Demangle(typeString);
		std::string className = full_type;

		size_t pos = className.find("std::");
		while (pos != std::string::npos)
		{
			className = className.substr(0, pos) + className.substr(pos + 5, className.length());
			pos = className.find("std::");
		}

		pos = className.find("class ");
		while (pos != std::string::npos)
		{
			className = className.substr(0, pos) + className.substr(pos + 6, className.length());
			pos = className.find("class ");
		}
		pos = className.find("struct ");
		while (pos != std::string::npos)
		{
			className = className.substr(0, pos) + className.substr(pos + 7, className.length());
			pos = className.find("struct ");
		}
		
		pos = className.find("> >");
		while (pos != std::string::npos)
		{
			className = className.substr(0, pos) + ">>"+className.substr(pos + 3, className.length());
			pos = className.find("> >");
		}
		return className;
	}

}
