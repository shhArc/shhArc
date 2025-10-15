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
#ifndef CONFIGURATIONFILE_H
#define CONFIGURATIONFILE_H


#include "../Common/SecureStl.h"
#include "../Common/Debug.h"
#include "../Common/Dictionary.h"
#include "TextFile.h"


namespace shh {

	class ConfigurationFile
	{
	public:
		static bool Read(ITextFile& file, StringKeyDictionary& map);
		static std::string ReadToken(ITextFile& file);
		static std::string ReadString(ITextFile& file);
		static double ReadNumber(ITextFile& file);
		static bool ReadBool(ITextFile& file, bool& value, std::string& token = ConfigurationFile::ourDummy);
		static std::string ReadBracketed(ITextFile &file, char beginchr, char endchr);
		static void EatWhite(ITextFile& file);

		static bool Write(OTextFile& file, const StringKeyDictionary& map, const std::string& key = "", const std::string& indent ="");


	private:

		static std::string ourDummy;
	};

}	// namespace shh

#endif
