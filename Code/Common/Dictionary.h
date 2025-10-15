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
#ifndef DICTIONARY_H
#define DICTIONARY_H




#include "SecureStl.h"
#include "Variant.h"
#include <map>
#include <string>
#include <string.h>



namespace shh {


	template<typename KEY>
	class Dictionary 
	{
		template<typename T> friend class IODictionary;
	
	public:


		class Sorter
		{

		public:
			KEY *myKey;
			unsigned int mySortId;


			Sorter() : mySortId (0), myKey(NULL) {}
			Sorter(const KEY& k, int id = 0) : mySortId(id), myKey(Dictionary<KEY>::NewKey(&k)){}

			void Null()
			{
				if (myKey)
				{
					delete myKey;
					myKey = NULL;
				}
			}
			inline const KEY& operator*() const { return *myKey; }
			inline bool operator<(const Sorter& other) const { return mySortId < other.mySortId; }
			inline bool operator>(const Sorter& other) const { return mySortId > other.mySortId; }
			inline bool operator==(const Sorter& other) const { return *myKey == *other.myKey; }
			inline bool operator!=(const Sorter& other) const { return *myKey != *other.myKey; }
			const Sorter& operator=(const Sorter& other)
			{ 
				Null();
				myKey = NewKey(other.myKey); 
				mySortId = other.mySortId; 
				return *this;
			}
			void Write(IOInterface& io, unsigned int version) const;
			void Read(IOInterface& io, unsigned int version);

		};

		struct CompareKeyByValue {
			bool operator()(const KEY* a, const KEY* b) const { return *a < *b; }
		};




		typedef std::map<KEY*, Variant*, CompareKeyByValue> Variables;
		typedef std::map<Sorter, Variant*> OrderedVariables;
		typedef typename Variables::iterator VariablesIterator;
		typedef typename Variables::const_iterator VariablesConstIterator;
		typedef typename OrderedVariables::iterator OrderedVariablesIterator;
		typedef typename OrderedVariables::const_iterator OrderedVariablesConstIterator;




		template<class T> static const T GetConfig(const KEY& variableKey, const T& defaultValue, const Dictionary<KEY> &config);
		template<class T> static bool SetConfig(const KEY& variableKey, const T& value, Dictionary<KEY> &config);


		Dictionary();
		Dictionary(const Dictionary& other);
		~Dictionary();


		const Dictionary& operator=(const Dictionary& other);

		bool operator==(const Dictionary& other) const;
		bool operator!=(const Dictionary& other) const;
		bool operator<(const Dictionary& other) const;
		bool operator>(const Dictionary& other) const;


		bool Exists(const KEY& variableKey) const;
		void Null(const KEY& variableKey);

		template<class T> bool IsType(const KEY& variableKey, const T* t = NULL) const;
		template<class T> bool IsType(const VariablesConstIterator& it, const T* t = NULL) const;

		const std::string Get(const KEY& variableKey, const char str[]) const;
		Variant const* const Get(const KEY& variableKey) const;
		template<class T> const T Get(const KEY& variableKey, const T& defaultValue) const;
		template<class T> T* GetPtr(const KEY& variableKey, T* defaultValue);

		void Set(const char variableKey[], const char value[]);
		template<class K> void Set(const K& variableKey, const char value[]);
		template<class K> void Set(const K& variableKey, const Variant& variant);
		template<class K, class T> void Set(const K& variableKey, const T& value);

		int Size() const;

		VariablesConstIterator Begin() const;
		VariablesConstIterator End() const;

		KEY GetFromIt(const VariablesConstIterator& it, const char defaultValue[]) const;
		template<class T> const T GetFromIt(const VariablesConstIterator& it, const T& defaultValue) const;
		template<class T> T* GetPtr(const VariablesConstIterator& it, T* defaultValue);

		OrderedVariablesConstIterator OrderedBegin() const;
		OrderedVariablesConstIterator OrderedEnd() const;


		KEY GetFromIt(const OrderedVariablesConstIterator& it, const char defaultValue[]) const;
		template<class T> const T GetFromIt(const OrderedVariablesConstIterator& it, const T& defaultValue) const;
		template<class T> T* GetPtr(const OrderedVariablesConstIterator& it, T* defaultValue);
		int IncNextArrayIndex();
		int GetNextArrayIndex() const;

		void SetNextArrayIndex(unsigned int i);

		void Clean();
		void Merge(const Dictionary<KEY> &other);

		void Write(IOInterface& io, int version) const;
		void Read(IOInterface& io, int version);
	

		static KEY* NewKey(const Variant* k);
		static KEY* NewKey(const void* k);

		bool ReadPrivileges(const std::string& name, Privileges& privileges) const;

	protected:


		Variables myVariables;
		OrderedVariables myOrderedVariables;
		unsigned int myNextArrayIndex;

		void AddSorter(const KEY& variableKey, Variant* n);
		static KEY* Clone(const void* k);

	};


	typedef Dictionary<std::string> StringKeyDictionary;
	typedef Dictionary<unsigned int> ArrayKeyDictionary;
	typedef Dictionary<Variant> VariantKeyDictionary;


	inline void ConvertVariantDictToStringDict(const VariantKeyDictionary& vd, StringKeyDictionary& sd);
	inline void ConvertStringDictToVarientDict(const StringKeyDictionary& sd, VariantKeyDictionary& vd);
	inline void ConvertArrayDictToVarientDict(const ArrayKeyDictionary& ad, VariantKeyDictionary& vd);

	// Helper Inlines ////////////////////////////////////////////////////////

	// --------------------------------------------------------------------------						
	// Function:	ConvertVariantDictToStringDict
	// Description:	converts a variant key dict to a string key dict
	// Arguments:	variant dict, string dict to fill
	// Returns:		none
	// --------------------------------------------------------------------------
	inline void ConvertVariantDictToStringDict(const VariantKeyDictionary& vd, StringKeyDictionary &sd)
	{
		for (VariantKeyDictionary::VariablesConstIterator it = vd.Begin(); it != vd.End(); it++)
		{
			std::string key;
			if (it->first->Get(key))
			{
				VariantKeyDictionary dict;
				if (it->second->Get(dict))
				{
					StringKeyDictionary subDict;
					ConvertVariantDictToStringDict(dict, subDict);
					sd.Set(key, subDict);
				}
				else
				{
					sd.Set(key, *it->second);
				}
			}
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	ConvertVariantDictToStringDict
	// Description:	converts a string key dict to a variant key dict
	// Arguments:	string dict, variant dict to fill
	// Returns:		none
	// --------------------------------------------------------------------------
	inline void ConvertStringDictToVarientDict(const StringKeyDictionary& sd, VariantKeyDictionary& vd)
	{
		for (StringKeyDictionary::VariablesConstIterator it = sd.Begin(); it != sd.End(); it++)
		{
			if (it->second->IsType<StringKeyDictionary>())
			{
				StringKeyDictionary dict;
				if (it->second->Get(dict))
				{
					VariantKeyDictionary subDict;
					ConvertStringDictToVarientDict(dict, subDict);
					vd.Set(NonVariant<std::string>(*it->first), subDict);
				}
			}
			if (it->second->IsType<ArrayKeyDictionary>())
			{
				ArrayKeyDictionary dict;
				if (it->second->Get(dict))
				{
					VariantKeyDictionary subDict;
					ConvertArrayDictToVarientDict(dict, subDict);
					vd.Set(NonVariant<std::string>(*it->first), subDict);
				}
			}
			else
			{
				vd.Set(NonVariant<std::string>(*it->first), *it->second);
			}
		}

	}


	// --------------------------------------------------------------------------						
	// Function:	ConvertUArrayDictToVarientDict
	// Description:	converts a array key dict to a variant key dict
	// Arguments:	array dict, variant dict to fill
	// Returns:		none
	// --------------------------------------------------------------------------
	inline void ConvertArrayDictToVarientDict(const ArrayKeyDictionary& ad, VariantKeyDictionary& vd)
	{
		for (ArrayKeyDictionary::VariablesConstIterator it = ad.Begin(); it != ad.End(); it++)
		{
			if (it->second->IsType<StringKeyDictionary>())
			{
				StringKeyDictionary dict;
				if (it->second->Get(dict))
				{
					VariantKeyDictionary subDict;
					ConvertStringDictToVarientDict(dict, subDict);
					vd.Set(NonVariant<unsigned int>(*it->first), subDict);
				}
			}
			if (it->second->IsType<ArrayKeyDictionary>())
			{
				ArrayKeyDictionary dict;
				if (it->second->Get(dict))
				{
					VariantKeyDictionary subDict;
					ConvertArrayDictToVarientDict(dict, subDict);
					vd.Set(NonVariant<unsigned int>(*it->first), subDict);
				}
			}
			else
			{
				vd.Set(NonVariant<unsigned int>(*it->first), *it->second);
			}
		}

	}

	// Dictionary Inlines ////////////////////////////////////////////////////////

	// --------------------------------------------------------------------------						
	// Function:	GetConfig
	// Description:	gets a variable of template type from key in given config
	//				dictionary
	// Arguments:	key, value to return if not found, dictionary to search
	// Returns:		value found or default value if not found
	// --------------------------------------------------------------------------
	template<typename KEY>
	template<class T> const T Dictionary<KEY>::GetConfig(const KEY& variableKey, const T& defaultValue, const Dictionary<KEY> &config)
	{
		T value;
		Variant const* const variant = config.Get(variableKey);
		if (variant == NULL || !variant->Get(value))
			return defaultValue;
		else
			return value;
	}


	// --------------------------------------------------------------------------						
	// Function:	SetConfig
	// Description:	Sets a variable of template type with key in given config
	//				dictionary
	// Arguments:	key, value, dictionary to place in
	// Returns:		true if set
	// --------------------------------------------------------------------------
	template<typename KEY>
	template<class T> bool Dictionary<KEY>::SetConfig(const KEY& variableKey, const T& value, Dictionary<KEY> &config)
	{
		config.Set(variableKey, NonVariant<T>(value));
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	Dictionary
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	template<typename KEY> Dictionary<KEY>::Dictionary() : myNextArrayIndex(0) 
	{}


	// --------------------------------------------------------------------------						
	// Function:	Dictionary
	// Description:	copy constructor
	// Arguments:	dict to copy
	// Returns:		none
	// --------------------------------------------------------------------------
	template<typename KEY> Dictionary<KEY>::Dictionary(const Dictionary<KEY>& other) : myNextArrayIndex(0) 
	{ 
		*this = other; 
	}


	// --------------------------------------------------------------------------						
	// Function:	~Dictionary
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	template<typename KEY> Dictionary<KEY>::~Dictionary() 
	{ 
		Clean(); 
	}


	// --------------------------------------------------------------------------						
	// Function:	=
	// Description:	assigner
	// Arguments:	dict to copy
	// Returns:		this
	// --------------------------------------------------------------------------
	template<typename KEY> const Dictionary<KEY>& Dictionary<KEY>::operator=(const Dictionary<KEY>& other)
	{
		Clean();
		for (OrderedVariablesConstIterator it = other.myOrderedVariables.begin(); it != other.myOrderedVariables.end(); it++)
		{
			Variant* v = it->second->NewClone();
			myVariables[NewKey(it->first.myKey)] = v;
			myOrderedVariables[Sorter(*it->first, IncNextArrayIndex())] = v;
		}
		myNextArrayIndex = other.myNextArrayIndex;
		return *this;
	}


	// --------------------------------------------------------------------------						
	// Function:	==
	// Description:	equals operator
	// Arguments:	dict to compare
	// Returns:		true if equal
	// --------------------------------------------------------------------------
	template<typename KEY> bool Dictionary<KEY>::operator==(const Dictionary<KEY>& other) const
	{
		OrderedVariablesConstIterator it = myOrderedVariables.begin();
		OrderedVariablesConstIterator oit = other.myOrderedVariables.begin();
		for (; it != myOrderedVariables.end() && oit != other.myOrderedVariables.end(); it++, oit++)
		{
			if (it->first != oit->first || *it->second != *oit->second)
				return false;
		}
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	!=
	// Description:	not equals operator
	// Arguments:	dict to compare
	// Returns:		true if not equal
	// --------------------------------------------------------------------------
	template<typename KEY> bool Dictionary<KEY>::operator!=(const Dictionary<KEY>& other) const
	{
		OrderedVariablesConstIterator it = myOrderedVariables.begin();
		OrderedVariablesConstIterator oit = other.myOrderedVariables.begin();
		for (; it != myOrderedVariables.end() && oit != other.myOrderedVariables.end(); it++, oit++)
		{
			if (it->first == oit->first && *it->second == *oit->second)
				return false;
		}
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	<
	// Description:	less than operator (only uses keys)
	// Arguments:	dict to compare
	// Returns:		true if less than
	// --------------------------------------------------------------------------
	template<typename KEY> bool Dictionary<KEY>::operator<(const Dictionary<KEY>& other) const
	{
		OrderedVariablesConstIterator it = myOrderedVariables.begin();
		OrderedVariablesConstIterator oit = other.myOrderedVariables.begin();
		for (; it != myOrderedVariables.end() && oit != other.myOrderedVariables.end(); it++, oit++)
		{
			if (it->first > oit->first)
				return false;
		}
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	>
	// Description:	greater than operator (only uses keys)
	// Arguments:	dict to compare
	// Returns:		true if greater than
	// --------------------------------------------------------------------------
	template<typename KEY> bool Dictionary<KEY>::operator>(const Dictionary<KEY>& other) const
	{
		OrderedVariablesConstIterator it = myOrderedVariables.begin();
		OrderedVariablesConstIterator oit = other.myOrderedVariables.begin();
		for (; it != myOrderedVariables.end() && oit != other.myOrderedVariables.end(); it++, oit++)
		{
			if (it->first < oit->first)
				return false;
		}
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	Exist
	// Description:	test is key is in dictionary
	// Arguments:	key
	// Returns:		true if exists
	// --------------------------------------------------------------------------
	template<typename KEY> bool Dictionary<KEY>::Exists(const KEY& variableKey) const 
	{ 
		return myVariables.find((KEY*)&variableKey) != myVariables.end(); 
	}


	// --------------------------------------------------------------------------						
	// Function:	Null
	// Description:	removes key and associated variable
	// Arguments:	key
	// Returns:		none
	// --------------------------------------------------------------------------
	template<typename KEY> void Dictionary<KEY>::Null(const KEY& variableKey)
	{
		VariablesIterator it = myVariables.find((KEY*)&variableKey);
		if (it != myVariables.end())
		{
			delete it->first;
			delete it->second;
			myVariables.erase(it);
			OrderedVariablesIterator oit = myOrderedVariables.begin();
			OrderedVariablesIterator oend = myOrderedVariables.end();
			while (oit != oend)
			{
				if (*oit->first.myKey == variableKey)
				{
					((Sorter*)(&oit->first))->Null();
					myOrderedVariables.erase(oit);
					return;
				}
				oit++;
			}
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	IsType
	// Description:	tests type of variable with given key
	// Arguments:	key, pointer to type
	// Returns:		true if variable is of type
	// --------------------------------------------------------------------------
	template<typename KEY>
	template<class T> bool Dictionary<KEY>::IsType(const KEY& variableKey, const T* t) const
	{
		VariablesConstIterator it = myVariables.find((KEY*)&variableKey);
		if (it == myVariables.end() || !it->second->IsType(t))
			return false;

		return it->second->IsType(t);
	}


	// --------------------------------------------------------------------------						
	// Function:	IsType
	// Description:	tests type of variable with given key
	// Arguments:	key, pointer to type
	// Returns:		true if variable is of type
	// --------------------------------------------------------------------------
	template<typename KEY>
	template<class T> bool Dictionary<KEY>::IsType(const VariablesConstIterator& it, const T* t) const
	{
		if (it == myVariables.end() || !it->second->IsType(t))
			return false;

		return it->second->IsType(t);
	}

	// --------------------------------------------------------------------------						
	// Function:	Get
	// Description:	gets variable with given key
	// Arguments:	key, default string if not found
	// Returns:		variable or default value
	// --------------------------------------------------------------------------
	template<typename KEY>
	const std::string Dictionary<KEY>::Get(const KEY& variableKey, const char str[]) const
	{
		std::string value;
		std::string defaultValue(str);
		VariablesConstIterator it = myVariables.find((KEY*)&variableKey);
		if (it == myVariables.end() || !it->second->Get(value))
			return defaultValue;
		else
			return value;
	}
	template<typename KEY> Variant const* const Dictionary<KEY>::Get(const KEY& variableKey) const
	{
		VariablesConstIterator it = myVariables.find((KEY*)&variableKey);
		if (it == myVariables.end())
			return NULL;

		return it->second;
	}
	template<typename KEY>
	template<class T> const T Dictionary<KEY>::Get(const KEY& variableKey, const T& defaultValue) const
	{
		T value;
		VariablesConstIterator it = myVariables.find((KEY*)&variableKey);
		if (it == myVariables.end() || !it->second->Get(value))
			return defaultValue;
		else
			return value;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetPtr
	// Description:	gets pointer variable with given key
	// Arguments:	key, default value if not found
	// Returns:		pointer to variable or default value
	// --------------------------------------------------------------------------
	template<typename KEY>
	template<class T> T* Dictionary<KEY>::GetPtr(const KEY& variableKey, T* defaultValue)
	{
		static T* value;
		VariablesConstIterator it = myVariables.find((KEY*)&variableKey);
		if (it == myVariables.end() || !it->second->GetPtr(value))
			return defaultValue;
		else
			return value;
	}


	// --------------------------------------------------------------------------						
	// Function:	Set
	// Description:	Sets variable with given key
	// Arguments:	key, value 
	// Returns:		none
	// --------------------------------------------------------------------------
	template<typename KEY>
	void Dictionary<KEY>::Set(const char variableKey[], const char value[])
	{
		Set(KEY(variableKey), std::string(value));
	}
	template<typename KEY>
	template<class K> void Dictionary<KEY>::Set(const K& variableKey, const char value[]) 
	{ 
		Set(variableKey, std::string(value)); 
	}
	template<typename KEY> 
	template<class K> void Dictionary<KEY>::Set(const K& variableKey, const Variant& variant)
	{
		K* key = (K*)&variableKey;
		KEY* exactKey = static_cast<KEY*>(key);
		Null(*exactKey);
		Variant* n = variant.NewClone();
		myVariables[NewKey(exactKey)] = n;
		AddSorter(*exactKey, n);
	}
	template<typename KEY>
	template<class K, class T> void Dictionary<KEY>::Set(const K& variableKey, const T& value)
	{
		K* key = (K*)&variableKey;
		KEY* exactKey = static_cast<KEY*>(key);
		Null(*exactKey);
		NonVariant<T>* n = new NonVariant<T>(value);
		myVariables[NewKey(exactKey)] = n;
		AddSorter(*exactKey, n);
	}


	// --------------------------------------------------------------------------						
	// Function:	Size
	// Description:	returns size of dictionary
	// Arguments:	none
	// Returns:		size
	// --------------------------------------------------------------------------
	template<typename KEY> int Dictionary<KEY>::Size() const 
	{ 
		return (int)myVariables.size(); 
	}


	// --------------------------------------------------------------------------						
	// Function:	Begin
	// Description:	returns start iterator of dictionary
	// Arguments:	none
	// Returns:		iterator
	// --------------------------------------------------------------------------
	template<typename KEY> typename Dictionary<KEY>::VariablesConstIterator Dictionary<KEY>::Begin() const 
	{ 
		return myVariables.begin(); 
	}


	// --------------------------------------------------------------------------						
	// Function:	End
	// Description:	returns end iterator of dictionary
	// Arguments:	none
	// Returns:		iterator
	// --------------------------------------------------------------------------
	template<typename KEY> typename Dictionary<KEY>::VariablesConstIterator Dictionary<KEY>::End() const 
	{ 
		return myVariables.end(); 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetFromIt
	// Description:	returns variable from iterator of dictionary
	// Arguments:	none
	// Returns:		variable
	// --------------------------------------------------------------------------
	template<typename KEY> KEY Dictionary<KEY>::GetFromIt(const VariablesConstIterator& it, const char defaultValue[]) const 
	{ 
		return GetFromIt(it, KEY(defaultValue)); 
	}
	template<typename KEY>
	template<class T> const T Dictionary<KEY>::GetFromIt(const VariablesConstIterator& it, const T& defaultValue) const
	{
		T value;
		if (it == myVariables.end() || !it->second->Get(value))
			return defaultValue;
		else
			return value;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetPtr
	// Description:	returns variable pointer from iterator of dictionary
	// Arguments:	none
	// Returns:		variable
	// --------------------------------------------------------------------------
	template<typename KEY>
	template<class T> T* Dictionary<KEY>::GetPtr(const VariablesConstIterator& it, T* defaultValue)
	{
		static T* value;
		if (it == myVariables.end() || !it->second->GetPtr(value))
			return defaultValue;
		else
			return value;
	}


	// --------------------------------------------------------------------------						
	// Function:	OrderedBegin
	// Description:	returns start iterator of ordered dictionary
	// Arguments:	none
	// Returns:		iterator
	// --------------------------------------------------------------------------
	template<typename KEY> typename Dictionary<KEY>::OrderedVariablesConstIterator Dictionary<KEY>::OrderedBegin() const { return myOrderedVariables.begin(); }


	// --------------------------------------------------------------------------						
	// Function:	OrderedEnd
	// Description:	returns end iterator of ordered dictionary
	// Arguments:	none
	// Returns:		iterator
	// --------------------------------------------------------------------------
	template<typename KEY> typename Dictionary<KEY>::OrderedVariablesConstIterator Dictionary<KEY>::OrderedEnd() const { return myOrderedVariables.end(); }


	// --------------------------------------------------------------------------						
	// Function:	GetFromIt
	// Description:	returns variable from iterator of ordered dictionary
	// Arguments:	none
	// Returns:		variable
	// --------------------------------------------------------------------------
	template<typename KEY> KEY Dictionary<KEY>::GetFromIt(const OrderedVariablesConstIterator& it, const char defaultValue[]) const 
	{ 
		return GetFromIt(it, KEY(defaultValue)); 
	}
	template<typename KEY>
	template<class T> const T Dictionary<KEY>::GetFromIt(const OrderedVariablesConstIterator& it, const T& defaultValue) const
	{
		T value;
		if (it == myOrderedVariables.end() || !it->second->Get(value))
			return defaultValue;
		else
			return value;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetPtr
	// Description:	returns variable pointer from iterator of ordered dictionary
	// Arguments:	none
	// Returns:		variable
	// --------------------------------------------------------------------------
	template<typename KEY>
	template<class T> T* Dictionary<KEY>::GetPtr(const OrderedVariablesConstIterator& it, T* defaultValue)
	{
		T* value;
		if (it == myOrderedVariables.end() || !it->second->GetPtr(value))
			return defaultValue;
		else
			return value;
	}


	// --------------------------------------------------------------------------						
	// Function:	IncNextArrayIndex
	// Description:	increments variable array index
	// Arguments:	none
	// Returns:		last index
	// --------------------------------------------------------------------------
	template<typename KEY> int Dictionary<KEY>::IncNextArrayIndex() 
	{ 
		return myNextArrayIndex++; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetNextArrayIndex
	// Description:	gets next variable array index
	// Arguments:	none
	// Returns:		index
	// --------------------------------------------------------------------------
	template<typename KEY> int Dictionary<KEY>::GetNextArrayIndex() const 
	{ 
		return myNextArrayIndex; 
	}


	// --------------------------------------------------------------------------						
	// Function:	SetNextArrayIndex
	// Description:	sets next variable array index
	// Arguments:	index
	// Returns:		none
	// --------------------------------------------------------------------------
	template<typename KEY> void Dictionary<KEY>::SetNextArrayIndex(unsigned int i)
	{
		if (i > myNextArrayIndex)
			myNextArrayIndex = i;
	}


	// --------------------------------------------------------------------------						
	// Function:	Clean
	// Description:	removes all keys and variables
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	template<typename KEY> void Dictionary<KEY>::Clean()
	{
		for (VariablesIterator it = myVariables.begin(); it != myVariables.end(); it++)
		{
			delete it->first;
			delete it->second;
		}
		myVariables.clear();

		for (OrderedVariablesIterator oit = myOrderedVariables.begin(); oit != myOrderedVariables.end(); oit++)
			((Sorter*)(&oit->first))->Null();
		myOrderedVariables.clear();
		myNextArrayIndex = 0;
	}


	// --------------------------------------------------------------------------						
	// Function:	Merges
	// Description:	merges other dictionary into this
	// Arguments:	other dictionary
	// Returns:		none
	// --------------------------------------------------------------------------
	template<typename KEY> void Dictionary<KEY>::Merge(const Dictionary<KEY>& other)
	{
		for (StringKeyDictionary::VariablesConstIterator it = other.Begin(); it != other.End(); it++)
		{
			KEY name = *it->first;
			if (!Exists(name))
			{
				Set(name, it->second);
			}
			else
			{
				Dictionary<KEY> otherSubDict;
				bool ok1 = it->second->Get(otherSubDict);
				Dictionary<KEY> subDict;
				subDict = Get(name, subDict);
				if (ok1 && subDict.Size() != 0)
					subDict.Merge(otherSubDict);
				else
					Set(name, it->second);
			}
		}

	}

	// --------------------------------------------------------------------------						
	// Function:	NewKey
	// Description:	clones key
	// Arguments:	key
	// Returns:		none
	// --------------------------------------------------------------------------
	template<typename KEY> KEY* Dictionary<KEY>::NewKey(const Variant* k) 
	{ 
		return k->NewClone(); 
	}
	template<typename KEY> KEY* Dictionary<KEY>::NewKey(const void* k) 
	{ 
		return Clone(k); 
	}


	// --------------------------------------------------------------------------						
	// Function:	AddSorter
	// Description:	adds key to ordered list of keys with pointer to variable
	// Arguments:	key, variable pointer
	// Returns:		none
	// --------------------------------------------------------------------------
	template<typename KEY> void Dictionary<KEY>::AddSorter(const KEY& variableKey, Variant* n)
	{
		OrderedVariablesIterator oit = myOrderedVariables.begin();
		OrderedVariablesIterator oend = myOrderedVariables.end();
		while (oit != oend)
		{
			if (*oit->first.myKey == variableKey)
			{
				oit->second = n;
				return;
			}
			oit++;
		}
		myOrderedVariables[Sorter(variableKey, IncNextArrayIndex())] = n;
	}


	// --------------------------------------------------------------------------						
	// Function:	Clone
	// Description:	clones key
	// Arguments:	key
	// Returns:		none
	// --------------------------------------------------------------------------
	template<typename KEY> KEY* Dictionary<KEY>::Clone(const void* k) { return new KEY(*(KEY*)k); }


	// --------------------------------------------------------------------------						
	// Function:	ReadPrivileges
	// Description:	reads privileges from a dictionary
	// Arguments:	name of privilege table, privileges to return
	// Returns:		true if read
	// --------------------------------------------------------------------------
	template<typename KEY> bool Dictionary<KEY>::ReadPrivileges(const std::string& name, Privileges& privileges) const
	{
		privileges = NoPrivilege;
		ArrayKeyDictionary p;
		p = Get(name, p);

		for (ArrayKeyDictionary::VariablesConstIterator pit = p.Begin(); pit != p.End(); pit++)
		{
			std::string id;
			if (pit->second->Get(id))
			{
				if (id == "basic")
					privileges = privileges | BasicPrivilege;
				else if (id == "slave")
					privileges = privileges | SlavePrivilege;
				else if (id == "schema")
					privileges = privileges | SchemaPrivilege;
				else if (id == "master")
					privileges = privileges | MasterPrivilege;
				else if (id == "agent")
					privileges = privileges | AgentPrivilege;
				else if (id == "world")
					privileges = privileges | WorldPrivilege;
				else if (id == "god")
					privileges = privileges | GodPrivilege;
			}
		}

		return privileges != 0;
	}
} // namespace shh
#endif
