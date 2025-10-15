//////////////////////////////////////////////////////////////////////////////
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
#ifndef VARIANT_H
#define VARIANT_H

#include "SecureStl.h"
#include "TypeLog.h"
#include <map>
#include <string>


namespace shh {

	class IOInterface;
	template<class V> class NonVariant;
	template<class V> class IONonVariant;

	// Variant /////////////////////////////////////////////////////////////

	class Variant
	{

	public:


		class NullType
		{
		public:
			bool operator<(const NullType& other) const { return true; }
			bool operator==(const NullType& other) const { return true; }
			bool operator!=(const NullType& other) const { return true; }
		};

		virtual ~Variant() {};
		template<class T> bool Get(T& value) const;
		template<class T> bool GetPtr(T*& value) const;
		template<class T> bool IsType(const T* t = NULL) const;

		virtual const void* GetValuePtr() const = 0;
		virtual unsigned int GetTypeId() const = 0;
		virtual BaseType* GetRegisteredType() const = 0;

		virtual const Variant* New() = 0;
		virtual Variant* NewClone() const = 0;

		virtual Variant &ConvertToIO() const = 0;
		virtual void Write(IOInterface& io, unsigned int version) const = 0;
		virtual void Read(IOInterface& io, unsigned int version) = 0;


		static void* ConstructArchive(IOInterface& io);
		static unsigned int GetNextTypeId();
		virtual std::string GetArchiveAlias() const;

		virtual bool operator<(const Variant& other) const = 0;
		virtual bool operator==(const Variant& other) const = 0;	
		virtual bool operator!=(const Variant& other) const = 0;


	private:
		
		static unsigned int ourLastTypeId;

	};


	// NonVariant /////////////////////////////////////////////////////////////

	template<class V> class NonVariant : public Variant
	{

		template<typename T> friend class IODictionary;
		friend class IONonVariant<V>;

	public:

		NonVariant();
		NonVariant(const V& v);
		NonVariant(const NonVariant<V>& v);

		V& GetValue();
		const V& GetValue() const;
		virtual const void* GetValuePtr() const;

		static unsigned int GetOurTypeId();
		virtual unsigned int GetTypeId() const;
		virtual BaseType* GetRegisteredType() const;

		virtual const Variant* New();
		virtual Variant* NewClone() const;
		
		virtual Variant& ConvertToIO() const;
		virtual void Write(IOInterface& io, unsigned int version) const;
		virtual void Read(IOInterface& io, unsigned int version);

		virtual bool operator<(const Variant& other) const;
		virtual bool operator==(const Variant& other) const;
		virtual bool operator!=(const Variant& other) const;

	
	protected:

		V myValue;
		static unsigned int ourTypeId;

	};


	template<class V> unsigned int NonVariant<V>::ourTypeId = Variant::GetNextTypeId();

	// Variant Inlines //////////////////////////////////////////////////////////////////
	
	
	// --------------------------------------------------------------------------						
	// Function:	Get
	// Description:	gets templated value from variant
	// Arguments:	var to store get in
	// Returns:		true if got
	// --------------------------------------------------------------------------
	template<class T> bool Variant::Get(T& value) const
	{
		const NonVariant<T>* v = dynamic_cast<const NonVariant<T>*>(this);
		if (!v)
			return false;
		value = v->GetValue();
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetPtr
	// Description:	gets templated pointer value from variant
	// Arguments:	var to store get in
	// Returns:		true if got
	// --------------------------------------------------------------------------
	template<class T> bool Variant::GetPtr(T*& value) const
	{
		const NonVariant<T>* v = dynamic_cast<const NonVariant<T>*>(this);
		if (!v)
			return false;
		value = (T*)&v->GetValue();
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	IsType
	// Description:	test if variant is if templated type
	// Arguments:	none
	// Returns:		true if is type
	// --------------------------------------------------------------------------
	template<class T> bool Variant::IsType(const T* t) const
	{
		NonVariant<T> const* const v = dynamic_cast<NonVariant<T> const* const>(this);
		if (v == NULL)
			return false;
		else
			return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetNextTypeId
	// Description:	increments and gets new type id
	// Arguments:	none
	// Returns:		id
	// --------------------------------------------------------------------------
	inline unsigned int Variant::GetNextTypeId() 
	{ 
		return ++ourLastTypeId; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetArchiveAlias
	// Description:	returns name of type as appears in archive file
	// Arguments:	none
	// Returns:		string
	// --------------------------------------------------------------------------
	inline std::string Variant::GetArchiveAlias() const
	{ 
		return ""; 
	}




	// NonVariant Inlines ///////////////////////////////////////////////////////////////

	// --------------------------------------------------------------------------						
	// Function:	NonVariant
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class V> NonVariant<V>::NonVariant() 
	{}


	// --------------------------------------------------------------------------						
	// Function:	NonVariant
	// Description:	constructor
	// Arguments:	variant value
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class V> NonVariant<V>::NonVariant(const V& v) : myValue(v) 
	{};


	// --------------------------------------------------------------------------						
	// Function:	NonVariant	
	// Description:	constructor
	// Arguments:	variant value
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class V> NonVariant<V>::NonVariant(const NonVariant<V>& v) : myValue(v.myValue)
	{};


	// --------------------------------------------------------------------------						
	// Function:	GetValue
	// Description:	get variant value
	// Arguments:	none
	// Returns:		value
	// --------------------------------------------------------------------------
	template<class V> V& NonVariant<V>::GetValue() 
	{ 
		return myValue; 
	}
	template<class V> const V& NonVariant<V>::GetValue() const 
	{ 
		return myValue; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetValuePtr
	// Description:	pointer to the value stored in the variant
	// Arguments:	none
	// Returns:		pointer to value
	// --------------------------------------------------------------------------
	template<class V> const void* NonVariant<V>::GetValuePtr() const 
	{ 
		return &myValue; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetOurTypeId
	// Description:	get type id of nonvariant type
	// Arguments:	none
	// Returns:		id
	// --------------------------------------------------------------------------
	template<class V> unsigned int NonVariant<V>::GetOurTypeId() 
	{ 
		return ourTypeId; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetTypeId
	// Description:	get type id of nonvariant type
	// Arguments:	none
	// Returns:		id
	// --------------------------------------------------------------------------
	template<class V> unsigned int NonVariant<V>::GetTypeId() const 
	{ 
		return ourTypeId; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetRegisteredType
	// Description:	get type class of variants value
	// Arguments:	none
	// Returns:		type
	// --------------------------------------------------------------------------
	template<class V> BaseType* NonVariant<V>::GetRegisteredType() const 
	{ 
		return TypeLog<V>::GetStaticType();
	}


	// --------------------------------------------------------------------------						
	// Function:	New
	// Description:	create new nonvariant of type
	// Arguments:	none
	// Returns:		new object
	// --------------------------------------------------------------------------
	template<class V> const Variant* NonVariant<V>::New() 
	{ 
		return new NonVariant<V>(); 
	}


	// --------------------------------------------------------------------------						
	// Function:	New
	// Description:	create new nonvariant of type that is a clone of this
	// Arguments:	none
	// Returns:		new object
	// --------------------------------------------------------------------------
	template<class V> Variant* NonVariant<V>::NewClone() const 
	{ 
		return new NonVariant<V>(myValue); 
	}


	// --------------------------------------------------------------------------						
	// Function:	Write
	// Description:	writes variant to file
	// Arguments:	file interface, variant version
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class V> void NonVariant<V>::Write(IOInterface& io, unsigned int version) const 
	{};


	// --------------------------------------------------------------------------						
	// Function:	Read
	// Description:	reads variant from file
	// Arguments:	file interface, variant version
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class V> void NonVariant<V>::Read(IOInterface& io, unsigned int version) 
	{};


	// --------------------------------------------------------------------------						
	// Function:	<
	// Description:	less than
	// Arguments:	other object
	// Returns:		if true
	// --------------------------------------------------------------------------
	template<class V> bool NonVariant<V>::operator<(const Variant& other) const 
	{ 
		return ourTypeId < other.GetTypeId() || (ourTypeId == other.GetTypeId() && myValue < *(V*)other.GetValuePtr()); 
	}




	// --------------------------------------------------------------------------						
	// Function:	==
	// Description:	equals
	// Arguments:	other object
	// Returns:		if true
	// --------------------------------------------------------------------------
	template<class V> bool NonVariant<V>::operator==(const Variant& other) const 
	{ 
		return ourTypeId == other.GetTypeId() && myValue == *(V*)other.GetValuePtr(); 
	}


	// --------------------------------------------------------------------------						
	// Function:	!=
	// Description:	not equals
	// Arguments:	other object
	// Returns:		if true
	// --------------------------------------------------------------------------
	template<class V> bool NonVariant<V>::operator!=(const Variant& other) const 
	{ 
		return ourTypeId != other.GetTypeId() && myValue != *(V*)other.GetValuePtr(); 
	}










} // namespace shh
#endif
