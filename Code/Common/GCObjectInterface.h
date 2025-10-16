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
#ifndef GCOBJECTINTERFACE_H
#define GCOBJECTINTERFACE_H


#include "SecureStl.h"


namespace shh {


	template<class BASE> class GCObjectInterface : public virtual BASE
	{
		template<typename T, typename B> friend void SetMemoryStart(T*, GCObjectInterface<B>*);

	public:

		GCObjectInterface() {};
		virtual ~GCObjectInterface() {};
		virtual bool Finalize(GCObjectInterface<BASE>* gc);
		inline bool IsDying() const;
		inline bool IsDeleteable() const;
	
		template< typename T >
		static inline void DestroyObjectVirtuallyIfPossible(BASE* object, T* exactObject);

	protected:

		inline void SetGCMemoryStart(void* mem);

	};



	// --------------------------------------------------------------------------						
	// Function:	Finalize
	// Description:	finalizes object before deletion
	// Arguments:	this object ptr
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class BASE>
	bool GCObjectInterface<BASE>::Finalize(GCObjectInterface<BASE>* gc)
	{
		return BASE::Finalize(gc);
	}


	// --------------------------------------------------------------------------						
	// Function:	IsDying
	// Description:	tests if dying
	// Arguments:	none
	// Returns:		bool
	// --------------------------------------------------------------------------
	template<class BASE>
	inline bool GCObjectInterface<BASE>::IsDying() const
	{
		return BASE::IsDying();
	}


	// --------------------------------------------------------------------------						
	// Function:	IsDeleteable
	// Description:	tests if deletable
	// Arguments:	none
	// Returns:		bool
	// --------------------------------------------------------------------------
	template<class BASE>
	inline bool GCObjectInterface<BASE>::IsDeleteable() const
	{
		return BASE::IsDeleteable();
	}


	// --------------------------------------------------------------------------						
	// Function:	DestroyObjectVirtuallyIfPossible
	// Description:	destroys object
	// Arguments:	this ptr, this ptr
	// Returns:		none
	// --------------------------------------------------------------------------
	template< typename BASE >
	template< typename T >
	inline void GCObjectInterface<BASE>::DestroyObjectVirtuallyIfPossible(BASE* object, T* exactObject)
	{
		object->Finalize(dynamic_cast<GCObjectInterface<BASE>*>(object));
		if (object->IsDeleteable())
		{
			delete object;
		}
		else
		{
			object->BASE::NullInfo();
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	SetGCMemoryStart
	// Description:	sets memory start of object (needed for multiple inheritance
	// Arguments:	mem start of object
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class BASE>
	inline void GCObjectInterface<BASE>::SetGCMemoryStart(void* mem)
	{
		BASE::SetGCMemoryStart(mem);
	}
}

#endif
