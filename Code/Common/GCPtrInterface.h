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
#ifndef GCPTRINTERFACE_H
#define GCPTRINTERFACE_H


#include "SecureStl.h"


namespace shh {


	// GCPtrInterface ///////////////////////////////////////////////////////////////////////////

	template< typename T, template <typename T> class BASE> class GCPtrInterface : public BASE<T>
	{

		friend typename T;

	public:

		GCPtrInterface();
		explicit GCPtrInterface(T* object);
		template< typename Other > GCPtrInterface(GCPtrInterface<Other, BASE> const& other);
		template< typename Other > GCPtrInterface(GCPtrInterface<Other, BASE> const& other, bool dummy);
		GCPtrInterface(GCPtrInterface const& other);
		~GCPtrInterface();


		template< typename Other > inline bool DynamicCast(GCPtrInterface<Other, BASE> const& other);
		template< typename Other > inline bool StaticCast(GCPtrInterface<Other, BASE> const& other);
		template< typename Other > inline GCPtrInterface& operator=(GCPtrInterface< Other, BASE > const& other);
		template< typename Other > inline bool operator<(GCPtrInterface< Other, BASE > const& other) const;
		inline operator bool() const;

		inline bool IsValid() const;
		inline void Destroy();
		inline void SetNull();

		inline T* operator->() const;
		inline T& operator*() const;

		inline T* GetObject() const;
		inline bool IsMemoryManaged() const;

	};


	// --------------------------------------------------------------------------						
	// Function:	DynamicCast
	// Description:	does what is say on the tin
	// Arguments:	object to cast
	// Returns:		cast object
	// --------------------------------------------------------------------------
	template<template <typename T> class BASE, typename A, typename B >
	inline GCPtrInterface<A, BASE> DynamicCast(GCPtrInterface<B, BASE> const& b, GCPtrInterface<A, BASE>* aa = (GCPtrInterface<A, BASE>*)0)
	{
		GCPtrInterface<A, BASE> a;
		a.DynamicCast(b);
		return a;
	}

	// --------------------------------------------------------------------------						
	// Function:	StaticCast
	// Description:	does what is say on the tin
	// Arguments:	object to cast
	// Returns:		cast object
	// --------------------------------------------------------------------------
	template<template <typename T> class BASE, typename A, typename B >
	inline GCPtrInterface<A, BASE> StaticCast(GCPtrInterface<B, BASE> const& b, GCPtrInterface<A, BASE>* aa = (GCPtrInterface<A, BASE>*)0)
	{
		GCPtrInterface<A, BASE> a;
		a.StaticCast(b);
		return a;
	}

	// --------------------------------------------------------------------------						
	// Function:	==
	// Description:	compare is same object pointed to
	// Arguments:	object a, object b
	// Returns:		if same
	// --------------------------------------------------------------------------
	template<template <typename T> class BASE, typename A, typename B >
	inline bool operator==(GCPtrInterface<A, BASE> const& a, GCPtrInterface<B, BASE> const& b)
	{
		return (BASE<A>&)a == (BASE<B>&)b;
	}


	// --------------------------------------------------------------------------						
	// Function:	!=
	// Description:	compare is not same object pointed to
	// Arguments:	object a, object b
	// Returns:		if not same
	// --------------------------------------------------------------------------
	template<template <typename T> class BASE, typename A, typename B >
	inline bool operator!=(GCPtrInterface<A, BASE> const& a, GCPtrInterface<B, BASE> const& b)
	{
		return (BASE<A>&)a != (BASE<B>&)b;
	}



	// GCPtr Inlines ////////////////////////////////////////////////////////////////


	// --------------------------------------------------------------------------						
	// Function:	GCPtrInterface
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	template< typename T, template <typename> class BASE> GCPtrInterface<T, BASE>::GCPtrInterface() :
		BASE<T>()
	{}
	

	// --------------------------------------------------------------------------						
	// Function:	GCPtrInterface
	// Description:	constructor
	// Arguments:	object to this will point to
	// Returns:		none
	// --------------------------------------------------------------------------
	template< typename T, template <typename> class BASE> GCPtrInterface<T, BASE>::GCPtrInterface(T* object) :
		BASE<T>(object)
	{ 
	}
	


	// --------------------------------------------------------------------------						
	// Function:	GCPtrInterface
	// Description:	constructor
	// Arguments:	ptr to object this will point to
	// Returns:		none
	// --------------------------------------------------------------------------
	template< typename T, template <typename> class BASE> 
	template< typename Other > GCPtrInterface<T, BASE>::GCPtrInterface(GCPtrInterface<Other, BASE> const& other) :
		BASE<T>(other)
	{ 
	}


	// --------------------------------------------------------------------------						
	// Function:	GCPtrInterface
	// Description:	constructor
	// Arguments:	ptr to object this will point to
	// Returns:		none
	// --------------------------------------------------------------------------
	template< typename T, template <typename> class BASE> 
	template< typename Other > GCPtrInterface<T, BASE>::GCPtrInterface(GCPtrInterface<Other, BASE> const& other, bool dummy) :
		BASE<T>(other, dummy)
	{ 
	} 	//Static-cast constructor
	

	// --------------------------------------------------------------------------						
	// Function:	GCPtrInterface
	// Description:	constructor
	// Arguments:	ptr to object this will point to
	// Returns:		none
	// --------------------------------------------------------------------------
	template< typename T, template <typename> class BASE> GCPtrInterface<T, BASE>::GCPtrInterface(GCPtrInterface const& other) :
		BASE<T>(other)
	{ 
	}
	

	// --------------------------------------------------------------------------						
	// Function:	~GCPtrInterface
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	template< typename T, template <typename> class BASE> GCPtrInterface<T, BASE>::~GCPtrInterface()
	{ 
	}


	// --------------------------------------------------------------------------						
	// Function:	DynamicCast
	// Description:	cast object sent in and assigns to this pointer
	// Arguments:	object to cast
	// Returns:		if sucessful
	// --------------------------------------------------------------------------
	template< typename T, template <typename> class BASE>
	template< typename Other > inline bool GCPtrInterface<T, BASE>::DynamicCast(GCPtrInterface<Other, BASE> const& other)
	{
		*this = GCPtrInterface();
		return BASE<T>::DynamicCast(other);
	}


	// --------------------------------------------------------------------------						
	// Function:	StaticCast
	// Description:	cast object sent in and assigns to this pointer
	// Arguments:	object to cast
	// Returns:		if sucessful
	// --------------------------------------------------------------------------
	template< typename T, template <typename> class BASE>
	template< typename Other > inline bool GCPtrInterface<T, BASE>::StaticCast(GCPtrInterface<Other, BASE> const& other)
	{
		*this = GCPtrInterface();
		return BASE<T>::StaticCast(other);
	}


	// --------------------------------------------------------------------------						
	// Function:	=
	// Description:	assigns pointer
	// Arguments:	pointer to set to
	// Returns:		this
	// --------------------------------------------------------------------------	
	template< typename T, template <typename> class BASE>
	template< typename Other > inline GCPtrInterface<T, BASE>& GCPtrInterface<T, BASE>::operator=(GCPtrInterface< Other, BASE > const& other)
	{ 
		BASE<T>::Assign(other);
		return *this;
	}


	// --------------------------------------------------------------------------						
	// Function:	<
	// Description:	test if object address if less than other pointer
	// Arguments:	other pointer
	// Returns:		true if less
	// --------------------------------------------------------------------------
	template< typename T, template <typename> class BASE>
	template< typename Other > inline bool GCPtrInterface<T, BASE>::operator<(GCPtrInterface< Other, BASE > const& other) const
	{
		return (*(BASE<T>*)this) < (*(BASE<Other>*)&other);
	}
	

	// --------------------------------------------------------------------------						
	// Function:	bool
	// Description:	returns memory pointer is valid
	// Arguments:	none
	// Returns:		true if valid
	// --------------------------------------------------------------------------
	template< typename T, template <typename> class BASE>
	inline GCPtrInterface<T, BASE>::operator bool() const
	{
		return IsValid();
	}

	// --------------------------------------------------------------------------						
	// Function:	IsValid
	// Description:	test if object pointed to is valid
	// Arguments:	none
	// Returns:		bool
	// --------------------------------------------------------------------------
	template< typename T, template <typename> class BASE> inline bool GCPtrInterface<T, BASE>::IsValid() const
	{
		return  BASE<T>::IsValid();
	}


	// --------------------------------------------------------------------------						
	// Function:	Destroy
	// Description:	destroys object
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	template< typename T, template <typename> class BASE> inline void GCPtrInterface<T, BASE>::Destroy()
	{
		BASE<T>::Destroy();
	}


	// --------------------------------------------------------------------------						
	// Function:	SetNull
	// Description:	clear pointer
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	template< typename T, template <typename> class BASE> inline void GCPtrInterface<T, BASE>::SetNull()
	{
		BASE<T>::Swap(*this);
	}


	// --------------------------------------------------------------------------						
	// Function:	->
	// Description:	dereference
	// Arguments:	none
	// Returns:		object pointed to
	// --------------------------------------------------------------------------
	template< typename T, template <typename> class BASE> inline T* GCPtrInterface<T, BASE>::operator->() const
	{
		return BASE<T>::GetObject();
	}


	// --------------------------------------------------------------------------						
	// Function:	*
	// Description:	dereference
	// Arguments:	none
	// Returns:		object pointed to
	// --------------------------------------------------------------------------
	template< typename T, template <typename> class BASE> inline T& GCPtrInterface<T, BASE>::operator*() const
	{
		return *BASE<T>::GetObject();;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetObject
	// Description:	dereference
	// Arguments:	none
	// Returns:		object pointed to
	// --------------------------------------------------------------------------
	template< typename T, template <typename> class BASE> inline T* GCPtrInterface<T, BASE>::GetObject() const
	{
		return BASE<T>::GetObject();
	}


	// --------------------------------------------------------------------------						
	// Function:	IsMemoryManaged
	// Description:	returns if memory managed
	// Arguments:	none
	// Returns:		if managed
	// --------------------------------------------------------------------------
	template< typename T, template <typename> class BASE> inline bool GCPtrInterface<T, BASE>::IsMemoryManaged() const
	{
		return BASE<T>::IsMemoryManaged();
	}


}

#endif // GCPTR_H
