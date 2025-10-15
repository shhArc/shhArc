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

#ifndef WHOLEMODULE_H
#define WHOLEMODULE_H


#include "../Common/SecureStl.h"
#include "../Arc/Module.h"




namespace shh {



	class WholeModule : public Module
	{
	public:

		virtual bool Register(const std::string& alias, const StringKeyDictionary& sd, Privileges privileges);
		virtual std::string GetName() const { return GetNameStatic(); }
		static std::string GetNameStatic() { return StripName(std::string(typeid(WholeModule).name())); }

		static ExecutionState CreateCollection(std::string& collectionName, unsigned int& id);
		static ExecutionState DestroyCollectionName(std::string& collectionName);
		static ExecutionState DestroyCollectionId(unsigned int& collectionId);
		static ExecutionState DestroyPartNameName(std::string& collectionName, std::string& partName);
		static ExecutionState DestroyPartNameId(std::string& collectionName, unsigned int& partId);
		static ExecutionState DestroyPartIdName(unsigned int& collectionId, std::string& partName);
		static ExecutionState DestroyPartIdId(unsigned int& collectionId, unsigned int& partId);

	};

}
#endif // AGENTMODULE_H

