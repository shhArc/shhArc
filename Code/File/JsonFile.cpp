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
#include "JsonFile.h"
#include <json/json.h>

namespace shh {

	static bool ConvertToDict(const Json::Value& object, StringKeyDictionary& dict);
	static bool ConvertToDict(const Json::Value& object, ArrayKeyDictionary& dict);
	static bool ConvertToValue(const StringKeyDictionary& dict, Json::Value& object);
	static bool ConvertToValue(const ArrayKeyDictionary& dict, Json::Value& object);



	// --------------------------------------------------------------------------						
	// Function:	Read
	// Description:	reads file and store in doct
	// Arguments:	file, dict
	// Returns:		if successfil
	// --------------------------------------------------------------------------
	bool JsonFile::Read(IBinaryFile& file, StringKeyDictionary& dict)
	{
		Json::Value root;
		Json::CharReaderBuilder rbuilder;
		std::string errs;

		bool parsingSuccessful = Json::parseFromStream(rbuilder, file.GetStream(), &root, &errs);

		if (!parsingSuccessful)
			return false;
		
		RELEASE_ASSERT(root.isObject());
		return ConvertToDict(root, dict);
		
	}
	
	
	bool ConvertToDict(const Json::Value& object, StringKeyDictionary& dict)
	{
		// JsonCpp uses getMemberNames() to get the keys of an object
		Json::Value::Members members = object.getMemberNames();

		for (const std::string& key : members) 
		{
			const Json::Value& value = object[key]; // Access the value by key
			Json::ValueType type = value.type();


			if (value.isNull())
			{
				dict.Set(key, Variant::NullType());
			}
			else if (value.isBool()) 
			{
				dict.Set(key, value.asBool());
			}
			else if (type == Json::intValue)
			{
				dict.Set(key, (long)value.asInt64());
			}
			else if (type == Json::realValue)
			{
				dict.Set(key, (double)value.asDouble());
			}
			else if (value.isString()) 
			{
				dict.Set(key, value.asString());
			}
			else if (value.isArray())
			{
				ArrayKeyDictionary subDict;
				if (ConvertToDict(value, subDict))
					dict.Set(key, subDict);
			}
			else if (value.isObject())
			{
				StringKeyDictionary subDict;
				if (ConvertToDict(value, subDict))
					dict.Set(key, subDict);
			}
		}
		return true;
	}
	



	bool ConvertToDict(const Json::Value& object, ArrayKeyDictionary& dict)
	{
		for (Json::ArrayIndex i = 0; i < object.size(); i++) 
		{
			const Json::Value& value = object[i]; // Access the value by index
			Json::ValueType type = value.type();


			if (value.isNull())
			{
				dict.Set(i, Variant::NullType());
			}
			else if (value.isBool())
			{
				dict.Set(i, value.asBool());
			}
			else if (type == Json::intValue)
			{
				dict.Set(i, (long)value.asInt64());
			}
			else if (type == Json::realValue)
			{
				dict.Set(i, (double)value.asDouble());
			}
			else if (value.isString())
			{
				dict.Set(i, value.asString());
			}
			else if (value.isArray())
			{
				ArrayKeyDictionary subDict;
				if (ConvertToDict(value, subDict))
					dict.Set(i, subDict);
			}
			else if (value.isObject())
			{
				StringKeyDictionary subDict;
				if (ConvertToDict(value, subDict))
					dict.Set(i, subDict);
			}
		}
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	Write
	// Description:	writes dict to file
	// Arguments:	file, dict
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool JsonFile::Write(OBinaryFile& file, const StringKeyDictionary& dict)
	{
		Json::Value root;
		ConvertToValue(dict, root);
		file.GetStream() << root;
		return true;
	}


	bool ConvertToValue(const StringKeyDictionary& dict, Json::Value& object)
	{

		for (StringKeyDictionary::VariablesConstIterator it = dict.Begin(); it != dict.End(); it++)
		{
			std::string key = *it->first;
			Variant &value = *it->second;

			if (value.IsType<Variant::NullType>())
			{
				Variant::NullType data;
				value.Get(data);
				//object[key.c_str()] = data;
			}
			else if (value.IsType<bool>())
			{
				bool data;
				value.Get(data);
				object[key] = data;
			}
			else if (value.IsType<int>())
			{
				int data;
				value.Get(data);
				object[key] = data;
			}
			else if (value.IsType<double>())
			{
				double data;
				value.Get(data);
				object[key] = data;
			}
			else if (value.IsType<std::string>())
			{
				std::string data;
				value.Get(data);
				object[key] = data;
			}
			else if (value.IsType<ArrayKeyDictionary>())
			{
				Json::Value subDict;
				ArrayKeyDictionary data;
				value.Get(data);
				if (ConvertToValue(data, subDict))
					object[key] = subDict;
			}
			else if (value.IsType<StringKeyDictionary>())
			{
				Json::Value subDict;
				StringKeyDictionary data;
				value.Get(data);
				if (ConvertToValue(data, subDict))
					object[key] = subDict;
			}
		}
		return true;
	}




	bool ConvertToValue(const ArrayKeyDictionary& dict, Json::Value& object)
	{
		int i = 0;
		for (ArrayKeyDictionary::VariablesConstIterator it = dict.Begin(); it != dict.End(); it++, i++)
		{
			Variant& value = *it->second;

			if (value.IsType<Variant::NullType>())
			{
				Variant::NullType data;
				value.Get(data);
				//object[i] = data;
			}
			else if (value.IsType<bool>())
			{
				bool data;
				value.Get(data);
				object[i] = data;
			}
			else if (value.IsType<int>())
			{
				int data;
				value.Get(data);
				object[i] = data;
			}
			else if (value.IsType<double>())
			{
				double data;
				value.Get(data);
				object[i] = data;
			}
			else if (value.IsType<std::string>())
			{
				std::string data;
				value.Get(data);
				object[i] = data;
			}
			else if (value.IsType<ArrayKeyDictionary>())
			{
				Json::Value subDict;
				ArrayKeyDictionary data;
				value.Get(data);
				if (ConvertToValue(data, subDict))
					object[i] = subDict;
			}
			else if (value.IsType<StringKeyDictionary>())
			{
				Json::Value subDict;
				StringKeyDictionary data;
				value.Get(data);
				if (ConvertToValue(data, subDict))
					object[i] = subDict;
			}
		}		
		return true;
	}


} // namespace shh
