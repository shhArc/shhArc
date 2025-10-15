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

#include "PreciseTime.h"

#ifdef _WIN64
#include <windows.h>
#pragma optimize("g",off)
#else
#include <time.h>
#endif

namespace shh
{
#ifdef _WIN64
	static double GetMultiplier()
	{
		LARGE_INTEGER i;
		::QueryPerformanceFrequency(&i);
		return 1.0/double(i.QuadPart);
	}

	static LARGE_INTEGER GetTimeOrigin()
	{
		LARGE_INTEGER i;
		::QueryPerformanceCounter(&i);
		return i;
	}

	double GetPreciseTime()
	{
		static double multiplier = GetMultiplier();
		static LARGE_INTEGER timeOrigin = GetTimeOrigin();
		LARGE_INTEGER i;
		::QueryPerformanceCounter(&i);


		i.QuadPart = i.QuadPart - timeOrigin.QuadPart;
		return double(i.QuadPart)*multiplier;
	}
#else

	double GetPreciseTime()
	{
		return (double)clock();
	}
#endif

}