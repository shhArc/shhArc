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

#ifndef DEBUG_H
#define DEBUG_H

#include "SecureStl.h"

#ifdef _WIN64

#include <intrin.h>
#define DEBUGBREAK __debugbreak();

#else

#include <pthread.h>
#define DEBUGBREAK __builtin_trap();
#define TCHAR char
#define __TEXT(quote) quote
#define _MAX_PATH 256
#endif


namespace shh {

	class Classifier;

	enum ReportCode { RC_IGNORE, RC_RETRY, RC_ABORT };
	typedef ReportCode(*DebugReportFunction)(const char*);

	class TraceInterface
	{
	public:
		TraceInterface();
		virtual ~TraceInterface();
		virtual void Trace(const Classifier& cls, const char* text) const = 0;
		static void StaticTrace(const Classifier& cls, const char* text);
	};

	DebugReportFunction& GetErrorReportFunction();
	DebugReportFunction& GetMessageReportFunction();
	void OurTrace(const Classifier& cls, const char* format, ...);
	void OurReleaseTrace(const char* format, ...);
	void OurErrorTrace(const char* format, ...);
	void OurDebugTrace(const char* format, ...);


#ifdef _DEBUG
	bool OurDebugAssert(const char* file, int line, const char* expr, const char* type);
#define DEBUG_ASSERT(EXPR)\
		if (!(EXPR))\
		{\
			if (shh::OurDebugAssert(__FILE__, __LINE__, #EXPR,"DEBUG_ASSERT"))\
				DEBUGBREAK\
		}

#define RELEASE_ASSERT(EXPR)\
		if (!(EXPR))\
		{\
			if (OurDebugAssert(__FILE__, __LINE__, #EXPR,"RELEASE_ASSERT"))\
				DEBUGBREAK\
		}

#else
#define DEBUG_ASSERT(EXPR)

	void OurReleaseAssert(const char* file, int line, const char* expr);
#define RELEASE_ASSERT(EXPR)\
		if (!(EXPR))\
		{\
			shh::OurReleaseAssert(__FILE__, __LINE__, #EXPR);\
		}

#endif // _DEBUG

}

#define RELEASE_TRACE shh::OurReleaseTrace
#define ERROR_TRACE shh::OurErrorTrace

#ifdef _DEBUG
#define DEBUG_TRACE shh::OurDebugTrace
#else
#define DEBUG_TRACE
#endif // _DEBUG


#endif // DEBUG_H

