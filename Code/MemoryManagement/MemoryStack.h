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

#ifndef MEMORYSTACK_H
#define MEMORYSTACK_H

#include "../Config/GCPtr.h"
#include "SecureStl.h"
#include <map>


namespace shh {

	class MemoryFrame;



	// MemoryStack /////////////////////////////////////////////////////////////////////////////////////////////////////////


	class MemoryStack : public MemoryLocator
	{

		friend class MemoryFrame;

	public:

		typedef std::vector<MemoryFrame*> Stack;

		class MoveCallbackInterface : public GCObject
		{
		public:
			virtual ~MoveCallbackInterface() {};
			virtual void MemoryMoved(MemoryStack* m, MemorySize delta, Direction dir) = 0;
		};

		static unsigned int ourGrowRate;
		int myShrinkLimit;
		MemorySize myMinSize;
		unsigned int myAllocatorBlocks;
		static unsigned int ourDefaultAllocatorBlocks;



		MemoryStack();
		MemoryStack(unsigned int size, unsigned int allocatorBlocks = 0);
		virtual ~MemoryStack();
		void SetMemory(unsigned int size, unsigned int allocatorBlocks = 0);
		void SetMemory(char* mem, unsigned int size, unsigned int allocatorBlocks);
		void Clean();
		

		virtual void Move(MemorySize baseDelta, MemoryLocator::Direction baseDir);	
		MemorySize Grow(MemorySize min);
		bool Shrink(bool trimAll = false);
		void ShiftAbove(MemoryFrame* f);
		void Defrag();
		void Pack();


		MemoryFrame* AddFrame();
		MemoryFrame* GetTopFrame() const;
		void AddMemoryMoveCallback(const GCPtr<MoveCallbackInterface>& c);

		void* Malloc(MemorySize size);

	protected:


		bool myIsDeleteableMemory;
		char* myMemory;

		bool TrimFrame(MemoryFrame* t);
		bool ExpandFrame(MemoryFrame* e);

	private:

		Stack myStack;
		std::vector< GCPtr<MoveCallbackInterface> > myMemoryMovedCallbacks;


		char* New(MemorySize size);
		void Delete(char* oldBase);

		MemorySize Resize(MemorySize size);
		virtual void Register(MemoryFrame& f);
		virtual void Unregister(MemoryFrame& f);

	};

}

#endif // MEMORYSTACK_H
