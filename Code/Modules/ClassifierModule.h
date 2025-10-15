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

#ifndef CLASSIFIERMODULE_H
#define CLASSIFIERMODULE_H


#include "../Common/SecureStl.h"
#include "../Common/Classifier.h"
#include "../Arc/Module.h"


namespace shh {

	class ClassifierModule : public Module
	{
	public:

		virtual bool RegisterTypes(const std::string& alias, const StringKeyDictionary& sd);
		virtual bool Register(const std::string& alias, const StringKeyDictionary& sd, Privileges privileges);
		virtual std::string GetName() const { return GetNameStatic(); }
		static std::string GetNameStatic() { return StripName(std::string(typeid(ClassifierModule).name())); }

		static std::string Stringer(void const* const data);
		static bool Valuer(const std::string& format, void*& data, int& type);

		static int TypeCheck(lua_State* L);

		static ExecutionState New0(Classifier& result);
		static ExecutionState Clear(Classifier& c);
		static ExecutionState Add(Classifier& c, std::string& label);
		static ExecutionState Remove(Classifier& c, std::string& label);
		static ExecutionState Intersect(Classifier& c1, Classifier& c2, bool& result);
		static ExecutionState Superset(Classifier& c1, Classifier& c2, bool& result);
		static ExecutionState Subset(Classifier& c1, Classifier& c2, bool& result);
		static ExecutionState ToString(Classifier& c, std::string& result);
		static ExecutionState MetaFuncADD(Classifier& c1, Classifier& c2, Classifier& result);
		static ExecutionState MetaFuncSUB(Classifier& c1, Classifier& c2, Classifier& result);


	};

};


#endif	//CLASSIFIERMODULE_H