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

#ifndef SOFTTYPE_H
#define SOFTTYPE_H

namespace shh
{
	class IOInterface;
}

#include "../Common/SecureStl.h"
#include "../Common/Debug.h"
#include "../Common/Dictionary.h"
#include "../Config/GCPtr.h"
#include <vector>
#include <typeinfo>



namespace shh {



	class SoftType
	{
		typedef int (*GarbageChecker)();
		typedef std::vector<GarbageChecker> GarbageCheckers;

	public:

		
		typedef void (*GarbageFunction)(void *);
		typedef std::vector<GarbageFunction> GarbageFunctions;
	
		static inline bool IsUserType(void const* const);
		static inline bool IsUserType(bool const* const);
		static inline bool IsUserType(float const* const);
		static inline bool IsUserType(double const* const);
		static inline bool IsUserType(int const* const);
		static inline bool IsUserType(unsigned int const* const);
		static inline bool IsUserType(long const* const);
		static inline bool IsUserType(unsigned long const* const);
		static inline bool IsUserType(long long const* const);
		static inline bool IsUserType(unsigned long long const* const);
		static inline bool IsUserType(std::string const* const);
		static inline bool IsUserType(VariantKeyDictionary const* const);

		static inline bool IsString(std::string const* const);
		static inline bool IsString(void const* const);

		static inline bool IsDictionary(VariantKeyDictionary const* const);
		static inline bool IsDictionary(void const* const);

		static inline bool IsNumber(float const* const);
		static inline bool IsNumber(double const* const);
		static inline bool IsNumber(int const* const);
		static inline bool IsNumber(unsigned int const* const);
		static inline bool IsNumber(long const* const);
		static inline bool IsNumber(unsigned long const* const);
		static inline bool IsNumber(long long const* const);
		static inline bool IsNumber(unsigned long long const* const);
		static inline bool IsNumber(void const* const);

		static inline bool IsFloat(float const* const);
		static inline bool IsFloat(double const* const);
		static inline bool IsFloat(void const* const);

		static inline bool IsInteger(int const* const);
		static inline bool IsInteger(unsigned int const* const);
		static inline bool IsInteger(long const* const);
		static inline bool IsInteger(unsigned long const* const);
		static inline bool IsInteger(long long const* const);
		static inline bool IsInteger(unsigned long long const* const);
		static inline bool IsInteger(void const* const);

		static inline bool IsBoolean(bool const* const);
		static inline bool IsBoolean(void*);

		static inline bool IsGCPtr(void*);
		template<class T> static inline bool IsGCPtr(GCPtr<T>* o);

		static inline void NullGCPtr(void*);
		template<class T> static inline void NullGCPtr(GCPtr<T>* o);

		static inline const char* GetString(std::string const* const s);
		static inline const char* GetString(char const* const s);
		static inline const char* GetString(void const* const);

		static inline const VariantKeyDictionary* GetDictionary(VariantKeyDictionary const* const d);
		static inline const VariantKeyDictionary* GetDictionary(void const* const);

		static inline double GetNumber(void const* const);
		static inline double GetNumber(float const* const n);
		static inline double GetNumber(double const* const n);
		static inline int GetNumber(int const* const n);
		static inline unsigned int GetNumber(unsigned int const* const n);
		static inline long GetNumber(long const* const n);
		static inline unsigned long GetNumber(unsigned long const* const n);
		static inline long long GetNumber(long long const* const n);
		static inline unsigned long long GetNumber(unsigned long long const* const n);

		static inline double GetFloat(void const* const);
		static inline double GetFloat(float const* const n);
		static inline double GetFloat(double const* const n);

		static inline int GetInteger(void const* const);
		static inline int GetInteger(int const* const n);
		static inline unsigned int GetInteger(unsigned int const* const n);
		static inline long GetInteger(long const* const n);
		static inline unsigned long GetInteger(unsigned long const* const n);
		static inline long long GetInteger(long long const* const n);
		static inline unsigned long long GetInteger(unsigned long long const* const n);

		static inline bool GetBoolean(void const* const n);
		static inline bool GetBoolean(float const* const n);
		static inline bool GetBoolean(double const* const n);
		static inline bool GetBoolean(bool const* const n);

		static inline GCPtr<GCObject> GetGCPtr(void*);
		static inline GCPtr<GCObject> GetGCPtr(GCPtr<GCObject>* p);

		static inline int CheckGarbageCollected();

	protected:


		static inline void RegisterGarbageCheckFunction(GarbageChecker g);

	private:

		static GarbageCheckers ourGarbageCheckers;
	
	};


	// --------------------------------------------------------------------------						
	// Function:	IsUserType
	// Description:	best match override functions so that a pointer can be checked 
	//				to see if it is a user type 
	// Arguments:	pointer to object to be checked
	// Returns:		true if is a user type
	// --------------------------------------------------------------------------						
	inline bool SoftType::IsUserType(void const* const) { return true; };
	inline bool SoftType::IsUserType(bool const* const) { return false; };
	inline bool SoftType::IsUserType(float const* const) { return false; };
	inline bool SoftType::IsUserType(double const* const) { return false; };
	inline bool SoftType::IsUserType(int const* const) { return false; };
	inline bool SoftType::IsUserType(unsigned int const* const) { return false; };
	inline bool SoftType::IsUserType(long const* const) { return false; };
	inline bool SoftType::IsUserType(unsigned long const* const) { return false; };
	inline bool SoftType::IsUserType(long long const* const) { return false; };
	inline bool SoftType::IsUserType(unsigned long long const* const) { return false; };
	inline bool SoftType::IsUserType(std::string const* const) { return false; };
	inline bool SoftType::IsUserType(VariantKeyDictionary const* const) { return false; };


	// --------------------------------------------------------------------------						
	// Function:	IsString
	// Description:	best match override functions so that a pointer can be checked 
	//				to see if it is a user type of a std::string
	// Arguments:	pointer to object to be checked
	// Returns:		true if is a std::string
	// --------------------------------------------------------------------------						
	inline bool SoftType::IsString(std::string const* const) { return true; };
	inline bool SoftType::IsString(void const* const) { return false; };


	// --------------------------------------------------------------------------						
	// Function:	IsDictionary
	// Description:	best match override functions so that a pointer can be checked 
	//				to see if it is a user type of a std::string
	// Arguments:	pointer to object to be checked
	// Returns:		true if is a VariantKeyDictionary
	// --------------------------------------------------------------------------						
	inline bool SoftType::IsDictionary(VariantKeyDictionary const* const) { return true; };
	inline bool SoftType::IsDictionary(void const* const) { return false; };


	// --------------------------------------------------------------------------						
	// Function:	IsNumber
	// Description:	best match override functions so that a pointer can be checked 
	//				to see if it is a user type of a float, double, int or unsigned
	// Arguments:	pointer to object to be checked
	// Returns:		true if is a number
	// --------------------------------------------------------------------------						
	inline bool SoftType::IsNumber(float const* const) { return true; };
	inline bool SoftType::IsNumber(double const* const) { return true; };
	inline bool SoftType::IsNumber(int const* const) { return true; };
	inline bool SoftType::IsNumber(unsigned int const* const) { return true; };
	inline bool SoftType::IsNumber(long const* const) { return true; };
	inline bool SoftType::IsNumber(unsigned long const* const) { return true; };
	inline bool SoftType::IsNumber(long long const* const) { return true; };
	inline bool SoftType::IsNumber(unsigned long long const* const) { return true; };
	inline bool SoftType::IsNumber(void const* const) { return false; };


	// --------------------------------------------------------------------------						
	// Function:	IsFloat
	// Description:	best match override functions so that a pointer can be checked 
	//				to see if it is a user type of a float, double 
	// Arguments:	pointer to object to be checked
	// Returns:		true if is a float
	// --------------------------------------------------------------------------						
	inline bool SoftType::IsFloat(float const* const) { return true; };
	inline bool SoftType::IsFloat(double const* const) { return true; };
	inline bool SoftType::IsFloat(void const* const) { return false; };


	// --------------------------------------------------------------------------						
	// Function:	IsInteger
	// Description:	best match override functions so that a pointer can be checked 
	//				to see if it is a user type of a float, double or inint or 
	//				unsigned int
	// Arguments:	pointer to object to be checked
	// Returns:		true if is a integer
	// --------------------------------------------------------------------------						
	inline bool SoftType::IsInteger(int const* const) { return true; };
	inline bool SoftType::IsInteger(unsigned int const* const) { return true; };
	inline bool SoftType::IsInteger(long const* const) { return true; };
	inline bool SoftType::IsInteger(unsigned long const* const) { return true; };
	inline bool SoftType::IsInteger(long long const* const) { return true; };
	inline bool SoftType::IsInteger(unsigned long long const* const) { return true; };
	inline bool SoftType::IsInteger(void const* const) { return false; };


	// --------------------------------------------------------------------------						
	// Function:	IsBoolean
	// Description:	best match override functions so can tell if a pointer to and 
	//				object is to an object that is a boolean
	// Arguments:	pointer to object to be checked
	// Returns:		true if boolean
	// --------------------------------------------------------------------------						
	inline bool SoftType::IsBoolean(bool const* const) { return true; };
	inline bool SoftType::IsBoolean(void*) { return false; };


	// --------------------------------------------------------------------------						
	// Function:	IsGCPtr
	// Description:	best match override functions so can tell if a pointer to and 
	//				object is to an object that is a GCPtr
	// Arguments:	pointer to object to be checked
	// Returns:		true if GCPtr
	// --------------------------------------------------------------------------						
	inline bool SoftType::IsGCPtr(void*) { return false; };
	template<class T> inline bool SoftType::IsGCPtr(GCPtr<T>* o) { return true; }


	// --------------------------------------------------------------------------						
	// Function:	NullGCPtr
	// Description:	nulls a gcptr object if it is one
	// Arguments:	pointer to object to be hulled
	// Returns:		none
	// --------------------------------------------------------------------------						
	inline void SoftType::NullGCPtr(void*) {};
	template<class T> inline void SoftType::NullGCPtr(GCPtr<T>* o) { o->SetNull(); }


	// --------------------------------------------------------------------------						
	// Function:	GetString
	// Description:	best match override functions so that a pointer can be converted 
	//				to char * if it is a std::string or an empty string otherwise
	// Arguments:	pointer to object to be got
	// Returns:		char * of string
	// --------------------------------------------------------------------------						
	inline const char* SoftType::GetString(std::string const* const s) { return s->c_str(); };
	inline const char* SoftType::GetString(char const* const s) { return s; };
	inline const char* SoftType::GetString(void const* const)
	{
		const char* empty = "";
		return empty;
	};


	// --------------------------------------------------------------------------						
	// Function:	GetDictionary
	// Description:	
	// Arguments:	pointer to object to be got
	// Returns:		
	// --------------------------------------------------------------------------						
	inline const VariantKeyDictionary* SoftType::GetDictionary(VariantKeyDictionary const* const d) { return (VariantKeyDictionary*)d; };
	inline const VariantKeyDictionary* SoftType::GetDictionary(void const* const)
	{
		const VariantKeyDictionary* empty = NULL;
		return empty;
	};


	// --------------------------------------------------------------------------						
	// Function:	GetNumber
	// Description:	best match override functions so that a pointer can be converted 
	//				to double, float, integer or unsigned int, else 0
	//				double are accepted by lua
	// Arguments:	pointer to object to be got
	// Returns:		double accepted by lua
	// --------------------------------------------------------------------------						
	inline double SoftType::GetNumber(void const* const) { return 0; };
	inline double SoftType::GetNumber(float const* const n) { return *n; };
	inline double SoftType::GetNumber(double const* const n) { return *n; };
	inline int SoftType::GetNumber(int const* const n) { return *n; };
	inline unsigned int SoftType::GetNumber(unsigned int const* const n) { return *n; };
	inline long SoftType::GetNumber(long const* const n) { return *n; };
	inline unsigned long SoftType::GetNumber(unsigned long const* const n) { return *n; };
	inline long long SoftType::GetNumber(long long const* const n) { return *n; };
	inline unsigned long long SoftType::GetNumber(unsigned long long const* const n) { return *n; };


	// --------------------------------------------------------------------------						
	// Function:	GetFloat
	// Description:	best match override functions so that a pointer can be converted 
	//				to double, float, integer or unsigned int, else 0
	//				double are accepted by lua
	// Arguments:	pointer to object to be got
	// Returns:		double accepted by lua
	// --------------------------------------------------------------------------						
	inline double SoftType::GetFloat(void const* const) { return 0; };
	inline double SoftType::GetFloat(float const* const n) { return *n; };
	inline double SoftType::GetFloat(double const* const n) { return *n; };


	// --------------------------------------------------------------------------						
	// Function:	GetInteger
	// Description:	best match override functions so that a pointer can be converted 
	//				to double, float, integer or unsigned int, else 0
	//				double are accepted by lua
	// Arguments:	pointer to object to be got
	// Returns:		double accepted by lua
	// --------------------------------------------------------------------------						
	inline int SoftType::GetInteger(void const* const) { return 0; };
	inline int SoftType::GetInteger(int const* const n) { return *n; };
	inline unsigned int SoftType::GetInteger(unsigned int const* const n) { return *n; };
	inline long SoftType::GetInteger(long const* const n) { return *n; };
	inline unsigned long SoftType::GetInteger(unsigned long const* const n) { return *n; };
	inline long long SoftType::GetInteger(long long const* const n) { return *n; };
	inline unsigned long long SoftType::GetInteger(unsigned long long const* const n) { return *n; };


	// --------------------------------------------------------------------------						
	// Function:	GetBoolean
	// Description:	best match override functions so that a pointer can be converted 
	//				to bool if it is a bool, else false
	// Arguments:	pointer to object to be got
	// Returns:		bool value
	// --------------------------------------------------------------------------	
	inline bool SoftType::GetBoolean(void const* const n) { return false; };
	inline bool SoftType::GetBoolean(float const* const n) { return false; };
	inline bool SoftType::GetBoolean(double const* const n) { return false; };
	inline bool SoftType::GetBoolean(bool const* const n) { return *n; };


	// --------------------------------------------------------------------------						
	// Function:	GetGCPtr
	// Description:	best match override functions so that a pointer can be converted 
	//				to a GCPtr
	// Arguments:	pointer to object to be got
	// Returns:		GCPtr
	// --------------------------------------------------------------------------						
	inline GCPtr<GCObject> SoftType::GetGCPtr(void*) { return GCPtr<GCObject>(); };
	inline GCPtr<GCObject> SoftType::GetGCPtr(GCPtr<GCObject>* p) { return *p; };


	// --------------------------------------------------------------------------						
	// Function:	CheckGarbageCollected
	// Description:	returns the number of object in lua that have not been garbage 
	//				collected. Should return 0 on close of app else there is a
	//				problem.
	//				*** this should be called before exiting the app ***
	// Arguments:	None
	// Returns:		returns the number of object in lua that have not been garbage 
	//				collected.
	// --------------------------------------------------------------------------						
	inline int SoftType::CheckGarbageCollected()
	{
		int remaining = 0;
		for (int i = 0; i != ourGarbageCheckers.size(); i++)
		{
			GarbageChecker func = ourGarbageCheckers[i];
			int n = (*func)();
			if (n > 0)
				remaining += n;
		}
		return remaining;
	}


	// --------------------------------------------------------------------------						
	// Function:	RegisterGarbageCheckFunction
	// Description:	Called when a particualr data type is registerd. This registers
	//				with the base class a fucntion to check the garbage collection
	//				state of the registered data type
	// Arguments:	garbage checker function
	// Returns:		none
	// --------------------------------------------------------------------------						
	inline void SoftType::RegisterGarbageCheckFunction(GarbageChecker g) 
	{ 
		ourGarbageCheckers.push_back(g); 
	}


} // namespace shh

#endif 
