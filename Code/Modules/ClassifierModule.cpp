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
#pragma warning( disable : 4786 4503 )
#endif


#include "../Arc/Api.h"
#include "ClassifierModule.h"	

//! /type Classifier
//! Collection of labels.


namespace shh {


	// --------------------------------------------------------------------------						
	// Function:	RegisterTypes
	// Description:	registers data types to be used by a process (done first)
	// Arguments:	alies of module and type, dictionary of configuration vars
	// Returns:		if sucessfull
	// --------------------------------------------------------------------------
	bool ClassifierModule::RegisterTypes(const std::string& alias, const StringKeyDictionary& sd)
	{
		Classifier* example = NULL;
		Api::LuaRegisterType(example, alias, NULL, NULL, false);
		return Module::RegisterTypes(alias, sd);
	}


	// --------------------------------------------------------------------------						
	// Function:	Register
	// Description:	registers functions and variables to be used by a process
	// Arguments:	alies of module and type, dictionary of configuration vars
	// Returns:		if sucessfull
	// --------------------------------------------------------------------------
	bool ClassifierModule::Register(const std::string& alias, const StringKeyDictionary& sd, Privileges privileges)
	{
		GCPtr<Module> me(this);

		Classifier* example = NULL;
		Api::OpenNamespace("TypeCheck");
		if (Api::GetImplementation() == Lua)
			Api::LuaRegisterFunction(alias, TypeCheck);
		Api::CloseNamespace();

		Api::RegisterFunction(alias, New0, 1, me);
		Api::RegisterMemberFunction(example, "Clear", Clear, 2, me);
		Api::RegisterMemberFunction(example, "Add", Add, 3, me);
		Api::RegisterMemberFunction(example, "Remove", Remove, 3, me);
		Api::RegisterMemberFunction(example, "Intersect", Intersect, 3, me);
		Api::RegisterMemberFunction(example, "Superset", Superset, 3, me);
		Api::RegisterMemberFunction(example, "Subset", Subset, 3, me);
		Api::RegisterMemberFunction(example, "ToString", ToString, 2, me);

		Api::RegisterMemberFunction(example, "__add", MetaFuncADD, 3, me);
		Api::RegisterMemberFunction(example, "__sub", MetaFuncSUB, 3, me);
		
		return Module::Register(alias, sd, privileges);
	}


	// --------------------------------------------------------------------------						
	// Function:	Stringer
	// Description:	converts a value of type used by this module to a string
	//				for printing
	// Arguments:	void pointer to variable of type used by this module
	// Returns:		string
	// --------------------------------------------------------------------------
	std::string ClassifierModule::Stringer(void const* const data)
	{
		Classifier* c = (Classifier*)data;
		return Type< Classifier>::GetTypeName() + c->DumpString();
	}


	// --------------------------------------------------------------------------						
	// Function:	Valuer
	// Description:	converts a string to the tyoe used by this module
	// Arguments:	string to convert, pointer to newed data, type of the
	//				newed data
	// Returns:		if successfull
	// --------------------------------------------------------------------------
	bool ClassifierModule::Valuer(const std::string & format, void*& data, int& type)
	{
		Classifier* c = new Classifier();
		int start = (int)format.find("(") + 1;
		int finalend = (int)format.find(")") - 1;
		std::string datastring = format.substr(start, finalend);
		while (!datastring.empty())
		{
			int end = (int)datastring.find(",");
			std::string str = datastring.substr(0, end - 1);
			datastring = datastring.substr(end + 1, datastring.size());
			c->Add(str);
		}
		data = c;
		type = Type<Classifier>::GetTypeId();
		return true;
	}

	//! /namespace TypeCheck
	//! /function Classifier
	//! /param Classifier variable
	//! /returns boolean
	//! Returns whether variable is a Classifier.
	int ClassifierModule::TypeCheck(lua_State * L)
	{
		Api::LuaCheckNumArguments(L, 1);
		bool ok = LuaTypeBase::GetArgumentType(L, 1) == LuaType<Classifier>::GetLuaId();
		LuaApiTemplate::Return(L, ok);
		return 1;
	}



	//! /member shhCONSTRUCTOR
	//! /function Classifier
	//! /returns classifier
	//! Returns a newly created classiffier.
	ExecutionState ClassifierModule::New0(Classifier& result)
	{
		return ExecutionOk;
	}


	//! /member Classifier
	//! /function Clear
	//! Clears the classifier.
	ExecutionState ClassifierModule::Clear(Classifier& c)
	{
		c.Reset();
		return ExecutionOk;
	}


	//! /member Classifier
	//! /function Add
	//! /param string label_to_add
	//! Adds a string label to the Classifier.
	ExecutionState ClassifierModule::Add(Classifier& c, std::string &label)
	{
		c.Add(label);
		return ExecutionOk;
	}


	//! /member Classifier
	//! /function Remove
	//! /param string label_to_remove
	//! Removes a string label from the Classifier.
	ExecutionState ClassifierModule::Remove(Classifier & c, std::string & label)
	{
		c.Remove(label);
		return ExecutionOk;
	}


	//! /member Classifier
	//! /function Intersect
	//! /param classifier other
	//! /returns boolean
	//! Returns whether the Classifier labels intersect with given others labels.
	ExecutionState ClassifierModule::Intersect(Classifier& c1, Classifier& c2, bool &result)
	{
		result = c1.Match(c2, Classifier::MATCH_ANY);
		return ExecutionOk;
	}

	//! /member Classifier
	//! /function Superset
	//! /param classifier other
	//! /returns boolean
	//! Returns whether classifier matches exactly or is a superset of other classifier.
	ExecutionState ClassifierModule::Superset(Classifier& c1, Classifier& c2, bool& result)
	{
		result = c1.Match(c2, Classifier::MATCH_ALL);
		return ExecutionOk;
	}


	//! /member Classifier
	//! /function Subset
	//! /param classifier other
	//! /returns boolean
	//! Returns whether the Classifier matches exactly or is a subset of other.
	ExecutionState ClassifierModule::Subset(Classifier& c1, Classifier& c2, bool& result)
	{
		result = c2.Match(c1, Classifier::MATCH_ALL);
		return ExecutionOk;
	}


	//! /member Classifier
	//! /function DumpString
	//! /returns string
	//! Returns the list of labels in classifier as a space separated string.
	ExecutionState ClassifierModule::ToString(Classifier& c, std::string &result)
	{
		return ExecutionOk;
	}

	
	//! /member Classifier
	//! /function +
	//! /param classifier arg1
	//! /param classifier arg2
	//! Returns a classifiers that is the combination of arg1 and arg2.
	ExecutionState ClassifierModule::MetaFuncADD(Classifier& c1, Classifier& c2, Classifier& result)
	{
		result = c1;
		result.Add(c2);
		return ExecutionOk;
	}


	//! /member Classifier
	//! /function -
	//! /param Classifier arg1
	//! /param Classifier arg2
	//! Returns a classifier that is arg1 with any labels in arg2 removed from it.
	ExecutionState ClassifierModule::MetaFuncSUB(Classifier& c1, Classifier& c2, Classifier& result)
	{
		result = c1;
		result.Remove(c2);
		return ExecutionOk;
	}


}

