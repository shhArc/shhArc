///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 David K Bhowmik. All rights reserved.
// This file is part of shhArc.
//
// This Software is available under the MIT License with a No Modification clause
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

#ifndef DLLINTERFACE_H
#define DLLINTERFACE_H

#include "G"CPtr.h"

namespace shh {

	class Registry;

	struct DllInitializers
	{
		enum ReportCode { RC_IGNORE, RC_RETRY, RC_ABORT };
		typedef ReportCode(*DebugReportFunction)(const char*);

		DebugReportFunction& (*GetErrorReportFunction)();
		DebugReportFunction& (*GetMessageReportFunction)();
	};

	typedef void (*DllRegisteringFunction)(Registry* registry, const DllInitializers& init, const std::string& cmdLine);

	//void SetDll(Registry* r, MemoryManager* mm);

	/*
	DllRegisteringFunction God::LoadDll(std::string name)
	{
#ifdef _DEBUG
		name += "_d";
#endif
#ifdef _WIN64
		HMODULE LoadMe = LoadLibraryA((LPCSTR)name.c_str());
		if (LoadMe)
		{
			DllRegisteringFunction func = (DllRegisteringFunction)GetProcAddress(LoadMe, "RegisterDll");
			if (func == NULL)
				ERROR_TRACE("Failed to find dll interface function RegisterDll in %s.\n", name.c_str());
			else
				return func;
		}
		else
		{
			DWORD dw = GetLastError();
			ERROR_TRACE("%s failed to load with error %d", name.c_str(), dw);
		}
#endif
		return NULL;
	}
	*/
}
#endif
