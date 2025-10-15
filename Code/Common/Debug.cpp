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

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "SecureStl.h"
#include "Debug.h"
#include "Exception.h"
#include "Classifier.h"

#ifdef _WIN64
#include <windows.h>
#include <crtdbg.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <set>
#include <iostream>


#ifndef _vsnprintf
#define _vsnprintf vsnprintf
#endif

namespace shh {

	// --------------------------------------------------------------------------						
	// Function:	GetErrorReportFunction
	// Description:	gets the function that displays error message
	// Arguments:	none
	// Returns:		reference to function pointer (so function can be set too)
	// --------------------------------------------------------------------------
	DebugReportFunction& GetErrorReportFunction()
	{
		static DebugReportFunction errorReportFunction = NULL;
		return errorReportFunction;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetMessageReportFunction
	// Description:	gets the function that displays general messages
	// Arguments:	none
	// Returns:		reference to function pointer (so function can be set too)
	// --------------------------------------------------------------------------
	DebugReportFunction& GetMessageReportFunction()
	{
		static DebugReportFunction messageReportFunction = NULL;
		return messageReportFunction;
	}


	class BuiltinTrace : public TraceInterface
	{

		void Trace(const Classifier& cls, const char* text) const
		{
			std::string full = cls.DumpString() + " " + text;
#ifdef _WIN64
			std::wstring wide(full.begin(), full.end());
			OutputDebugString((LPCWSTR)wide.c_str());
#else
			std::cerr << full.c_str();
#endif
			DebugReportFunction errorReportFunction = GetErrorReportFunction();
			if (errorReportFunction != NULL)
				if (cls.Get().find("error") != cls.Get().end())
					errorReportFunction(full.c_str());
		}
	};


	static std::set<TraceInterface*> ourTracers;

	// --------------------------------------------------------------------------						
	// Function:	TraceInterface
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	TraceInterface::TraceInterface()
	{
		ourTracers.insert(this);
	}


	// --------------------------------------------------------------------------						
	// Function:	~TraceInterface
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	TraceInterface::~TraceInterface()
	{
		ourTracers.erase(this);
	}


	// --------------------------------------------------------------------------						
	// Function:	StaticTrace
	// Description:	dispatches a trace message to all existing tracers
	// Arguments:	classifier for trace, text to trace
	// Returns:		none
	// --------------------------------------------------------------------------
	void TraceInterface::StaticTrace(const Classifier& cls, const char* text)
	{
		// Create builtin tracer now, so we're sure it always exists while logging does.
		static BuiltinTrace ourBuiltinTrace;

		for (std::set<TraceInterface*>::iterator it = ourTracers.begin();
			it != ourTracers.end(); ++it)
		{
			(*it)->Trace(cls, text);
		}
	}

	
	// --------------------------------------------------------------------------						
	// Function:	Report
	// Description:	dispatches a trace message to all existing tracers and
	//				displays a message report using GetMessageReportFunction
	// Arguments:	text to trace, text to report
	// Returns:		false if report returns ignore, true is report returns retry
	//				else quit app
	// --------------------------------------------------------------------------
	static bool Report(char* trace, char* report)
	{
		// output trace message.
		static const Classifier ourReportTraceClassifier =
			Classifier() << "report" << "engine";
		OurTrace(ourReportTraceClassifier, trace);

		int code = 0;
		DebugReportFunction messageReportFunction = GetMessageReportFunction();
		if (messageReportFunction != NULL)
			code = messageReportFunction(report);
		
		// output dialog box.

		// Handle user choice.
		if (code == RC_IGNORE)
			return false;

		if (code == RC_RETRY)
			return true;
#ifdef _WIN64
		ExitProcess(1);
#else
		exit(1);
#endif

		return true;
	}


#ifdef _DEBUG
	// --------------------------------------------------------------------------						
	// Function:	OurDebugAssert
	// Description:	dispatches a trace message to all existing tracers
	//				and reports a message with GetMessageReportFunction
	// Arguments:	file and line of assert, assert expression, type
	// Returns:		false if report returns ignore, true is report returns retry
	//				else quit app
	// --------------------------------------------------------------------------
	bool OurDebugAssert(const char* file, int line, const char* expr, const char* type)
	{
		static bool reentrant = false;

		if (reentrant)
		{
			// Assertion code has reentered itself
			DEBUGBREAK
		}
		reentrant = true;

		char trace[_MAX_PATH * 2];
		char report[_MAX_PATH * 2];

		std::string str = "%s(%hs) failed...\nFile: %hs\nLine: %d\n";
		static std::wstring wide(str.begin(), str.end());
		sprintf(trace, str.c_str(), type, expr, file, line);

		str = "%s(%hs) failed...\nFile: %hs\nLine: %d\n (Click Retry to debug)";
		wide =  std::wstring(str.begin(), str.end());
		sprintf(report, str.c_str(), type, expr, file, line);

		bool ret = Report((char*)trace, (char*)report);
		reentrant = false;

		const char xformat[] = "DEBUG_ASSERT(%hs) failed...\nFile: %hs\nLine: %d\n";
		Exception::Throw(xformat, expr, file, line);
		return ret;
	}
#endif // _DEBUG

	// --------------------------------------------------------------------------						
	// Function:	OurReleaseAssert
	// Description:	Throws exception
	// Arguments:	file and line of assert, assert expression
	// Returns:		false if report returns ignore, true is report returns retry
	//				else quit app
	// --------------------------------------------------------------------------
	void OurReleaseAssert(const char* file, int line, const char* expr)
	{
		const char xformat[] = "RELEASE_ASSERT(%hs) failed...\nFile: %hs\nLine: %d\n";
		Exception::Throw(xformat, expr, file, line);
	}


	// --------------------------------------------------------------------------						
	// Function:	OurInternalTrace
	// Description:	dispatches a trace message to all existing tracers
	//				(used internally by all public Debug, Release, Error 
	//				Trance etc...)
	// Arguments:	classifier for trace, text format to trace, variable args for 
	//				text format
	// Returns:		none
	// --------------------------------------------------------------------------
	void OurInternalTrace(const Classifier& cls, const char* format, va_list ap)
	{
		const int bufferSize = 64 * 1024;
		char trace[bufferSize];

		_vsnprintf(trace, bufferSize - 1, format, ap);
		TraceInterface::StaticTrace(cls, trace);
	}


	// --------------------------------------------------------------------------						
	// Function:	OurTrace
	// Description:	dispatches a trace message to all existing tracers
	// Arguments:	classifier for trace, text format to trace, variable args for 
	//				text format
	// Returns:		none
	// --------------------------------------------------------------------------
	void OurTrace(const Classifier& cls, const char* format, ...)
	{
		va_list ap;
		va_start(ap, format);
		OurInternalTrace(cls, format, ap);
		va_end(ap);
	}


	// --------------------------------------------------------------------------						
	// Function:	OurDebugTrace
	// Description:	dispatches a trace message to all existing tracers, uses 
	//				debug and engine classifers
	// Arguments:	text format to trace, variable args for 
	//				text format
	// Returns:		none
	// --------------------------------------------------------------------------
	void OurDebugTrace(const char* format, ...)
	{
		static const Classifier ourDebugTraceClassifier =
			Classifier() << "debug" << "engine";

		va_list ap;
		va_start(ap, format);
		OurInternalTrace(ourDebugTraceClassifier, format, ap);
		va_end(ap);
	}


	// --------------------------------------------------------------------------						
	// Function:	OurReleaseTrace
	// Description:	dispatches a trace message to all existing tracers, uses
	//				release and engine classifiers
	// Arguments:	text format to trace, variable args for 
	//				text format
	// Returns:		none
	// --------------------------------------------------------------------------
	void OurReleaseTrace(const char* format, ...)
	{
		static const Classifier ourReleaseTraceClassifier =
			Classifier() << "release" << "engine";

		va_list ap;
		va_start(ap, format);
		OurInternalTrace(ourReleaseTraceClassifier, format, ap);
		va_end(ap);
	}


	// --------------------------------------------------------------------------						
	// Function:	OurErrorTrace
	// Description:	dispatches a trace message to all existing tracers, uses
	//				error and engine classifiers
	// Arguments:	text format to trace, variable args for 
	//				text format
	// Returns:		none
	// --------------------------------------------------------------------------
	void OurErrorTrace(const char* format, ...)
	{
		static const Classifier ourErrorTraceClassifier =
			Classifier() << "error" << "engine";

		va_list ap;
		va_start(ap, format);
		try
		{
			OurInternalTrace(ourErrorTraceClassifier, format, ap);
		}
		catch (...)
		{
			DebugReportFunction errorReportFunction = GetErrorReportFunction();
			if (errorReportFunction != NULL)
			{
				char report[200];
				std::string str = "Warning: Error tracing system unstable due to previous errors.\nFormat string: %s";
				static std::wstring wide(str.begin(), str.end());
				sprintf(report, str.c_str(), format);
				errorReportFunction((const char*)report);
			}
		}

		va_end(ap);
	}

} 
