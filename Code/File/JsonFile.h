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
#ifndef JSONFILE_H
#define JSONFILE_H


#include "../Common/SecureStl.h"
#include "../Common/Debug.h"
#include "../Common/Dictionary.h"
#include "BinaryFile.h"
#include "IOVariant.h"



namespace shh {

	class JsonFile
	{
	public:
		static bool Read(IBinaryFile& file, StringKeyDictionary& dict);
		static bool Write(OBinaryFile& file, const StringKeyDictionary& dict);
	};

}	// namespace shh

#endif
