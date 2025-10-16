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

#ifndef MEMORYMANAGEMENT_H
#define MEMORYMANAGEMENT_H

#include "SecureStl.h"
#include "MemoryManager.h"
#include "MemoryStack.h"
#include "MemoryFrame.h"


namespace shh {


#define CONFIGURE_SHHARC_MEMORYMANAGEMENT(DICT) ConfigureMemoryManagementSHHARC(DICT)
#define UPDATE_SHHARC_MEMORYMANAGER(ARG) 


#define DECLARE_SHHARC_MEMORY_MANAGED(CLASS) \
	public: \
	inline void* operator new(const size_t bytes){ GCPtr<CLASS>::ourMemoryManaged = true; void *p = ourAllocator->Allocate(); MemoryLocator::MemoryDestructor<CLASS>::Add(p); return p; } \
	inline void* operator new(const size_t bytes, shh::MemoryStack* m) { GCPtr<CLASS>::ourMemoryManaged = true; void* p = m->Malloc((unsigned int)bytes); MemoryLocator::MemoryDestructor<CLASS>::Add(p); return p; } \
	inline void* operator new(const size_t bytes, shh::MemoryFrame* m) { GCPtr<CLASS>::ourMemoryManaged = true; void* p = m->Malloc((unsigned int)bytes); MemoryLocator::MemoryDestructor<CLASS>::Add(p); return p; } \
	inline void* operator new(const size_t bytes, void *place ){ return place; } \
	inline void operator delete(void* mem, shh::MemoryFrame *m){ } \
	inline void operator delete(void* mem) { \
	MemoryLocator::Header& head = MemoryLocator::GetHeader(mem); \
	head.myLocator->Deallocate(mem); } \
	private: \
	static shh::Allocator* ourAllocator;

#define IMPLEMENT_SHHARC_MEMORY_MANAGED(CLASS) \
	shh::Allocator *CLASS::ourAllocator = shh::MemoryManager::GetManager().GetAllocator(sizeof(CLASS)); 

	template<class T> class Dictionary;
	void ConfigureMemoryManagementSHHARC(const  Dictionary<std::string>& memory);

}
#endif //MEMORYDEFINES_H
