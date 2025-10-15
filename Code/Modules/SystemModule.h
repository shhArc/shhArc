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

#ifndef SYSTEMMODULE_H
#define SYSTEMMODULE_H


#include "../Common/SecureStl.h"
#include "../Arc/Module.h"

namespace shh {


	class SystemModule : public Module
	{
	public:


		virtual bool Register(const std::string& alias, const StringKeyDictionary& sd, Privileges privileges);
		virtual std::string GetName() const { return GetNameStatic(); }
		static std::string GetNameStatic() { return StripName(std::string(typeid(SystemModule).name())); }

			
		static ExecutionState AbsoluteTime(double &t);
		static ExecutionState DateTimeString(std::string &dt);
		static ExecutionState GMDateTimeString(std::string &dt);
		static ExecutionState USADateTimeString(std::string &dt);
		static ExecutionState IsValidPath(std::string &path, bool &ok);
		static ExecutionState GetDirectoryContents(std::string &directory, std::string &wildcard, unsigned int &flags, VariantKeyDictionary &contents);
		static ExecutionState GetLabeledPath(std::string &label, std::string &fullPath);
		static ExecutionState SetLabeledPath(std::string& label, std::string& fullPath);

	};

}

#endif // SHHARCMODULE_H