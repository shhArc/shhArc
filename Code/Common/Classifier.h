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

#ifndef CLASSIFIER_H
#define CLASSIFIER_H

#include "SecureStl.h"
#include "Debug.h"
#include <set>
#include <map>
#include <string>

namespace shh {

	class  Classifier
	{

	public:

		typedef unsigned int Id;
		typedef std::set<Id> Ids;
		typedef std::set< std::string >Strings;

		enum MatchType
		{
			_MATCH_FIRST_ = 0,

			MATCH_EXACT,
			MATCH_ANY,
			MATCH_ALL,
			MATCH_NONE,

			_MATCH_LAST_,
		};

		Classifier();

		const Strings& Get() const { return myStrings; }

		void Add(const Classifier& other);
		void Add(const char clsrr[]);
		void Add(const std::string& clsrr);
		void Remove(const Classifier& other);
		void Remove(const char clsrr[]);
		void Remove(const std::string& clsrr);
		void Reset();


		// pattern matching
		bool Empty() const { return myIds.empty(); }

		bool operator==(const Classifier& other) const;
		bool operator!=(const Classifier& other) const;
		bool operator <(const Classifier& other) const;

		bool Match(const Classifier& other, const MatchType type) const;

		std::string DumpString() const;
		static Id GetUID(const std::string& cls);

		Classifier& operator <<(const char cls[]);

		

	private:

		typedef std::map< std::string, unsigned int > Registry;

		static Registry ourRegistry;
		static unsigned int ourNextId;

		Strings myStrings;
		Ids myIds;


		static unsigned int Register(const std::string& clsrr);
		inline static bool RegisterString(const std::string& s, Ids& ids) { ids.insert(Classifier::Register(s)); return true; }



	};

} 

#endif // CLASSIFIER_H

