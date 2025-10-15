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
#include "ConfigurationFile.h"
#include "IOVariant.h"
#include <algorithm>
#include <vector>

namespace shh {

	class Ignorer {};
	std::string ConfigurationFile::ourDummy;

	// --------------------------------------------------------------------------						
	// Function:	>>
	// Description:	reads file
	// Arguments:	file
	// Returns:		value
	// --------------------------------------------------------------------------

	ITextFile& operator >>(ITextFile& file, Ignorer& i)
	{
		ConfigurationFile::EatWhite(file);
		file.Get();
		return file;
	}



	// --------------------------------------------------------------------------						
	// Function:	Read
	// Description:	reads file and store in map
	// Arguments:	file, map
	// Returns:		if successfil
	// --------------------------------------------------------------------------
	bool ConfigurationFile::Read(ITextFile& file, StringKeyDictionary& map)
	{
		char c = file.Peek();
		if (c == '{')
			file.Get();

		while (!file.Eof())
		{
			EatWhite(file);

			// remove comments
			c = file.Peek();
			if (c == '/')
			{
				while (c != '\n')
					c = file.Get();
				EatWhite(file);
				c = file.Peek();
			}

			if (c == -1)
				return true;

			// end of map
			if (c == '}')
			{
				file.Get();
				return true;
			}

			if ((c < 65 || (c > 90 && c < 97 && c != 95) || c > 122))
				return false;

			std::string key = ReadToken(file);


			int startarr = (int)key.rfind("[");
			int endarr = (int)key.rfind("]");
			bool isArray = false;
			int index = -1;
			if (startarr != -1 && endarr != -1)
			{
				isArray = true;
				if (endarr != key.size() - 1 || startarr > endarr)
					return false;

				if (endarr - startarr > 1)
					index = atoi(key.substr(startarr + 1, endarr - startarr - 1).c_str());

				key = key.substr(0, startarr);
			}

			EatWhite(file);
			c = file.Peek();

			if (c == -1)
				return false;

			if (c == '}')
			{
				file.Get();
				return false;
			}

			StringKeyDictionary* mapToInsert = &map;
			StringKeyDictionary v;
			if (isArray)
			{
				StringKeyDictionary* temp = NULL;
				if (!map.Exists(key) || !map.IsType(key, temp))
					map.Set(key, StringKeyDictionary());

				mapToInsert = map.GetPtr(key, &v);
				if (index == -1)
					index = mapToInsert->GetNextArrayIndex();

				mapToInsert->SetNextArrayIndex(index + 1);

				char buffer[20];
				sprintf(buffer, "%d", index);
				key = buffer;
				const std::string lead = "0000000000";
				key = lead.substr(0, 10 - key.length()) + key;
			}

			std::vector<int> integers;
			std::vector<double> doubles;
			bool gotValue = false;
			bool readOnlyDoubles = false;
			bool readOnlyIntegers = false;

			do
			{
				EatWhite(file);

				c = file.Peek();

				if (c == '\"')
				{
					if (readOnlyDoubles || readOnlyIntegers)
						return false;

					mapToInsert->Set(key, ReadString(file));
				}
				else if ((c >= 48 && c <= 57) || c == '-')
				{
					std::string number;
					
					file >> number;
					if (number.find(".") != -1 && !readOnlyIntegers)
					{
						readOnlyDoubles = true;
						double value = std::stod(number);
						doubles.push_back(value);
					}
					else
					{	
						readOnlyIntegers = true;
						int value = std::stoi(number);
						integers.push_back(value);
					}
				}
				else if (!gotValue && ((c >= 65 && c <= 90) || (c >= 97 && c <= 122)))
				{
					if (readOnlyDoubles || readOnlyIntegers)
						return false;

					// bools and tokens
					bool value;
					std::string token;
					if (ReadBool(file, value, token))
						mapToInsert->Set(key, value);
					else
						return false;
					//mapToInsert->Set(key, token);
				}
				else if (c == '{')
				{
					if (readOnlyDoubles || readOnlyIntegers)
						return false;

					StringKeyDictionary value;
					mapToInsert->Set(key, value);
					StringKeyDictionary* newConf = mapToInsert->GetPtr(key, &value);
					if (!Read(file, *newConf))
						return false;
				}
				else if (c == '(')
				{
					if (readOnlyDoubles || readOnlyIntegers)
						return false;

					mapToInsert->Set(key, ReadBracketed(file, '(', ')'));
				}
				else
				{
					if (!gotValue) // got another var name when should have value
						return false;
					break;
				}

				gotValue = true;

			} while (true);

			if (doubles.size() == 1)
				mapToInsert->Set(key, doubles[0]);
			else if (doubles.size() > 1)
				mapToInsert->Set(key, doubles);
			else if (integers.size() == 1)
				mapToInsert->Set(key, integers[0]);
			else if (integers.size() > 1)
				mapToInsert->Set(key, integers);
		}
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	ReadToken
	// Description:	reads a token from the file
	// Arguments:	file
	// Returns:		string token
	// --------------------------------------------------------------------------
	std::string ConfigurationFile::ReadToken(ITextFile& file)
	{
		char c = file.Peek();
		std::string value;
		while (file && c != ' ' && c != '=' && c != '\n' && c != 10 && c != 13)
		{
			c = file.Get();
			value.append(1, c);
			c = file.Peek();
		}
		return value;
	}


	// --------------------------------------------------------------------------						
	// Function:	ReadString
	// Description:	reads string from file
	// Arguments:	file
	// Returns:		string
	// --------------------------------------------------------------------------
	std::string ConfigurationFile::ReadString(ITextFile& file)
	{
		file.Get();
		char c = file.Get();
		std::string value;
		while (file && c != '\"')
		{
			value.append(1, c);
			c = file.Get();
		}
		return value;
	}


	// --------------------------------------------------------------------------						
	// Function:	Readmber
	// Description:	reads number from file
	// Arguments:	file
	// Returns:		number
	// --------------------------------------------------------------------------
	double ConfigurationFile::ReadNumber(ITextFile& file)
	{
		std::string number;
		double value = 0;
		file >> number;
		return value;
	}


	// --------------------------------------------------------------------------						
	// Function:	ReadBool
	// Description:	reads bool from file
	// Arguments:	file
	// Returns:		bool
	// --------------------------------------------------------------------------
	bool ConfigurationFile::ReadBool(ITextFile& file, bool& value, std::string& token)
	{
		token = ReadToken(file);
		std::string s = token;
		std::transform(s.begin(), s.end(), s.begin(), tolower);
		if (s == "true")
		{
			value = true;
			return true;
		}
		else if (s == "false")
		{
			value = false;
			return true;
		}
		return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	ReadString
	// Description:	reads string from file that is bracketed by specific chars
	// Arguments:	file, begin braacketr, end bracket char
	// Returns:		string
	// --------------------------------------------------------------------------
	std::string ConfigurationFile::ReadBracketed(ITextFile& file, char beginchr, char endchr)
	{
		std::string value;
		char c = file.Get();

		int count = 1;
		while (file && count > 0)
		{
			value.append(1, c);
			c = file.get();

			if (c == beginchr)
				count++;
			else if (c == endchr)
				count--;
		}
		value.append(1, c);
		return value;
	}


	// --------------------------------------------------------------------------						
	// Function:	EatWhite
	// Description:	removes immediate white space
	// Arguments:	file
	// Returns:		none
	// --------------------------------------------------------------------------
	void ConfigurationFile::EatWhite(ITextFile& file)
	{
		char c = ' ';
		while (!file.Eof() && (c = file.Peek(), c == '\t' || c == '\n' || c == ' ' || c == 13 || c == 10))
			file.Get();
	}


	// --------------------------------------------------------------------------						
	// Function:	Write
	// Description:	writes map to file
	// Arguments:	file, map
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool ConfigurationFile::Write(OTextFile& file, const StringKeyDictionary& map, const std::string& key, const std::string& indent)
	{

		StringKeyDictionary::Variables::const_iterator it = map.Begin();
		StringKeyDictionary::Variables::const_iterator end = map.End();

		while (it != end)
		{
			StringKeyDictionary v;
			bool isArray = false;
			if (map.IsType(it, &v))
			{
				StringKeyDictionary temp;
				StringKeyDictionary* x = const_cast<StringKeyDictionary*>(&map)->GetPtr(it, &temp);
				if (x->GetNextArrayIndex() == 0)
					isArray = true;
			}

			char c = *it->first->c_str();
			if (!isArray && c >= 48 && c <= 57)
			{
				file << indent;
				file << key;
				file << "[";
				file << *it->first;
				file << "]";
			}
			else if (!isArray)
			{
				file << indent;
				file << *it->first;
			}


			
			double d = 0;
			double i = 0;
			bool b = false;
			std::string s;
			std::vector<double> dv;
			std::vector<int> di;
			if (map.IsType(it, &s))
			{
				s = map.GetFromIt(it, s);
				file << " " << "\"" << s << "\"";
			}
			else if (map.IsType(it, &d))
			{
				double d = map.GetFromIt(it, (double)0);
				file << " " << std::to_string(d);
			}
			else if (map.IsType(it, &i))
			{
				int i = map.GetFromIt(it, (int)0);
				file << " " << std::to_string(i);
			}
			else if (map.IsType(it, &b))
			{
				file << " " << ((bool)map.GetFromIt(it, false) ? "true" : "false");
			}
			else if (map.IsType(it, &dv))
			{
				std::vector<double> dv = map.GetFromIt(it, dv);
				for (int a = 0; a != dv.size(); ++a)
					file << " " << dv[a];
			}
			else if (map.IsType(it, &di))
			{
				std::vector<int> dv = map.GetFromIt(it, di);
				for (int a = 0; a != di.size(); ++a)
					file << " " << di[a];
			}
			else if (map.IsType(it, &v))
			{
				if (!isArray)
				{
					file.Put('\n');
					file << indent;
					file << "{";
					file.Put('\n');
				}
				StringKeyDictionary v;
				Write(file, *const_cast<StringKeyDictionary*>(&map)->GetPtr(it, &v), *it->first, isArray ? indent : indent + "\t");
				if (!isArray)
				{
					file << indent;
					file << "}";
				}
			}
			else
			{
				return false;
			}
			if (map.GetNextArrayIndex() != 0)
				file.Put('\n');
			it++;
		}
		return true;
	}

} // namespace shh
