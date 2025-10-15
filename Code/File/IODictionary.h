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
#ifndef IODictionary_H
#define IODictionary_H

#include "../Common/SecureStl.h"
#include "../Common/Debug.h"
#include "../Common/Dictionary.h"
#include "IOInterface.h"
#include "Archive.h"
#include <map>
#include <string>



namespace shh {


	template<typename KEY>
	class IODictionary 
	{

		DECLARE_ARCHIVE(IODictionary);

	public:

		typedef std::map<Variant*, unsigned int> OutMap;
		typedef std::map<unsigned int, Variant*> InMap;

	
		class IOSorter
		{
			DECLARE_ARCHIVE(IODictionary::IOSorter)

		public:

			IOSorter();
			IOSorter(typename Dictionary<KEY>::Sorter& other);

			void Write(IOInterface& io, int version) const;
			void Read(IOInterface& io, int version);

			typename Dictionary<KEY>::Sorter* mySorter;
			static unsigned int ourVersion;
		};



		// IODictionary ////////////////////////////////////////////////////////////////////////////////////////


		IODictionary();
		IODictionary(Dictionary<KEY>& other);


		void Read(IOInterface& io, int version);
		void Write(IOInterface& io, int version) const;

		static void WriteKey(const Variant* key, IOInterface& io, int version);
		template<typename K> static void WriteKey(const K* key, IOInterface& io, int version);
		static void ReadKey(Variant*& key, IOInterface& io, int version);
		template<typename K> static void ReadKey(K*& key, IOInterface& io, int version);

		virtual std::string GetArchiveAlias() const;
		Dictionary<KEY>& GetDictionary();

		static unsigned int ourVersion;

	private:

		Dictionary<KEY>* myDictionary;
		mutable OutMap myOutMap;
		mutable InMap myInMap;

	};

	template<typename KEY> unsigned int IODictionary<KEY>::ourVersion = 0;
	template<typename KEY> unsigned int IODictionary<KEY>::IOSorter::ourVersion = 0;

	template<typename KEY> std::string IODictionary< KEY >::ourArchiveAlias = Archive::template RegisterClassWithArchive<IODictionary<KEY>>(std::string("Dictionary")+typeid(KEY).name(), NULL);
	template<typename KEY> unsigned int IODictionary<KEY>::ourArchiveVersion = IODictionary<KEY>::ourVersion;

	template<typename KEY> std::string IODictionary<KEY>::IOSorter::ourArchiveAlias = Archive::template RegisterClassWithArchive<IODictionary<KEY>::IOSorter>(std::string("Dictionary") + typeid(KEY).name(), NULL);
	template<typename KEY> unsigned int IODictionary<KEY>::IOSorter::ourArchiveVersion = IODictionary<KEY>::IOSorter::ourVersion;

	//IMPLEMENT_ARCHIVE_TEMPLATE(IODictionary<KEY>::IOSorter, KEY, "Dictionary_Sorter");

	
	// IODictionary:: IOSorter Inlines /////////////////////////////////////////////////

	// --------------------------------------------------------------------------						
	// Function:	IOSorter
	// Description: constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	template<typename KEY> IODictionary<KEY>::IOSorter::IOSorter()
	{}


	// --------------------------------------------------------------------------						
	// Function:	IOSorter
	// Description: copy ructor
	// Arguments:	other
	// Returns:		none
	// --------------------------------------------------------------------------
	template<typename KEY> IODictionary<KEY>::IOSorter::IOSorter(typename Dictionary<KEY>::Sorter& other) :
		mySorter(&other) 
	{}


	// --------------------------------------------------------------------------						
	// Function:	Write
	// Description: writes sorter
	// Arguments:	stream interface, version
	// Returns:		none
	// --------------------------------------------------------------------------
	template<typename KEY> void  IODictionary<KEY>::IOSorter::Write(IOInterface& io, int version) const
	{
		INITIALIZE_SERIAL_OUT(io);

		IODictionary<KEY>::WriteKey(mySorter->myKey, io, version);
		SERIALIZE_OUT(io, mySorter->mySortId);

		FINALIZE_SERIAL(io);
	}


	// --------------------------------------------------------------------------						
	// Function:	Read
	// Description: reads sorter
	// Arguments:	stream interface, version
	// Returns:		none
	// --------------------------------------------------------------------------
	template<typename KEY> void IODictionary<KEY>::IOSorter::Read(IOInterface& io, int version)
	{
		INITIALIZE_SERIAL_IN(io);

		IODictionary<KEY>::ReadKey(mySorter->myKey, io, version);
		SERIALIZE_IN(io, mySorter->mySortId);

		FINALIZE_SERIAL(io);
	}


	// IODictionary //////////////////////////////////////////////////////////////


	// --------------------------------------------------------------------------						
	// Function:	IODictionary
	// Description: constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	template<typename KEY> IODictionary<KEY>::IODictionary() { myDictionary = new Dictionary<KEY>; };


	// --------------------------------------------------------------------------						
	// Function:	IODictionary
	// Description: copy constructor
	// Arguments:	other
	// Returns:		none
	// --------------------------------------------------------------------------
	template<typename KEY> IODictionary<KEY>::IODictionary(Dictionary<KEY>& other) : myDictionary(&other) { }


	// --------------------------------------------------------------------------						
	// Function:	Read
	// Description: reads dictionary
	// Arguments:	stream interface, version
	// Returns:		none
	// --------------------------------------------------------------------------
	template<typename KEY> void IODictionary<KEY>::Read(IOInterface& io, int version)
	{
		INITIALIZE_SERIAL_IN(io);

		unsigned int size;
		SERIALIZE_IN(io, size);
		for (unsigned int count = 0; count != size; count++)
		{
			unsigned int id;
			SERIALIZE_IN(io, id);

			KEY* key;
			ReadKey(key, io, version);

			std::string alias;
			SERIALIZE_IN(io, alias);
			std::string dictAlias = IODictionary::ourArchiveAlias;
			std::string dictIntAlias = IODictionary<unsigned int>::ourArchiveAlias;
			std::string dictStrAlias = IODictionary<std::string>::ourArchiveAlias;
			std::string dictVarAlias = IODictionary<Variant>::ourArchiveAlias;
			if (alias == dictAlias)
			{
				NonVariant<Dictionary<KEY>>* d = new NonVariant<Dictionary<KEY>>();
				IODictionary iodict(d->myValue);
				SERIALIZE_IN(io, iodict);
				myInMap[id] = d;
				myDictionary->myVariables[key] = d;
			}
			else if (alias == dictIntAlias)
			{
				NonVariant<Dictionary<unsigned int>>* d = new NonVariant<Dictionary<unsigned int>>();
				IODictionary<unsigned int> iodict(d->myValue);
				SERIALIZE_IN(io, iodict);
				myInMap[id] = d;
				myDictionary->myVariables[key] = d;
			}
			else if (alias == dictStrAlias)
			{
				NonVariant<Dictionary<std::string>>* d = new NonVariant<Dictionary<std::string>>();
				IODictionary<std::string> iodict(d->myValue);
				SERIALIZE_IN(io, iodict);
				myInMap[id] = d;
				myDictionary->myVariables[key] = d;
			}
			else if (alias == dictVarAlias)
			{
				NonVariant<Dictionary<Variant>>* d = new NonVariant<Dictionary<Variant>>();
				IODictionary<Variant> iodict(d->myValue);
				SERIALIZE_IN(io, iodict);
				myInMap[id] = d;
				myDictionary->myVariables[key] = d;
			}
			else
			{
				Variant* iovariant = reinterpret_cast<Variant*>(Archive::GetConstructFunc(alias)(io));
				Variant* variant = (Variant*)iovariant->New();
				iovariant->Read(io, version);
				myInMap[id] = variant;
				myDictionary->myVariables[key] = variant;
			}

		}

		size = (int)myDictionary->myOrderedVariables.size();
		SERIALIZE_IN(io, size);
		for (unsigned int count = 0; count != size; count++)
		{
			typename Dictionary<KEY>::Sorter sorter;
			IOSorter s(sorter);
			SERIALIZE_IN(io, s);
			unsigned int id;
			SERIALIZE_IN(io, id);
			Variant* v = myInMap.find(id)->second;
			myDictionary->myOrderedVariables[sorter] = v;
		}

		SERIALIZE_IN(io, myDictionary->myNextArrayIndex);

		FINALIZE_SERIAL(io);

	}


	// --------------------------------------------------------------------------						
	// Function:	Writes
	// Description: writes dictionary
	// Arguments:	stream interface, version
	// Returns:		none
	// --------------------------------------------------------------------------
	template<typename KEY> void IODictionary<KEY>::Write(IOInterface& io, int version) const
	{
		INITIALIZE_SERIAL_OUT(io);

		int nextId = 0;

		unsigned int size = (unsigned int)myDictionary->myVariables.size();
		SERIALIZE_OUT(io, size);
		for (typename Dictionary<KEY>::VariablesConstIterator vit = myDictionary->myVariables.begin(); vit != myDictionary->myVariables.end(); vit++)
		{
			myOutMap[vit->second] = nextId;

			SERIALIZE_OUT(io, nextId++);

			WriteKey(vit->first, io, version);

			Variant& iovariant = vit->second->ConvertToIO();
			Dictionary<KEY>* k;
			Dictionary<unsigned int>* i;
			Dictionary<std::string>* s;
			Dictionary<Variant>* v;
			if (vit->second->GetPtr(k))
			{
				std::string dictAlias = IODictionary::ourArchiveAlias;
				SERIALIZE_OUT(io, dictAlias);
				IODictionary iodict(*k);
				SERIALIZE_OUT(io, iodict);
			}
			else if (vit->second->GetPtr(i))
			{
				std::string dictAlias = IODictionary<unsigned int>::ourArchiveAlias;
				SERIALIZE_OUT(io, dictAlias);
				IODictionary<unsigned int> iodict(*i);
				SERIALIZE_OUT(io, iodict);
			}
			else if (vit->second->GetPtr(s))
			{
				std::string dictAlias = IODictionary<std::string>::ourArchiveAlias;
				SERIALIZE_OUT(io, dictAlias);
				IODictionary<std::string> iodict(*s);
				SERIALIZE_OUT(io, iodict);
			}
			else if (vit->second->GetPtr(v))
			{
				std::string dictAlias = IODictionary<Variant>::ourArchiveAlias;
				SERIALIZE_OUT(io, dictAlias);
				IODictionary<Variant> iodict(*v);
				SERIALIZE_OUT(io, iodict);
			}
			else
			{
				SERIALIZE_OUT(io, iovariant.GetArchiveAlias());
				iovariant.Write(io, version);
			}
		}

		size = (int)myDictionary->myOrderedVariables.size();
		SERIALIZE_OUT(io, size);
		for (typename Dictionary<KEY>::OrderedVariablesConstIterator ovit = myDictionary->myOrderedVariables.begin(); ovit != myDictionary->myOrderedVariables.end(); ovit++)
		{
			IOSorter s((typename Dictionary<KEY>::Sorter&)ovit->first);
			SERIALIZE_OUT(io, s);
			unsigned int id = myOutMap.find(ovit->second)->second;
			SERIALIZE_OUT(io, id);
		}

		SERIALIZE_OUT(io, myDictionary->myNextArrayIndex);

		FINALIZE_SERIAL(io);
	}


	// --------------------------------------------------------------------------						
	// Function:	WriteKey
	// Description: write element key
	// Arguments:	key, stream interface, version
	// Returns:		none
	// --------------------------------------------------------------------------
	template<typename KEY> void IODictionary<KEY>::WriteKey(const Variant* key, IOInterface& io, int version)
	{
		Variant& iovariant = key->ConvertToIO();
		SERIALIZE_OUT(io, iovariant.GetArchiveAlias());
		Dictionary<KEY>* v;
		if (key->GetPtr(v))
		{
			IODictionary iodict(*v);
			SERIALIZE_OUT(io, iodict);
		}
		else
		{
			iovariant.Write(io, version);
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	WriteKey
	// Description: write element key
	// Arguments:	key, stream interface, version
	// Returns:		none
	// --------------------------------------------------------------------------
	template<typename KEY>
	template<typename K> void IODictionary<KEY>::WriteKey(const K* key, IOInterface& io, int version)
	{
		SERIALIZE_OUT(io, *key);
	}


	// --------------------------------------------------------------------------						
	// Function:	ReadKey
	// Description: write element key
	// Arguments:	key, stream interface, version
	// Returns:		none
	// --------------------------------------------------------------------------
	template<typename KEY> void IODictionary<KEY>::ReadKey(Variant*& key, IOInterface& io, int version)
	{
		std::string keyAlias;
		SERIALIZE_IN(io, keyAlias);
		Variant* iovariant = reinterpret_cast<Variant*>(Archive::GetConstructFunc(keyAlias)(io));
		key = (Variant*)iovariant->New();
		iovariant->Read(io, version);
	}


	// --------------------------------------------------------------------------						
	// Function:	ReadKey
	// Description: write element key
	// Arguments:	key, stream interface, version
	// Returns:		none
	// --------------------------------------------------------------------------
	template<typename KEY>
	template<typename K> void IODictionary<KEY>::ReadKey(K*& key, IOInterface& io, int version)
	{
		key = new K();
		SERIALIZE_IN(io, *key);
	}


	// --------------------------------------------------------------------------						
	// Function:	GetArchiveAlias
	// Description: returns alias for disctionary in file
	// Arguments:	none
	// Returns:		alias
	// --------------------------------------------------------------------------
	template<typename KEY> std::string IODictionary<KEY>::GetArchiveAlias() const { return ourArchiveAlias; }


	// --------------------------------------------------------------------------						
	// Function:	GetDictionary
	// Description: returns this io dictionary as a base dicionary
	// Arguments:	none
	// Returns:		dictionary
	// --------------------------------------------------------------------------
	template<typename KEY>
	Dictionary<KEY>& IODictionary<KEY>::GetDictionary(){  return *myDictionary; }


	
} // namespace shh
#endif
