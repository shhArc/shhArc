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

#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H


#include "../Common/SecureStl.h"
#include "../Common/Debug.h"
#include <map>
#include <string>
#include <vector>

namespace shh
{

	class FileSystem
	{
	public:
		
		typedef enum Filter { GetFiles = 1, GetDirectories, GetEverything } Filter;


		static bool GetHomeDirectory(std::string& directory);
		static bool GetDesktopDirectory(std::string& directory);
		static bool GetMyDocumentsDirectory(std::string& directory);
		static void GetWorkingDirectory(std::string& directory);
		static bool SetWorkingDirectory(const std::string& directory);

		static bool IsValidPath(const std::string& path);
		static bool IsValidDirectory(const std::string& path);
		static bool IsValidFile(const std::string& file);

		static bool ExtractDrive(const std::string& path, std::string& drive);
		static bool ExtractDirectory(const std::string& path, std::string& directory);
		static bool ExtractExtension(const std::string& path, std::string& extension);
		static bool ExtractTitle(const std::string& path, std::string& filename);
		static bool ExtractFilename(const std::string& path, std::string& filename);

		static bool Copy(const std::string& source, const std::string& destination, const bool overwrite);
		static bool Move(const std::string& source, const std::string& destination);
		static bool Delete(const std::string& path);
		static bool MakeDirectory(const std::string& name);

		static bool GetDirectoryContents(const std::string& directory, const std::string& wildcard, const FileSystem::Filter flags, std::vector< std::string>& contents);

		static int Oldest(const std::string& f1, const std::string& f2);
		static bool GetFileSize(const std::string& file, int& size);

		static bool SetVariable(std::string name, const std::string& value);
		static bool GetVariable(std::string name, std::string& value);
		static bool Resolve(const std::string& path, std::string& newPath);
		
		static void AddReadPath(const std::string& s);
		static void AddWritePath(const std::string& s);
		static void RemoveReadPath(const std::string& s);
		static void RemoveWritePath(const std::string& s);
		static bool IsValidReadPath(const std::string& s, std::string& error);
		static bool IsValidWritePath(const std::string& s, std::string& error);

	private:
		FileSystem();
		~FileSystem();

		static bool ExpandValue(const std::string& value, std::string& expanded);

		typedef std::pair< std::string, std::string> Variable;
		typedef std::map< std::string, std::string> VariableMap;
		typedef std::vector< std::string > Paths;
		
		static VariableMap ourVariableMap;
		static Paths ourReadPaths;
		static Paths ourWritePaths;

		static const std::string ourExtSeperator;
		static const std::string ourDirSeperator;
		static const std::string ourDriveSeperator;
		static const std::string ourVariableDelimiter;

		static bool IsValidPath(const std::string& s, const std::vector< std::string >& paths, const std::string& type, std::string& error);

	};

} // namespace shh
#endif // FILE_SYSTEM_H
