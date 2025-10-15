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


#include "../Config/GCPtr.h"
#include "../Arc/Api.h"
#include "../File/FileSystem.h"
#include "SystemModule.h"
#include <string>
#include <time.h> 
#include <sys/timeb.h>
#include <stdarg.h>

#ifdef _WIN64
#define ftime _ftime
#define timeb _timeb
#define snprintf _snprintf
#define ctime_r _ctime_r
#endif

//! /library System
//! Operating system level functions.

namespace shh {


	// --------------------------------------------------------------------------						
	// Function:	Register
	// Description:	registers functions and variables to be used by a process
	// Arguments:	alies of module and type, dictionary of configuration vars
	// Returns:		if sucessfull
	// --------------------------------------------------------------------------
	bool SystemModule::Register(const std::string& alias, const StringKeyDictionary& sd, Privileges privileges)
	{
		GCPtr<Module> me(this);

		Api::OpenNamespace(alias);

		Api::RegisterFunction("AbsoluteTime", AbsoluteTime, 1, me);
		Api::RegisterFunction("DateTimeString", DateTimeString, 1, me);
		Api::RegisterFunction("GMDateTimeString", GMDateTimeString, 1, me);
		Api::RegisterFunction("USADateTimeString", USADateTimeString, 1, me);
		Api::RegisterFunction("IsValidPath", IsValidPath, 2, me);
		Api::RegisterFunction("GetDirectoryContents", GetDirectoryContents, 4, me);
		Api::RegisterFunction("GetLabeledPath", GetLabeledPath, 2, me);

		if (!(privileges & GodPrivilege))
			Api::RegisterFunction("SetLabeledPath", SetLabeledPath, 3, me);

		Api::CloseNamespace();

		return true;
	}


	//! /namespace System
	//! /function AbsoluteTime
	//! /returns double
	//! Returns the system time.
	ExecutionState SystemModule::AbsoluteTime(double &t)
	{
		time_t ltime;
		time(&ltime);
		t = (double)ltime;
		return ExecutionOk;
	}


	//! /namespace System
	//! /function DateTimeString
	//! /returns string
	//! Returns data and time as a string.
	ExecutionState SystemModule::DateTimeString(std::string &dt)
	{
		time_t aclock;
		time(&aclock);
		struct tm* time = localtime(&aclock);
		std::string timestr = asctime(time);
		dt = timestr.substr(0, timestr.length() - 1);
		return ExecutionOk;
	}


	//! /namespace System
	//! /function GMDateTimeString
	//! /returns string
	//! Returns Greenwich Mean Time and Date as a string.
	ExecutionState SystemModule::GMDateTimeString(std::string &dt)
	{
		time_t aclock;
		time(&aclock);
		struct tm* time = gmtime(&aclock);
		std::string timestr = asctime(time);
		dt = timestr.substr(0, timestr.length() - 1);
		return ExecutionOk;
	}


	//! /namespace System
	//! /function USADateTimeString
	//! /returns string
	//! Returns USA date and time as a string.
	ExecutionState SystemModule::USADateTimeString(std::string &dt)
	{
		char tmpbuf[128];
		std::string timestr;
		time_t mytime = time(NULL);
		strftime(tmpbuf, 50, "%b. %d, %Y", localtime(&mytime));
		timestr = tmpbuf;
#ifdef _WIN64
		_strtime(tmpbuf);
#else
		ctime_r(&mytime, tmpbuf);
#endif
		struct timeb tstruct;
		ftime(&tstruct);
		timestr += " ";
		timestr += tmpbuf;
		timestr += " ";
		snprintf(tmpbuf, 127, "%u", tstruct.millitm);
		timestr += tmpbuf;

		dt = timestr.substr(0, timestr.length());
		return ExecutionOk;
	}


	//! /namespace System
	//! /function IsValidPath
	//! /param string path
	//! /returns boolean
	//! Checks if path exists.
	ExecutionState SystemModule::IsValidPath(std::string &path, bool &ok)
	{
		ok = FileSystem::IsValidPath(path);
		return ExecutionOk;
	}


	//! /namespace System
	//! /function GetDirectoryContents
	//! /param string directory+path
	//! /param string wildcard
	//! /param int flags
	//! /returns table_of_strings
	//! Returns table of the contents of the directory path.
	//! flags: GetFiles = 1, GetDirectories = 2, GetEverything = 3
	ExecutionState SystemModule::GetDirectoryContents(std::string &directory, std::string &wildcard, unsigned int &flags, VariantKeyDictionary &contents)
	{
		std::vector<std::string> dirs;
		FileSystem::GetDirectoryContents(directory, wildcard, (FileSystem::Filter)flags, dirs);
		for (int i = 0; i != dirs.size(); i++)
			contents.Set(NonVariant<int>(i + 1), dirs[i]);
		
		return ExecutionOk;
	}


	//! /namespace System
	//! /function GetLabeledPath
	//! /param string label
	//! /returns string
	//! Returns the path that of given label.
	ExecutionState SystemModule::GetLabeledPath(std::string &label, std::string &fullPath)
	{
		FileSystem::GetVariable(label, fullPath);
		return ExecutionOk;
	}


	//! /namespace System
	//! /function SetLabeledPath
	//! /privilege God
	//! /param string label
	//! /param string path
	//! Sets the path that of given label.
	ExecutionState SystemModule::SetLabeledPath(std::string& label, std::string& fullPath)
	{
		FileSystem::SetVariable(label, fullPath);
		return ExecutionOk;
	}
}