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

#ifndef MEMORYFRAME_H
#define MEMORYFRAME_H

#include "../Config/GCPtr.h"
#include "SecureStl.h"
#include "../GCPtr/MemoryLocator.h"
#include "../GCPtr/MemoryPtr.h"
#include "Allocator.h"
#include <map>
#include <string.h>

namespace shh {



	class MemoryFrame : public MemoryLocator
	{

		friend class MemoryStack;

	public:

		inline void* operator new(const size_t bytes);
		inline void* operator new(const size_t bytes, void* place);
		inline void operator delete(void* mem);

	
		virtual ~MemoryFrame();

		MemorySize Destroy(void* value);
		MemorySize Copy(char* value);

		void* Malloc(MemorySize size);
		bool Reserve(MemoryFrame& s, MemorySize size = 0);
		bool Unreserve();
		void AddTrimReserve(unsigned int r);
		bool Trim();
		bool Expand();




		inline bool IsBlanked() const;
		void Blank();
		void Clear();
		void ClearRange(MemorySize low, MemorySize high);
		char* AppendCharArray(const char* value, MemorySize size);
		char* AppendBlank(MemorySize size);
		template <class T> T* Append(const T* value, int extraSpace = 0, char* top = NULL);



		inline char* GetEnd() const;
		inline char* GetTop() const;
		inline MemoryStack* GetOwningStack() const;
	
		void Defrag();

	protected:

		int myTrimReserve;
		bool myBlanked;

		MemoryFrame(MemoryStack* owner);
		bool SetSize(MemorySize size);

		// virtual from MemoryLocator
		virtual void Move(MemorySize baseDelta, Direction baseDir);
		virtual bool Invalidate();

	private:

		static Allocator* ourAllocator;
	};




	template< class T >
	class MemoryCopyer
	{
	public:

		static void Add(void* p);
		static void Copy(void* v, MemoryFrame* b);
	};


	template< class T >
	class MemoryCastable
	{
	public:

		static void Add(void* p);
		static bool Castable(void* v, const std::type_info& t);

		template<class C> static bool IsObject(const C* t, C* dummy = NULL) { return true; }
		template<class C> static bool IsObject(const void* t, C* dummy = NULL) { return false; }
	};




	// MemoryFrame Inlines ////////////////////////////////////////////////



	// --------------------------------------------------------------------------						
	// Function:	new
	// Description:	new data within memory frame
	// Arguments:	size to allocate
	// Returns:		none
	// --------------------------------------------------------------------------
	inline void* MemoryFrame::operator new(const size_t bytes) 
	{ 
		GCPtr<MemoryFrame>::ourMemoryManaged = true; 
		return ourAllocator->Allocate(); 
	}


	// --------------------------------------------------------------------------						
	// Function:	new
	// Description:	placement new
	// Arguments:	size to allocate, address to place
	// Returns:		none
	// --------------------------------------------------------------------------
	inline void* MemoryFrame::operator new(const size_t bytes, void* place) 
	{ 
		return place; 
	}


	// --------------------------------------------------------------------------						
	// Function:	delete
	// Description:	delete object in frame
	// Arguments:	pointer to object
	// Returns:		none
	// --------------------------------------------------------------------------
	inline void  MemoryFrame::operator delete(void* mem)
	{
		MemoryLocator::Header& head = MemoryLocator::GetHeader(mem); \
			head.myLocator->Deallocate(mem);
	}


	// --------------------------------------------------------------------------						
	// Function:	Append
	// Description:	place object at end of frame
	// Arguments:	object, extra space to add with it, last top of frame
	// Returns:		object added
	// --------------------------------------------------------------------------
	template <class T> 	T* MemoryFrame::Append(const T* value, int extraSpace, char* top)
	{
		T* newValue;

		// check if writing over old data
		char* oldTop = NULL;
		if (top && myTop > top)
		{
			oldTop = myTop;
			myTop = top;
		}


		// placement new on frame
		if (value == 0)
			newValue = static_cast<T*>(new (this)T());
		else
			newValue = static_cast<T*>(new (this)T(*value));

		Header& header = GetVariableHeader((char*)newValue);
		header.mySize += extraSpace;
		myTop += extraSpace;
		static MemoryCastable<T> castable;
		header.myCastable = castable.Castable;
		static MemoryDestructor<T> destructor;
		header.myDestructor = destructor.Destroy;
		static MemoryCopyer<T> copyer;
		header.myCopyer = copyer.Copy;

		if (oldTop > myTop)
			myTop = oldTop;

		return newValue;
	}


	// --------------------------------------------------------------------------						
	// Function:	IsBlanked
	// Description:	returns if frame is blank
	// Arguments:	none
	// Returns:		bool
	// --------------------------------------------------------------------------
	inline bool  MemoryFrame::IsBlanked() const
	{ 
		return myBlanked; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetEnd
	// Description:	get end of frame
	// Arguments:	none
	// Returns:		end ptr
	// --------------------------------------------------------------------------
	inline char* MemoryFrame::GetEnd() const
	{ 
		return myEnd; 
	}
	

	// --------------------------------------------------------------------------						
	// Function:	GetTop
	// Description:	get top of frame
	// Arguments:	none
	// Returns:		top ptr
	// --------------------------------------------------------------------------
	inline char* MemoryFrame::GetTop() const
	{
		return myTop; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetOwningStack
	// Description:	get stack that owns frame
	// Arguments:	none
	// Returns:		staxck
	// --------------------------------------------------------------------------
	inline MemoryStack* MemoryFrame::GetOwningStack() const
	{ 
		return reinterpret_cast<MemoryStack*>(myOwner); 
	}



	// MemoryCopier Inlines ///////////////////////////////////////////////////


	// --------------------------------------------------------------------------						
	// Function:	Add
	// Description:	adds copy function to header
	// Arguments:	function pointer
	// Returns:		none
	// --------------------------------------------------------------------------
	template< class T > void MemoryCopyer<T>::Add(void* p)
	{
		MemoryLocator::GetVariableHeader(p).myCopyer = &Copy;
	}


	// --------------------------------------------------------------------------						
	// Function:	Copy
	// Description:	copies object to top of frame
	// Arguments:	object ptr, memory frame
	// Returns:		none
	// --------------------------------------------------------------------------
	template< class T > void MemoryCopyer<T>::Copy(void* v, MemoryFrame* b)
	{
		T* data = reinterpret_cast<T*>(v);
		b->Append(data);
	}



	// MemoryCastable Inlines /////////////////////////////////////////////////


	// --------------------------------------------------------------------------						
	// Function:	Add
	// Description:	adds cast function to header
	// Arguments:	function pointer
	// Returns:		none
	// --------------------------------------------------------------------------
	template< class T > void MemoryCastable<T>::Add(void* p)
	{
		MemoryLocator::GetVariableHeader(p).myCaster = &Castable;
	}


	// --------------------------------------------------------------------------						
	// Function:	Castable
	// Description:	test if castable object to GCObject
	// Arguments:	object ptr, type to cast to
	// Returns:		none
	// --------------------------------------------------------------------------
	template< class T > bool  MemoryCastable<T>::Castable(void* v, const std::type_info& t)
	{
		T* data = reinterpret_cast<T*>(v);
		if (t == typeid(GCObjectBase))
			return IsObject<GCObjectBase>(data);
		else
			return false;
	}
}
#endif //MEMORYFRAME_H
