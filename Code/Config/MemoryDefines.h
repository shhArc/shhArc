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

#ifndef MEMORYDEFINES_H
#define MEMORYDEFINES_H

#include "SecureStl.h"
#include "Config.h"
#include <MemoryManagement.h>
#include "../Common/Dictionary.h"


#define DECLARE_NOT_MEMORY_MANAGED(CLASS) 
#define IMPLEMENT_NOT_MEMORY_MANAGED(CLASS)
#define MOT_CONFIGURE_MEMORYMANAGEMENT(DICT)
#define MOT_UPDATE_MEMORYMANAGER(ARG)

#if USE_SHHARC_MEMORY_MANAGEMENT
#define CONFIGURE_MEMORYMANAGEMENT(DICT) CONFIGURE_SHHARC_MEMORYMANAGEMENT(DICT)
#define UPDATE_MEMORYMANAGER(ARG) UPDATE_SHHARC_MEMORYMANAGER(ARG)
#define DECLARE_MEMORY_MANAGED(CLASS) DECLARE_SHHARC_MEMORY_MANAGED(CLASS)
#define IMPLEMENT_MEMORY_MANAGED(CLASS) IMPLEMENT_SHHARC_MEMORY_MANAGED(CLASS)
#else
#define CONFIGURE_MEMORYMANAGEMENT(DICT) MOT_CONFIGURE_MEMORYMANAGEMENT(DICT)
#define UPDATE_MEMORYMANAGER(ARG) MOT_UPDATE_MEMORYMANAGER(ARG)
#define DECLARE_MEMORY_MANAGED(CLASS) DECLARE_NOT_MANAGED(CLASS)
#define IMPLEMENT_MEMORY_MANAGED(CLASS) IMPLEMENT_NOT_MEMORY_MANAGED(CLASS)
#endif


#endif
