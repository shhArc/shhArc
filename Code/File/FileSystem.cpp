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


#include "string.h"
#ifdef _WIN64
#include <windows.h>
#include <shlwapi.h>
#include <shlobj.h>
#else
#define MAX_PATH 256
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#define _snprintf snprintf
#endif

#include "FileSystem.h"
#include "BinaryFile.h"
#include <stdio.h>
#include <ctype.h>
#include <algorithm>
#include <fstream>
#include <string>

namespace shh {

	const std::string FileSystem::ourExtSeperator = ".";
	const std::string FileSystem::ourDirSeperator = "/";
	const std::string FileSystem::ourDriveSeperator = ":";
	const std::string FileSystem::ourVariableDelimiter = "%";


	FileSystem::VariableMap FileSystem::ourVariableMap;
	FileSystem::Paths FileSystem::ourReadPaths;
	FileSystem::Paths FileSystem::ourWritePaths;


#ifdef _WIN64
	std::string WideToCStr(const wchar_t* wideStr)
	{
		char buffer[MAX_PATH + 1];
		wcstombs(buffer, wideStr, MAX_PATH + 1);
		return std::string(buffer);
	}

	std::vector<wchar_t> CStrToWide(const std::string &cStr)
	{
		const int len = (int)cStr.size();
		std::vector<wchar_t> wStr(len+1);
		std::copy(cStr.c_str(), cStr.c_str() + len, wStr.begin());
		wStr.push_back(0);
		return wStr;
	}

#endif

#define WC_QUES '?'
#define WC_STAR '*'

	/* string and character match/no-match return codes */

	enum
	{
		WC_MATCH = 0,     /* char/string-match succeed */
		WC_MISMATCH,      /* general char/string-match fail */
		WC_PAT_NULL_PTR,  /* (char *) pattern == NULL */
		WC_CAN_NULL_PTR,  /* (char *) candidate == NULL */
		WC_PAT_TOO_SHORT, /* too few pattern chars to satisfy count */
		WC_CAN_TOO_SHORT  /* too few candidate chars to satisy '?' in pattern */
	};


	// --------------------------------------------------------------------------						
	// Function:	ch_eq
	// Description:	checks if two chars are equal
	// Arguments:	char 1, char 2, case sensitive
	// Returns:		none
	// --------------------------------------------------------------------------
	static int ch_eq(int c1, int c2, int do_case)
	{
		if (do_case == 0)      /* do_case == 0, ignore  case */
		{
			c1 = tolower(c1);
			c2 = tolower(c2);
		}
		return (c1 == c2) ? WC_MATCH : WC_MISMATCH;
	}

	// --------------------------------------------------------------------------						
	// Function:	wc_strncmp
	// Description:	compare wide string to match pattern
	// Arguments:	pattern, string to compare, case sensitive, do wild cards
	// Returns:		success flags
	// --------------------------------------------------------------------------
	int wc_strncmp(const char* pattern, const char* candidate, int count, int do_case, int do_wildcards)
	{
		const char* can_start;
		unsigned int star_was_prev_char;
		unsigned int ques_was_prev_char;
		unsigned int retval;

		if (pattern == NULL)           /* avoid any pesky coredump, please */
		{
			return WC_PAT_NULL_PTR;
		}

		if (candidate == NULL)
		{
			return WC_CAN_NULL_PTR;
		}

		/* match loop runs, comparing pattern and candidate strings char by char,
		until (1) the end of the pattern string is found or (2) somebody found
		some kind of mismatch condition. we deal with four cases in this loop:
		-- pattern char is '?'; or
		-- pattern char is '*'; or
		-- candidate string is exhausted; or
		-- none of the above, pattern char is just a normal char.  */

		can_start = candidate;         /* to calc n chars compared at exit time */
		star_was_prev_char = 0;        /* show previous character was not '*' */
		ques_was_prev_char = 0;        /* and it wasn't '?', either */
		retval = WC_MATCH;             /* assume success */

		while (retval == WC_MATCH && *pattern != '\0')
		{

			if (do_wildcards != 0)       /* honoring wildcards? */
			{

				/* first: pattern-string character == '?' */

				if (*pattern == WC_QUES)   /* better be another char in candidate */
				{
					ques_was_prev_char = 1;  /* but we don't care what it is */
				}

				/* second: pattern-string character == '*' */

				else if (*pattern == WC_STAR)
				{
					star_was_prev_char = 1;  /* we'll need to resync later */
					++pattern;
					continue;                /* rid adjacent stars */
				}
			}

			/* third: any more characters in candidate string? */

			if (*candidate == '\0')
			{
				retval = WC_CAN_TOO_SHORT;
			}

			else if (ques_was_prev_char > 0) /* all set if we just saw ques */
			{
				ques_was_prev_char = 0;        /* reset and go check next pair */
			}

			/* fourth: plain old char compare; but resync first if necessary */

			else
			{
				if (star_was_prev_char > 0)
				{
					star_was_prev_char = 0;

					/* resynchronize (see note in header) candidate with pattern */

					while (WC_MISMATCH == ch_eq(*pattern, *candidate, do_case))
					{
						if (*candidate++ == '\0')
						{
							retval = WC_CAN_TOO_SHORT;
							break;
						}
					}
				}           /* end of re-sync, resume normal-type scan */

				/* finally, after all that rigamarole upstairs: the main char compare */

				else  /* if star or ques was not last previous character */
				{
					retval = ch_eq(*pattern, *candidate, do_case);
				}
			}

			++candidate;  /* point next chars in candidate and pattern strings */
			++pattern;    /*   and go for next compare */

		}               /* while (retval == WC_MATCH && *pattern != '\0') */


		if (retval == WC_MATCH)
		{

			/* we found end of pattern string, so we've a match to this point; now make
			sure "count" arg is satisfied. we'll deem it so and succeed the call if:
			- we matched at least "count" chars; or
			- we matched fewer than "count" chars and:
			- pattern is same length as candidate; or
			- we're honoring wildcards and the final pattern char was star.
			spell these tests right out /completely/ in the code */

			int min;
			int nmatch;

			min = (count < 0) ?                  /* if count negative, no abbrev: */
				(int)strlen(can_start) :           /*   must match two entire strings */
				count;                        /* but count >= 0, can abbreviate */

			nmatch = (int)(candidate - can_start);      /* n chars we did in fact match */

			if (nmatch >= min)                   /* matched enough? */
			{                                    /* yes; retval == WC_MATCH here */
				;
			}
			else if (*candidate == '\0')         /* or cand same length as pattern? */
			{                                    /* yes, all set */
				;
			}
			else if (star_was_prev_char != 0)    /* or final char was star? */
			{                                    /* yes, succeed that case, too */
				;
			}

			else                                 /* otherwise, fail */
			{
				retval = WC_PAT_TOO_SHORT;
			}
		}

		return retval;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetFullPath
	// Description:	get the full path of file name
	// Arguments:	file name, buffer size, destination buffer,
	//				pointer to the file name at the end
	// Returns:		length of string in buffer
	// --------------------------------------------------------------------------
	int GetFullPath(const std::string& fileName, int bufferLength, std::string& buffer, char* filePart)
	{
		const std::string prepend = "";

		const int srcLen = (int)fileName.length();
		const int destLen = srcLen + (int)prepend.length();

		buffer = prepend + fileName;

		if (filePart != 0)
		{
			filePart = (char*)buffer.c_str();
			for (int i = destLen; i > 0; --i)
			{
				if ((buffer[i] == '\\') || (buffer[i] == '/'))
				{
					filePart = (char*)buffer.c_str() + i + 1;
					break;
				}
			}
		}

		return destLen;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetHomeDirectory
	// Description:	returns home dir
	// Arguments:	var to store in
	// Returns:		if successfull
	// --------------------------------------------------------------------------
	bool FileSystem::GetHomeDirectory(std::string& directory)
	{
#ifdef _WIN64
		wchar_t szAppData[MAX_PATH+1];
		if(::SHGetSpecialFolderPath(NULL, szAppData, CSIDL_APPDATA, TRUE))
		{
			directory = WideToCStr(szAppData);
			return true;
		}

		return false;
#else
		return false;
#endif
	}


	// --------------------------------------------------------------------------						
	// Function:	GetDesktopDirectory
	// Description:	returns desktop dir
	// Arguments:	var to store in
	// Returns:		if successfull
	// --------------------------------------------------------------------------
	bool FileSystem::GetDesktopDirectory(std::string& directory)
	{
#ifdef _WIN64
		wchar_t szData[MAX_PATH];
		if(::SHGetSpecialFolderPath(NULL, szData, CSIDL_DESKTOP, TRUE))
		{
			directory = WideToCStr(szData);
			return true;
		}

		return false;
#else
		return false;
#endif
	}


	// --------------------------------------------------------------------------						
	// Function:	GetMyDocumentsDirectory
	// Description:	returns documents dir
	// Arguments:	var to store in
	// Returns:		if successfull
	// --------------------------------------------------------------------------	
	bool FileSystem::GetMyDocumentsDirectory(std::string& directory)
	{
	#ifdef _WIN64
		HKEY hKey;
		LONG lRes = RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders", 0, KEY_READ, &hKey);

		wchar_t szBuffer[512];
		DWORD dwBufferSize = sizeof(szBuffer);
		ULONG nError;
		nError = RegQueryValueExW(hKey, L"Personal", 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
		if (ERROR_SUCCESS != nError)
			return false;

		directory = WideToCStr(szBuffer);
		return true;
	#else
		return false;
	#endif
	}
	

	// --------------------------------------------------------------------------						
	// Function:	SetWorkingDirectory
	// Description:	sets working dir
	// Arguments:	directory
	// Returns:		if successfull
	// --------------------------------------------------------------------------
	bool FileSystem::SetWorkingDirectory(const std::string& directory)
	{
#ifdef _WIN64	
		return ::SetCurrentDirectory((LPCWSTR)CStrToWide(directory).data()) > 0;
#else
		return chdir(directory.c_str());
#endif
	}
	


	// --------------------------------------------------------------------------						
	// Function:	GetWorkingDirectory
	// Description:	returns working dir
	// Arguments:	var to store in
	// Returns:		if successfull
	// --------------------------------------------------------------------------	
	void FileSystem::GetWorkingDirectory(std::string& directory)
	{
#ifdef _WIN64
		wchar_t buffer[MAX_PATH + 1];
		::GetCurrentDirectory(MAX_PATH, (LPWSTR)buffer);
		directory = WideToCStr(buffer);
#else
		char buffer[MAX_PATH + 1];
		getcwd(buffer, MAX_PATH)
		directory = std::string(buffer);;
#endif
	}


	// --------------------------------------------------------------------------						
	// Function:	IsValidPath
	// Description:	check if path exists
	// Arguments:	path
	// Returns:		if path exists
	// --------------------------------------------------------------------------
	bool FileSystem::IsValidPath(const std::string& path)
	{
#ifdef _WIN64
		return (::GetFileAttributes((LPCWSTR)CStrToWide(path).data()) != -1);
#else
		struct stat buffer;
		return stat(path.c_str(), &buffer) == 0;
#endif
	}
	

	// --------------------------------------------------------------------------						
	// Function:	IsValidDirectory
	// Description:	check if dir exists
	// Arguments:	directory
	// Returns:		if dir exists
	// --------------------------------------------------------------------------
	bool FileSystem::IsValidDirectory(const std::string& directory)
	{
#ifdef _WIN64
		DWORD rv = GetFileAttributes((LPCWSTR)CStrToWide(directory).data());
		if(rv == INVALID_FILE_ATTRIBUTES)
			return false;
		return (rv & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
		struct stat buffer;
		if (stat(directory.c_str(), &buffer) != 0)
			return false;
		return (buffer.st_mode & S_IFDIR) != 0;
#endif
	}


	// --------------------------------------------------------------------------						
	// Function:	IsValidFile
	// Description:	check if file exists
	// Arguments:	file
	// Returns:		if file exists
	// --------------------------------------------------------------------------
	bool FileSystem::IsValidFile(const std::string& file)
	{
#ifdef _WIN64
		DWORD rv = ::GetFileAttributes(CStrToWide(file).data());
		if(rv == INVALID_FILE_ATTRIBUTES)
			return false;
		return (rv & FILE_ATTRIBUTE_DIRECTORY) == 0;
#else
		struct stat buffer;
		if (stat(file.c_str(), &buffer) != 0)
			return false;
		return (buffer.st_mode & S_IFREG) != 0;
#endif
	}


	// --------------------------------------------------------------------------						
	// Function:	ExtractDrive
	// Description:	extract drive from path
	// Arguments:	path, returned drive
	// Returns:		if sucessfull
	// --------------------------------------------------------------------------
	bool FileSystem::ExtractDrive(const std::string& path, std::string& drive)
	{
		std::string::size_type pos = path.find_first_of(ourDriveSeperator);
		if (pos != std::string::npos && pos == 1)
		{
			drive = path.substr(0, pos + 1);
			return true;
		}

		drive.erase();
		return false;
	
	}


	// --------------------------------------------------------------------------						
	// Function:	ExtractDirectory
	// Description:	extract directory from path
	// Arguments:	path, returned driectory
	// Returns:		if sucessfull
	// --------------------------------------------------------------------------
	bool FileSystem::ExtractDirectory(const std::string& path, std::string& directory)
	{
		std::string::size_type pos = path.find_last_of(ourDirSeperator);
		if (pos != std::string::npos)
		{
			directory = path.substr(0, pos);
			return true;
		}
		
		directory.erase();
		return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	ExtractExtension
	// Description:	extract extension from path
	// Arguments:	path, returned extension
	// Returns:		if sucessfull
	// --------------------------------------------------------------------------
	bool FileSystem::ExtractExtension(const std::string& path, std::string& extension)
	{
		std::string::size_type dirPos = path.find_last_of(ourDirSeperator);
		std::string::size_type extPos = path.find_last_of(ourExtSeperator);

		if (extPos != std::string::npos && (dirPos == std::string::npos || extPos > dirPos))
		{
			extension = path.substr(extPos + 1);
			return true;
		}

		extension.erase();
		return false;
	}



	// --------------------------------------------------------------------------						
	// Function:	ExtractTitle
	// Description:	extract title from path
	// Arguments:	path, returned title
	// Returns:		if sucessfull
	// --------------------------------------------------------------------------
	bool FileSystem::ExtractTitle(const std::string& path, std::string& title)
	{
		if (ExtractFilename(path, title))
		{
			std::string::size_type extPos = title.find_last_of(ourExtSeperator);

			if (extPos != std::string::npos)
				title = title.substr(0, extPos);

			return true;
		}

		return false;
	}

	
	// --------------------------------------------------------------------------						
	// Function:	ExtractFilename
	// Description:	extract file name from path
	// Arguments:	path, returned file name
	// Returns:		if sucessfull
	// --------------------------------------------------------------------------
	bool FileSystem::ExtractFilename(const std::string& path, std::string& filename)
	{
		std::string::size_type start = 0;
		std::string::size_type dirPos = path.find_last_of(ourDirSeperator);
		if (dirPos != std::string::npos)
			start = dirPos + 1;
		std::string::size_type len = path.length() - start;
		if (len != 0)
		{
			filename = path.substr(start, len);
			return true;
		}

		filename.erase();
		return false;		
	}


	// --------------------------------------------------------------------------						
	// Function:	Copy
	// Description:	copy file
	// Arguments:	source file, destinstation file, whether to over write destination
	// Returns:		if sucessfull
	// --------------------------------------------------------------------------
	bool FileSystem::Copy(const std::string& source, const std::string& destination, const bool overwrite)
	{
#ifdef _WIN64
		return ::CopyFile((LPCWSTR)CStrToWide(source).data(), (LPCWSTR)CStrToWide(destination).data(), !overwrite) != 0;
#else
		IBinaryFile ifile(source);
		OBinaryFile ofile(destination);
		while (!ifile.Eof())
		{
			char c;
			ifile >> c;
			ofile << c;
		}
		return true;
#endif
	}


	// --------------------------------------------------------------------------						
	// Function:	Move
	// Description:	move file
	// Arguments:	source file, destinstation file
	// Returns:		if sucessfull
	// --------------------------------------------------------------------------
	bool FileSystem::Move(const std::string& source, const std::string& destination)
	{
#ifdef _WIN64
		return ::MoveFile((LPCWSTR)CStrToWide(source).data(), (LPCWSTR)CStrToWide(destination).data()) != 0;
#else
		Copy(source, destination, true);
		Delete(source);
#endif
	}


	// --------------------------------------------------------------------------						
	// Function:	Delete
	// Description:	delete file
	// Arguments:	file to delete
	// Returns:		if sucessfull
	// --------------------------------------------------------------------------
	bool FileSystem::Delete(const std::string& path)
	{
#ifdef _WIN64
		std::vector<wchar_t> wpath = CStrToWide(path);
		if (::DeleteFile((LPCWSTR)wpath.data()) == 0)
		{
			Paths contents;
			GetDirectoryContents(path, "*.*", GetEverything, contents);
			for (int f = 0; f != contents.size(); f++)
			{
				Delete(path + "/" + contents[f]);
			}
			return ::RemoveDirectory((LPCWSTR)wpath.data()) != 0;
		}
		return true;
#else
		if (IsValidDirectory(path))
		{
			Paths contents;
			GetDirectoryContents(path, "*.*", GetEverything, contents);
			for (int a = 0; a != contents.size(); a++)
			{
				std::string temp = path + "/" + contents[a];
				remove(temp.c_str());
			}
			return rmdir(path.c_str()) == 0;

		}
		else
		{
			remove(path.c_str()) == 0;
		}
#endif
	}


	// --------------------------------------------------------------------------						
	// Function:	MakeDirectory
	// Description:	create directory
	// Arguments:	directory
	// Returns:		if sucessfull
	// --------------------------------------------------------------------------
	bool FileSystem::MakeDirectory(const std::string& name)
	{
#ifdef _WIN64
		return ::CreateDirectory((LPCWSTR)CStrToWide(name).data(), NULL) == TRUE;
#else
		return mkdir(name.c_str(), 0) != ENOENT;
#endif
	}


	// --------------------------------------------------------------------------						
	// Function:	GetDirectoryContents
	// Description:	get directory conecnts
	// Arguments:	directory, filter, flags of what to get, result
	// Returns:		if sucessfull
	// --------------------------------------------------------------------------
	bool FileSystem::GetDirectoryContents(const std::string& directory, const std::string& wildcard, const FileSystem::Filter flags, FileSystem::Paths& contents)
	{

		if (!IsValidDirectory(directory))
			return false;

		std::string answer;
		char* dummy = NULL;
		GetFullPath(directory, MAX_PATH, answer, dummy);
		answer += "/";
		answer += wildcard;
#ifdef _WIN64
		WIN32_FIND_DATA file;
		HANDLE handle = FindFirstFile((LPCWSTR)CStrToWide(answer).data(), &file);

		if (handle == INVALID_HANDLE_VALUE)
			return true;

		if (file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if ((flags & GetDirectories) && (strcmp(WideToCStr(file.cFileName).c_str(), ".") != 0) && (strcmp(WideToCStr(file.cFileName).c_str(), "..") != 0))
				contents.push_back(WideToCStr(file.cFileName));
		}
		else
		{
			if (flags & GetFiles)
				contents.push_back(WideToCStr(file.cFileName));
		}

		while (FindNextFile(handle, &file))
		{
			if (file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if((flags & GetDirectories) && ((strcmp(WideToCStr(file.cFileName).c_str(), ".") != 0) && (strcmp(WideToCStr(file.cFileName).c_str(), "..") != 0)))
					contents.push_back(WideToCStr(file.cFileName));
			}
			else
			{
				if (flags & GetFiles)
					contents.push_back(WideToCStr(file.cFileName));
			}
		}

		FindClose(handle);
#else
		unsigned char isFolder = 0x4;
		DIR* dir;
		struct dirent* dirEntry;
		dir = opendir(directory.c_str());
		while (dirEntry = readdir(dir))
		{
			if (wc_strncmp(wildcard.c_str(), dirEntry->d_name, -1, 1, 1) == WC_MATCH)
			{
				if (dirEntry->d_type == isFolder)
				{
					if (flags & GetDirectories)
					{
						if ((strcmp(dirEntry->d_name, ".") != 0) && (strcmp(dirEntry->d_name, "..") != 0))
						{
							contents.push_back(dirEntry->d_name);
						}
					}
				}
				else
				{
					if (flags & GetFiles)
						contents.push_back(dirEntry->d_name);
				}
			}
		}

		closedir(dir);
#endif
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	ExpandValue
	// Description:	epand alias string to path
	// Arguments:	alias, path
	// Returns:		if successfull
	// --------------------------------------------------------------------------
	bool FileSystem::ExpandValue(const std::string& value, std::string& expanded)
	{
		std::string temp = value;
		std::string name;
		std::string::size_type start;
		std::string::size_type end;

		expanded.erase();
		while(true)
		{
			start = temp.find_first_of(ourVariableDelimiter);
			if (start == std::string::npos)
			{
				expanded.append(temp);
				return true;
			}
		
			expanded.append(temp.substr(0, start));
			temp = value.substr(start + 1, temp.length() - start - 1);
			end = temp.find_first_of(ourVariableDelimiter);
			if (end == std::string::npos)
			{
				expanded = "";
				return false;
			}

			name = temp.substr(0, end);
			std::string lowerName(name);
			std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), tolower);
			VariableMap::iterator it = ourVariableMap.find(lowerName);
			if (it == ourVariableMap.end())
			{
				expanded = "";
				return false;
			}
			
			expanded.append(it->second);
			temp = temp.substr(end + 1, temp.length() - end - 1);

		}
	}


	// --------------------------------------------------------------------------						
	// Function:	SetVariable
	// Description:	sets a file variable 
	// Arguments:	vriable name, value
	// Returns:		if successfull
	// --------------------------------------------------------------------------
	bool FileSystem::SetVariable(std::string name, const std::string& value)
	{
		std::transform(name.begin(), name.end(), name.begin(), tolower);
		std::string expandedValue;
		if(!ExpandValue(value, expandedValue))
			return false;

		VariableMap::iterator it = ourVariableMap.find(name);
		if (it == ourVariableMap.end())
		{
			Variable var(name, expandedValue);
			ourVariableMap.insert(var);
		}
		else
		{
			it->second = expandedValue;
		}
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetVariable
	// Description:	gets a file variable 
	// Arguments:	vriable name, result value
	// Returns:		if successfull
	// --------------------------------------------------------------------------
	bool FileSystem::GetVariable(std::string name, std::string& value)
	{
		std::transform(name.begin(), name.end(), name.begin(), tolower);
		VariableMap::iterator it = ourVariableMap.find(name);
		if (it == ourVariableMap.end())
		{
			value = "";
			return false;
		}
		else
		{
			value = it->second;
			return true;
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	Resolve
	// Description:	expands path from aliases and retuns full path
	// Arguments:	path, new path
	// Returns:		if successfull
	// --------------------------------------------------------------------------
	bool FileSystem::Resolve(const std::string& path, std::string& newPath)
	{
		newPath = "";
		std::string expandedPath;
		if(!ExpandValue(path, expandedPath))
			return false;

		std::string answer;
		char* dummy = NULL;
		if(!GetFullPath(expandedPath, MAX_PATH, answer, dummy))
			return false;
		
		newPath = std::string(answer);
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	AddReadPath
	// Description:	add path allowed to reead from
	// Arguments:	path
	// Returns:		none
	// --------------------------------------------------------------------------
	void FileSystem::AddReadPath(const std::string& s)
	{
		std::string expandedPath;
		ExpandValue(s, expandedPath);
		Paths::iterator it = std::find(ourReadPaths.begin(), ourReadPaths.end(), expandedPath);
		if (it == ourReadPaths.end())
			ourReadPaths.push_back(expandedPath);
	}



	// --------------------------------------------------------------------------						
	// Function:	AddWritePath
	// Description:	add path allowed to write to
	// Arguments:	path
	// Returns:		none
	// --------------------------------------------------------------------------
	void FileSystem::AddWritePath(const std::string& s)
	{
		std::string expandedPath;
		ExpandValue(s, expandedPath);
		Paths::iterator it = std::find(ourWritePaths.begin(), ourWritePaths.end(), expandedPath);
		if (it == ourWritePaths.end())
			ourWritePaths.push_back(expandedPath);
	}


	// --------------------------------------------------------------------------						
	// Function:	RemoveReadPath
	// Description:	removes path allowed to read from
	// Arguments:	path
	// Returns:		none
	// --------------------------------------------------------------------------
	void FileSystem::RemoveReadPath(const std::string& s)
	{
		Paths::iterator it = std::find(ourReadPaths.begin(), ourReadPaths.end(), s);
		if (it != ourReadPaths.end())
			ourReadPaths.erase(it);
	}


	// --------------------------------------------------------------------------						
	// Function:	RemoveWritePath
	// Description:	removes path allowed to write to
	// Arguments:	path
	// Returns:		none
	// --------------------------------------------------------------------------
	void FileSystem::RemoveWritePath(const std::string& s)
	{
		Paths::iterator it = std::find(ourWritePaths.begin(), ourWritePaths.end(), s);
		if (it != ourWritePaths.end())
			ourWritePaths.erase(it);
	}


	// --------------------------------------------------------------------------						
	// Function:	IsValidReadPath
	// Description:	test if allowed read path
	// Arguments:	path, error message
	// Returns:		if successfull
	// --------------------------------------------------------------------------
	bool FileSystem::IsValidReadPath(const std::string& s, std::string& error)
	{
		return IsValidPath(s, ourReadPaths, "read", error);
	}


	// --------------------------------------------------------------------------						
	// Function:	IsValidWritePath
	// Description:	test if allowed write path
	// Arguments:	path, error message
	// Returns:		if successfull
	// --------------------------------------------------------------------------
	bool FileSystem::IsValidWritePath(const std::string& s, std::string& error)
	{
		return IsValidPath(s, ourWritePaths, "write", error);
	}


	// --------------------------------------------------------------------------						
	// Function:	IsValidPath
	// Description:	test if allowed path
	// Arguments:	path, paths to compare to, type for error message, error message
	// Returns:		if successfull
	// --------------------------------------------------------------------------
	bool FileSystem::IsValidPath(const std::string& s, const std::vector< std::string >& paths, const std::string& type, std::string& error)
	{
		bool ok = false;
		int i = 0;
		for (; i != paths.size(); i++)
		{
			if (paths[i].size() <= s.size() && s.substr(0, paths[i].size()) == paths[i])
			{
				ok = true;
				break;
			}
		}

		if (!ok)
		{
			char buffer[100];
			_snprintf(buffer, 100, "Invalid %s of %s: Path must be in a subdirectory of an allowed %s path.", type.c_str(), s.c_str(), type.c_str());
			error = buffer;
			return false;
		}

		if (paths[i].size() + 1 <= s.size())
		{
			std::string subdir = s.substr(paths[i].size() + 1, s.size());
			bool pathReposition = subdir.find(":") != -1 || subdir.rfind("..") != -1;
			if (pathReposition)
			{
				char buffer[100];
				_snprintf(buffer, 100, "Invalid %s of %s: Directory repostitioning not permitted.", type.c_str(), s.c_str());
				error = buffer;
				return false;
			}
		}
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	Oldest
	// Description:	test which is the oldest file
	// Arguments:	file 1, file 3
	// Returns:		oldest
	// --------------------------------------------------------------------------
	int FileSystem::Oldest(const std::string& f1, const std::string& f2)
	{
#ifdef _WIN64
		HANDLE      hF1, hF2;
		FILETIME    f1FileDate, f2FileDate;
		SYSTEMTIME t1, t2;
		int status;


		hF1 = CreateFile((LPCWSTR)CStrToWide(f1).data(), FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_DIRECTORY, NULL);
		hF2 = CreateFile((LPCWSTR)CStrToWide(f2).data(), FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_DIRECTORY, NULL);


		if (hF1 == INVALID_HANDLE_VALUE && hF2 != INVALID_HANDLE_VALUE)
			return 2;
		else if (hF1 != INVALID_HANDLE_VALUE && hF2 == INVALID_HANDLE_VALUE)
			return 1;
		if (hF1 == INVALID_HANDLE_VALUE && hF2 == INVALID_HANDLE_VALUE)
			return -1;

		status = GetFileTime(hF1, NULL, NULL, &f1FileDate);
		status = GetFileTime(hF2, NULL, NULL, &f2FileDate);

		FileTimeToSystemTime(&f1FileDate, &t1);
		FileTimeToSystemTime(&f2FileDate, &t2);
		int returnVal = 0;
		if (t1.wYear > t2.wYear)
		{
			returnVal = 2;
		}
		else if (t1.wYear < t2.wYear)
		{
			returnVal = 1;
		}
		else
		{

			if (t1.wMonth > t2.wMonth)
			{
				returnVal = 2;
			}
			else if (t1.wMonth < t2.wMonth)
			{
				returnVal = 1;
			}
			else
			{
				if (t1.wDay > t2.wDay)
				{
					returnVal = 2;
				}
				else if (t1.wDay < t2.wDay)
				{
					returnVal = 1;
				}
				else
				{
					if (t1.wHour > t2.wHour)
					{
						returnVal = 2;
					}
					else if (t1.wHour < t2.wHour)
					{
						returnVal = 1;
					}
					else
					{
						if (t1.wMinute > t2.wMinute)
						{
							returnVal = 2;
						}
						else if (t1.wMinute < t2.wMinute)
						{
							returnVal = 1;
						}
						else
						{
							if (t1.wSecond > t2.wSecond)
							{
								returnVal = 2;
							}
							else if (t1.wSecond < t2.wSecond)
							{
								returnVal = 1;
							}
							else
							{
								if (t1.wMilliseconds > t2.wMilliseconds)
								{
									returnVal = 2;
								}
								else if (t1.wMilliseconds < t2.wMilliseconds)
								{
									returnVal = 1;
								}
								else
								{
									returnVal = 0;
								}
							}

						}
					}
				}
			}
		}
		CloseHandle(hF1);
		CloseHandle(hF2);
		return returnVal;
#else
		struct stat buffer1, buffer2;
		int o1 = stat(f1.c_str(), &buffer1);
		int o2 = stat(f2.c_str(), &buffer2);
		if (o1 != 0 && o2 == 0)
			return 2;
		else if (o1 == 0 && o2 != 0)
			return 1;
		if (o1 == 0 && o2 == 0)
			return -1;

		if (buffer1.st_mtime < buffer1.st_mtime)
			return 1;
		else if (buffer2.st_mtime < buffer1.st_mtime)
			return 2;
		else
			return 0;
#endif
	}


	// --------------------------------------------------------------------------						
	// Function:	GetFileSize
	// Description:	gets file size
	// Arguments:	file name, size returned
	// Returns:		issuccessfull
	// --------------------------------------------------------------------------
	bool FileSystem::GetFileSize(const std::string& file, int& size)
	{
		std::fstream in(file.c_str());
		if (!in.is_open())
			return false;

		in.seekg(0, std::ios::end);
		size = (int)in.tellg();

		in.close();
		return true;
	}

}
