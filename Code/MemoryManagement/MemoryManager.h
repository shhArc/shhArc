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

#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "SecureStl.h"
#include "../GCPtr/MemoryLocator.h"
#include "Allocator.h"
#include "Chunk.h"
#include <vector>
#include <map>

namespace shh {

	class MemoryFrame;
	class Mutex;

	class MemoryManager
	{
	public:


		class StdAllocatorSize
		{
		public:
			static unsigned int GetSingle() { return ourSingle; }
			static unsigned int GetMultiple() { return ourMultiple; }
			static unsigned int ourSingle;
			static unsigned int ourMultiple;
		};

		template<class T, class S = StdAllocatorSize >
		class StdAllocator : public std::allocator<T>
		{
		public:
			typedef typename std::allocator<T>::pointer pointer;
			typedef typename std::allocator<T>::size_type size_type;

			StdAllocator();
			template<class C > StdAllocator(const StdAllocator<C, S>& a);
			pointer allocate(size_type _Count);
			pointer allocate(size_type _Count, const void* _Hint);
			void deallocate(pointer _Ptr, size_type _Count);
			template<class _Other>
			struct rebind
			{
				typedef StdAllocator<_Other, S> other;
			};


		private:

			static Allocator* ourAllocator;

		};
		
		typedef std::map<MemorySize, Allocator*> Allocators;


		static MemoryManager& GetManager();
		static void CloseManager();

		Allocator* GetAllocator(MemorySize dataSize, unsigned int maxBlocks = 0);
		void SetMaxBlocks(unsigned int maxBlocks);
		void SetDynamicDefrag(float limitingRatio);
		void RecordUsage();
		void Defrag();


	private:

		static MemoryManager* ourManager;
		static int ourGranularity;
		unsigned int myMaxBlocks;
		float myLimitingRatio;
		Allocators myAllocators;

		Mutex* myMutex;

		MemoryManager();
		~MemoryManager();
	};

	template<class T, class S > Allocator* MemoryManager::StdAllocator<T, S>::ourAllocator = NULL;




	// MemoryManager::StdAllocator Inlines //////////////////////////////////////////////////////////////////


	// --------------------------------------------------------------------------						
	// Function:	StdAllocator
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class T, class S > MemoryManager::StdAllocator<T,S>::StdAllocator()
	{ 
		if (ourAllocator == NULL) ourAllocator = MemoryManager::GetManager().GetAllocator(sizeof(T), S::GetSingle()); 
	}


	// --------------------------------------------------------------------------						
	// Function:	StdAllocator
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class T, class S >
	template<class C > MemoryManager::StdAllocator<T, S>::StdAllocator(const StdAllocator<C, S>& a)
	{ 
		if (ourAllocator == NULL) ourAllocator = MemoryManager::GetManager().GetAllocator(sizeof(T), S::GetSingle()); 
	}


	// --------------------------------------------------------------------------						
	// Function:	allocate
	// Description:	allocator for stl container objects (vecotrs etc)
	// Arguments:	number of elements in container
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class T, class S > typename MemoryManager::StdAllocator<T, S>::pointer MemoryManager::StdAllocator<T, S>::allocate(size_type _Count)
	{
		if (_Count == 1)
			return (pointer)ourAllocator->Allocate();
		Allocator* allocator = MemoryManager::GetManager().GetAllocator(sizeof(T) * _Count, S::GetMultiple());
		return (pointer)allocator->Allocate();
	}


	// --------------------------------------------------------------------------						
	// Function:	allocate
	// Description:	allocator for stl container objects (vecotrs etc)
	// Arguments:	number of elements in container
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class T, class S >	typename MemoryManager::StdAllocator<T,S>::pointer MemoryManager::StdAllocator<T, S>::allocate(size_type _Count, const void* _Hint)
	{ 
		return allocate(_Count); 
	}


	// --------------------------------------------------------------------------						
	// Function:	deallocate
	// Description:	deallocator for stl container objects (vecotrs etc)
	// Arguments:	pointer to container, size of container
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class T, class S = StdAllocatorSize > 	void MemoryManager::StdAllocator<T, S>::deallocate(pointer _Ptr, size_type _Count)
	{ 
		Allocator::Deallocate(_Ptr); 
	}

}
#endif // MEMORY_MANAGER_H
