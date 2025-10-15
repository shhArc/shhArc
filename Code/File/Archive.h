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

#ifndef ARCHIVE_H
#define ARCHIVE_H

#include "../Common/SecureStl.h"
#include "../Common/Debug.h"
#include "../Common/Exception.h"
#include "../Config/GCPtr.h"
#include "IOInterface.h"
#include <string>
#include <map>

namespace shh
{

	class Archive : public IOInterface
	{
	public:

		typedef void* (*InFunc)(IOInterface&, void*);
		typedef void (*OutFunc)(IOInterface&, const void*);
		typedef void* (*ConstructFunc)(IOInterface&);

		Archive(bool useTags, unsigned int version);
		Archive(IOInterface& io, bool useTags,unsigned  int version);
		~Archive() { Close(); }
		
		virtual void Open(const std::string& filename, IOInterface::Flags flags);
		virtual void Close();
		virtual int Peek();
		virtual bool Eof();
		virtual std::streamoff GetPos();
		virtual void SeekOff(std::streamoff pos, seekdir way);




		ConstructionList Read();
		template<class T> void Write(const T& object);

		typedef std::map<std::string, InFunc> InFuncs;
		typedef std::map<std::string, OutFunc> OutFuncs;
		typedef std::map<std::string, ConstructFunc> ConstructFuncs;

		static InFuncs& GetInFuncs();
		static OutFuncs& GetOutFuncs();
		static ConstructFuncs& GetConstructFuncs();
		static InFunc& GetInFunc(const std::string className);
		static OutFunc& GetOutFunc(const std::string className);
		static ConstructFunc& GetConstructFunc(const std::string className);

		template<class T> static std::string RegisterClassWithArchive(std::string classname, T* dummy = NULL);
		template<class T> static bool RegisterConstruct(std::string& classname, ConstructFunc func);
		template<class T, class C> void LoadPtr(T& object, const C& c);
		template<class T> void SavePtr(const T& object);

		virtual ConstructionPair InstanticateClass();

		virtual IOInterface& Read(std::string& object) { *myIO >> object; return *this; }
		virtual IOInterface& Read(char& object) { *myIO >> object; return *this; }
		virtual IOInterface& Read(unsigned char& object) { *myIO >> object; return *this; }
		virtual IOInterface& Read(short& object) { *myIO >> object; return *this; };
		virtual IOInterface& Read(unsigned short& object) { *myIO >> object; return *this; }
		virtual IOInterface& Read(int& object) { *myIO >> object; return *this; }
		virtual IOInterface& Read(unsigned int& object) { *myIO >> object; return *this; }
		virtual IOInterface& Read(long int& object) { *myIO >> object; return *this; }
		virtual IOInterface& Read(unsigned long int& object) { *myIO >> object; return *this; }
		virtual IOInterface& Read(__int64& object) { *myIO >> object; return *this; }
		virtual IOInterface& Read(float& object) { *myIO >> object; return *this; }
		virtual IOInterface& Read(double& object) { *myIO >> object; return *this; }
		virtual IOInterface& Read(bool& object) { *myIO >> object; return *this; }

		virtual IOInterface& Write(const std::string& object) { myIO->SetVarname(myVarname); *myIO << object; return *this; }
		virtual IOInterface& Write(const char* object) { myIO->SetVarname(myVarname); *myIO << object; return *this; }
		virtual IOInterface& Write(const char object) { myIO->SetVarname(myVarname); *myIO << object; return *this; }
		virtual IOInterface& Write(const unsigned char object) { myIO->SetVarname(myVarname); *myIO << object; return *this; }
		virtual IOInterface& Write(const short object) { myIO->SetVarname(myVarname); *myIO << object; return *this; };
		virtual IOInterface& Write(const unsigned short object) { myIO->SetVarname(myVarname); *myIO << object; return *this; }
		virtual IOInterface& Write(const int object) { myIO->SetVarname(myVarname); *myIO << object; return *this; }
		virtual IOInterface& Write(const unsigned int object) { myIO->SetVarname(myVarname); *myIO << object; return *this; };
		virtual IOInterface& Write(const long int object) { myIO->SetVarname(myVarname); *myIO << object; return *this; }
		virtual IOInterface& Write(const unsigned long int object) { myIO->SetVarname(myVarname); *myIO << object; return *this; }
		virtual IOInterface& Write(const __int64 object) { myIO->SetVarname(myVarname); *myIO << object; return *this; }
		virtual IOInterface& Write(const float object) { myIO->SetVarname(myVarname); *myIO << object; return *this; }
		virtual IOInterface& Write(const double object) { myIO->SetVarname(myVarname); *myIO << object; return *this; }
		virtual IOInterface& Write(const bool object) { myIO->SetVarname(myVarname); *myIO << object; return *this; }

		typedef std::map<std::string, void*> NoCreateClass;
		NoCreateClass myNoCreateClasses;


		template<class T> static bool IsClassPtr(T*);
		template<class T> static bool IsClassPtr(T);
		template<class T> static T& ClassType(T*);
		template<class T> static T& ClassType(T);

		virtual void InitializeStreamIn(std::string& classname);
		virtual void InitializeStreamOut(const std::string& classname);
		virtual void FinalizeClassSerial();


		template<class T> IOInterface& operator >>(T& object);
		template<class T> IOInterface& operator <<(const T& object);

	private:

		typedef std::map<unsigned int, void*> InPtrs;
		typedef std::map<void*, unsigned int> OutPtrs;


		unsigned int myNextPtrId;
		InPtrs myInPtrs;
		OutPtrs myOutPtrs;

		IOInterface* myIO;

		bool myCreatedIO;

		template<class T> void SetAsPtr(T*& object, void* p);
		template<class T> void SetAsPtr(T& object, void* p);
		template<class T> void* GetAsPtr(T* object);
		template<class T> void* GetAsPtr(T& object);

		template<class T> void WritePtr(T* o);
		template<class T> void WritePtr(T o);
		template<class T> void ReadPtr(T* o);
		template<class T> void ReadPtr(T o);
		template<class T> void ConstructPtr(T*& o, ConstructFunc func);
		template<class T> void ConstructPtr(T& o, ConstructFunc func);

	};

	// Archive Inlines ///////////////////////////////////////////////////////////////////////

	// --------------------------------------------------------------------------						
	// Function:	Write
	// Description:	write object to archive
	// Arguments:	object
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class T> void Archive::Write(const T& object)
	{
		T::WriteArchive(*this, myVersion, &object);
	}


	// --------------------------------------------------------------------------						
	// Function:	RegisterClassWithArchive
	// Description:	register a class for IO
	// Arguments:	class name
	// Returns:		class name
	// --------------------------------------------------------------------------
	template<class T> static std::string Archive::RegisterClassWithArchive(std::string classname, T* dummy)
	{
		GetInFuncs()[classname] = &T::ReadArchive;
		GetOutFuncs()[classname] = &T::WriteArchive;
		GetConstructFuncs()[classname] = &T::ConstructArchive;
		return classname;
	}


	// --------------------------------------------------------------------------						
	// Function:	RegisterConstruct
	// Description:	register class for construction
	// Arguments:	class name, contructor func
	// Returns:		tru
	// --------------------------------------------------------------------------
	template<class T> static bool Archive::RegisterConstruct(std::string& classname, ConstructFunc func)
	{
		GetConstructFuncs()[classname] = func;
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	LoadPtr
	// Description:	loads a point from archive and sets it (create if needed)
	// Arguments:	object
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class T, class C> void Archive::LoadPtr(T& object, const C& c)
	{
		unsigned int id;
		*myIO >> id;
		InPtrs::iterator it = myInPtrs.find(id);
		if (it != myInPtrs.end())
		{
			SetAsPtr(object, it->second);
		}
		else
		{
			ConstructFuncs::iterator it = GetConstructFuncs().find(C::ourArchiveAlias);
			if (it == GetConstructFuncs().end())
				Exception::Throw("Failed to find archive constructor function %s", C::ourArchiveAlias);
			ConstructPtr(object, *it->second);
			myInPtrs[id] = GetAsPtr(object);
			ReadClass<T>(object);
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	SavePtr
	// Description:	saves a pointer to archive
	// Arguments:	object
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class T> void Archive::SavePtr(const T& object)
	{
		void* o = GetAsPtr(object);
		OutPtrs::iterator it = myOutPtrs.find(o);
		if (it != myOutPtrs.end())
		{
			myIO->SetVarname(myVarname); *myIO << it->second;
		}
		else
		{
			++myNextPtrId;
			myIO->SetVarname(myVarname);
			*myIO << myNextPtrId;
			myOutPtrs[o] = myNextPtrId;
			WritePtr(object);
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	IsClassPtr
	// Description:	tests if class pointer
	// Arguments:	object
	// Returns:		true
	// --------------------------------------------------------------------------
	template<class T> static bool Archive::IsClassPtr(T*) { return true; }


	// --------------------------------------------------------------------------						
	// Function:	IsClassPtr
	// Description:	tests if class pointer
	// Arguments:	object
	// Returns:		false
	// --------------------------------------------------------------------------
	template<class T> static bool Archive::IsClassPtr(T) { return false; }
	

	// --------------------------------------------------------------------------						
	// Function:	ClassType
	// Description: returns null object from object pointer type
	// Arguments:	object ptr
	// Returns:		null object
	// --------------------------------------------------------------------------
	template<class T> static T& Archive::ClassType(T*) { return *(T*)NULL; }
	

	// --------------------------------------------------------------------------						
	// Function:	ClassType
	// Description: returns null object from object pointer
	// Arguments:	object ptr
	// Returns:		null object
	// --------------------------------------------------------------------------
	template<class T> static T& Archive::ClassType(T) { return *(T*)NULL; }


	// --------------------------------------------------------------------------						
	// Function:	>>
	// Description:	reads object from archive
	// Arguments:	object
	// Returns:		IO Interface
	// --------------------------------------------------------------------------
	template<class T> IOInterface& Archive::operator >>(T& object)
	{
		if (Archive::IsClassPtr(object))
		{
			T *t = NULL;
			LoadPtr<T>(object, Archive::ClassType(*t));
		}
		else
		{
			ReadClass<T>(object);
		}
		return *this;
	}


	// --------------------------------------------------------------------------						
	// Function:	<<
	// Description:	write object to archive
	// Arguments:	object
	// Returns:		IO Interface
	// --------------------------------------------------------------------------
	template<class T> IOInterface& Archive::operator <<(const T& object)
	{
		if (Archive::IsClassPtr(object))
			SavePtr<T>(object);
		else
			WriteClass<T>(object);
		return *this;
	}


	// --------------------------------------------------------------------------						
	// Function:	SetAsPtr
	// Description:	copy object from pointer
	// Arguments:	destination object, pointer
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class T> void Archive::SetAsPtr(T*& object, void* p) { object = (T*)p; }


	// --------------------------------------------------------------------------						
	// Function:	SetAsPtr
	// Description:	copy object from pointer
	// Arguments:	destination object, pointer
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class T> void Archive::SetAsPtr(T& object, void* p) {}


	// --------------------------------------------------------------------------						
	// Function:	GetAsPtr
	// Description:	returns object pointer
	// Arguments:	object
	// Returns:		pointer
	// --------------------------------------------------------------------------
	template<class T> void* Archive::GetAsPtr(T* object) { return object; }


	// --------------------------------------------------------------------------						
	// Function:	GetAsPtr
	// Description:	returns object pointer
	// Arguments:	object
	// Returns:		pointer
	// --------------------------------------------------------------------------
	template<class T> void* Archive::GetAsPtr(T& object) { return NULL; }


	// --------------------------------------------------------------------------						
	// Function:	WritePtr
	// Description:	write object to archive
	// Arguments:	object
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class T> void Archive::WritePtr(T* o) { o->Write(*myIO, myVersion); };


	// --------------------------------------------------------------------------						
	// Function:	WritePtr
	// Description:	write object to archive
	// Arguments:	object
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class T> void Archive::WritePtr(T o) {};


	// --------------------------------------------------------------------------						
	// Function:	ReadsPtr
	// Description:	reads object from archive
	// Arguments:	object
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class T> void Archive::ReadPtr(T* o) { o->Read(*myIO, myVersion); };


	// --------------------------------------------------------------------------						
	// Function:	ReadsPtr
	// Description:	reads object from archive
	// Arguments:	object
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class T> void Archive::ReadPtr(T o) {};


	// --------------------------------------------------------------------------						
	// Function:	ConstructPtr
	// Description:	constructs object from archive read in
	// Arguments:	object, constructor func
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class T> void Archive::ConstructPtr(T*& o, ConstructFunc func) { o = (T*)func(*this); }


	// --------------------------------------------------------------------------						
	// Function:	ConstructPtr
	// Description:	constructs object from archive read in
	// Arguments:	object, constructor func
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class T> void Archive::ConstructPtr(T& o, ConstructFunc func) {}
}	

#endif