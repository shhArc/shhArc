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

#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "SecureStl.h"
#include "../GCPtr/MemoryLocator.h"





namespace shh {

	class Chunk;
	class Mutex;

	class Allocator
	{
	public:

		Allocator();
		Allocator(MemorySize blockSize, unsigned int maxBlocks, float limitingRatio);
		~Allocator();
		void* Allocate();
		static void Deallocate(void* p);

		void SetDynamicDefrag(float limitingRatio);
		void RecordUsage();
		void Defrag();
		inline unsigned int GetMaxBlocks() const;
		void SetMaxBlocks(unsigned int blocks);


	private:

		typedef std::vector<Chunk*> Chunks;

		
		MemorySize myDataSize;
		unsigned int myMaxBlocks;
		unsigned int myBlocksInUse;
		Chunks myChunks;
		Chunk* myAllocatingChunk;
		float myHardLimitingRatio;
		float myLimitingRatio;
		int myPeak;
		int myTrough;
		Mutex* myMutex;
	};


	// Allocator Inlines //////////////////////////////////////////////////////////



	// --------------------------------------------------------------------------						
	// Function:	GetMaxBlocks
	// Description:	returns maximum number of blocks allowed in allocator
	// Arguments:	none
	// Returns:		max blocks
	// --------------------------------------------------------------------------
	inline unsigned int Allocator::GetMaxBlocks() const 
	{ 
		return myMaxBlocks; 
	}

} 
#endif // ALLOCATOR_H
