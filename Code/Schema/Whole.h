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

#ifndef WHOLE_H
#define WHOLE_H


#include "../Common/SecureStl.h"
#include "Collection.h"

namespace shh {


	class Whole
	{

	public:
	
		unsigned int CreateCollection(const std::string collectionName, GCPtr<Collection> &collection);
		bool GetCollection(const std::string& collectionName, GCPtr<Collection> &collection);
		bool GetCollection(unsigned int id, GCPtr<Collection>& collection);
		unsigned int AddPart(const std::string& collectionName, const std::string& name, GCPtr<GCObject> object);
		unsigned int AddPart(unsigned int collectionId, const std::string &partName, GCPtr<GCObject>& obj);
		bool GetPart(const std::string& collectionName, unsigned int id, GCPtr<GCObject>& obj);
		bool GetPart(const std::string& collectionName, const std::string partName, GCPtr<GCObject>& obj);
		bool GetPart(unsigned int collectionId, unsigned int id, GCPtr<GCObject>& obj);
		bool GetPart(unsigned int collectionId, const std::string partName, GCPtr<GCObject>& obj);
		void DestroyCollection(const std::string& collectionName);
		void DestroyCollection(unsigned int collectionId);

		static Whole*GetActiveWhole();
		
	private:

		Collection myCollections;

	};

}
#endif // WHOLE_H