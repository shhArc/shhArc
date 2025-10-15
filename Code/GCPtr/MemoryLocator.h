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

#ifndef MEMORYLOCATOR_H
#define MEMORYLOCATOR_H


#include "SecureStl.h"
#include <memory> 
#include <typeinfo>
#include <map>
#include <vector>


// MemoryLocator //////////////////////////////////////////////////////////////////////////////////////////////////////

namespace shh {

	class MemoryInfo;
	class MemoryFrame;
	class MemoryStack;


	union MemoryOffset
	{
		MemoryOffset() : ptr(NULL) {}
		char* ptr;
		unsigned int value;
	};

	typedef std::size_t MemorySize;

	class MemoryLocator
	{

		friend class MemoryInfo;
		friend class MemoryFrame;
		friend class MemoryStack;

	public:

		class NonDestructable {};

		template< class T >
		class MemoryDestructor
		{
		public:

			static bool IsPtr(void*);
			static bool IsPtr(void**);
			static bool IsNonDestructable(MemoryLocator::NonDestructable*);
			static bool IsNonDestructable(void*);

			static void Add(void* p);
			static void Destroy(void* v);
		};


		class Sorter
		{
		public:
			MemoryLocator* myLocator;

			Sorter(); 
			Sorter(MemoryLocator * l);
			bool operator==(const Sorter& o) const;
			bool operator<(const Sorter& o) const;
			bool operator>(const Sorter& o) const;
		};

		typedef std::pair< const MemoryLocator::Sorter, MemoryLocator* > LocatorNode;

		class Header
		{
		public:
			Header(MemoryLocator*l) : myInUse(false), myLocator(l), myDestructor(NULL), myMemoryInfo(NULL) {}	// dont initialize myMemoryInfo cos may have a pointers before construction (ie staticmemory)
			bool myInUse;
			MemoryLocator* myLocator;
			void (*myDestructor)(void*);
			MemoryInfo* myMemoryInfo;
		};

		class FixedHeader : public Header
		{
		public:
			FixedHeader(MemoryLocator* l) : Header(l), myPreviousHeader(NULL), myNextHeader(NULL), myVariable(false) {}
			FixedHeader* myPreviousHeader;
			FixedHeader* myNextHeader;
			bool myVariable;
		};

		class VariableHeader : public Header
		{
		public:
			VariableHeader(MemoryLocator* l) : Header(l), mySize(0), myCastable(0), myCopyer(0), myVariable(true) {}

			MemorySize mySize;
			bool (*myCastable)(void*, const std::type_info& t);
			void (*myCopyer)(void*, MemoryFrame*);
			bool myVariable;
		};



		enum { fixedHeaderSize = sizeof(MemoryLocator::FixedHeader) };
		enum { variableHeaderSize = sizeof(MemoryLocator::VariableHeader) };
		typedef enum Direction { None = 0, Up, Down } Direction;


		MemoryLocator(bool variableSize, char* base = NULL);
		virtual ~MemoryLocator();

		virtual Header* Deallocate(void* p, bool invalidateInfo = true);

		virtual void AddMemoryLocator(MemoryLocator* l) {}
		virtual void RemoveMemoryLocator(MemoryLocator* l) {}

		inline MemoryOffset AddressOffset(void* absolute) const;
		inline char* AbsoluteAddress(MemorySize offset) const;
		inline char* GetBase() const;
		inline MemoryLocator* GetOwner() const;
		inline bool IsValid() const;

		static inline bool IsVariableSizeData(void* address);
		static inline FixedHeader& GetFixedHeader(void* address);
		static inline VariableHeader& GetVariableHeader(void* address);
		static inline Header& GetHeader(void* address);

	protected:

		bool myVariableSize;
		char* myBase;
		char* myEnd;
		char* myTop;
		Header* myInUseHead;
		MemoryLocator* myOwner;


		virtual void RegisterMoveAll(MemorySize baseDelta, Direction baseDir, MemorySize offsetDelta = 0, Direction offsetDir = None);
		virtual bool Invalidate();
		inline char*& GetBase(MemoryLocator& l);

	};



	// MemoryLocator::MemoryDestructor Inlines ///////////////////////////////////////////


	// --------------------------------------------------------------------------						
	// Function:	IsPtr
	// Description:	returns if arg passed in is a C pointer
	// Arguments:	pointer to test
	// Returns:		false
	// --------------------------------------------------------------------------
	template< class T > bool MemoryLocator::MemoryDestructor<T>::IsPtr(void*)
	{ 
		return false; 
	}
	

	// --------------------------------------------------------------------------						
	// Function:	IsPtr
	// Description:	returns if arg passed in is a C pointer
	// Arguments:	pointer to test
	// Returns:		tue
	// --------------------------------------------------------------------------
	template< class T > bool IsPtr(void**)
	{ 
		return true; 
	}


	// --------------------------------------------------------------------------						
	// Function:	IsNonDestructable
	// Description:	returns if object pointer passed in is of base type NonDestructable
	// Arguments:	object pointer to test
	// Returns:		true
	// --------------------------------------------------------------------------
	template< class T > bool  MemoryLocator::MemoryDestructor<T>::IsNonDestructable(MemoryLocator::NonDestructable*)
	{ 
		return true; 
	}


	// --------------------------------------------------------------------------						
	// Function:	IsNonDestructable
	// Description:	returns if object pointer passed in is of base type NonDestructable
	// Arguments:	object pointer to test
	// Returns:		false
	// --------------------------------------------------------------------------
	template< class T > bool  MemoryLocator::MemoryDestructor<T>::IsNonDestructable(void*)
	{ 
		return false; 
	}


	// --------------------------------------------------------------------------						
	// Function:	Add
	// Description:	sets functions in pointer header
	// Arguments:	pointer to memory address of object/type
	// Returns:		none
	// --------------------------------------------------------------------------
	template< class T > void  MemoryLocator::MemoryDestructor<T>::Add(void* p)
	{
		MemoryLocator::GetHeader(p).myDestructor = &Destroy;
	}


	// --------------------------------------------------------------------------						
	// Function:	Destroy
	// Description:	calls destructor
	// Arguments:	pointer to memory address of object/type
	// Returns:		none
	// --------------------------------------------------------------------------
	template< class T > void  MemoryLocator::MemoryDestructor<T>::Destroy(void* v)
	{
		T* data = reinterpret_cast<T*>(v);
		if (IsNonDestructable(data))
			return;
		if (!IsPtr(data))
			data->~T();
	}


	
	// MemooryLocator Inlines ///////////////////////////////////////////////////////////////////

	// --------------------------------------------------------------------------						
	// Function:	AddressOffset
	// Description:	calculated memory offset from base
	// Arguments:	address
	// Returns:		offset
	// --------------------------------------------------------------------------
	inline MemoryOffset MemoryLocator::AddressOffset(void* absolute) const
	{
		MemoryOffset offset;
		offset.ptr = static_cast<char*>(absolute) - reinterpret_cast<MemorySize>(myBase);
		return offset;
	}


	// --------------------------------------------------------------------------						
	// Function:	AbsoluteAddress
	// Description:	calculated the absolute address from offset
	// Arguments:	ofset
	// Returns:		pointer to absolute address
	// --------------------------------------------------------------------------
	inline char* MemoryLocator::AbsoluteAddress(MemorySize offset) const 
	{ 
		return myBase + offset; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetBase
	// Description:	returns memory base
	// Arguments:	none
	// Returns:		address of base
	// --------------------------------------------------------------------------
	inline char* MemoryLocator::GetBase() const 
	{ 
		return myBase; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetOwner
	// Description:	returns owning MemoryLocator
	// Arguments:	none
	// Returns:		MemoryLocator
	// --------------------------------------------------------------------------
	inline MemoryLocator* MemoryLocator::GetOwner() const 
	{ 
		return myOwner; 
	}


	// --------------------------------------------------------------------------						
	// Function:	IsValid
	// Description:	returns if valid memory locator (must have owner)
	// Arguments:	none
	// Returns:		bool
	// --------------------------------------------------------------------------
	inline bool MemoryLocator::IsValid() const 
	{ 
		return myOwner != NULL; 
	}


	// --------------------------------------------------------------------------						
	// Function:	IsVariableSizeData
	// Description:	return if object has a MemorySize paramater
	// Arguments:	address of object type
	// Returns:		bool
	// --------------------------------------------------------------------------
	inline bool MemoryLocator::IsVariableSizeData(void* address) 
	{ 
		return *reinterpret_cast<bool*>(((char*)address) - sizeof(MemorySize)); 
	}
	

	// --------------------------------------------------------------------------						
	// Function:	GetFixedHeader
	// Description:	returns header for fixed size object
	// Arguments:	address of object/type
	// Returns:		fixed header
	// --------------------------------------------------------------------------
	inline MemoryLocator::FixedHeader& MemoryLocator::GetFixedHeader(void* address)
	{ 
		return *reinterpret_cast<MemoryLocator::FixedHeader*>(((char*)address) - fixedHeaderSize); 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetVariableHeader
	// Description:	returns header for variable size object
	// Arguments:	address of object/type
	// Returns:		variable header
	// --------------------------------------------------------------------------
	inline MemoryLocator::VariableHeader& MemoryLocator::GetVariableHeader(void* address)
	{ 
		return *reinterpret_cast<VariableHeader*>(((char*)address) - variableHeaderSize); 
	}
	

	// --------------------------------------------------------------------------						
	// Function:	GetHeader
	// Description:	returns base header for object
	// Arguments:	address of object/type
	// Returns:		header
	// --------------------------------------------------------------------------
	inline MemoryLocator::Header& MemoryLocator::GetHeader(void* address)
	{ 
		return IsVariableSizeData(address) ? (Header&)GetVariableHeader(address) : (Header&)GetFixedHeader(address); 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetBase
	// Description:	returns memory base of the locator
	// Arguments:	memory locator
	// Returns:		address
	// --------------------------------------------------------------------------
	inline char*& MemoryLocator::GetBase(MemoryLocator& l) 
	{ 
		return l.myBase; 
	}



}

#endif //MEMORYLOCATOR_H
