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
#ifndef IOVARIANT_H
#define IOVARIANT_H

#include "../Common/SecureStl.h"
#include "../Common/Debug.h"
#include "../Common/Variant.h"
#include "../Arc/Type.inl"
#include "IOInterface.h"
#include "Archive.h"
#include "IODictionary.h"
#include <map>
#include <string>





namespace shh {

	class Variant;

	std::string ExtractClassName(const char* typeString);

	
	template<class V> class IONonVariant : public  NonVariant<V>
	{
		DECLARE_ARCHIVE(IONonVariant<V>)

		friend NonVariant <V>;

	public:

		IONonVariant();

		virtual void Write(IOInterface& io, unsigned int version) const;
		virtual void Read(IOInterface& io, unsigned int version);
		virtual const Variant* New();
		virtual std::string GetArchiveAlias() const;

	protected:

		const NonVariant<V>* myPtr;

	};


	// --------------------------------------------------------------------------						
	// Function:	ConvertToIO
	// Description:	converts non variant to io non variant so has virtual io 
	//				functionsbut returns as base class
	// Arguments:	none
	// Returns:		base variant 
	// --------------------------------------------------------------------------	
	template<class V> Variant& NonVariant<V>::ConvertToIO() const
	{
		static IONonVariant<V> temp;
		temp.myPtr = this;
		return temp;
	}



	// IONonVariant ///////////////////////////////////////////////////////


	// --------------------------------------------------------------------------						
	// Function:	IONonVariant
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class V> IONonVariant<V>::IONonVariant() :
		myPtr(NULL)
	{}


	// --------------------------------------------------------------------------						
	// Function:	Write
	// Description:	serializes out object
	// Arguments:	io stream interface, version
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class V> void IONonVariant<V>::Write(IOInterface& io, unsigned int version) const
	{
		INITIALIZE_SERIAL_OUT(io);
		Dictionary<Variant>* v;
		if (this->GetPtr(v))
		{
			IODictionary<Variant> iodict(*v);
			IODictionary<Variant>::WriteArchive(io, &iodict);
		}
		else
		{
			io.Write(myPtr->myValue);
		}
		FINALIZE_SERIAL(io);
	}


	// --------------------------------------------------------------------------						
	// Function:	Read
	// Description:	serializes in object
	// Arguments:	io stream interface, version
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class V> void IONonVariant<V>::Read(IOInterface& io, unsigned int version)
	{
		INITIALIZE_SERIAL_IN(io);
		io.Read((V&)myPtr->myValue);
		FINALIZE_SERIAL(io);
	}


	// --------------------------------------------------------------------------						
	// Function:	New
	// Description:	creates a new non variant
	// Arguments:	none
	// Returns:		returns new object
	// --------------------------------------------------------------------------
	template<class V> const Variant* IONonVariant<V>::New()
	{
		myPtr = new NonVariant<V>();
		return myPtr;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetArchiveAlias
	// Description:	get alias name used in archive for type
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class V> std::string IONonVariant<V>::GetArchiveAlias() const
	{
		return ourArchiveAlias;
	}

	template<class V> std::string IONonVariant<V>::ourArchiveAlias = Archive::RegisterClassWithArchive(std::string("Variant_") + std::string(ExtractClassName(typeid(V).name())), (IONonVariant<V>*)NULL);
	template<class V> unsigned int IONonVariant<V>::ourArchiveVersion = 0;



} // namespace shh
#endif
