	////////////////////////////////////////////////////////////////////////////////////////////////
	// __CALL

	template<  typename F, __TYPENAMES >
	class __CALL : public Registry::CallInterface
	{
	public:

		__CALL(::std::string fumctionName, F funmction, unsigned int firstReturnArg) :
			myFunctionName(fumctionName),
			myFunction(funmction),
			myFirstReturnArg(firstReturnArg)
		{}

		virtual ExecutionState Call(void* data) { return CallPolicy<F>(data); }
		virtual void* GetFunction() { return myFunction; }


		template< typename F > ExecutionState CallPolicy(void* data) const
		{
			LuaDispatchData* dispatchData = static_cast<LuaDispatchData*>(data);
			__DISPATCH
			__GETARGS
			__EXECUTE
			__RETURNARGS
			return rv;
		}


	private:

		::std::string myFunctionName;
		F myFunction;
		unsigned int myFirstReturnArg;
	};

	template< __TYPENAMES >
	void RegisterLuaFunction(const ::std::string& functionName, ExecutionState(*f)(__ARGSREFLIST), unsigned int firstReturnArg, const GCPtr<ModuleInterface>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(__ARGSREFLIST);
		typedef __TYPELIST(__ARGLIST) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef __CALL< Function, __ARGLIST > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg ="Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaFunction(functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}

	template< typename C, __TYPENAMES >
	void RegisterLuaMemberFunction(const C* ownerExampe, const ::std::string& functionName, ExecutionState(*f)(__ARGSREFLIST), unsigned int firstReturnArg, const GCPtr<ModuleInterface>& module)
	{
		lua_State* L = LuaApi::GetCurrentLuaState();
		typedef ExecutionState(*Function)(__ARGSREFLIST);
		typedef __TYPELIST(__ARGLIST) Arguments;
		typedef Parameters< Function, Arguments > FunctionType;
		typedef __CALL< Function, __ARGLIST > Call;
		GCPtr<Call> call(new Call(functionName, f, firstReturnArg));
		FunctionType parameter(f, firstReturnArg);
		Registry::OverloadTable::ArgumentTypes argumentTypes = parameter.GetSendTypeIds();
		GCPtr<Registry::OverloadTable> ok = Registry::GetRegistry().AddModuleFunction(module, functionName, argumentTypes, call);
		if (!ok.IsValid())
		{
			::std::string msg = "Could not registger function " + functionName + " from module " + module->GetName();
			THROW(msg.c_str());
		}
		GCPtr<Registry::OverloadTable>* ot = new GCPtr<Registry::OverloadTable>(ok);
		RegisterLuaMemberFunction(ownerExampe, functionName, LuaApi::LuaDispatcher, ot, TypeLog<GCPtr<Registry::OverloadTable>>::GetTypeId());
		return;
	}




