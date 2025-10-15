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

#ifndef LUATYPE_H
#define LUATYPE_H

#include "../Common/SecureStl.h"
#include "../Common/Debug.h"
#include "../VM/SoftType.h"
#include "../Arc/Registry.h"
#include "../Arc/Type.h"
#include "LuaWrapper.h"
#include "LuaHelperFunctions.h"


namespace shh {

	class LuaProcess;
	template <class T> class LuaType;

	void LuaThrowScriptError(const char* format, ...);



	// LuaTypeBase ////////////////////////////////////////////////////////////////////


	class LuaTypeBase : public SoftType
	{
	public:

		static inline const LuaTypeId GetArgumentType(lua_State* L, int arg);
		static inline const Registry::OverloadTable::SharedTypes& GetSharedTypes();
		static void SetCastableTypes();

	protected:
		static Registry::OverloadTable::SharedTypes ourSharedTypes;

		static bool GetTable(const std::string& tableName, TValue& table, GCPtr<LuaProcess>& LuaProcess);						
		static void PushString(lua_State* L, const std::string& value);					
		static lua_State* GetLuaState(bool throwError = true);

	private:

		static std::vector<std::string> ourExcludedNameSpaces;

	};



	// LuaType ////////////////////////////////////////////////////////////////////

	template <class T>
	class LuaType : public Type<T>, public LuaTypeBase
	{
	public:

		LuaType(bool allowInterVM);
		virtual void* Destroy(void* v);


		static inline int GetLuaId();
		static inline bool IsUserType();
		static inline bool IsString();
		static inline bool IsDictionary();
		static inline bool IsNumber();
		static inline bool IsFloat();
		static inline bool IsInteger();
		static inline bool IsBoolean();
		static inline bool IsGCPtr();
		static inline bool IsVector();
		static inline bool IsList();
		static inline bool IsMap();

		static inline bool GetVariable(const std::string& tableName, const std::string& s, T& ret, GCPtr<LuaProcess>& LuaProcess);
		static inline bool GetVariable(const TValue& table, const std::string& s, T& ret);
		static inline bool SetVariable(const std::string& tableName, const std::string& s, const T& val, GCPtr<LuaProcess>& LuaProcess, bool overWrite);
		static inline bool SetVariable(const TValue& table, const std::string& s, const T& val, bool overWrite);			
		static inline void ClearVariable(const std::string& tableName, const std::string& s, GCPtr<LuaProcess>& LuaProcess);
		static inline void ClearVariable(const TValue& table, const std::string& s);


		static void GetArgument(lua_State* L, int arg, T*& object, bool throwOnError = true);
		static bool IsArgumentOfType(lua_State* L, int arg);
		static void* AddUserData(lua_State* L, T* data);
		
		static bool Equals(void const* const o1, void const* const o2);
		static int CallObjectNew(lua_State* L);
		static int MetaFunc_BasicNew(lua_State* L);
		static int MetaFunc_GC(lua_State* L);
		static int MetaFunc_ADD(lua_State* L);
		static int MetaFunc_SUB(lua_State* L);
		static int MetaFunc_DIV(lua_State* L);
		static int MetaFunc_MUL(lua_State* L);

		static unsigned int Register(const std::string& name, StringFunction stringer, ValueFunction valuer, bool allowInterVM, bool regsiterConstructors = true);
		static void RegisterMathematical();

		static int GarbageCheck();

		static T* Push(lua_State* L, const T& value);						
		static T* PushNew(lua_State* L);
		static void ExternalPushUserData(void* v);
		static void ExternalPushBoolean(void* v);
		static void ExternalPushFloat(void* v);
		static void ExternalPushInteger(void* v);
		static void ExternalPushString(void* v);
		static void ExternalPushDictionary(void* v);

	private:


		static std::string ourScriptName;
		static int ourReferences;
		static inline bool GetVal(double const* const v, const TValue* var, T& ret);
		static inline bool GetVal(std::string const* const v, const TValue* var, T& ret);
		static inline bool GetVal(bool const* const v, const TValue* var, T& ret);
		static inline bool GetVal(void const* const v, const TValue* var, T& ret);

		static std::string DefaultStringer(void const* const data);
		static bool DefaultValuer(const std::string& format, void*& data, int& type);

	};



	template<class T> std::string LuaType<T>::ourScriptName;
	template<class T> int LuaType<T>::ourReferences = 0;
	


	// LuaTypeBase Inlines ////////////////////////////////////////////////

	// --------------------------------------------------------------------------						
	// Function:	GetArgument
	// Description:	gets the data type of the argument number asked for from stack
	// Arguments:	args number
	// Returns:		type
	// --------------------------------------------------------------------------
	inline const LuaTypeId LuaTypeBase::GetArgumentType(lua_State* L, int arg)
	{ 
		return LuaGetStackTypeId(L, arg); 
	}



	// --------------------------------------------------------------------------						
	// Function:	GetSharedTypes
	// Description:	gets number and integer subtype to allow float to alsi=o be 
	//				regognised aS doubleS and unsigned ints to alsobe recognised
	//				as ints
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	inline const Registry::OverloadTable::SharedTypes& LuaTypeBase::GetSharedTypes()
	{ 
		return ourSharedTypes; 
	}



	// LuaType Inlines ///////////////////////////////////////////////////////////


	// --------------------------------------------------------------------------						
	// Function:	LuaType
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------						
	template <class T> LuaType<T>::LuaType(bool allowInterVM) : Type<T>(LuaImp, allowInterVM)
	{}


	// --------------------------------------------------------------------------						
	// Function:	Destroy
	// Description:	destruction of objecy
	// Arguments:	pointer to object to be destroyed
	// Returns:		none
	// --------------------------------------------------------------------------						
	template <class T> void* LuaType<T>::Destroy(void* v)
	{
#ifdef _DEBUG			
		ourReferences++;
#endif
		return Type<T>::Destroy(v);
	}


	// --------------------------------------------------------------------------						
	// Function:	GetLuaId
	// Description:	gets type id as used in lua
	// Arguments:	none
	// Returns:		type id
	// --------------------------------------------------------------------------						
	template <class T> inline int LuaType<T>::GetLuaId()
	{
		if (IsDictionary())
			return LUA_TTABLE;
		else if (IsUserType())
			return -(int)Type<T>::GetTypeId();
		else if (IsString())
			return LUA_TSTRING;
		else if (IsNumber())
			return LUA_TNUMBER;
		else if (IsBoolean())
			return LUA_TBOOLEAN;
		else
			return 0;
	}


	// --------------------------------------------------------------------------						
	// Function:	IsUserType
	// Description:	tests if this class template is of this type
	// Arguments:	none
	// Returns:		true if is
	// --------------------------------------------------------------------------						
	template <class T> inline bool LuaType<T>::IsUserType()
	{
		T* t = NULL;
		return LuaTypeBase::IsUserType(t);
	}


	// --------------------------------------------------------------------------						
	// Function:	IsString
	// Description:	tests if this class template is of type string
	// Arguments:	none
	// Returns:		true if is a std::string
	// --------------------------------------------------------------------------						
	template <class T> inline bool LuaType<T>::IsString()
	{
		T* t = NULL;
		return LuaTypeBase::IsString(t);
	}


	// --------------------------------------------------------------------------						
	// Function:	IsDictionary
	// Description:	tests if this class template is of type Dictionary
	// Arguments:	none
	// Returns:		true if is a dictionary
	// --------------------------------------------------------------------------						
	template <class T> inline bool LuaType<T>::IsDictionary()
	{
		T* t = NULL;
		return SoftType::IsDictionary(t);
	}


	// --------------------------------------------------------------------------						
	// Function:	IsNumber
	// Description:	tests if this class template is of type number
	// Arguments:	none
	// Returns:		true if is a number
	// --------------------------------------------------------------------------						
	template <class T> inline bool LuaType<T>::IsNumber()
	{
		T* t = NULL;
		return SoftType::IsNumber(t);
	}


	// --------------------------------------------------------------------------						
	// Function:	IsFloat
	// Description:	tests if this class template is of type float
	// Arguments:	none
	// Returns:		true if is a float
	// --------------------------------------------------------------------------						
	template <class T> inline bool LuaType<T>::IsFloat()
	{
		T* t = NULL;
		return SoftType::IsFloat(t);
	}


	// --------------------------------------------------------------------------						
	// Function:	IsInteger
	// Description:	tests if this class template is of typeinteger
	// Arguments:	none
	// Returns:		true if is a integer
	// --------------------------------------------------------------------------						
	template <class T> inline bool LuaType<T>::IsInteger()
	{
		T* t = NULL;
		return SoftType::IsInteger(t);
	}


	// --------------------------------------------------------------------------						
	// Function:	IsBoolean
	// Description:	tests if this class template is of type boolean
	// Arguments:	none
	// Returns:		true if is a boolean
	// --------------------------------------------------------------------------						
	template <class T> inline bool LuaType<T>::IsBoolean()
	{
		T* t = NULL;
		return SoftType::IsBoolean(t);
	}


	// --------------------------------------------------------------------------						
	// Function:	IsGCPtr
	// Description:	tests if this class template is of type GCPtr
	// Arguments:	none
	// Returns:		true if GCPtr
	// --------------------------------------------------------------------------						
	template <class T> bool LuaType<T>::IsGCPtr()
	{
		T* t = NULL;
		return SoftType::IsGCPtr(t);
	}


	// --------------------------------------------------------------------------						
	// Function:	IsVector
	// Description:	tests if this class template is of type std::vector
	// Arguments:	none
	// Returns:		true if is a vector
	// --------------------------------------------------------------------------						
	template <class T> inline bool LuaType<T>::IsVector()
	{
		T* t = NULL;
		return SoftType::IsVector(t);
	}


	// --------------------------------------------------------------------------						
	// Function:	IsList
	// Description:	tests if this class template is of type std::list
	// Arguments:	none
	// Returns:		true if is a list
	// --------------------------------------------------------------------------						
	template <class T> inline bool LuaType<T>::IsList()
	{
		T* t = NULL;
		return SoftType::IsList(t);
	}


	// --------------------------------------------------------------------------						
	// Function:	IsMap
	// Description:	tests if this class template is of type std::map
	// Arguments:	none
	// Returns:		true if is a map
	// --------------------------------------------------------------------------						
	template <class T> inline bool LuaType<T>::IsMap()
	{
		T* t = NULL;
		return SoftType::IsMap(t);
	}


	// --------------------------------------------------------------------------						
	// Function:	GetVariable
	// Description:	gets a variable of type from a table in the given process
	// Arguments:	table name, variable name, return value of type, LuaProcess
	// Returns:		true if of type and returned
	// --------------------------------------------------------------------------						
	template <class T> inline bool LuaType<T>::GetVariable(const std::string& tableName, const std::string& s, T& ret, GCPtr<LuaProcess>& luaProcess)
	{
		TValue table;
		if (!GetTable(tableName, table, luaProcess))
			return false;

		return GetVariable(table, s, ret);
	}


	// --------------------------------------------------------------------------						
	// Function:	GetVariable
	// Description:	gets a variable of type from the table in the current luaProcess
	// Arguments:	table , variable name  return value of type
	// Returns:		true if of type and returned
	// --------------------------------------------------------------------------						
	template <class T> inline bool LuaType<T>::GetVariable(const TValue& table, const std::string& s, T& ret)
	{
		lua_State* L = GetLuaState();
		const TValue* var = LuaGetTableValue(LuaGetTable(&table), LuaNewString(L, s.c_str()));

		return GetVal(&ret, var, ret);
	}


	// --------------------------------------------------------------------------						
	// Function:	SetVariable
	// Description:	adds a variable of type in the table in the given LuaProcess
	// Arguments:	table name, variable name, return value of type, LuaProcess
	//				whether overwrite of var exists of same name
	// Returns:		true if adds false if already exists or fails
	// --------------------------------------------------------------------------						
	template <class T> inline bool LuaType<T>::SetVariable(const std::string& tableName, const std::string& s, const T& val, GCPtr<LuaProcess>& LuaProcess, bool overWrite)
	{
		TValue table;
		if (!GetTable(tableName, table, LuaProcess))
			return false;

		return SetVariable(table, s, val, overWrite);
	}


	// --------------------------------------------------------------------------						
	// Function:	SetVariable
	// Description:	adds a variable of type in the table in the current LuaProcess
	// Arguments:	table , variable name, return value of type, whether 
	//				overwrite of var exists of same name
	// Returns:		true if adds false if already exists or fails
	// --------------------------------------------------------------------------						
	template <class T> inline bool LuaType<T>::SetVariable(const TValue& table, const std::string& s, const T& val, bool overWrite)
	{
		lua_State* L = GetLuaState();
		const TValue* var = LuaGetTableValue(LuaGetTable(&table), LuaNewString(L, s.c_str()));
		if (!overWrite && LuaGetTypeId(var) != LUA_TNIL)
			return false;

		// set tabel got as globals
		LuaSetStackValue(L, 0, &table);
		LuaIncStack(L);

		// add variable value
		PushString(L, s);
		Push(L, val);
		lua_settable(L, LuaGetStackSize(L) - 2);
		lua_pop(L, 1);

		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	ClearVariable
	// Description:	clears a variable in the table in the given LuaProcess
	// Arguments:	table name, variable name, LuaProcess
	// Returns:		none
	// --------------------------------------------------------------------------						
	template <class T> inline void LuaType<T>::ClearVariable(const std::string& tableName, const std::string& s, GCPtr<LuaProcess>& LuaProcess)
	{
		TValue table;
		if (!GetTable(tableName, table, LuaProcess))
			return;

		ClearVariable(table, s);
		return;
	}


	// --------------------------------------------------------------------------						
	// Function:	ClearVariable
	// Description:	clears a variable of type in the table in the current LuaProcess
	// Arguments:	table , variable name, return value of type
	// Returns:		none
	// --------------------------------------------------------------------------						
	template <class T> inline void LuaType<T>::ClearVariable(const TValue& table, const std::string& s)
	{
		lua_State* L = GetLuaState();
		const TValue* var = LuaGetTableValue(LuaGetTable(&table), LuaNewString(L, s.c_str()));
		if (LuaGetTypeId(var) == LUA_TNIL)
			return;

		// set tabel got as globals
		*LuaGetStackSize(L) = table;
		LuaIncStack(L);

		// add variable value
		PushString(L, s);
		lua_pushnil(L);
		lua_settable(L, LuaGetStackSize(L) - 2);
		lua_pop(L, 1);

		return;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetArgument
	// Description:	gets the given stack argument of templated type from lua state
	// Arguments:	lua state, argument num to get, pointer to object to return
	//				throw error if cant get for any reason
	// Returns:		none
	// --------------------------------------------------------------------------						
	template <class T> void LuaType<T>::GetArgument(lua_State* L, int arg, T*& object, bool throwOnError)
	{

		if (Type<T>::GetTypeId() == 0)
			LuaThrowScriptError("Data type not registered with Process.");

		if (LuaGetStackSize(L) < arg && throwOnError)
			LuaThrowScriptError("Invalid argument %d.", arg);

		int type = LuaGetStackTypeId(L, arg);

		if (type != LuaType<T>::GetLuaId() && throwOnError)
			LuaThrowScriptError("Invalid argument %d.", arg);

		object = static_cast<T*>(LuaGetStackUserData(L, arg));
	}


	// --------------------------------------------------------------------------						
	// Function:	IsArgumentType
	// Description:	checks stack argument is of our type
	// Arguments:	lua state working with, argument num to get
	// Returns:		bool
	// --------------------------------------------------------------------------						
	template <class T> bool LuaType<T>::IsArgumentOfType(lua_State* L, int arg)
	{
		if (LuaGetStackSize(L) <= arg || LuaGetTypeId(LuaGetStackValue(L, arg)) != Type<T>::GetTypeId())
			return false;
		else
			return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	AddUserData
	// Description: adds a precreated user data to the lua state
	// Arguments:	LuaState, data
	// Returns:		pointer to lua userdata created
	// --------------------------------------------------------------------------						
	template <class T> void* LuaType<T>::AddUserData(lua_State* L, T* data)
	{
		void* p = lua_newuserdata(L, sizeof(LuaUserData));
		shh::LuaUserData* lud = static_cast<shh::LuaUserData*>(p);
		lud->myShhType = (void*)LuaUserData::ourMarker;
		lud->myData = data;
		lud->myTypeId = Type<T>::GetTypeId();
		lud->myType = Type<T>::GetStatic();
		int ok = luaL_getmetatable(L, Type<T>::GetTypeName().c_str());
		DEBUG_ASSERT(ok);
		lua_setmetatable(L, -2);

#ifdef _DEBUG			
		ourReferences++;
#endif

		return p;
	}


	// --------------------------------------------------------------------------						
	// Function:	Equals
	// Description:	compares two data types for dereferenced equivalence
	// Arguments:	object to compare, object to compare
	// Returns:		true if equal
	// --------------------------------------------------------------------------						
	template <class T> bool LuaType<T>::Equals(void const* const o1, void const* const o2)
	{
		// all registerd classes typed must have and assignment operator
		return *static_cast<T const* const>(o1) == *static_cast<T const* const>(o2);
	}


	// --------------------------------------------------------------------------						
	// Function:	CallObjectNew
	// Description: default func tp allow using using Object(42) to make new 
	//				objects in script
	// Arguments:	lua state
	// Returns:		1
	// --------------------------------------------------------------------------			
	template <class T> int LuaType<T>::CallObjectNew(lua_State* L)
	{
		lua_remove(L, 1);
		MetaFunc_BasicNew(L);
		return 1;
	}


	// --------------------------------------------------------------------------						
	// Function:	MetaFunc_BasicNew
	// Description: defualt func allows using T() to make new objects in script
	// Arguments:	lua state
	// Returns:		1
	// --------------------------------------------------------------------------						
	template <class T> int LuaType<T>::MetaFunc_BasicNew(lua_State* L)
	{
		T* data = new T();
		AddUserData(L, data);
		luaL_getmetatable(L, Type<T>::GetTypeName().c_str());
		lua_setmetatable(L, -2);
		return 1;
	}


	// --------------------------------------------------------------------------						
	// Function:	MetaFunc_GC
	// Description:	garbage collection function for templated data type,
	//				hidden from use by lua and template class.
	// Arguments:	lua state
	// Returns:		0
	// --------------------------------------------------------------------------						
	template <class T> int LuaType<T>::MetaFunc_GC(lua_State* L)
	{
		void* data = lua_touserdata(L, 1);
		LuaUserData* lud = static_cast<shh::LuaUserData*>(data);
		T* p = reinterpret_cast<T*>(lud->myData);
		delete p;
#ifdef _DEBUG
		ourReferences--;
#endif		
		return 0;
	}


	// --------------------------------------------------------------------------						
	// Function:	MetaFunc_ADD
	// Description:	generic handler for add operations. 
	// Arguments:	lua state
	// Returns:		1
	// --------------------------------------------------------------------------						
	template <class T> int LuaType<T>::MetaFunc_ADD(lua_State* L)
	{
		if(GetArgument(L, 0) != GetArgument(L, 1))
			LuaProcess::LuaThrowScriptError("Invalid arguments.");

		T* o1 = NULL;
		T* o2 = NULL;
		if (!GetArgument(L, 0, o1))
			LuaProcess::LuaThrowScriptError("Invalid argument 1.");

		if (!GetArgument(L, 1, o2))
			LuaProcess::LuaThrowScriptError("Invalid argument 2.");

		T* result = PushNew(L);
		result = *o1 + *o2;
		return 1;

	}


	// --------------------------------------------------------------------------						
	// Function:	MetaFunc_SUB
	// Description:	generic handler for subtract operations.
	// Arguments:	lua state
	// Returns:		1
	// --------------------------------------------------------------------------						
	template <class T> int LuaType<T>::MetaFunc_SUB(lua_State* L)
	{
		if(GetArgument(L, 0) != GetArgument(L, 1))
			LuaProcess::LuaThrowScriptError("Invalid arguments.");

		T* o1 = NULL;
		T* o2 = NULL;
		if (!GetArgument(L, 0, o1))
			LuaProcess::LuaThrowScriptError("Invalid argument 1.");

		if (!GetArgument(L, 1, o2))
			LuaProcess::LuaThrowScriptError("Invalid argument 2.");

		T* result = PushNew(L);
		result = *o1 - *o2;

		return 1;

	}


	// --------------------------------------------------------------------------						
	// Function:	MetaFunc_DIV
	// Description:	generic handler for divide operations.
	// Arguments:	lua state
	// Returns:		1
	// --------------------------------------------------------------------------						
	template <class T> int LuaType<T>::MetaFunc_DIV(lua_State* L)
	{
		if(GetArgument(L, 0) != GetArgument(L, 1))
			LuaProcess::LuaThrowScriptError("Invalid arguments.");

		T* o1 = NULL;
		T* o2 = NULL;
		if (!GetArgument(L, 0, o1))
			LuaProcess::LuaThrowScriptError("Invalid argument 1.");

		if (!GetArgument(L, 1, o2))
			LuaProcess::LuaThrowScriptError("Invalid argument 2.");

		T* result = PushNew(L);
		result = *o1 / *o2;
		return 1;

	}


	// --------------------------------------------------------------------------						
	// Function:	MetaFunc_MUL
	// Description:	generic Handler for multiply operations. 
	// Arguments:	lua state
	// Returns:		1
	// --------------------------------------------------------------------------						
	template <class T> int LuaType<T>::MetaFunc_MUL(lua_State* L)
	{
		if(GetArgument(L, 0) != GetArgument(L, 1))
			LuaProcess::LuaThrowScriptError("Invalid arguments.");

		T* o1 = NULL;
		T* o2 = NULL;
		if (!GetArgument(L, 0, o1))
			LuaProcess::LuaThrowScriptError("Invalid argument 1.");

		if (!GetArgument(L, 1, o2))
			LuaProcess::LuaThrowScriptError("Invalid argument 2.");

		T* result = PushNew(L);
		result = *o1 * *o2;
		return 1;

	}


	// --------------------------------------------------------------------------						
	// Function:	Register
	// Description:	registers templated data type with the current LuaProcess, 
	//				assures consistancy accross all LuaProcesss
	// Arguments:	name as appears in lua script
	//				funciton used to convert type to a string
	//				function used to convert type to a map of values
	//				whether to allow this type to be passed in inter vm messaging
	//				whether to register default contructor funcs
	// Returns:		type id
	// --------------------------------------------------------------------------						
	template <class T> unsigned int LuaType<T>::Register(const std::string& name, StringFunction stringer, ValueFunction valuer, bool allowInterVM, bool regsiterConstructors)
	{
		lua_State* L = GetLuaState();

		// sets type id
		bool newRegistryType = Type<T>::SetType();
		if (IsFloat() || IsInteger())
			Type<T>::SetSubType();


		const BaseType* type = NULL;
		if (newRegistryType)
		{
			// this type hasnt been registered before so add necisarry set up
			Type<T> exactType(LuaImp, allowInterVM);
			type = &exactType;

			Type<T>::ourImplementationIds[Lua] = GetLuaId();

			if (IsUserType())
			{
				Type<T>::ourPushImplementations[Lua] = ExternalPushUserData;
				Type<T>::ourIntegralType = false;
			}
			else if (IsDictionary())
			{
				Type<T>::ourPushImplementations[Lua] = ExternalPushDictionary;
			}
			else if (IsBoolean())
			{
				Type<T>::ourPushImplementations[Lua] = ExternalPushBoolean;
			}
			else if (IsFloat())
			{
				Type<T>::ourPushImplementations[Lua] = ExternalPushFloat;
			}
			else if (IsInteger())
			{
				Type<T>::ourPushImplementations[Lua] = ExternalPushInteger;
			}
			else if (IsString())
			{
				Type<T>::ourPushImplementations[Lua] = ExternalPushString;
			}
			

			Registry::GetRegistry().RegisterType(type, abs(GetLuaId()), Lua);

			unsigned int typeId = type->GetId();

			if (!IsDictionary())
				Registry::GetRegistry().RegisterFunctions(typeId, Equals, (stringer ? stringer : DefaultStringer), (valuer ? valuer : DefaultValuer));

			if (!IsDictionary() && IsUserType())
				RegisterGarbageCheckFunction(GarbageCheck);


		}
		else
		{
			type = Registry::GetRegistry().GetType(Type<T>::GetTypeId());
		}

		if (IsUserType())
		{
			// add userdata type hooks for GC and in script function calls
			ourScriptName = name;

			luaL_newmetatable(L, type->GetName().c_str());


			lua_pushcfunction(L, MetaFunc_GC);
			lua_setfield(L, -2, "__gc");
			lua_pushvalue(L, -1);
			lua_setfield(L, -2, "__index");

			if (regsiterConstructors)
			{
				lua_pushcfunction(L, MetaFunc_BasicNew);
				lua_setfield(L, -2, "new");

				// Set a metatable for the metatable
				// This allows using Object(42) to make new objects
				lua_newtable(L);
				lua_pushcfunction(L, CallObjectNew);
				lua_setfield(L, -2, "__call");
				lua_setmetatable(L, -2);

				lua_register(L, ourScriptName.c_str(), MetaFunc_BasicNew);
			}

			LuaDecStack(L);

		}


		return Type<T>::GetTypeId();
	}

	
	// --------------------------------------------------------------------------						
	// Function:	RegisterMathematical
	// Description:	registers templated data type generic add, subtract multiply, 
	//				divide Handlers with the current LuaProcess
	//				(only deal with two object of same type) 
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------						
	template <class T> void LuaType<T>::RegisterMathematical()
	{
		lua_State* L = GetLuaState();

		if (luaL_newmetatable(GetLuaState(), Type<T>::GetTypeName()) != 0)

			lua_pushcfunction(L, &MetaFunc_ADD);
		lua_setfield(L, -2, "__add");
		lua_pushcfunction(L, &MetaFunc_SUB);
		lua_setfield(L, -2, "__sub");
		lua_pushcfunction(L, &MetaFunc_MUL);
		lua_setfield(L, -2, "__mul");
		lua_pushcfunction(L, &MetaFunc_DIV);
		lua_setfield(L, -2, "__div");

		LuaDecStack(L);

	}


	// --------------------------------------------------------------------------						
	// Function:	GarbageCheck
	// Description:	returns the number of object of given user type not
	//				garbage collected
	// Arguments:	none
	// Returns:		no not collected
	// --------------------------------------------------------------------------						
	template <class T> int LuaType<T>::GarbageCheck()
	{
		DEBUG_ASSERT(ourReferences < 1);	// deleted to few
		DEBUG_ASSERT(ourReferences > -1);	// deleted to many ??
		return ourReferences;
	}


	// --------------------------------------------------------------------------						
	// Function:	Push
	// Description:	pushes value of templated type onto lua state, 
	// Arguments:	lua sate, ptrs all data types
	// Returns:		true can be done, false if not type accepted by lua or not
	//				registered user type
	// --------------------------------------------------------------------------						
	template <class T> T* LuaType<T>::Push(lua_State* L, const T& value)
	{
		if (IsBoolean())
			lua_pushboolean(L, GetBoolean(&value));
		else if (IsFloat())
			lua_pushnumber(L, GetFloat(&value));
		else if (IsInteger())
			lua_pushinteger(L, GetInteger(&value));
		else if (IsString())
			lua_pushstring(L, GetString(&value));
		else if (IsDictionary())
			LuaHelperFunctions::PushDictionary(L, GetDictionary(&value));
		else if (IsUserType())
		{
			if (Type<T>::GetTypeId() >= Registry::BASE_TYPEID)
			{
				T *newData = PushNew(L);
				*newData = *(T*)&value;
				return newData;
			}
			RELEASE_ASSERT(false);	// returning non script registered type

		}
		else
			RELEASE_ASSERT(false);

		return NULL;
	}


	// --------------------------------------------------------------------------						
	// Function:	PushNew
	// Description:	create new data of this type and pushes data of templated type
	//				on to lua state
	// Arguments:	lua state
	// Returns:		pointer to new object
	// --------------------------------------------------------------------------						
	template <class T> T* LuaType<T>::PushNew(lua_State* L)
	{
		T* newData = new T();
		if (LuaType<T>::IsUserType())
			AddUserData(L, newData);
		return newData;
	}


	// --------------------------------------------------------------------------						
	// Function:	ExternalPush
	// Description:	pushes data of templated type on to LuaProcess. 
	// Arguments:	pointer to object to be pushed 
	// Returns:		none
	// --------------------------------------------------------------------------						
	template <class T> void LuaType<T>::ExternalPushUserData(void* v)
	{
		lua_State* L =LuaProcess::GetCurrentLuaState();
		T* value = static_cast<T*>(v);
		LuaType<T>::AddUserData(L, value);
		return;
	}
	template <class T> void LuaType<T>::ExternalPushBoolean(void* v)
	{
		lua_State* L =LuaProcess::GetCurrentLuaState();
		T* value = static_cast<T*>(v);
		bool* n = static_cast<bool*>(v);
		lua_pushboolean(L, *n);
		return;
	}
	template <class T> void LuaType<T>::ExternalPushFloat(void* v)
	{
		lua_State* L =LuaProcess::GetCurrentLuaState();
		T* value = static_cast<T*>(v);
		double* n = static_cast<double*>(v);
		lua_pushnumber(L, *n);
		return;
	}
	template <class T> void LuaType<T>::ExternalPushInteger(void* v)
	{
		lua_State* L =LuaProcess::GetCurrentLuaState();
		T* value = static_cast<T*>(v);
		long long* i = static_cast<long long*>(v);
		lua_pushinteger(L, *i);
		return;
	}
	template <class T> void LuaType<T>::ExternalPushString(void* v)
	{
		lua_State* L =LuaProcess::GetCurrentLuaState();
		T* value = static_cast<T*>(v);
		std::string* s = static_cast<std::string*>(v);
		lua_pushstring(L, GetString(s->c_str()));
		//TString* ts = LuaNewString(L, s->c_str());
		//LuaIncStack(L);
		//TValue* o = LuaGetStackValue(L, -1);
		//LuaSetString(o, ts);
		return;
	}
	template <class T> void LuaType<T>::ExternalPushDictionary(void* v)
	{
		lua_State* L = LuaProcess::GetCurrentLuaState();
		VariantKeyDictionary* value = static_cast<VariantKeyDictionary*>(v);
		LuaHelperFunctions::PushDictionary(L, value);
		return;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetVal
	// Description:	best match override functions so that a lua object can be
	//				converted to a value of type requested (if possible)
	// Arguments:	dummy data type ptr, tvalue of data, pointer to object got
	// Returns:		double accepted by lua
	// --------------------------------------------------------------------------						
	template <class T> inline bool LuaType<T>::GetVal(double const* const v, const TValue* var, T& ret)
	{
		// TODO: Fix this up to use proper boolean type when we have it

		if (LuaGetTypeId(var) != LUA_TNUMBER)
			return false;

		lua_State* L = GetLuaState();
		*LuaGetStackSize(L) = *var;
		LuaIncStack(L);
		ret = lua_tonumber(L, -1);
		lua_pop(L, 1);
		return true;
	}
	template <class T> inline bool LuaType<T>::GetVal(std::string const* const v, const TValue* var, T& ret)
	{
		if (LuaGetTypeId(var) != LUA_TSTRING)
			return false;

		lua_State* L = GetLuaState();
		*LuaGetStackSize(L) = *var;
		LuaIncStack(L);
		ret = lua_tostring(L, -1);
		lua_pop(L, 1);
		return true;
	}

	template <class T> inline bool LuaType<T>::GetVal(bool const* const v, const TValue* var, T& ret)
	{
		if (LuaGetTypeId(var) != LUA_TNUMBER)
			return false;

		lua_State* L = GetLuaState();
		*LuaGetStackSize(L) = *var;
		LuaIncStack(L);
		ret = lua_tonumber(L, -1) == 1.0 ? true : false;

		lua_pop(L, 1);
		return true;
	}

	template <class T> inline bool LuaType<T>::GetVal(void const* const v, const TValue* var, T& ret)
	{
		if (ttype(var) == LUA_TUSERDATA && LuaGetTypeId(var) == Type<T>::GetTypeId())
		{
			ret = *static_cast<T*>(uvalue(var)->uv);
			return true;
		}
		return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	DefaultStringer
	// Description:	unprintable type function
	// Arguments:	data to print
	// Returns:		string
	// --------------------------------------------------------------------------						
	template <class T> std::string LuaType<T>::DefaultStringer(void const* const data)
	{
		char temp[50];
		std::string format = Type<T>::GetTypeName();
		if (LuaType<bool>::GetTypeName() == format)
		{
			format += "(%b)";
			_snprintf(temp, 49, format.c_str(), *(bool*)data);
		}
		else if (LuaType<float>::GetTypeName() == format)
		{
			format += "(%f)";
			_snprintf(temp, 49, format.c_str(), *(float*)data);
		}
		else if (LuaType<double>::GetTypeName() == format)
		{
			format += "(%f)";
			_snprintf(temp, 49, format.c_str(), *(double*)data);
		}
		else if (LuaType<int>::GetTypeName() == format)
		{
			format += "(%d)";
			_snprintf(temp, 49, format.c_str(), *(int*)data);
		}
		else if (LuaType<unsigned int>::GetTypeName() == format)
		{
			format += "(%d)";
			_snprintf(temp, 49, format.c_str(), *(unsigned int*)data);
		}
		else if (LuaType<long>::GetTypeName() == format)
		{
			format += "(%d)";
			_snprintf(temp, 49, format.c_str(), *(long*)data);
		}
		else if (LuaType<unsigned long>::GetTypeName() == format)
		{
			format += "(%d)";
			_snprintf(temp, 49, format.c_str(), *(unsigned long*)data);
		}
		else if (LuaType<long long>::GetTypeName() == format)
		{
			format += "(%d)";
			_snprintf(temp, 49, format.c_str(), *(long long*)data);
		}
		else if (LuaType<unsigned long long>::GetTypeName() == format)
		{
			format += "(%d)";
			_snprintf(temp, 49, format.c_str(), *(unsigned long long*)data);
		}
		else if (LuaType<std::string>::GetTypeName() == format)
		{
			format += "(%s)";
			_snprintf(temp, 49, format.c_str(), *(std::string*)data);
		}
		else
		{
			_snprintf(temp, 49, "ERROR");
		}
		return temp;
	}

	// --------------------------------------------------------------------------						
	// Function:	DefaultValuer
	// Description:	unvaluable data type function, sets map entry for name to 0
	// Arguments:	data as string,  data to return, type id of returned data
	// Returns:		is successful
	// --------------------------------------------------------------------------	
	template <class T> bool  LuaType<T>::DefaultValuer(const std::string& format, void*& data, int& type)
	{
		lua_State* L =LuaProcess::GetCurrentLuaState();
		int start = (int)format.find("(") + 1;
		int end = (int)format.find(")") - 1;
		std::string name = format.substr(0, start - 1);
		std::string datastring = format.substr(start, end);
		if (name == LuaType<bool>::GetTypeName())
		{
			bool* v = new bool();
			*v = datastring == "true" ? true : false;
			data = v;
			type = Type<bool>::GetTypeId();
			return true;
		}
		if (name == LuaType<float>::GetTypeName())
		{
			float* v = new float(std::stof(datastring));
			data = v;
			type = Type<float>::GetTypeId();
			return true;
		}
		else if (name == LuaType<double>::GetTypeName())
		{
			double* v = new double(std::stod(datastring));
			data = v;
			type = Type<double>::GetTypeId();
			return true;
		}
		else if (name == LuaType <int>::GetTypeName())
		{
			int* v = new int(std::stoi(datastring));
			data = v;
			type = Type<int>::GetTypeId();
			return true;
		}
		else if (name == LuaType <unsigned int>::GetTypeName())
		{
			unsigned int* v = new unsigned int(std::stoul(datastring));
			data = v;
			type = Type<unsigned int>::GetTypeId();
			return true;
		}
		else if (name == LuaType <long>::GetTypeName())
		{
			long* v = new long(std::stoi(datastring));
			data = v;
			type = Type<long>::GetTypeId();
			return true;
		}
		else if (name == LuaType <unsigned long>::GetTypeName())
		{
			unsigned long* v = new unsigned long(std::stoul(datastring));
			data = v;
			type = Type<unsigned long>::GetTypeId();
			return true;
		}
		else if (name == LuaType <long long>::GetTypeName())
		{
			long long* v = new long long(std::stoi(datastring));
			data = v;
			type = Type<long long>::GetTypeId();
			return true;
		}
		else if (name == LuaType <unsigned long long>::GetTypeName())
		{
			unsigned long long* v = new unsigned long long(std::stoul(datastring));
			data = v;
			type = Type<unsigned long long>::GetTypeId();
			return true;
		}
		else if (name == LuaType <std::string>::GetTypeName())
		{
			std::string* v = new std::string(datastring);
			data = v;
			type = Type<std::string>::GetTypeId();
			return true;
		}
		data = NULL;
		type = 0;
		return false;
	}

}
#endif