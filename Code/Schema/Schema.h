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

#ifndef SCHEMA_H
#define SCHEMA_H


#include "../Common/SecureStl.h"
#include "../Common/Dictionary.h"
#include "../Common/Classifier.h"
#include "../Config/GCPtr.h"
#include "../Arc/Environment.h"
#include "../VM/Object.h"
#include <vector>


namespace shh
{
	class Schema : public virtual GCObject
	{
	public:

		typedef std::vector<GCPtr<Schema>> Schemas;

		class NullType {};

		static bool LoadSchema(const GCPtr<Environment>& env, const GCPtr<Schema>& parent, const std::string& file, const std::string& metaFile);
		static bool ValidateSchema(const GCPtr<Environment>& env, const StringKeyDictionary& spec, const StringKeyDictionary& meta);
		static bool ExpressSchema(const GCPtr<Environment>& env, const GCPtr<Schema>& parent, const StringKeyDictionary& spec);
		static GCPtr<Object> Create(const GCPtr<Environment>& env, const GCPtr<Schema>& parent, const std::string& typeName, const std::string& className, const StringKeyDictionary& spec);

		virtual bool RequiresConfiguration() const;
		virtual bool Configure(const StringKeyDictionary& conf);
		const GCPtr<Schema>& GetParent() const;
		const Schemas& GetSchemas() const;
		Schemas GetSubSchemas(const std::string& type) const;
		void AddSchema(const GCPtr<Schema>& s);
		void RemoveSchema(const GCPtr<Schema>& s);
		void DestorySchemas();

		const std::string &GetName() const;
		const std::string& GetType() const;
		unsigned int GetTypeCode() const;
		static unsigned int GetTypeCode(const std::string& type);
		bool IsExpressed() const;

		static const std::string ourSchemaFileExtension;


	protected:

		std::string myType;
		std::string myName;
		unsigned int myTypeCode;
		GCPtr<Schema> myParent;
		Schemas mySchemas;
		bool myExpressed;

		static Classifier ourTypeCodes;

		Schema();
		virtual ~Schema();
		

	};
}

#endif