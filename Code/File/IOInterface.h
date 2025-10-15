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

#ifndef IO_INTERFACE_H
#define IO_INTERFACE_H


#include "../Common/SecureStl.h"
#include "../Common/Debug.h"
#include "../Common/Exception.h"
#include <list>
#include <vector>
#include <map>
#include <set>
#include <deque>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>

#ifndef _WIN64
#define __int64 long long
#endif

#define NS(S) std::string(typeid(S).name())
#define TYPENAME(S) NS(S).substr(NS(S).rfind(" ")+1, NS(S).length()-NS(S).find(" "))
#define INITIALIZE_SERIAL_IN(IOSTREAM)  if(IOSTREAM.myArchiveTags){ std::string c; IOSTREAM.InitializeStreamIn(c); DEBUG_ASSERT(c == ourArchiveAlias); }
#define INITIALIZE_SERIAL_OUT(IOSTREAM) if(IOSTREAM.myArchiveTags){ IOSTREAM.InitializeStreamOut(ourArchiveAlias); };

#define FINALIZE_SERIAL(IOSTREAM) if(IOSTREAM.myArchiveTags) IOSTREAM.FinalizeClassSerial();
#define SERIALIZE_OUT(IOSTREAM, OBJECT) IOSTREAM.SetVarname(#OBJECT); IOSTREAM << OBJECT;
#define SERIALIZE_IN(IOSTREAM, OBJECT) IOSTREAM.SetVarname(#OBJECT); IOSTREAM >> OBJECT;

#define DECLARE_ARCHIVE(CLASSNAME) public: \
										static void WriteArchive(IOInterface &io, const void* object) { io << ourArchiveAlias; io << ourArchiveVersion; static_cast<const CLASSNAME*>(object)->Write(io, ourArchiveVersion); }\
										static void *ReadArchive(IOInterface &io, void* object){ std::string alias; io >> alias; unsigned int version; io >> version; if(!object) object = new CLASSNAME();static_cast<CLASSNAME*>(object)->Read(io, version); return object; }\
										static void *ConstructArchive(IOInterface &io){ return new CLASSNAME(); }\
										static std::string ourArchiveAlias;\
										static unsigned int ourArchiveVersion;\
									private:


#define IMPLEMENT_ARCHIVE(CLASSNAME, ALIAS)	std::string CLASSNAME::ourArchiveAlias =  Archive::template RegisterClassWithArchive<CLASSNAME>(std::string(ALIAS), NULL);\
											unsigned int CLASSNAME::ourArchiveVersion = CLASSNAME::ourVersion;
#define IMPLEMENT_ARCHIVE_TEMPLATE(CLASSNAME, T, ALIAS) template<typename T> std::string CLASSNAME<T>::ourArchiveAlias =  Archive::template RegisterClassWithArchive<CLASSNAME<T>>(std::string(ALIAS), NULL);\
														template<> unsigned int CLASSNAME<T>::ourArchiveVersion = CLASSNAME::ourVersion;							

namespace shh {

	class StrStreamBuf : public std::streambuf
	{
	public:
		char* IPtr() { return gptr(); }
		char* IBegin() { return eback(); }
		char* IEnd() { return egptr(); }
		char* OPtr() { return pptr(); }
		char* OBegin() { return pbase(); }
		char* OEnd() { return epptr(); }
	};

	class IOInterface
	{
	public:

		enum Flags { None = 0, In = std::fstream::in, Out = std::fstream::out };

		typedef std::streambuf::off_type seekpos;
		typedef std::char_traits<char>::pos_type pos_type;
		enum seekdir { start = std::ios_base::beg, current = std::ios_base::cur, end = std::ios_base::end };

		typedef std::pair<std::string, void*> ConstructionPair;
		typedef std::vector< ConstructionPair > ConstructionList;


		IOInterface();
		virtual ~IOInterface();

		virtual void Open(const std::string& filename, IOInterface::Flags flags) = 0;

		virtual unsigned int Read(char* buffer, unsigned int count);
		virtual unsigned int Write(const char* buffer, unsigned int count);


		virtual IOInterface& Read(std::stringstream& object);
		virtual IOInterface& Read(std::string& object);
		virtual IOInterface& Read(char& object);
		virtual IOInterface& Read(unsigned char& object);
		virtual IOInterface& Read(short& object);
		virtual IOInterface& Read(unsigned short& object);
		virtual IOInterface& Read(int& object);
		virtual IOInterface& Read(unsigned int& object);
		virtual IOInterface& Read(long int& object);
		virtual IOInterface& Read(unsigned long int& object);
		virtual IOInterface& Read(__int64& object);
		virtual IOInterface& Read(float& object);
		virtual IOInterface& Read(double& object);
		virtual IOInterface& Read(bool& object);
		template<class T>  IOInterface& Read(T& object);

		virtual IOInterface& Write(const std::stringstream& object);
		virtual IOInterface& Write(const std::string& object);
		virtual IOInterface& Write(const char* object);
		virtual IOInterface& Write(const char object);
		virtual IOInterface& Write(const unsigned char object);
		virtual IOInterface& Write(const short object);
		virtual IOInterface& Write(const unsigned short object);
		virtual IOInterface& Write(const int object);
		virtual IOInterface& Write(const unsigned int object);
		virtual IOInterface& Write(const long int object);
		virtual IOInterface& Write(const unsigned long int object);
		virtual IOInterface& Write(const __int64 object);
		virtual IOInterface& Write(const float object);
		virtual IOInterface& Write(const double object);
		virtual IOInterface& Write(const bool object);
		template<class T>  IOInterface& Write(const T& object);

		template<class T> IOInterface& ReadClass(T& object);
		template<class T> IOInterface& WriteClass(const T& object);

		virtual ConstructionPair InstanticateClass();
		virtual void InitializeStreamIn(std::string& classname);
		virtual void InitializeStreamOut(const std::string& classname);
		virtual void InitializeStreamOut(const char classname[]);

		virtual void FinalizeClassSerial();
		void SetVarname(const std::string& name);
		void ClearVarname(const std::string& name);

		virtual int Peek() = 0;
		virtual bool Eof() = 0;
		virtual std::streamoff GetPos() = 0;
		virtual void SeekOff(seekpos off, seekdir way) = 0;


		int peek();
		bool eof();
		int get();
		operator void* ();

		template<class T> static bool IsClassPtr(T*);
		template<class T> static bool IsClassPtr(T);
		template<class T> static T& ClassType(T*);
		template<class T> static T& ClassType(T);

		int myVersion;
		bool myArchiveTags;


	protected:

		std::string myVarname;

		template<class T> void WriteClassInternal(T* object);
		template<class T> void WriteClassInternal(T& object);
		template<class T> void ReadClassInternal(T* object);
		template<class T> void ReadClassInternal(T& object);

	};



	// IOInterface Inlines //////////////////////////////////////////////////////////////////////


	// --------------------------------------------------------------------------						
	// Function:	Read
	// Description:	undefined behaviour to catch errors
	// Arguments:	object
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class T>  IOInterface& IOInterface::Read(T& object)
	{
		Exception::Throw("Attempting to read from istream when not implemented");
		return *this;
	}


	// --------------------------------------------------------------------------						
	// Function:	Write
	// Description:	undefined behaviour to catch errors
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class T>  IOInterface& IOInterface::Write(const T& object)
	{
		Exception::Throw("Attempting to write to an ostream when not implemented");
		return *this;
	};


	// --------------------------------------------------------------------------						
	// Function:	IOInterface
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class T> IOInterface& IOInterface::ReadClass(T& object)
	{
		ReadClassInternal(object);
		return *this;
	}


	// --------------------------------------------------------------------------						
	// Function:	WriteClass
	// Description:	writes class object
	// Arguments:	object
	// Returns:		io steam interface
	// --------------------------------------------------------------------------
	template<class T> IOInterface& IOInterface::WriteClass(const T& object)
	{
		WriteClassInternal(object);
		return *this;
	}


	// --------------------------------------------------------------------------						
	// Function:	IsClassPtr
	// Description:	tests if a class pointer
	// Arguments:	pointer
	// Returns:		true is is
	// --------------------------------------------------------------------------
	template<class T> static bool IOInterface::IsClassPtr(T*) { return true; }


	// --------------------------------------------------------------------------						
	// Function:	IsClassPtr
	// Description:	tests if a class pointer
	// Arguments:	pointer
	// Returns:		true is is
	// --------------------------------------------------------------------------
	template<class T> static bool IOInterface::IsClassPtr(T) { return false; }


	// --------------------------------------------------------------------------						
	// Function:	ClassType
	// Description:	returns null object of class type
	// Arguments:	none
	// Returns:		object
	// --------------------------------------------------------------------------
	template<class T> static T& IOInterface::ClassType(T*) { return *(T*)NULL; }


	// --------------------------------------------------------------------------						
	// Function:	ClassType
	// Description:	returns null object of class type
	// Arguments:	none
	// Returns:		object
	// --------------------------------------------------------------------------
	template<class T> static T& IOInterface::ClassType(T) { return *(T*)NULL; }


	// --------------------------------------------------------------------------						
	// Function:	WriteClassInternal
	// Description:	writes class object
	// Arguments:	object
	// Returns:		io steam interface
	// --------------------------------------------------------------------------
	template<class T> void IOInterface::WriteClassInternal(T* object) { object->Write(*this, myVersion); }


	// --------------------------------------------------------------------------						
	// Function:	WriteClassInternal
	// Description:	writes class object
	// Arguments:	object
	// Returns:		io steam interface
	// --------------------------------------------------------------------------
	template<class T> void IOInterface::WriteClassInternal(T& object) { object.Write(*this, myVersion); }


	// --------------------------------------------------------------------------						
	// Function:	ReadClassInternal
	// Description:	read class object
	// Arguments:	object
	// Returns:		io steam interface
	// --------------------------------------------------------------------------
	template<class T> void IOInterface::ReadClassInternal(T* object) { object->Read(*this, myVersion); }


	// --------------------------------------------------------------------------						
	// Function:	ReadClassInternal
	// Description:	read class object
	// Arguments:	object
	// Returns:		io steam interface
	// --------------------------------------------------------------------------
	template<class T> void IOInterface::ReadClassInternal(T& object) { object.Read(*this, myVersion); }


	// General Sream Defintions ///////////////////////////////////////////////////////////////////


	// --------------------------------------------------------------------------						
	// Function:	>>
	// Description:	overloaded stream in
	// Arguments:	io stream insterface, object
	// Returns:		io interface
	// --------------------------------------------------------------------------
	IOInterface& operator >>(IOInterface& io, std::stringstream& object);
	IOInterface& operator >>(IOInterface& io, std::string& object);
	IOInterface& operator >>(IOInterface& io, std::string& object);
	IOInterface& operator >>(IOInterface& io, char* object);	// undefined no read in char operator
	IOInterface& operator >>(IOInterface& io, char& object);
	IOInterface& operator >>(IOInterface& io, unsigned char& object);
	IOInterface& operator >>(IOInterface& io, short& object);
	IOInterface& operator >>(IOInterface& io, unsigned short& object);
	IOInterface& operator >>(IOInterface& io, int& object);
	IOInterface& operator >>(IOInterface& io, unsigned int& object);
	IOInterface& operator >>(IOInterface& io, long int& object);
	IOInterface& operator >>(IOInterface& io, unsigned long int& object);
	IOInterface& operator >>(IOInterface& io, __int64& object);
	IOInterface& operator >>(IOInterface& io, float& object);
	IOInterface& operator >>(IOInterface& io, double& object);
	IOInterface& operator >>(IOInterface& io, bool& object);


	// Declared but not defined to catch reading and writing of pointers
	// to non-persistent objects. The program won't link.
	IOInterface& operator >>(IOInterface& io, void*& object);

	template<class T>
	IOInterface& operator >>(IOInterface& io, std::vector<T>& object)
	{
		if (io.myArchiveTags)
		{
			std::string c;
			io.InitializeStreamIn(c);
			DEBUG_ASSERT(c == "vector");
		}
		object.clear();
		unsigned int siz;
		io >> siz;
		object.resize(siz);
		for (unsigned int i = 0; i < siz; ++i)
		{
			io >> object[i];
		}
		FINALIZE_SERIAL(io);
		return io;
	}


	template<class Key, class Value>
	IOInterface& operator >>(IOInterface& io, std::map<Key, Value>& object)
	{
		if (io.myArchiveTags)
		{
			std::string c;
			io.InitializeStreamIn(c);
			DEBUG_ASSERT(c == "map");
		}

		object.clear();
		unsigned int siz;
		io >> siz;
		for (unsigned int i = 0; i < siz; ++i)
		{
			Key a;
			io >> a;
			object[a] = Value();
			io >> object[a];
		}
		FINALIZE_SERIAL(io);
		return io;
	}




	template<class T, class A>
	IOInterface& operator >>(IOInterface& io, std::list<T, A>& object)
	{
		if (io.myArchiveTags)
		{
			std::string c;
			io.InitializeStreamIn(c);
			DEBUG_ASSERT(c == "list");
		}
		object.clear();
		unsigned int siz;
		io >> siz;
		T temp;
		for (unsigned int i = 0; i < siz; ++i)
		{
			io >> temp;
			object.push_back(temp);
		}
		FINALIZE_SERIAL(io);
		return io;
	}



	template<class A, class B>
	IOInterface& operator >>(IOInterface& io, std::pair<A, B>& object)
	{
		return (io >> object.first >> object.second);
	}



	template<class Value>
	IOInterface& operator >>(IOInterface& io, std::set<Value>& sett)
	{
		if (io.myArchiveTags)
		{
			std::string c;
			io.InitializeStreamIn(c);
			DEBUG_ASSERT(c == "set");
		}
		unsigned int size;
		io >> size;
		while (size--)
		{
			Value thing;
			io >> thing;
			sett.insert(thing);
		}
		FINALIZE_SERIAL(io);
		return io;
	}



	template<class Key, class Pred>
	IOInterface& operator >>(IOInterface& io, std::multiset<Key, Pred>& sett)
	{
		if (io.myArchiveTags)
		{
			std::string c;
			io.InitializeStreamIn(c);
			DEBUG_ASSERT(c == "multiset");
		}
		unsigned int size;
		io >> size;
		while (size--)
		{
			Key key;
			io >> key;
			sett.insert(key);
		}
		FINALIZE_SERIAL(io);
		return io;
	}


	template<class C>
	IOInterface& operator >>(IOInterface& io, std::deque<C>& value)
	{
		if (io.myArchiveTags)
		{
			std::string c;
			io.InitializeStreamIn(c);
			DEBUG_ASSERT(c == "deque");
		}
		unsigned int n;
		io >> n;
		for (int i = 0; i < n; i++) {
			value.push_back(C());
			io >> value.back();
		}
		FINALIZE_SERIAL(io);
		return io;
	}

	// general template handlers conflict with ::std template handlers
	// dont know best conversion
	template< class T >
	IOInterface& operator >>(IOInterface& io, T& value)
	{
		T::ReadArchive(io, (void*)&value);
		return io;
	};


	// --------------------------------------------------------------------------						
	// Function:	<<
	// Description:	overloaded stream out
	// Arguments:	io stream insterface, object
	// Returns:		io interface
	// --------------------------------------------------------------------------
	IOInterface& operator <<(IOInterface& io, const std::stringstream& object);
	IOInterface& operator <<(IOInterface& io, const std::string& object);
	IOInterface& operator <<(IOInterface& io, const std::string& object);
	IOInterface& operator <<(IOInterface& io, const char* object);	// does not write out null terminator hence no read in
	IOInterface& operator <<(IOInterface& io, const char object);
	IOInterface& operator <<(IOInterface& io, const unsigned char object);
	IOInterface& operator <<(IOInterface& io, const short object);
	IOInterface& operator <<(IOInterface& io, const unsigned short object);
	IOInterface& operator <<(IOInterface& io, const int object);
	IOInterface& operator <<(IOInterface& io, const unsigned int object);
	IOInterface& operator <<(IOInterface& io, const long int object);
	IOInterface& operator <<(IOInterface& io, const unsigned long int object);
	IOInterface& operator <<(IOInterface& io, const __int64 object);
	IOInterface& operator <<(IOInterface& io, const float object);
	IOInterface& operator <<(IOInterface& io, const double object);
	IOInterface& operator <<(IOInterface& io, const bool object);

	// Declared but not defined to catch reading and writing of pointers
	// to non-persistent objects. The program won't link.
	IOInterface& operator <<(IOInterface& io, const void* object);


	template<class T>
	IOInterface& operator <<(IOInterface& io, std::vector<T> const& object)
	{
		if (io.myArchiveTags)
			io.InitializeStreamOut("vector");
		io.SetVarname("size");
		io << object.size();
		io.SetVarname("value");
		for (unsigned int i = 0; i < object.size(); ++i)
			io << object[i];
		FINALIZE_SERIAL(io);
		return io;
	}

	template<class Key, class Value>
	IOInterface& operator <<(IOInterface& io, std::map<Key, Value> const& object)
	{
		if (io.myArchiveTags)
			io.InitializeStreamOut("map");
		io.SetVarname("size");
		io << object.size();
		for (typename std::map<Key, Value>::const_iterator it = object.begin(); it != object.end(); ++it)
		{
			io.SetVarname("key");
			io << it->first;
			io.SetVarname("value");
			io << it->second;
		}
		FINALIZE_SERIAL(io);
		return io;
	}
	template<class T, class A>
	IOInterface& operator <<(IOInterface& io, std::list<T, A> const& object)
	{
		if (io.myArchiveTags)
			io.InitializeStreamOut("list");
		io.SetVarname("size");
		io << object.size();
		io.SetVarname("value");
		for (typename std::list<T, A>::const_iterator it = object.begin(); it != object.end(); ++it)
		{
			io << *it;
		}
		FINALIZE_SERIAL(io);
		return io;
	}

	template<class A, class B>
	IOInterface& operator <<(IOInterface& io, std::pair<A, B> const& object)
	{
		io.SetVarname("pair");
		return (io << object.first << object.second);
	}

	template<class Value>
	IOInterface& operator <<(IOInterface& io, std::set<Value> const& sett)
	{
		if (io.myArchiveTags)
			io.InitializeStreamOut("set");
		typename std::set<Value>::const_iterator it;
		io.SetVarname("size");
		io << (unsigned int)(sett.size());
		io.SetVarname("value");
		for (it = sett.begin(); it != sett.end(); ++it)
			io << *it;
		FINALIZE_SERIAL(io);
		return io;
	}

	template<class Key, class Pred>
	IOInterface& operator <<(IOInterface& io, std::multiset<Key, Pred> const& sett)
	{
		if (io.myArchiveTags)
			io.InitializeStreamOut("multiset");

		typename std::multiset<Key, Pred>::const_iterator it;
		io.SetVarname("size");
		io << (unsigned int)(sett.size());
		io.SetVarname("value");
		for (it = sett.begin(); it != sett.end(); ++it)
			io << *it;
		FINALIZE_SERIAL(io);
		return io;
	}

	template<class C>
	IOInterface& operator <<(IOInterface& io, std::deque<C> const& value)
	{
		if (io.myArchiveTags)
			io.InitializeStreamOut("deque");

		io.SetVarname("size");
		io << (unsigned int)(value.size());
		io.SetVarname("value");
		for (typename std::deque<C>::const_iterator i = value.begin(); i != value.end(); ++i) {
			io << *i;
		}
		FINALIZE_SERIAL(io);
		return io;
	}

	template< class T >
	IOInterface& operator <<(IOInterface& io, const T& value)
	{
		T::WriteArchive(io, (void*)&value);
		return io;
	};



} // namespace shh


#endif // IO_INTERFACE_H
