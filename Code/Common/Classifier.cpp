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

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Classifier.h"


namespace shh {

	Classifier::Registry Classifier::ourRegistry;
	unsigned int Classifier::ourNextId = 1;

	//////////////////////////////////////////////////////////////////////////////////////////

	template<class C, class I, class _Fn>
	inline bool for_each(C& c, I f, I l, _Fn fn)
	{
		while (f != l)
		{
			if (!fn(*f, c))
				return false;
			++f;
		}
		return true;
	}

	Classifier::Ids::const_iterator find_from(const Classifier::Id& id,
		Classifier::Ids::const_iterator& it, const Classifier::Ids::const_iterator& end)
	{
		while (it != end && id < *it)
			it++;

		return it;
	}

	bool InsertId(const unsigned int& i, std::set<unsigned int>& o) { o.insert(i); return true; }
	bool EraseId(const unsigned int& i, std::set<unsigned int>& o) { o.erase(i); return true; }
	bool InsertString(const std::string& i, std::set<std::string>& o) { o.insert(i); return true; }
	bool EraseString(const  std::string& i, std::set<std::string>& o) { o.erase(i); return true; }

	struct WhileFound
	{
		WhileFound(const Classifier::Ids::const_iterator& start, const Classifier::Ids::const_iterator& end) : myLastFound(start), myEnd(end) {}
		bool operator()(const Classifier::Id& i, const Classifier::Ids& t)
		{
			myLastFound = find_from(i, myLastFound, myEnd);
			return (myLastFound != t.end() && *myLastFound == i);
		}
		Classifier::Ids::const_iterator myLastFound;
		Classifier::Ids::const_iterator myEnd;
	};

	struct WhileNotFound
	{
		WhileNotFound(const Classifier::Ids::const_iterator& start, const Classifier::Ids::const_iterator& end) : myLastFound(start), myEnd(end) {}
		bool operator()(const Classifier::Id& i, const Classifier::Ids& t)
		{
			myLastFound = find_from(i, myLastFound, myEnd);
			return (myLastFound == t.end() || *myLastFound != i);
		}
		Classifier::Ids::const_iterator myLastFound;
		Classifier::Ids::const_iterator myEnd;
	};

	//////////////////////////////////////////////////////////////////////////////////////////

	// --------------------------------------------------------------------------						
	// Function:	Classifier
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Classifier::Classifier()
	{};


	// --------------------------------------------------------------------------						
	// Function:	Register
	// Description:	registers a new classifier string
	// Arguments:	classifier string
	// Returns:		none
	// --------------------------------------------------------------------------
	inline unsigned int Classifier::Register(const std::string& cls)
	{
		Registry::iterator it = ourRegistry.find(cls);
		if (it == ourRegistry.end())
		{
			ourRegistry[cls] = ourNextId;
			return ourNextId++;
		}
		else
		{
			return it->second;
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	Add
	// Description:	adds classifer strings from another calssifier 
	// Arguments:	other classifier
	// Returns:		none
	// --------------------------------------------------------------------------
	void Classifier::Add(const Classifier& other)
	{
		Strings& s = const_cast<Strings&>(other.myStrings);
		for_each(myIds, other.myIds.begin(), other.myIds.end(), InsertId);
		for_each(myStrings, s.begin(), s.end(), InsertString);
	}


	// --------------------------------------------------------------------------						
	// Function:	Add
	// Description:	adds a new classifier string
	// Arguments:	string
	// Returns:		none
	// --------------------------------------------------------------------------
	void Classifier::Add(const char cls[])
	{
		if (myStrings.find(cls) != myStrings.end())
			return;

		if (myIds.insert(Register(cls)).second)
			myStrings.insert(cls);
	}


	// --------------------------------------------------------------------------						
	// Function:	Add
	// Description:	adds a new classifier string
	// Arguments:	string
	// Returns:		none
	// --------------------------------------------------------------------------
	void Classifier::Add(const std::string& cls)
	{
		if (myStrings.find(cls) != myStrings.end())
			return;

		if (myIds.insert(Register(cls)).second)
			myStrings.insert(cls);
	}


	// --------------------------------------------------------------------------						
	// Function:	<<
	// Description:	adds a new classifier string
	// Arguments:	string
	// Returns:		none
	// --------------------------------------------------------------------------
	Classifier& Classifier::operator <<(const char cls[]) 
	{
		Add(cls);
		return *this;
	}


	// --------------------------------------------------------------------------						
	// Function:	Remove
	// Description:	removes a classifier string
	// Arguments:	string
	// Returns:		none
	// --------------------------------------------------------------------------
	void Classifier::Remove(const char cls[])
	{
		Strings::iterator it = myStrings.find(cls);
		if (it == myStrings.end())
			return;

		myStrings.erase(it);
		unsigned int id = (ourRegistry.find(cls))->second;
		myIds.erase(myIds.find(id));
	}


	// --------------------------------------------------------------------------						
	// Function:	Remove
	// Description:	removes a classifier string
	// Arguments:	string
	// Returns:		none
	// --------------------------------------------------------------------------
	void Classifier::Remove(const std::string& cls)
	{
		Strings::iterator it = myStrings.find(cls);
		if (it == myStrings.end())
			return;

		myStrings.erase(it);
		unsigned int id = (ourRegistry.find(cls))->second;
		myIds.erase(myIds.find(id));
	}


	// --------------------------------------------------------------------------						
	// Function:	Remove
	// Description:	removes from this classifer the classifier strings contained
	//				in classifer passed in
	// Arguments:	other classifer
	// Returns:		none
	// --------------------------------------------------------------------------
	void Classifier::Remove(const Classifier& other)
	{
		if ((myIds.size() == 0) || (other.myIds.size() == 0))
			return;

		for_each(myIds, other.myIds.begin(), other.myIds.end(), EraseId);
		for_each(myStrings, other.myStrings.begin(), other.myStrings.end(), EraseString);
	}


	// --------------------------------------------------------------------------						
	// Function:	Reset
	// Description:	clears the classifier
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Classifier::Reset()
	{
		myStrings.clear();
		myIds.clear();
	}


	// --------------------------------------------------------------------------						
	// Function:	==
	// Description:	compare the classifier to another
	// Arguments:	other classifier
	// Returns:		if equal
	// --------------------------------------------------------------------------
	bool Classifier::operator==(const Classifier& other) const
	{
		return myIds == other.myIds;
	}



	// --------------------------------------------------------------------------						
	// Function:	!=
	// Description:	compare the classifier to another
	// Arguments:	other classifier
	// Returns:		if not equal
	// --------------------------------------------------------------------------
	bool Classifier::operator!=(const Classifier& other) const
	{
		return !(myIds == other.myIds);
	}




	// --------------------------------------------------------------------------						
	// Function:	<
	// Description:	compare the classifier size
	// Arguments:	other classifier
	// Returns:		if size is less than other
	// --------------------------------------------------------------------------
	bool Classifier::operator <(const Classifier& other) const
	{ 
		return myIds.size() < other.myIds.size(); 
	}


	// --------------------------------------------------------------------------						
	// Function:	Match
	// Description:	matches the classifier to another
	// Arguments:	other classifier, type of match:
	//				MATCH_ALL = this classifer is contained withing other
	//				MATCH_ANY = some of this classifer is contained in other
	//				MATCH_NONE = none of this classifier is contained in other
	//				MATCH_EXACT = this and other classifer are the same
	// Returns:		true if matched
	// --------------------------------------------------------------------------
	bool Classifier::Match(const Classifier& o, const MatchType type) const
	{
		const Classifier::Ids& other = o.myIds;
		switch (type)
		{
		case MATCH_ALL:
		{
			if (myIds.size() != other.size())
				return false;

			WhileFound match(myIds.begin(), myIds.end());
			for_each(*const_cast<Classifier::Ids*>(&myIds), other.begin(), other.end(), match);
			return match.myLastFound != myIds.end();
			break;
		}
		case MATCH_ANY:
		{
			if (other.size() == 0)
				return true;
			if (myIds.size() == 0 && other.size() != 0)
				return false;

			WhileNotFound match(myIds.begin(), myIds.end());
			for_each(*const_cast<Classifier::Ids*>(&myIds), other.begin(), other.end(), match);
			return match.myLastFound != myIds.end();
		}
		case MATCH_NONE:
		{
			if ((other.size() == 0) || (myIds.size() == 0))
				return true;

			WhileNotFound match(myIds.begin(), myIds.end());
			for_each(*const_cast<Classifier::Ids*>(&myIds), other.begin(), other.end(), match);
			return match.myLastFound == myIds.end();
		}
		default:	// MATCH_EXACT
			return myIds == other;
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	DumpString
	// Description:	returns a comma speatated likst of classifer strings
	//				in this classifer
	// Arguments:	none
	// Returns:		string
	// --------------------------------------------------------------------------
	std::string Classifier::DumpString() const
	{
		std::string ret;

		Strings::const_iterator it = myStrings.begin();
		Strings::const_iterator end =myStrings.end();
		std::string sep = "";
		while (it != end)
		{
			ret += sep + *it;
			it++;
			sep = ",";
		}

		return std::string("(" + ret + " )");
	}


	// --------------------------------------------------------------------------						
	// Function:	GetUID
	// Description:	returns runtime UID of classifer string
	// Arguments:	none
	// Returns:		UID or 0 if not found
	// --------------------------------------------------------------------------
	Classifier::Id Classifier::GetUID(const std::string& cls)
	{
		Registry::iterator it = ourRegistry.find(cls);
		if (it == ourRegistry.end())
			return 0;
		else
			return it->second;
	}


} // namespace shh


