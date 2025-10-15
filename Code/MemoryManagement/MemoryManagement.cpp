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

#include "../Common/Dictionary.h"
#include "SecureStl.h"
#include "MemoryManager.h"
#include "MemoryStack.h"


namespace shh {

	void ConfigureMemoryManagementSHHARC(const Dictionary<std::string>& memory)
	{
		MemoryManager::GetManager().SetDynamicDefrag((float)memory.Get("limiting_ratio", (double)0.75));
		MemoryManager::GetManager().SetMaxBlocks((unsigned int)memory.Get("blocks", (long)256));
		MemoryManager::StdAllocatorSize::ourSingle = (unsigned int)memory.Get("stl_blocks", (long)MemoryManager::StdAllocatorSize::ourSingle);
		MemoryManager::StdAllocatorSize::ourMultiple = (unsigned int)memory.Get("multiple_stl_blocks", (long)MemoryManager::StdAllocatorSize::ourMultiple);
		MemoryStack::ourDefaultAllocatorBlocks = (unsigned int)memory.Get("stack_blocks", (long)MemoryStack::ourDefaultAllocatorBlocks);
		MemoryStack::ourGrowRate = (unsigned int)memory.Get("stack_grow_rate", (long)MemoryStack::ourGrowRate);
	}
}