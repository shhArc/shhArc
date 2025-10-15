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

#ifndef TYPELOG_H
#define TYPELOG_H

#include "SecureStl.h"
#include "BaseType.h"
#include <typeinfo>
#include <map>


namespace shh 
{

	class TypeLogBase
	{
	public:
		typedef std::pair<int, BaseType*> TypePair;
		typedef std::map< std::size_t, TypePair> Map;

		void SetTypeMap(Map* map);

	protected:
		static Map *ourTypeInfoMap;
	};



	template <class T>
	class TypeLog : public TypeLogBase	
	{

	public:

		static inline int GetTypeId();
		static inline const std::string& GetTypeName();
		static inline BaseType* GetStaticType();
		static void Register(int id, BaseType* baseType);
		static void Syncronize();


	private:

		TypeLog();
		~TypeLog();

		static int ourTypeId;
		static std::string ourTypeName;
		static BaseType *ourStatic;

	};

	template<class T> int TypeLog<T>::ourTypeId = 0;
	template<class T> std::string TypeLog<T>::ourTypeName;
	template<class T> BaseType* TypeLog<T>::ourStatic = NULL;


	// --------------------------------------------------------------------------						
	// Function:	GetTypeId
	// Description:	returns the registered type id of T
	// Arguments:	none
	// Returns:		id
	// --------------------------------------------------------------------------
	template <class T> inline int TypeLog<T>::GetTypeId()
	{ 
		return abs(ourTypeId); 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetTypeName
	// Description:	returns the registered type name of T
	// Arguments:	none
	// Returns:		name
	// --------------------------------------------------------------------------
	template <class T> inline const std::string& TypeLog<T>::GetTypeName()
	{ 
		return ourTypeName; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetStaticType
	// Description:	returns the registered type of T
	// Arguments:	none
	// Returns:		type
	// --------------------------------------------------------------------------
	template <class T> inline BaseType* TypeLog<T>::GetStaticType()
	{ 
		return ourStatic; 
	}


	// --------------------------------------------------------------------------						
	// Function:	Register
	// Description:	registered type of T woth Log
	// Arguments:	type id of T, type of T
	// Returns:		none
	// --------------------------------------------------------------------------
	template <class T> void TypeLog<T>::Register(int id, BaseType* baseType)
	{
		(*ourTypeInfoMap)[typeid(T).hash_code()] = TypePair(id, baseType);
	}


	// --------------------------------------------------------------------------						
	// Function:	Syncronize
	// Description:	sets the static member variables of LogType T to those 
	//				contained in ourTypeInfoMap
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	template <class T> void TypeLog<T>::Syncronize()
	{
		Map::iterator it = ourTypeInfoMap->find(typeid(T).hash_code());
		if (it != ourTypeInfoMap->end())
		{
			ourTypeId = it->second.first;
			ourStatic = it->second.second;
			ourTypeName = typeid(T).name();
		}
	}
}

#endif //BASETYPE_H
