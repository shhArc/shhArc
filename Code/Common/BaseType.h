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

#ifndef BASETYPE_H
#define BASETYPE_H

#include "SecureStl.h"
#include "Enums.h"
#include <string>
#include <vector>

namespace shh 
{
	class BaseType
	{
	public:

		typedef void (*Function)(void*);
		typedef std::vector<Function> PushImplementations;

		virtual bool IsIntegralType() const = 0;
		virtual int GetImplementationId(Implementation i) const = 0;
		virtual int GetId() const = 0;
		virtual int GetRawId() const = 0;
		virtual const std::string& GetName() const = 0;
		virtual const std::string& GetIdString() const = 0;
		virtual void* Clone(void* v) const = 0;
		virtual void* Destroy(void* v) const = 0;
		virtual bool Push(Implementation i, void* v) const = 0;
		virtual bool CanSendBetween(Implementation i) const = 0;
		virtual BaseType *GetStaticVirtual() const = 0;
		virtual bool SetDictionary(void* key, void* value, void* dict) const = 0;

	protected:

		BaseType() {};

	};
}

#endif //BASETYPE_H
