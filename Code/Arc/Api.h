/////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////

#ifndef API_H
#define API_H


#include "../LuaProcess/LuaApi.h"
#include "../LuaProcess/LuaApiTemplate.h"
#include "../LuaProcess/LuaProcess.h"
#include "../LuaProcess/LuaType.h"
#include "../LuaProcess/LuaWrapper.h"
#include "../Arc/Type.inl"
#include "../Arc/God.h"
#include "../File/IOVariant.h"
#include "../VM/Scheduler.h"
#include "../Schema/Whole.h"
#include "../Schema/Agent.h"
#include "../Schema/Node.h"

#define GENERICPROCESSCREATE shh::Process::Create
#define LUAPROCESSCREATE shh::LuaProcess::Create
#define NODECREATE shh::Node::Create
#define AGENTCREATE shh::Agent::Create

namespace shh {


	class Api
	{
	public:

		static void CloseDown();
		static void FinalizeGodObjects();
		static void FinalizeWorldObjects(const std::string& name);
		static bool CreateWorld(const ::std::string& name, const StringKeyDictionary& config, const std::string& templateRealm);
		static void DestroyWorld(const std::string& name);
		static void DestroyWorlds();
		static void CreateGod(const std::string& name, const std::string& realmTemplate);
		static void DestroyGod();
		static inline const GCPtr<God>& GetGod();
		static inline void UpdateGod(double until);
		static GCPtr<Class> CreateClass(const std::string& name, const std::string& typeName, const GCPtr<Process>& process, Registry::ProcessConstructor pc, Registry::ObjectConstructor oc);
		static Message* NewMessage(const std::string& name, const GCPtr<Messenger>& to, bool deletable = true);
		template<typename T> static inline void AddMsgArg(Message* msg, T* arg);
		template<typename T> static inline T* GetMsgReturnVal(Message* msg, unsigned int retrunValNum, T* dummy = NULL);
		static inline bool SendMsg(Message* msg, double delay);
		static inline const GCPtr<Agent> GetAgent();
		static inline const GCPtr<Node> GetNode();

		static inline const GCPtr<Process> GetCurrentProcess();
		static inline Implementation GetImplementation();
		static inline Privileges GetPrivileges();
		static inline const GCPtr<Environment>& GetHomeEnvironment();
		static inline const GCPtr<Environment>& GetCurrentEnvironment();
		static inline void OpenNamespace(const std::string& name);
		static inline void CloseNamespace();
		template<typename F>
		static inline void RegisterFunction(const ::std::string& functionName, F *f, unsigned int firstReturnArg, const GCPtr<Module>& module);
		template< typename C, typename F >
		static inline void RegisterMemberFunction(const C* ownerExampe, const ::std::string& functionName, F *f, unsigned int firstReturnArg, const GCPtr<Module>& module);
		static inline void RegisterHardClass(const GCPtr<Class>& cls);

		static inline unsigned int CreateCollection(const GCPtr<Whole>& w, std::string& collectionName);
		static inline void DestroyCollection(const GCPtr<Whole>& w, std::string& collectionName);
		static inline void DestroyCollection(const GCPtr<Whole>& w, unsigned int& collectionId);
		static inline unsigned int AddPart(const GCPtr<Whole>& w, std::string& collectionName, std::string name, GCPtr<GCObject> object);
		static inline unsigned int AddPart(const GCPtr<Whole>& w, unsigned int collectionId, std::string name, GCPtr<GCObject> object);
		static inline bool GetPart(const GCPtr<Whole>& w, std::string& collectionName, unsigned int id, GCPtr<GCObject>& obj);
		static inline bool GetPart(const GCPtr<Whole>& w, std::string& collectionName, std::string name, GCPtr<GCObject>& obj);
		static inline bool GetPart(const GCPtr<Whole>& w, unsigned int collectionId, unsigned int id, GCPtr<GCObject>& obj);
		static inline bool GetPart(const GCPtr<Whole>& w, unsigned int collectionId, std::string name, GCPtr<GCObject>& obj);
		static inline void DestroyPart(const GCPtr<Whole>& w, std::string& collectionName, std::string& partName);
		static inline void DestroyPart(const GCPtr<Whole>& w, std::string& collectionName, unsigned int& partId);
		static inline void DestroyPart(const GCPtr<Whole>& w, unsigned int& collectionId, std::string& partName);
		static inline void DestroyPart(const GCPtr<Whole>& w, unsigned int& collectionId, unsigned int& partId);

		static inline void LuaRegisterFunction(const std::string& name, LuaCFunction fn, void* data = NULL, LuaTypeId dataType = 0);
		static void LuaThrowScriptError(const char* format, ...);
		template<class T> static inline unsigned int LuaRegisterType(const T* example, const std::string& alias, StringFunction stringer, ValueFunction valuer, bool allowInterVM, bool regsiterConstructors = true);
		static inline int LuaDispatcher(lua_State* L) { return LuaApi::LuaDispatcher(L); }
		static inline const GCPtr<LuaProcess> LuaGetCurrentProcess();
		static inline lua_State* LuaGetCurrentLuaState();
		static inline unsigned int LuaGetNumArgs(lua_State* L);
		static inline void LuaCheckNumArguments(lua_State* L, unsigned int n);
		static inline void LuaCheckNumArgumentsGreaterOrEqual(lua_State* L, unsigned int n);
		static inline int LuaCheckNumArgumentRange(lua_State* L, unsigned int args[], unsigned int numSizes);
		static inline const int LuaGetArgumentType(lua_State* L, int arg);

		template<typename T> static inline void LuaGetArgument(lua_State* L, int arg, T& t);
		static inline const TValue* LuaGetArg(lua_State* L, int arg);

		static inline bool LuaRenameTableValue(const std::string& oldName, const std::string& newName);
		static inline void LuaGetErrorMessageDetails(std::string& errorMessage);
	};



	// --------------------------------------------------------------------------						
	// Function:	GetGod
	// Description:	Gets god.
	// Arguments:	none
	// Returns:		God
	// --------------------------------------------------------------------------
	inline const GCPtr<God> &Api::GetGod()
	{ 
		return God::GetGod(); 
	}


	// --------------------------------------------------------------------------						
	// Function:	UpdateGod
	// Description:	Updates God and all Worlds.
	// Arguments:	time to update until
	// Returns:		none
	// --------------------------------------------------------------------------
	inline void Api::UpdateGod(double until)
	{ 
		God::GetGod()->Update(until);	
	}


	// --------------------------------------------------------------------------						
	// Function:	CreateCollection
	// Description:	Creates a new collection to store parts in.
	// Arguments:	whole (e.g Agent) , collection name
	// Returns:		id
	// --------------------------------------------------------------------------
	unsigned int Api::CreateCollection(const GCPtr<Whole>& w, std::string& collectionName)
	{
		GCPtr<Collection> c;
		return w->CreateCollection(collectionName, c);
	}


	// --------------------------------------------------------------------------						
	// Function:	DestroyCollection
	// Description:	Destroys a collection and all its parts.
	// Arguments:	whole (e.g Agent) , collection name
	// Returns:		none
	// --------------------------------------------------------------------------
	void Api::DestroyCollection(const GCPtr<Whole>& w, std::string& collectionName)
	{
		w->DestroyCollection(collectionName);
	}


	// --------------------------------------------------------------------------						
	// Function:	DestroyCollection
	// Description:	Destroys a collection and all its parts.
	// Arguments:	whole (e.g Agent) , collection id
	// Returns:		none
	// --------------------------------------------------------------------------
	void Api::DestroyCollection(const GCPtr<Whole>& w, unsigned int& collectionId)
	{
		w->DestroyCollection(collectionId);
	}
	// --------------------------------------------------------------------------						
	// Function:	AddPart
	// Description:	Add part to a collection.
	// Arguments:	whole (e.g Agent), collection name, part name, part
	// Returns:		id or -1 if collection doesnt exist
	// --------------------------------------------------------------------------
	unsigned int Api::AddPart(const GCPtr<Whole>& w, std::string& collectionName, std::string name, GCPtr<GCObject> object)
	{
		return w->AddPart(collectionName, name, object);
	}


	// --------------------------------------------------------------------------						
	// Function:	AddPart
	// Description:	Add part to a collection.
	// Arguments:	whole (e.g Agent), collection id, part name, part
	// Returns:		id or -1 if collection doesnt exist
	// --------------------------------------------------------------------------
	unsigned int Api::AddPart(const GCPtr<Whole>& w, unsigned int collectionId, std::string name, GCPtr<GCObject> object)
	{
		return w->AddPart(collectionId, name, object);
	}



	// --------------------------------------------------------------------------						
	// Function:	GetPart
	// Description:	Gets part.
	// Arguments:	whole (e.g Agent) ,collection name, part id, part returned
	// Returns:		if got
	// --------------------------------------------------------------------------
	bool Api::GetPart(const GCPtr<Whole>& w, std::string& collectionName, unsigned int id, GCPtr<GCObject>& obj)
	{
		return w->GetPart(collectionName, id, obj);

	}


	// --------------------------------------------------------------------------						
	// Function:	GetPart
	// Description:	Get part.
	// Arguments:	whole (e.g Agent), collection name, part name, part returned
	// Returns:		if got
	// --------------------------------------------------------------------------
	bool Api::GetPart(const GCPtr<Whole>& w, std::string& collectionName, std::string name, GCPtr<GCObject>& obj)
	{
		return w->GetPart(collectionName, name, obj);
	}


	// --------------------------------------------------------------------------						
	// Function:	GetPart
	// Description:	Get part.
	// Arguments:	whole (e.g Agent), collection id, part id, part returned
	// Returns:		if got
	// --------------------------------------------------------------------------
	bool Api::GetPart(const GCPtr<Whole>& w, unsigned int collectionId, unsigned int id, GCPtr<GCObject>& obj)
	{
		return w->GetPart(collectionId, id, obj);
	}


	// --------------------------------------------------------------------------						
	// Function:	GetPart
	// Description:	Get part.
	// Arguments:	collection id, part name, part returned
	// Returns:		if got
	// --------------------------------------------------------------------------
	bool Api::GetPart(const GCPtr<Whole>& w, unsigned int collectionId, std::string name, GCPtr<GCObject>& obj)
	{
		return w->GetPart(collectionId, name, obj);
	}


	// --------------------------------------------------------------------------						
	// Function:	DestoryPart
	// Description:	Destroys a part.
	// Arguments:	whole (e.g Agent), collection name, part name
	// Returns:		none
	// --------------------------------------------------------------------------
	inline void Api::DestroyPart(const GCPtr<Whole>& w, std::string& collectionName, std::string& partName)
	{
		w->DestroyPart(collectionName, partName);
	}


	// --------------------------------------------------------------------------						
	// Function:	DestoryPart
	// Description:	Destroys a part.
	// Arguments:	whole (e.g Agent) , collection name, part Id
	// Returns:		none
	// --------------------------------------------------------------------------
	inline void Api::DestroyPart(const GCPtr<Whole>& w, std::string& collectionName, unsigned int& partId)
	{
		w->DestroyPart(collectionName, partId);
	}


	// --------------------------------------------------------------------------						
	// Function:	DestoryPart
	// Description:	Destroys a part.
	// Arguments:	whole (e.g Agent) , collection id, part name
	// Returns:		none
	// --------------------------------------------------------------------------
	inline void Api::DestroyPart(const GCPtr<Whole>& w, unsigned int& collectionId, std::string& partName)
	{
		w->DestroyPart(collectionId, partName);
	}


	// --------------------------------------------------------------------------						
	// Function:	DestoryPart
	// Description:	Destroys a part.
	// Arguments:	whole (e.g Agent) , collection name, part id
	// Returns:		none
	// --------------------------------------------------------------------------
	inline void Api::DestroyPart(const GCPtr<Whole>& w, unsigned int& collectionId, unsigned int& partId)
	{
		w->DestroyPart(collectionId, partId);
	}


	// --------------------------------------------------------------------------						
	// Function:	GetAgent
	// Description:	Gets the currently active Agent.
	// Arguments:	none
	// Returns:		Agent
	// --------------------------------------------------------------------------
	inline const GCPtr<Agent> Api::GetAgent()
	{
		GCPtr<Agent> agent;
		agent.DynamicCast(Scheduler::GetCurrentProcess()->GetVM());
		return agent;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetNode
	// Description:	Gets the currently active Node. 
	// Arguments:	none
	// Returns:		Node
	// --------------------------------------------------------------------------
	inline const GCPtr<Node> Api::GetNode()
	{
		GCPtr<Node> node;
		node.DynamicCast(Scheduler::GetCurrentProcess()->GetObject());
		return node;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetCurrentProcess
	// Description:	Gets the currently active Process.
	// Arguments:	none
	// Returns:		LuaProcess
	// --------------------------------------------------------------------------
	inline const GCPtr<Process> Api::GetCurrentProcess()
	{
		return Scheduler::GetCurrentProcess();
	}


	// --------------------------------------------------------------------------						
	// Function:	GetImplementation
	// Description:	Get the language implementation of the active Process.
	// Arguments:	none
	// Returns:		implementation
	// --------------------------------------------------------------------------
	inline Implementation Api::GetImplementation() 
	{ 
		return Scheduler::GetCurrentProcess()->GetImplementation(); 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetPrivileges
	// Description:	Get the privileges of the active Process.
	// Arguments:	none
	// Returns:		privileges
	// --------------------------------------------------------------------------
	inline Privileges Api::GetPrivileges()
	{
		Scheduler::GetCurrentProcess()->GetPrivileges();
	}


	// --------------------------------------------------------------------------						
	// Function:	GetHomeEnvironment
	// Description:	Get the home Environment of the active Process.
	// Arguments:	none
	// Returns:		Environment
	// --------------------------------------------------------------------------
	inline const GCPtr<Environment> &Api::GetHomeEnvironment()
	{
		return Scheduler::GetCurrentProcess()->GetHomeEnvironment();
	}


	// --------------------------------------------------------------------------						
	// Function:	GetCurrentEnvironment
	// Description:	get the home Environment of the active Process.
	// Arguments:	none
	// Returns:		Environment
	// --------------------------------------------------------------------------
	inline const GCPtr<Environment>& Api::GetCurrentEnvironment()
	{
		return Scheduler::GetCurrentProcess()->GetCurrentEnvironment();
	}


	// --------------------------------------------------------------------------						
	// Function:	AddMsgArg
	// Description:	Adds an argument to the message.
	// Arguments:	message, arg
	// Returns:		none
	// --------------------------------------------------------------------------
	template<typename T> inline void Api::AddMsgArg(Message* msg, T* arg)
	{ 
		msg->AddArgument(arg); 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetMsgReturnVal
	// Description:	Adds an argument to the message.
	// Arguments:	message, return value arg number
	// Returns:		value
	// --------------------------------------------------------------------------
	template<typename T> inline T* Api::GetMsgReturnVal(Message* msg, unsigned int retrunValNum, T* dummy)
	{ 
		return (T*)msg->GetReturnValues()[retrunValNum].GetValue(); 
	}


	// --------------------------------------------------------------------------						
	// Function:	SendMsg
	// Description:	Adds an argument to the message.
	// Arguments:	message, delay before receiving
	// Returns:		if sent
	// --------------------------------------------------------------------------
	inline bool Api::SendMsg(Message* msg, double delay)
	{ 
		return msg->SendMsg(delay, 0); 
	}



	// --------------------------------------------------------------------------						
	// Function:	OpenNamespace
	// Description:	Opens and if creates a name space in the current Process.
	// Arguments:	namespace
	// Returns:		none
	// --------------------------------------------------------------------------
	inline void Api::OpenNamespace(const std::string& name)
	{
		if (!name.empty())
		{
			if (Api::GetImplementation() == Lua)
				LuaApi::OpenNamespace(name);
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	CloseNamespace
	// Description:	Closes the currently open namespace in the current Process,
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	inline void Api::CloseNamespace()
	{
		if (Api::GetImplementation() == Lua)
			LuaApi::CloseNamespace();
	}


	// --------------------------------------------------------------------------						
	// Function:	LuaRegisterType
	// Description:	Registers a user data type with the Process.
	// Arguments:	example of user data type,
	//				name as appears in lua script,
	//				function to be used to convert value to string,
	//				function to be used to convert string to value,
	//				whether to allow this type to be passed between
	//				VMs when messaging,
	//				whether to add default constructors
	// Returns:		type id on register type
	// --------------------------------------------------------------------------
	template<class T> inline unsigned int Api::LuaRegisterType(const T* example, const std::string& alias, StringFunction stringer, ValueFunction valuer, bool allowInterVM, bool regsiterConstructors)
	{
		return LuaApiTemplate::RegisterType(example, alias, stringer, valuer, allowInterVM, regsiterConstructors);
	}


	// --------------------------------------------------------------------------						
	// Function:	RegisterFunction
	// Description:	Registers a C function to be used in script.
	// Arguments:	lua state,  function name, C function to register, user data 
	//				attached to function, type of user data attached
	// Returns:		if successful
	// --------------------------------------------------------------------------
	template< typename F >
	inline void Api::RegisterFunction(const ::std::string& functionName, F *f, unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		if (Api::GetImplementation() == Lua)
			RegisterLuaFunction(functionName, f, firstReturnArg, module);
	}



	// --------------------------------------------------------------------------						
	// Function:	RegisterMemberFunction
	// Description:	Registers a member function in the to be used in script.
	// Arguments:	example of owner type of the member function
	//				name of function, function, upvalue data
	//				for closure, type of the upvalue data
	// Returns:		none
	// --------------------------------------------------------------------------
	template< typename C, typename F >
	inline void Api::RegisterMemberFunction(const C* ownerExampe, const ::std::string& functionName, F *f, unsigned int firstReturnArg, const GCPtr<Module>& module)
	{
		if (Api::GetImplementation() == Lua)
			RegisterLuaMemberFunction(ownerExampe, functionName, f, firstReturnArg, module);
	}


	// --------------------------------------------------------------------------						
	// Function:	RegisterHardClass
	// Description:	Registers a C++ class so Objects of that class can be created.
	//				in script
	// Arguments:	Class
	// Returns:		none
	// --------------------------------------------------------------------------
	inline void Api::RegisterHardClass(const GCPtr<Class>& cls)
	{
		Registry::GetRegistry().RegisterHardClass(cls);
	}

	// --------------------------------------------------------------------------						
	// Function:	LuaRegisterFunction
	// Description:	Registers a Lua format function to be used in script.
	// Arguments:	name of function, function, upvalue data
	//				for closure, type of the upvalue data
	// Returns:		none
	// --------------------------------------------------------------------------						
	inline void Api::LuaRegisterFunction(const std::string& name, LuaCFunction fn, void* data, LuaTypeId dataType)
	{
		RegisterLuaFunction(name, fn, data, dataType);
	}


	// --------------------------------------------------------------------------						
	// Function:	LuaGetCurrentProcess
	// Description:	Gets the currently active LuaProcess.
	// Arguments:	none
	// Returns:		LuaProcess
	// --------------------------------------------------------------------------
	inline const GCPtr<LuaProcess> Api::LuaGetCurrentProcess() 
	{ 
		return LuaApi::GetCurrentLuaProcess(); 
	}


	// --------------------------------------------------------------------------						
	// Function:	LuaGetCurrentLuaState
	// Description:	Returns the lua state of the current Process.
	// Arguments:	none
	// Returns:		lua state
	// --------------------------------------------------------------------------						
	inline lua_State* Api::LuaGetCurrentLuaState() 
	{ 
		return LuaApi::GetCurrentLuaState(); 
	}
	

	// --------------------------------------------------------------------------						
	// Function:	LuaGetNumArgs
	// Description:	Returns the number of arguments in the lua state stack. 
	// Arguments:	lua state
	// Returns:		num args
	// --------------------------------------------------------------------------
	inline unsigned int Api::LuaGetNumArgs(lua_State* L) 
	{ 
		return LuaApi::GetNumArgs(L); 
	}
	

	// --------------------------------------------------------------------------						
	// Function:	LuaCheckNumArguments
	// Description:	Throws error if the number of arguments in the lua state 
	//				stack is not the same as that given.
	// Arguments:	lua state, num args required
	// Returns:		none
	// --------------------------------------------------------------------------
	inline void Api::LuaCheckNumArguments(lua_State* L, unsigned int n) 
	{ 
		return LuaApi::CheckNumArguments(L, n); 
	}


	// --------------------------------------------------------------------------						
	// Function:	LuaCheckNumArgumentsGreaterOrEqual
	// Description:	Throws error if the number of arguments is lass than given.
	// Arguments:	lua state, num args 
	// Returns:		none
	// --------------------------------------------------------------------------
	inline void Api::LuaCheckNumArgumentsGreaterOrEqual(lua_State* L, unsigned int n) 
	{ 
		LuaApi::CheckNumArgumentsGreaterOrEqual(L, n); 
	}
	

	// --------------------------------------------------------------------------						
	// Function:	LuaCheckNumArgumentRange
	// Description:	Checks if no args on stack is equal to one of the given 
	//				arg sizes, thows error if not.
	// Arguments:	lua state, args sizes, number of arg sizes to to check
	// Returns:		num args
	// --------------------------------------------------------------------------
	inline int Api::LuaCheckNumArgumentRange(lua_State* L, unsigned int args[], unsigned int numSizes) 
	{ 
		return LuaApi::CheckNumArgumentRange(L, args, numSizes); 
	}
	

	// --------------------------------------------------------------------------						
	// Function:	LuaGetArgumentType
	// Description:	Gets the data type of the argument number asked for.
	// Arguments:	lua state, args number 
	// Returns:		type id
	// --------------------------------------------------------------------------
	inline const int Api::LuaGetArgumentType(lua_State* L, int arg) 
	{ 
		return LuaApi::GetArgumentType(L, arg); 
	}


	// --------------------------------------------------------------------------						
	// Function:	LuaGetArgument
	// Description:	Gets the argument number asked for of type requested.
	// Arguments:	lua state, args numbeer, value of type to return
	//				throws error if fails
	// Returns:		none but referenced argument is set
	// --------------------------------------------------------------------------
	template<typename T> inline void Api::LuaGetArgument(lua_State* L, int arg, T& t) 
	{ 
		return LuaApi::GetArgument(L, arg, t); 
	}


	// --------------------------------------------------------------------------						
	// Function:	LuaGetArg
	// Description:	Gets the script argument of given number from stack.
	// Arguments:	lua state, num of arg required
	// Returns:		argument as lua object
	// --------------------------------------------------------------------------
	inline const TValue* Api::LuaGetArg(lua_State* L, int arg) 
	{ 
		return LuaApi::GetArg(L, arg); 
	}


	// --------------------------------------------------------------------------						
	// Function:	LuaRenameTableValue
	// Description:	Renames a variable in lua table, expects table on stack.
	// Arguments:	variable value, new name
	// Returns:		if successful
	// --------------------------------------------------------------------------
	 inline bool Api::LuaRenameTableValue(const std::string& oldName, const std::string& newName) 
	 { 
		 return LuaApi::RenameTableValue(oldName, newName); 
	 }


	// --------------------------------------------------------------------------						
	// Function:	LuaGetErrorMessageDetails
	// Description:	Create a full line numbered error message, but without
	//				the problem detail.
	// Arguments:	full line numbered etc error message to be added to
	// Returns:		none
	// --------------------------------------------------------------------------
	inline void Api::LuaGetErrorMessageDetails(std::string& errorMessage) 
	{ 
		LuaApi::GetErrorMessageDetails(errorMessage); 
	}

} // namespace shh


#endif