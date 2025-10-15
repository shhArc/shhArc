/////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 David K Bhowmik. All rights reserved.
// This file is part of shhArc.
//
// This Software is available under the MIT License with a No Modification clause
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
/////////////////////////////////////////////////////////////////////////////////

#ifndef TYPE_INL
#define TYPE_INL

#include "Type.h"
#include "../Common/Dictionary.h"

namespace shh
{ 

	// --------------------------------------------------------------------------						
	// Function:	SetDictionary
	// Description:	sets a value of this type in a dictionary
	// Arguments:	dictionary key, value, dictionary
	// Returns:		if successfull
	// --------------------------------------------------------------------------	
	template <class T> bool Type<T>::SetDictionary(void* key, void* value, void* dict) const
	{
		VariantKeyDictionary* vkDict = (VariantKeyDictionary*)dict;
		T* exactValue = (T*)value;
		Variant* exactKey = (Variant*)key;
		vkDict->Set(*exactKey, *exactValue);
		return true;
	}

}

#endif 
