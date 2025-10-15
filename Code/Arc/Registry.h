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

#ifndef REGISTRY_H
#define REGISTRY_H

#include "../Common/SecureStl.h"
#include "../Common/Exception.h"
#include "../Common/Enums.h"
#include "../Config/GCPtr.h"
#include "Module.h"
#include <vector>
#include <map>
#include <string>

namespace shh {

	typedef bool (*EqualsFunction)(void const* const, void const* const);
	typedef std::string(*StringFunction)(void const* const);
	typedef bool (*ValueFunction)(const std::string&, void *&, int&);

	class Module;
	class Realm;
	class Process;
	class Class;
	class ClassManager;
	class Object;
	
	class Registry : public GCObject
	{
		template <class T> friend class Type;

	public:

		enum TypeIds {
			BASE_TYPEID = 10
		};

		struct ClassSpec
		{
			enum InheritanceType { Abstract, Inheritable, Final };

			Implementation myImplementation;
			std::string myClassName;
			std::string myParentName;
			std::string myTypeName;
			InheritanceType myInheritanceType;
			std::string myPath;
			std::string myFilename;
			std::string myScript;
			std::string myOveridePrefix;
		};

		typedef std::map<std::string, ClassSpec> ClassSpecs;
		typedef bool (*ClassesSpecifier)(const std::string&, const std::string&, ClassSpecs&, bool, bool, const std::string&);
		typedef GCPtr<Process>(*ProcessConstructor)(Privileges, GCPtr<Process>);
		typedef GCPtr<Object>(*ObjectConstructor)(const GCPtr<ClassManager>&, const GCPtr<Class>&, const GCPtr<Process>&);

		struct ObjectInstantiator
		{
			Implementation myImplementation;
			ClassesSpecifier myClassesSpecifier;
			ProcessConstructor myProcessConstructor;
			ObjectConstructor myObjectConstructor;
			std::vector<std::string> myPaths;
		};

		typedef std::map<std::string, ObjectInstantiator> NamedObjectInstantiators;
		typedef std::vector<NamedObjectInstantiators> ObjectInstantiators;



		struct DispatchData
		{
			std::string myMessage;
		};

		class CallInterface : public GCObject
		{
			friend Registry;
		public:
			CallInterface() {}
			~CallInterface() {}
			virtual ExecutionState Call(void* data) = 0;
			virtual void * GetFunction() = 0;
		};

		class OverloadTable : public GCObject
		{
			friend class Registry;

		public:

			typedef std::vector<int> ArgumentTypes;
			typedef std::map<ArgumentTypes, GCPtr<CallInterface>> FunctionCalls;
			typedef std::pair<ArgumentTypes, GCPtr<CallInterface>> FunctionCallPair;
			typedef std::map<int, ArgumentTypes> SharedTypes;

			OverloadTable();
			OverloadTable(const std::string& funcName);
			~OverloadTable();
			ExecutionState Call(Implementation i, const Registry::OverloadTable::ArgumentTypes& args, const SharedTypes& sharedTypes, void* dispatchData);


		private:

			std::string myFunctionName;
			FunctionCalls myFunctionCalls;
			GCPtr<CallInterface> myVariableLenthFunction;

		};
		
	
		static Registry& GetRegistry();
		static bool SetRegistry(const GCPtr<Registry> &r);
		void CloseRegistry();



		EqualsFunction const GetEqualsFunction(int typeId) const;
		StringFunction const GetStringFunction(int typeId) const;
		ValueFunction const GetValueFunction(int typeId) const;

		void RegisterType(const BaseType *type, int impId, Implementation imp);
		BaseType const * const GetType(int typeId) const;
		const std::string& GetTypeName(int typeId) const;
		int GetTypeId(const std::string name) const;
		unsigned int GetLastTypeId() const;
		void SetCastableTypes();


		void RegisterFunctions(int typeId, EqualsFunction e, StringFunction s, ValueFunction v);
	
		GCPtr<Realm> RegisterRealm(const GCPtr<Realm>& m);
		void UnregisterRealm(const GCPtr<Realm>& m);

		GCPtr<Module> GetModule(const std::string& name) const;
		GCPtr<Module> RegisterModule(const GCPtr<Module>& m);
		bool RegisterModuleInProcess(const GCPtr<Process>& process, const std::string& moduleName, const std::string& nameAsAppearsInScript, const StringKeyDictionary& sd);
		GCPtr<OverloadTable> AddModuleFunction(const GCPtr<Module>& module, const std::string& functionName, OverloadTable::ArgumentTypes& argTypes, const GCPtr<CallInterface>& call);
		bool RegisterModuleTypes(const GCPtr<Process>& process, const StringKeyDictionary& moduleList);
		bool RegisterObjectInstantiator(const std::string& typeName, const ObjectInstantiator& i);
		bool RegisterHardClass(const GCPtr<Class>& cls);
		GCPtr<Class> GetHardClass(const std::string& typeName, const std::string& className) const;
		const NamedObjectInstantiators& GetObjectInstantiators(Implementation i) const;
	
	private:

		typedef std::vector<EqualsFunction> EqualsFunctions;
		typedef std::vector<StringFunction> StringFunctions;
		typedef std::vector<ValueFunction> ValueFunctions;
		typedef std::map < std::string, GCPtr<OverloadTable>> FunctionOverloadTables;
		typedef std::map < std::string, FunctionOverloadTables> ModuleFunctions;
		typedef std::map < std::string, GCPtr<Module>> Modules;
		typedef std::vector<BaseType*> Types;
		typedef std::map<std::string, int> TypeNames;
		typedef std::vector<std::vector<int>> TypeMap;
		typedef std::map<std::string, ObjectInstantiator> ObjectInstantiatorsMap;
		typedef std::map<Implementation, ObjectInstantiatorsMap> TypeInstantiators;
		typedef std::map<std::string, GCPtr<Class>> HardClasses;
		typedef std::map<std::string, HardClasses> TypeHardClasses;
		static GCPtr<Registry> *ourRegistry;

		int myNextTypeId;
		Types myTypes;
		TypeNames myTypeNames;
		TypeMap myShhToImpId;
		TypeMap myImpToShhId;

		EqualsFunctions myEqualsFunctions;
		StringFunctions myStringFunctions;
		ValueFunctions myValueFunctions;

		ModuleFunctions myModuleFunctions;
		Modules myModules;
		Modules myRealms;

		ObjectInstantiators myObjectInstantiators;
		TypeHardClasses myHardClasses;
	
		Registry();
		~Registry();

		int GetNewTypeId();

	};



}

#endif //REGISTRY_H
