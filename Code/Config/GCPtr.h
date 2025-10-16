///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 David K Bhowmik. All rights reserved.
//
/// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
///////////////////////////////////////////////////////////////////////////////
#ifndef GCPTR_H
#define GCPTR_H


#include "../Common/GCPtrInterface.h"
#include "../Common/GCObjectInterface.h"
#include "../GCPtr/GCPtrBase.h"

#define GCOBJECTBASE GCObjectBase
#define GCPTRBASE GCPtrBase

namespace shh {

	typedef GCObjectInterface<GCOBJECTBASE> GCObject;
	template <typename T> using GCPtr = GCPtrInterface<T, GCPTRBASE>;
	
	// --------------------------------------------------------------------------						
	// Function:	DestroyObjectVirtuallyIfPossible
	// Description:	delete virtually
	// Arguments:	object ptr, object ptr
	// Returns:		id
	// --------------------------------------------------------------------------
	template< typename T >
	inline void DestroyObjectVirtuallyIfPossible(GCObject* object, T* exactObject)
	{
		GCObject::DestroyObjectVirtuallyIfPossible(object, exactObject);
	}

	// --------------------------------------------------------------------------						
	// Function:	DestroyObjectVirtuallyIfPossible
	// Description:	delete virtually
	// Arguments:	object ptr, object ptr
	// Returns:		id
	// --------------------------------------------------------------------------
	template< typename T >
	inline void DestroyObjectVirtuallyIfPossible(void* object, T* exactObject)
	{
		delete exactObject;
	}
}

#endif // GCPTR_H
