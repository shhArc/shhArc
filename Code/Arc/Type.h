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

#ifndef TYPE_H
#define TYPE_H

#include "../Common/SecureStl.h"
#include "../Common/BaseType.h"
#include "../Config/GCPtr.h"
#include "Registry.h"
#include <string>
#include <vector>

namespace shh 
{


	template <class T>
	class Type : public BaseType
	{

	public:

		Type(SupportedImps supportedImp, bool allowInterVM);
		virtual bool IsIntegralType() const;
		virtual int GetImplementationId(Implementation i) const;
		virtual int GetId() const;
		static int GetTypeId();
		virtual int GetRawId() const;
		static inline int GetRawTypeId();
		virtual const std::string& GetName() const;
		static inline const std::string& GetTypeName();
		virtual const std::string& GetIdString() const;

		virtual void* Clone(void* v) const;
		virtual void* Destroy(void* v) const;

		virtual bool Push(Implementation i, void* v) const;
		virtual bool CanSendBetween(Implementation i) const;

		static BaseType* GetStatic();
		virtual BaseType* GetStaticVirtual() const;
		virtual bool SetDictionary(void* key, void* value, void* dict) const;


	protected:

		static bool SetType();
		static void SetSubType();

	protected:

		static PushImplementations ourPushImplementations;
		static std::vector<int> ourImplementationIds;
		static bool ourIntegralType;
	private:

		static int ourTypeId;
		static std::string ourTypeName;
		static Type ourStatic;
		static SupportedImps ourSupportedImps;
		static bool ourAllowInterVM;


	};

	template<class T> BaseType::PushImplementations Type<T>::ourPushImplementations;
	template<class T> std::vector<int> Type<T>::ourImplementationIds;
	template<class T> bool Type<T>::ourIntegralType = true;
	template<class T> int Type<T>::ourTypeId = 0;
	template<class T> std::string Type<T>::ourTypeName = typeid(T).name();
	template<class T> Type<T> Type<T>::ourStatic(NoImps, false);
	template<class T> SupportedImps Type<T>::ourSupportedImps;
	template<class T> bool Type<T>::ourAllowInterVM = false;


	// --------------------------------------------------------------------------						
	// Function:	Type
	// Description:	constructor
	// Arguments:	implemtation language type is being register for, whether 
	//				this type can be sent by inter VM messaging
	// Returns:		none
	// --------------------------------------------------------------------------
	template <class T> Type<T>::Type(SupportedImps supportedImp, bool allowInterVM)
	{
		ourSupportedImps = supportedImp | ourSupportedImps;
		ourAllowInterVM = allowInterVM;

		if (ourPushImplementations.empty())
		{
			ourPushImplementations.resize(TOTAL_IMPLEMENTATIONS);
			for (int i = 0; i != ourPushImplementations.size(); i++)
				ourPushImplementations[i] = NULL;
		}
		if (ourImplementationIds.empty())
		{
			ourImplementationIds.resize(TOTAL_IMPLEMENTATIONS);
			for (int i = 0; i != ourImplementationIds.size(); i++)
				ourImplementationIds[i] = 0;
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	IsIntegralType
	// Description:	returns if this type is integral to the language
	// Arguments:	none
	// Returns:		bool
	// --------------------------------------------------------------------------
	template <class T> bool Type<T>::IsIntegralType() const { return ourIntegralType; }


	// --------------------------------------------------------------------------						
	// Function:	GetImplementationId
	// Description:	returns id of type as used in a particular language
	// Arguments:	language
	// Returns:		type id
	// --------------------------------------------------------------------------
	template <class T> int Type<T>::GetImplementationId(Implementation i) const { return ourImplementationIds[i]; }
	

	// --------------------------------------------------------------------------						
	// Function:	GetId
	// Description:	returns absolute value of type id as used in registry
	// Arguments:	none
	// Returns:		type id
	// --------------------------------------------------------------------------	
	template <class T> int Type<T>::GetId() const { return GetTypeId(); }


	// --------------------------------------------------------------------------						
	// Function:	GetTypeId
	// Description:	returns absolute value of type id as used in registry
	// Arguments:	none
	// Returns:		type id
	// --------------------------------------------------------------------------
	template <class T> int Type<T>::GetTypeId() { return abs(ourTypeId); }


	// --------------------------------------------------------------------------						
	// Function:	GetRawId
	// Description:	returns type id as used in registry
	// Arguments:	none
	// Returns:		type id
	// --------------------------------------------------------------------------
	template <class T> int Type<T>::GetRawId() const { return GetRawTypeId(); }


	// --------------------------------------------------------------------------						
	// Function:	IsIntegralType
	// Description:	returns type id as used in registry
	// Arguments:	none
	// Returns:		type id
	// --------------------------------------------------------------------------
	template <class T> inline int Type<T>::GetRawTypeId() { return ourTypeId; }


	// --------------------------------------------------------------------------						
	// Function:	GetName
	// Description:	returns type name
	// Arguments:	none
	// Returns:		string
	// --------------------------------------------------------------------------
	template <class T>const std::string& Type<T>::GetName() const { return GetTypeName(); }


	// --------------------------------------------------------------------------						
	// Function:	GetTypeName
	// Description:	returns type name
	// Arguments:	none
	// Returns:		string
	// --------------------------------------------------------------------------
	template <class T> inline const std::string& Type<T>::GetTypeName() { return ourTypeName; }


	// --------------------------------------------------------------------------						
	// Function:	GetIdString
	// Description:	returns type name
	// Arguments:	none
	// Returns:		string
	// --------------------------------------------------------------------------
	template <class T> const std::string& Type<T>::GetIdString() const { return ourTypeName; }


	// --------------------------------------------------------------------------						
	// Function:	Clone
	// Description:	clones (new) a value of this type
	// Arguments:	pointer to value to clone
	// Returns:		new value
	// --------------------------------------------------------------------------
	template <class T> void* Type<T>::Clone(void* v) const
	{
		T* cloned = new T(*static_cast<T*>(v));
		return cloned;
	}
	

	// --------------------------------------------------------------------------						
	// Function:	Destroy
	// Description:	destroys (deletes) a value of this type
	// Arguments:	value
	// Returns:		NULL
	// --------------------------------------------------------------------------
	template <class T>void* Type<T>::Destroy(void* v) const { delete static_cast<T*>(v); 	return NULL; }


	// --------------------------------------------------------------------------						
	// Function:	Push
	// Description:	pushed a value on the current stack for a particular language
	// Arguments:	language, value to push
	// Returns:		if successfull
	// --------------------------------------------------------------------------
	template <class T> bool Type<T>::Push(Implementation i, void* v) const
	{
		if (ourPushImplementations[i] != NULL)
		{
			ourPushImplementations[i](v);
			return true;
		}
		else
		{
			return false;
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	CanSendBetween
	// Description:	returns if this type can be sent between languages
	// Arguments:	implementation of receiver
	// Returns:		bool
	// --------------------------------------------------------------------------
	template <class T> bool Type<T>::CanSendBetween(Implementation i) const
	{
		return (ourAllowInterVM && (ourSupportedImps == AllImps || ((1 << (int)i) & ourSupportedImps)));
	}


	// --------------------------------------------------------------------------						
	// Function:	GetStatic
	// Description:	returns static object of this type
	// Arguments:	none
	// Returns:		object
	// --------------------------------------------------------------------------
	template <class T> BaseType* Type<T>::GetStatic() { return &ourStatic; }
	
	
	// --------------------------------------------------------------------------						
	// Function:	GetStaticVirtual
	// Description:	returns static object of this type
	// Arguments:	none
	// Returns:		object
	// --------------------------------------------------------------------------	
	template <class T> BaseType* Type<T>::GetStaticVirtual() const { return &ourStatic; }
	
	

	// --------------------------------------------------------------------------						
	// Function:	SetType
	// Description:	sets the registered type id for this type
	// Arguments:	none
	// Returns:		true if newly set, false of already set
	// --------------------------------------------------------------------------	
	template <class T> bool Type<T>::SetType()
	{
		if (ourTypeId == 0)
		{
			ourTypeId = Registry::GetRegistry().GetNewTypeId();
			return true;
		}
		return false;
	}

	
	// --------------------------------------------------------------------------						
	// Function:	SetSubType
	// Description:	sets if this is a sub type of an integral
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------	
	template <class T> void Type<T>::SetSubType() { ourTypeId = -abs(ourTypeId); }

	
}

#endif //BASETYPE_H
