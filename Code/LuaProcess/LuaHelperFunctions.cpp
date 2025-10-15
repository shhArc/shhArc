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


#include "LuaHelperFunctions.h"
#include "../Config/GCPtr.h"
#include "../Arc/Registry.h"
#include "../Arc/Type.h"
#include "../Arc/Type.inl"
#include "../Common/Dictionary.h"
#include "LuaProcess.h"
#include "LuaApi.h"



#define luaL_getmetatable(L,n)	(lua_getfield(L, LUA_REGISTRYINDEX, (n)))


// --------------------------------------------------------------------------						
// Function:	LuaCheckContinue
// Description: to be placed in luaV_execute to test whether the code is 
//				being returned to and restores the state if is
// Arguments:	lua state, program code counter, call info
// Returns:		none
// --------------------------------------------------------------------------
void LuaCheckContinue(lua_State* L, const Instruction *&pc, CallInfo* ci)
{
	shh::GCPtr<shh::Process> p = shh::Scheduler::GetCurrentProcess();
	shh::ExecutionState entryState = p->GetState();
	if (entryState == shh::ExecutionContinue || entryState == shh::ExecutionReceivingCallback)
	{
		int numArgsReturnedByUncompletedCall = p->ContinueProcess();
		L->top.p -= numArgsReturnedByUncompletedCall;
		StkId base = L->top.p;
		if (p->GetState() == shh::ExecutionContinue || p->GetState() == shh::ExecutionReceivingCallback)
		{
			pc--;
		}
		else if (entryState == shh::ExecutionReceivingCallback)
		{
			unsigned int n = 0;
			if (p->GetCurrentMessage()->myCallbackMessage != (void*)-1)
			{
				n = p->GetCurrentMessage()->myCallbackMessage->InitiateCallback();
			}
			else
			{
				n = 1;
				bool* b = new bool(false);
				shh::Type<bool>::GetStatic()->Push(p->GetCurrentMessage()->myTo->GetImplementation(), b);
			}
			p->GetCurrentMessage()->myCallbackMessage = NULL;
			for (; n < (unsigned int)numArgsReturnedByUncompletedCall; n++)
				setnilvalue(s2v(base + n));
		}
	}
}


// --------------------------------------------------------------------------						
// Function:	LuaCheckYield
// Description: to be placed in luaV_execute to test whether the code should 
//				yield to return to later (stores relevant adata as required
// Arguments:	lua state, number of arguments in call stack that that the 
//				current lua code requires on return
// Returns:		none
// --------------------------------------------------------------------------
void LuaCheckYield(lua_State* L, int numArgsReturnedByUncompletedCall)
{
	shh::GCPtr<shh::SoftProcess> p;
	p.StaticCast(shh::Scheduler::GetCurrentProcess());

	bool timedOutNow = false;
	if (p->GetState() != shh::ExecutionTimedOut &&
		p->GetState() != shh::ExecutionYielded &&
		p->GetState() != shh::ExecutionAwaitingCallback)
	{
		if (p->IncTimeOut())
		{
			timedOutNow = true;
			p->YieldProcess(shh::ExecutionTimedOut, -1);
		}
	}

	shh::ExecutionState state = p->GetState();
	if (state == shh::ExecutionYielded || state == shh::ExecutionTimedOut || state == shh::ExecutionAwaitingCallback)
	{
		p->YieldProcess(state, timedOutNow || numArgsReturnedByUncompletedCall < 0 ? 0 : numArgsReturnedByUncompletedCall);
		L->status = LUA_YIELD;
		L->ci->u2.nyield = 0;
		luaD_throw(L, LUA_YIELD);
	}
}



namespace shh
{
	const void* LuaUserData::ourMarker = NULL;



	// --------------------------------------------------------------------------						
	// Function:	Clone	
	// Description:	clones lua data (recurively if a table)
	// Arguments:	state to clone from, state to clone to, value to clone, 
	//				prebuilt map of values (needed if a table has duplicate 
	//				reference, map of values already cloned
	// Returns:		true if cloned
	// --------------------------------------------------------------------------
	static bool Clone(lua_State* from, lua_State* to, const TValue* toClone, LuaHelperFunctions::ValueMap& valueMap, LuaHelperFunctions::ValueMap& alreadyClonedMap)
	{
		LuaTypeId type = LuaGetTypeId(toClone);


		if (type == LUA_TNIL || type == LUA_TLIGHTUSERDATA ||
			type == LUA_TBOOLEAN || type == LUA_TNUMBER ||
			(type == LUA_TFUNCTION && LuaGetFunctionType(toClone) == LUA_VLCF))
		{
			*LuaGetStackValue(to, -1) = *toClone;
		}
		else if (type == LUA_TTHREAD)
		{
			// dont clone threads
			return false;
		}
		else if (type == LUA_TTABLE)
		{

			Table* tableToClone = LuaGetTable(toClone);
			LuaHelperFunctions::ValueMap::iterator vit = valueMap.find(tableToClone);
			const TValue cloned = vit->second;

			LuaHelperFunctions::ValueMap::iterator tit = alreadyClonedMap.find(tableToClone);
			if (tit != alreadyClonedMap.end())
			{
				LuaSetStackValue(to, -1, &cloned);
				return true;
			}


			alreadyClonedMap[tableToClone] = cloned;
			Table* tableCloned = LuaGetTable(&cloned);

			// check if arked as nested global table
			if (tableCloned == NULL)
				return false;

			int grow = 6;
			EnsureFreeStack(from, grow);
			EnsureFreeStack(to, grow);

			LuaSetStackValue(from, 0, toClone);
			LuaIncStack(from);
			LuaSetStackValue(to, 0, &cloned);
			LuaIncStack(to);

			bool resizeNoString = false;
			lua_pushnil(from);

			while (lua_next(from, (from == to ? -3 : -2)))
			{
				char* str = NULL;
				if (LuaGetStackTypeId(from, -2) == LUA_TSTRING)
					str = (char*)lua_tostring(from, -2);

				TValue* value = LuaGetStackValue(from, -1);
				TValue* key = LuaGetStackValue(from, -2);



				LuaTypeId t = LuaGetTypeId(key);
				if(LuaGetTypeId(key) == LUA_TNIL)
				{
					LuaDecStack(from);
				}
				else
				{
					if (LuaGetTypeId(key) == LUA_TSTRING)
					{
						// if string key then setfield
						LuaIncStack(to);
						if (Clone(from, to, value, valueMap, alreadyClonedMap))
							lua_setfield(to, (from == to ? -4 : -2), str);
						else
							LuaDecStack(to);
					}
					else
					{		
						// else clone key
						LuaIncStack(to);
						Clone(from, to, key, valueMap, alreadyClonedMap);
						LuaIncStack(to);
						if (Clone(from, to, value, valueMap, alreadyClonedMap))
						{
							lua_settable(to, (from == to ? -5 : -3));
						}
						else
						{
							LuaDecStack(to);
							LuaDecStack(to);
						}
					}
					LuaDecStack(from);

				}

			}
			LuaDecStack(to);
			LuaDecStack(from);
			LuaSetStackValue(to, -1, &cloned);
		}
		else
		{
			void* v = NULL;
			if (type == LUA_TSTRING)
				v = LuaGetString(toClone);
			else if (abs(type) <= (int)Registry::GetRegistry().GetLastTypeId() && type < 0)
				v = LuaGetUserData(from, toClone);
			else if (type == LUA_TFUNCTION && LuaGetFunctionType(toClone) == LUA_VCCL)
				v = LuaGetCClosure(toClone);
			else if (type == LUA_TFUNCTION && LuaGetFunctionType(toClone) == LUA_VLCL)
				v = LuaGetLClosure(toClone);
			else if (type == LUA_TUSERDATA)
				v = LuaGetUserData(from, toClone);
			else
				LuaApi::ThrowScriptError("Type to clone not found");

			LuaHelperFunctions::ValueMap::iterator vit = valueMap.find(v);
			if (vit == valueMap.end())
				LuaApi::ThrowScriptError("Error copying table. Item not constructed");

			const TValue cloned = vit->second;
			LuaSetStackValue(to, -1, &cloned);
		}

		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	BuildValueMap	
	// Description:	builds a map of values needed when cloning if a table has 
	//				duplicate references
	// Arguments:	state to clone from, state to clone to, value to clone, 
	//				map of values to build, map of values already pre 
	//				initialized (ie if not global table or registry), whether 
	//				the table is global table, whether table is registry table
	// Returns:		cloned value in value map
	// --------------------------------------------------------------------------
	static TValue BuildValueMap(lua_State* from, lua_State* to, const TValue* toClone, LuaHelperFunctions::ValueMap& valueMap, LuaHelperFunctions::ValueMap& preInitMap, bool global = false, bool registry = false)
	{
		LuaTypeId type = LuaGetTypeId(toClone);

		if (type == LUA_TTHREAD ||
			type == LUA_TNIL || type == LUA_TLIGHTUSERDATA ||
			type == LUA_TBOOLEAN || type == LUA_TNUMBER ||
			(type == LUA_TFUNCTION && LuaGetFunctionType(toClone) == LUA_VLCF))
		{
			return TValue();
		}
		else
		{
			if (type == LUA_TTABLE)
			{
				Table* f = LuaGetTable(toClone);

				// check if table already has memeber built
				LuaHelperFunctions::ValueMap::iterator vit = valueMap.find(f);
				if (!global && !registry && vit != valueMap.end())
					return vit->second;

				// should be pre created as empty table
				LuaHelperFunctions::ValueMap::iterator pit = preInitMap.find(f);
				if (pit == preInitMap.end())
				{
					DEBUG_ASSERT(FALSE)
				}

				Table* pt = LuaGetTable(&pit->second);
				if (pt == NULL)
				{
					// nested global table DONT CLONE
					valueMap[f] = TValue();
					return TValue();
				}


				LuaSetStackValue(to, 0, &pit->second);
				LuaIncStack(to);


				TValue* cloned = LuaGetStackValue(to, -1);
				valueMap[f] = *cloned;
				LuaDecStack(to);

				LuaSetStackValue(from, 0, toClone);
				LuaIncStack(from);
				lua_pushnil(from);

				while (lua_next(from, -2))
				{
					char* str = NULL;
					if (LuaGetStackTypeId(from, -2) == LUA_TSTRING)
						str = (char*)lua_tostring(from, -2);

					if (LuaGetStackTypeId(from, -2) != LUA_TSTRING)
						BuildValueMap(from, to, LuaGetStackValue(from, -2), valueMap, preInitMap);

					TValue* key = LuaGetStackValue(from, -2);
					LuaTypeId t = LuaGetTypeId(key);
					if(LuaGetTypeId(key) == LUA_TNIL)
					{
						LuaDecStack(from);
					}
					else
					{
						TValue* value = LuaGetStackValue(from, -1);
						BuildValueMap(from, to, value, valueMap, preInitMap);
						LuaDecStack(from);
					}
				}
				LuaDecStack(from);
				return valueMap[f];
			}
			else if (abs(type) <= (int)Registry::GetRegistry().GetLastTypeId() && type < 0)
			{
				// shharc registered user type
				void* uToClone = LuaGetUserData(from, toClone);
				LuaHelperFunctions::ValueMap::iterator vit = valueMap.find(uToClone);
				if (vit != valueMap.end())
					return vit->second;

				lua_State* setter = from == to ? from : to;


				const BaseType* baseType = LuaGetType(from, toClone);
				if(baseType != NULL)
				{
					void* uCloned = baseType->Clone(uToClone);
					baseType->Push(Lua, uCloned);
					valueMap[uToClone] = *LuaGetStackValue(to, -1);
				}
				else
				{
					// not a registered shhArc LuaType so cant clone
					// this should only occur for upvalues that not shhArc 
					// LuaTypes that are added to a CClosure 
					valueMap[uToClone] = *toClone;
				}
				return valueMap[uToClone];
			}
			else if (type == LUA_TSTRING)
			{
				char* f = LuaGetString(toClone);
				LuaHelperFunctions::ValueMap::iterator vit = valueMap.find(f);
				if (vit != valueMap.end())
					return vit->second;

				TString* newString = LuaNewString(to, f);
				TValue* o = LuaGetStackValue(to, -1);
				LuaSetString(o, newString);

				if (LuaIsCollectable(toClone))
					LuaSetCollectable(o);
				
				TValue* cloned = LuaGetStackValue(to, -1);
				valueMap[f] = *cloned;
				return valueMap[f];
			}
			else if (type == LUA_TFUNCTION)
			{
				if (LuaGetFunctionType(toClone) == LUA_VCCL)
				{
					// c function closure copy func pointer and upvalues
					CClosure* clToClone = LuaGetCClosure(toClone);
					LuaHelperFunctions::ValueMap::iterator vit = valueMap.find(clToClone);
					if (vit != valueMap.end())
						return vit->second;

					for (int n = 0; n != clToClone->nupvalues; n++)
					{
						TValue upVal = BuildValueMap(from, to, &clToClone->upvalue[n], valueMap, preInitMap);
						LuaSetStackValue(to, 0, &upVal);
						LuaIncStack(to);
					}
					lua_pushcclosure(to, clToClone->f, clToClone->nupvalues);
					valueMap[clToClone] = *LuaGetStackValue(to, -1);
					LuaDecStack(to);
					return valueMap[clToClone];
				}
				else if (LuaGetFunctionType(toClone) == LUA_VLCL)
				{
					// lua function closure copy closure and upvalues
					LClosure* clToClone = LuaGetLClosure(toClone);
					LuaHelperFunctions::ValueMap::iterator vit = valueMap.find(clToClone);
					if (vit != valueMap.end())
						return vit->second;

					LClosure* clCloned = luaF_newLclosure(to, clToClone->nupvalues);

					luaF_initupvals(to, clCloned);
					for (int n = 0; n != clCloned->nupvalues; n++)
					{
						TValue value = BuildValueMap(from, to, clToClone->upvals[n]->v.p, valueMap, preInitMap);
						setobj(to, clCloned->upvals[n]->v.p, &value);

					}

					// copy proto type
					TValue protoToClone;
					LuaSetValue(&protoToClone, clToClone->p);
					TValue protoValue = BuildValueMap(from, to, &protoToClone, valueMap, preInitMap);
					clCloned->p = LuaGetProto(&protoValue);

					setclLvalue2s(to, to->top.p, clCloned);
					TValue* cloned = LuaGetStackValue(to, 0);
					valueMap[clToClone] = *cloned;
					return valueMap[clToClone];

				}
			}
			else if (type == LUA_TUSERDATA)
			{
				// userdata (non shharc type)
				void* v = LuaGetUserData(from, toClone);
				LuaHelperFunctions::ValueMap::iterator vit = valueMap.find(v);
				if (vit != valueMap.end())
					return vit->second;

				Udata* u = LuaGetUdata(toClone);
				void* pToClone = reinterpret_cast<char*>(getudatamem(u));
				void* pCloned = lua_newuserdata(to, u->len);
				memcpy(pCloned, pToClone, u->len);
				valueMap[pToClone] = *LuaGetStackValue(to, -1);
				LuaDecStack(to);
				return valueMap[pToClone];
			}
			else if (type == LUA_VPROTO)
			{
				Proto* pToClone = LuaGetProto(toClone);

				LuaHelperFunctions::ValueMap::iterator vit = valueMap.find(pToClone);
				if (vit != valueMap.end())
					return vit->second;

				Proto *pCloned = luaF_newproto(to);

				pCloned->numparams = pToClone->numparams;
				pCloned->is_vararg = pToClone->is_vararg;
				pCloned->maxstacksize = pToClone->maxstacksize;
				pCloned->linedefined = pToClone->linedefined;
				pCloned->lastlinedefined = pToClone->lastlinedefined;

				// clone source code
				TValue source;
				LuaSetString(&source, pToClone->source);
				pCloned->source = LuaNewString(to, LuaGetString(&source));


				// clone constants
				if (pToClone->sizek > 0)
				{
					pCloned->k = (TValue*)luaM_realloc_(to, pCloned->k, pCloned->sizek * sizeof(TValue), pToClone->sizek * sizeof(TValue));
					pCloned->sizek = pToClone->sizek;
					for (int k = 0; k != pToClone->sizek; k++)
					{
						if (ttisnil(&pToClone->k[k]))
							setnilvalue(&pCloned->k[k]);
						else if (ttisinteger(&pToClone->k[k]))
							pCloned->k[k] = pToClone->k[k];
						else
							pCloned->k[k] = BuildValueMap(from, to, &pToClone->k[k], valueMap, preInitMap);
					}
				}

				// clone sub proto types
				if (pToClone->sizep > 0)
				{
					pCloned->p = (Proto**)luaM_realloc_(to, pCloned->p, pCloned->sizep * sizeof(Proto*), pToClone->sizep * sizeof(Proto*));
					pCloned->sizep = pToClone->sizep;
					for (int p = 0; p != pToClone->sizep; p++)
					{
						if (pToClone->p[p] != NULL)
						{
							TValue protoToClone;
							LuaSetValue(&protoToClone, pToClone->p[p]);
							TValue protoValue = BuildValueMap(from, to, &protoToClone, valueMap, preInitMap);
							pCloned->p[p] = LuaGetProto(&protoValue);
						}
						else
						{
							pCloned->p[p] = NULL;
						}
					}
				}


				// clone upvalues
				if (pToClone->sizeupvalues > 0)
				{
					pCloned->upvalues = luaM_newvectorchecked(to, pToClone->sizeupvalues, Upvaldesc);
					pCloned->sizeupvalues = pToClone->sizeupvalues;
					for (int u = 0; u != pToClone->sizeupvalues; u++)
					{
						pCloned->upvalues[u].idx = pToClone->upvalues[u].idx;
						pCloned->upvalues[u].instack = pToClone->upvalues[u].instack;
						pCloned->upvalues[u].kind = pToClone->upvalues[u].kind;
						if (pToClone->upvalues[u].name == NULL)
						{
							pCloned->upvalues[u].name = NULL;
						}
						else
						{
							TValue name;
							LuaSetString(&name, pToClone->upvalues[u].name);
							pCloned->upvalues[u].name = LuaNewString(to, LuaGetString(&name));
						}
					}
				}

				// clone code
				if (pToClone->sizecode > 0)
				{
					pCloned->code = (Instruction*)luaM_realloc_(to, pCloned->code, pCloned->sizecode * sizeof(Instruction), pToClone->sizecode * sizeof(Instruction));
					pCloned->sizecode = pToClone->sizecode;
					memcpy(pCloned->code, pToClone->code, pToClone->sizecode * sizeof(Instruction));
				}
				
				// clone line info for debugging
				if (pToClone->sizelineinfo > 0)
				{
					pCloned->lineinfo = (ls_byte*)luaM_realloc_(to, pCloned->lineinfo, pCloned->sizelineinfo * sizeof(ls_byte), pToClone->sizelineinfo * sizeof(ls_byte));
					pCloned->sizelineinfo = pToClone->sizelineinfo;
					memcpy(pCloned->lineinfo, pToClone->lineinfo, pToClone->sizelineinfo * sizeof(ls_byte));
				}

				if (pToClone->sizeabslineinfo > 0)
				{
					pCloned->abslineinfo = (AbsLineInfo*)luaM_realloc_(to, pCloned->abslineinfo, pCloned->sizeabslineinfo * sizeof(AbsLineInfo), pToClone->sizeabslineinfo * sizeof(AbsLineInfo));
					pCloned->sizeabslineinfo = pToClone->sizeabslineinfo;
					memcpy(pCloned->abslineinfo, pToClone->abslineinfo, pToClone->sizeabslineinfo * sizeof(AbsLineInfo));
				}

				// clone local vars
				if (pToClone->sizelocvars > 0)
				{
					pCloned->locvars = (LocVar*)luaM_realloc_(to, pCloned->locvars, pCloned->sizelocvars * sizeof(LocVar), pToClone->sizelocvars * sizeof(LocVar));
					pCloned->sizelocvars = pToClone->sizelocvars;
					for (int l = 0; l != pToClone->sizelocvars; l++)
					{
						pCloned->locvars[l].endpc = pToClone->locvars[l].endpc;
						pCloned->locvars[l].startpc = pToClone->locvars[l].startpc;

						if (pToClone->locvars == NULL)
						{
							pCloned->locvars = NULL;
						}
						else
						{
							TValue local;
							LuaSetString(&local, pToClone->locvars[l].varname);
							pCloned->locvars[l].varname = LuaNewString(to, LuaGetString(&local));
						}
					}
				}
						
				
				TValue protoCloned;
				LuaSetValue(&protoCloned, pCloned);
				valueMap[pToClone] = protoCloned;
				return valueMap[pToClone];

			}
			else
			{
				LuaApi::ThrowScriptError("Type to clone not found");
			}
		}
		return *toClone;
	}


	// --------------------------------------------------------------------------						
	// Function:	PreInitTables	
	// Description:	recusively builds a map of keys in table
	// Arguments:	state to clone from, state to clone to, value to clone, 
	//				map of values pre initialized (ie if not global table or 
	//				registry), whether the 	table is global table, whether
	//				table is registry table
	// Returns:		none
	// --------------------------------------------------------------------------
	static void PreInitTables(lua_State* from, lua_State* to, const TValue* toClone, LuaHelperFunctions::ValueMap& preInitMap, bool global = false, bool registry = false)
	{
		LuaTypeId type = LuaGetTypeId(toClone);

		if (type == LUA_TTABLE)
		{
			Table* f = LuaGetTable(toClone);

			LuaHelperFunctions::ValueMap::iterator pit = preInitMap.find(f);
			if (!global && !registry && pit != preInitMap.end())
				return;

			if (!global && !registry)
			{
				if (LuaGetTable(LuaGetGlobalsValue(from)) == f)
				{
					// nested global table DONT CLONE
					preInitMap[f] = TValue();
					return;
				}
				lua_newtable(to);
				TValue* cloned = LuaGetStackValue(to, -1);
				preInitMap[f] = *cloned;
				LuaDecStack(to);
			}


			LuaSetStackValue(from, 0, toClone);
			LuaIncStack(from);
			lua_pushnil(from);

			while (lua_next(from, -2))
			{
				char* str = NULL;
				if (LuaGetStackTypeId(from, -2) == LUA_TSTRING)
					str = (char*)lua_tostring(from, -2);

				TValue* key = LuaGetStackValue(from, -2);
				LuaTypeId t = LuaGetTypeId(key);
				if(LuaGetTypeId(key) == LUA_TNIL)
				{
					LuaDecStack(from);
				}
				else 
				{
					if(LuaGetTypeId(key) == LUA_TTABLE)
						PreInitTables(from, to, key, preInitMap);
				
					TValue* value = LuaGetStackValue(from, -1);
					PreInitTables(from, to, value, preInitMap);
					LuaDecStack(from);
				}
			}
			LuaDecStack(from);
			return;
		}

		return;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetLuaTableObject
	// Description:	returns the object in the given table that is called
	//				by the key name
	// Arguments:	table, key
	// Returns:		object, else nil if not found
	// --------------------------------------------------------------------------
	const TValue* GetLuaTableObject(Table* t, TValue* key)
	{
		switch (ttype(key))
		{
		case LUA_TNUMBER:
			return luaH_getint(t, LuaGetInteger(key));
		case LUA_TSTRING:
			return LuaGetTableValue(t, LuaGetTString(key));
		case LUA_TBOOLEAN:
		case LUA_TLIGHTUSERDATA:
		case LUA_TTABLE:
		case LUA_TFUNCTION:
		case LUA_TUSERDATA:
		case LUA_TTHREAD:
			return LuaGetTableValue(t, key);
		default:
			return NULL;

		}

	}


	// --------------------------------------------------------------------------						
	// Function:	DeepCopy	
	// Description:	deep copies (if table)
	// Arguments:	state to clone from, state to clone to, value to clone, 
	//				whether the table is global table, whether table is registry 
	//				table
	// Returns:		none
	// --------------------------------------------------------------------------
	void LuaHelperFunctions::DeepCopy(lua_State* from, lua_State* to, const TValue* toClone, bool global, bool registry)
	{
		ValueMap preInitMap;
		ValueMap valueMap;
		ValueMap alreadyClonedMap;

		if (global)
		{
			// is global table
			Table* fgt = LuaGetTable(toClone);
			LuaGetGlobalsStack(to);
			TValue* gt = LuaGetStackValue(to, -1);
			valueMap[fgt] = *gt;
			preInitMap[fgt] = *gt;
			LuaDecStack(to);
		}
		else if (registry)
		{
			// is registry table
			Table* frt = LuaGetTable(toClone);
			TValue* rt = LuaGetRegistry(to);
			valueMap[frt] = *rt;
			preInitMap[frt] = *rt;
		}

		lua_gc(to, LUA_GCSTOP);
		PreInitTables(from, to, toClone, preInitMap, global, registry);
		BuildValueMap(from, to, toClone, valueMap, preInitMap, global, registry);
		LuaIncStack(to);
		Clone(from, to, toClone, valueMap, alreadyClonedMap);
		lua_gc(to, LUA_GCRESTART);
		return;
	}


	// --------------------------------------------------------------------------						
	// Function:	DeepCompare
	// Description:	compares two object for full value equvalence 
	//				(recursive if table)
	// Arguments:	lua state, object1, object2
	// Returns:		true if equal
	// --------------------------------------------------------------------------
	bool LuaHelperFunctions::DeepCompare(lua_State* L, const TValue* o1, const TValue* o2)
	{

		LuaTypeId type1 = LuaGetTypeId(o1);
		LuaTypeId type2 = LuaGetTypeId(o2);

		if (type1 != type2)
		{
			// not same types
			return false;
		}
		else
		{
			// same types check value 
			// table and some user types use reference so must be specially handled

			if (type1 == LUA_TTABLE)
			{
				// may be same data
				if (luaV_equalobj(L, o1, o2))
					return true;

				// not sure if order can be garanteed so match on key search

				int matched = 0;

				Table* const t1 = LuaGetTable(o1);
			

				LuaSetStackValue(L, 0, o1);
				LuaIncStack(L);
				lua_pushnil(L);

				while (lua_next(L, -2))
				{
					TValue* key1 = LuaGetStackValue(L, -2);
					TValue* value1 = LuaGetStackValue(L, -1);
					
					if(LuaGetTypeId(key1) == LUA_TNIL || LuaGetTypeId(value1) == LUA_TNIL)
					{
						LuaDecStack(L);
						continue;
					}

					const TValue* value2 = GetLuaTableObject(LuaGetTable(o2), key1);

					if (!value1 || !value2 || !DeepCompare(L, value1, value2))
					{
						LuaDecStack(L);
						LuaDecStack(L);
						LuaDecStack(L);
						LuaDecStack(L);
						return false;
					}
					matched++;
					LuaDecStack(L);
 				}
				LuaDecStack(L);



				int matchesRequired = 0;
				LuaSetStackValue(L, 0, o2);
				LuaIncStack(L);
				lua_pushnil(L);

				while (lua_next(L, -2))
 				{
					TValue* key1 = LuaGetStackValue(L, -2);
					TValue* value1 = LuaGetStackValue(L, -1);

					if(LuaGetTypeId(key1) == LUA_TNIL || LuaGetTypeId(value1) == LUA_TNIL)
					{
						LuaDecStack(L);
						continue;
					}

 					matchesRequired++;
					LuaDecStack(L);
				}
				LuaDecStack(L);


				return matched == matchesRequired;

			}
			else if (type1 < 0)
			{
				LuaTypeId userType = LuaGetTypeId(o1);
				EqualsFunction func = Registry::GetRegistry().GetEqualsFunction(userType);
				void* ud1 = LuaGetUdata(o1);
				void* ud2 = LuaGetUdata(o2);
				bool result = (*func) (ud1, ud2);
				return result;
			}
			else if (type1 == LUA_TUSERDATA)
			{
				void* ud1 = LuaGetUdata(o1);
				void* ud2 = LuaGetUdata(o2);
				return ud1 == ud2;
			}
			else
			{
				return luaV_equalobj(L, o1, o2);
			}
		}

	}


	// --------------------------------------------------------------------------						
	// Function:	Print	
	// Description:	prints a value (resurively if tableO
	// Arguments:	lua state, value, print format string
	// Returns:		none
	// --------------------------------------------------------------------------
	void LuaHelperFunctions::Print(lua_State* L, TValue* value, std::string &result, int indent)
	{
		for (int i = 0; i != indent; i++)
			result += "\t";

		int type = LuaGetTypeId(value);
		if (type == LUA_TNIL)
		{
			result += "*nil*";
		}
		else if (type == LUA_TTABLE)
		{
			result += "LuaTable {\n";
			LuaSetStackValue(L, 0, value);
			LuaIncStack(L);
			lua_pushnil(L);

			while (lua_next(L, -2))
			{
				char* str = NULL;
				if (LuaGetStackTypeId(L, -2) == LUA_TSTRING)
				{
					result += (char*)lua_tostring(L, -2);
					result += ": ";
				}

				TValue* v = LuaGetStackValue(L, -1);
				Print(L, v, result, indent + 1);
				LuaDecStack(L);
			}
			LuaDecStack(L);
			for (int i = 0; i != indent; i++)
				result += "\t";
			result += "}";
		}		
		else if (type == LUA_TFUNCTION)
		{
			result += "*function*";
		}
		if (type == LUA_TSTRING)
		{
			LuaSetStackValue(L, 0, value);
			std::string v = lua_tostring(L, 0);
			result += v;
		}
		else if (type == LUA_TNUMBER)
		{
			if (LuaGetSubType(value) == LUA_VNUMFLT)
			{
				LuaSetStackValue(L, 0, value);
				double v = lua_tonumber(L, 0);
				char temp[50];
				std::string format = "%f";
				_snprintf(temp, 49, format.c_str(), v);
				result += temp;
			}
			else
			{
				LuaSetStackValue(L, 0, value);
				int v = (int)lua_tointeger(L, 0);
				char temp[50];
				std::string format = "%d";
				_snprintf(temp, 49, format.c_str(), v);
				result += temp;
			}
		}
		else if (type == LUA_TBOOLEAN)
		{
			StringFunction func = Registry::GetRegistry().GetStringFunction(type);
			LuaSetStackValue(L, 0, value);
			bool v = lua_toboolean(L, 0);
			if(v)
				result += "true";
			else
				result += "false";
		}
		else if (type < 0)
		{
			StringFunction func = Registry::GetRegistry().GetStringFunction(type);
			void* v = LuaGetUserData(L, value);
			result += (*func)(v);
		}
		else
		{
			result += "**UNKNOWN TYPE**";
		}
		result += "\n";
	}


	// --------------------------------------------------------------------------						
	// Function:	PushVariant	
	// Description:	pushes a variant on to stack
	// Arguments:	lua state, variant
	// Returns:		is successful
	// --------------------------------------------------------------------------
	bool LuaHelperFunctions::PushVariant(lua_State* to, const Variant* toClone)
	{
		VariantKeyDictionary* dict;
		if (toClone->GetPtr(dict))
		{
			lua_newtable(to);
			TValue* cloned = LuaGetStackValue(to, -1);
			Table* tableCloned = LuaGetTable(cloned);

			for (typename VariantKeyDictionary::VariablesConstIterator vit = dict->Begin(); vit != dict->End(); vit++)
			{
				if (PushVariant(to, vit->first))
				{
					if (PushVariant(to, vit->second))
						lua_settable(to, -3);
					else
						LuaDecStack(to);
				}
			}
		}
		else 
		{
			void* v = (void*)toClone->GetValuePtr();
			BaseType* bt = toClone->GetRegisteredType();
			if (bt->GetImplementationId(Lua) < 0)
				v = bt->Clone(v);

			if (!bt->Push(Lua, v))
				return false;
		}
		
		
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	PushDictionary	
	// Description:	pushes a dictionary on to stack
	// Arguments:	lua state, dictionary
	// Returns:		is successful
	// --------------------------------------------------------------------------
	bool LuaHelperFunctions::PushDictionary(lua_State* to, const VariantKeyDictionary* dict)
	{
		lua_newtable(to);
		TValue* cloned = LuaGetStackValue(to, -1);
		Table* tableCloned = LuaGetTable(cloned);

		for (typename VariantKeyDictionary::VariablesConstIterator vit = dict->Begin(); vit != dict->End(); vit++)
		{
			if (PushVariant(to, vit->first))
			{
				if (PushVariant(to, vit->second))
					lua_settable(to, -3);
				else
					LuaDecStack(to);
			}
		}

		return true;
	}
	



	// --------------------------------------------------------------------------						
	// Function:	PopDictionary	
	// Description:	pops a dictionary from stack
	// Arguments:	lua state, value popped, dictionary to copy into,
	//				map of already copied items (if multiple refs)
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool LuaHelperFunctions::PopDictionary(lua_State* from, const TValue* toClone, VariantKeyDictionary* dict, ValueMap& alreadyClonedMap)
	{

		Table* tableToClone = LuaGetTable(toClone);
		ValueMap::iterator pit = alreadyClonedMap.find(tableToClone);
		if (pit != alreadyClonedMap.end())
			return false;

		alreadyClonedMap[tableToClone] = TValue();


		int grow = 6;
		EnsureFreeStack(from, grow);


		LuaSetStackValue(from, 0, toClone);
		LuaIncStack(from);


		bool resizeNoString = false;
		lua_pushnil(from);

		while (lua_next(from, -2))
		{
			char* str = NULL;
			if (LuaGetStackTypeId(from, -2) == LUA_TSTRING)
				str = (char*)lua_tostring(from, -2);

			TValue* value = LuaGetStackValue(from, -1);
			TValue* keyValue = LuaGetStackValue(from, -2);

			LuaTypeId valueType = LuaGetTypeId(value);
			LuaTypeId keyType = LuaGetTypeId(keyValue);


			if (valueType == LUA_TNIL || keyType == LUA_TNIL)
			{
				LuaDecStack(from);
			}
			else
			{
				
				Variant* key = NULL;
				NonVariant<bool> b;
				NonVariant<double> d;
				NonVariant<long long> i;
				NonVariant<std::string> s;
				NonVariant<VariantKeyDictionary> v;

				if (keyType == LUA_TTABLE)
				{
					
					if(!PopDictionary(from, keyValue, (VariantKeyDictionary*)&v.GetValue(), alreadyClonedMap))
						return false;

					key = static_cast<Variant*>(&v);
				}
				else if (keyType == LUA_TBOOLEAN)
				{
					b.GetValue() = LuaGetBoolean(keyValue);
					key = static_cast<Variant*>(&b);
				}
				else if (keyType == LUA_TNUMBER && LuaGetSubType(toClone) != LUA_VNUMFLT)
				{
					d.GetValue() = LuaGetNumber(keyValue);
					key = static_cast<Variant*>(&d);
				}
				else if (keyType == LUA_TNUMBER && LuaGetSubType(toClone) != LUA_VNUMINT)
				{
					i.GetValue() = LuaGetInteger(keyValue);
					key = static_cast<Variant*>(&i);
				}
				else if (keyType == LUA_TSTRING)
				{
					s.GetValue() = std::string(LuaGetString(keyValue));
					key = static_cast<Variant*>(&s);
				}
				else if (abs(keyType) <= (int)Registry::GetRegistry().GetLastTypeId() && keyType < 0)
				{
					void* v = LuaGetUserData(from, toClone);
					const BaseType* bt = LuaGetType(from, toClone);
					if(!bt->SetDictionary(&key, v, dict))
						return false;
				}
				else
				{
					return false;
				}

				
				bool ok = false;
				if (valueType == LUA_TTABLE)
				{
					VariantKeyDictionary v;
					ok = PopDictionary(from, value, &v, alreadyClonedMap);
					if(ok)
						dict->Set(*key, v);
				}
				else if (valueType == LUA_TBOOLEAN)
					ok = PopVariant<bool>(from, value, *key, dict, alreadyClonedMap);
				else if (valueType == LUA_TNUMBER && LuaGetSubType(toClone) != LUA_VNUMFLT)
					ok = PopVariant<double>(from, value, *key, dict, alreadyClonedMap);
				else if (valueType == LUA_TNUMBER && LuaGetSubType(toClone) != LUA_VNUMINT)
					ok = PopVariant<long long>(from, value, *key, dict, alreadyClonedMap);
				else if (valueType == LUA_TSTRING)
					ok = PopVariant<std::string>(from, value, *key, dict, alreadyClonedMap);
				else if (abs(valueType) <= (int)Registry::GetRegistry().GetLastTypeId() && valueType < 0)
				{
					void* v = LuaGetUserData(from, value);
					const BaseType* bt = LuaGetType(from, value);
					ok = bt->SetDictionary(key, v, dict);
				}
				if (!ok)
					return false;

				LuaDecStack(from);
			}

		}
		LuaDecStack(from);
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	PopDictionary	
	// Description:	pops a dictionary from stack
	// Arguments:	lua state, value popped, dictionary to copy into,
	//				map of already copied items (if multiple refs)
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool LuaHelperFunctions::PopDictionary(lua_State* from, const TValue* toClone, VariantKeyDictionary* dict)
	{
		ValueMap alreadyClonedMap;
		return PopDictionary(from, toClone, dict, alreadyClonedMap);
	}


	// --------------------------------------------------------------------------						
	// Function:	GetFunctionNames	
	// Description:	get names of all functions in a table
	// Arguments:	lua state, table, function names returned
	// Returns:		none
	// --------------------------------------------------------------------------
	void LuaHelperFunctions::GetFunctionNames(lua_State* L, TValue const* table, std::vector<std::string>& names)
	{
		LuaSetStackValue(L, 0, table);
		LuaIncStack(L);
		lua_pushnil(L);

		while (lua_next(L, -2))
		{
			TValue* key = LuaGetStackValue(L, -2);
			TValue* value = LuaGetStackValue(L, -1);

			LuaTypeId keyType = LuaGetTypeId(key);
			LuaTypeId valueType = LuaGetTypeId(value);

			std::string functionName(LuaGetString(key));
			if (valueType == LUA_TFUNCTION || keyType == LUA_TSTRING)
			{
				
				names.push_back(functionName);
			}
			LuaDecStack(L);
		}
		LuaDecStack(L);
		return;
	}


	// --------------------------------------------------------------------------						
	// Function:	OverideFunctions	
	// Description:	throws a lua erroroverrides function names in table
	// Arguments:	lua state, table, funcion names, prefix for overriddeen names  
	// Returns:		none
	// --------------------------------------------------------------------------
	void LuaHelperFunctions::OverideFunctions(lua_State* L, TValue const* table, const std::vector<std::string>& names, const std::string& overidePrefix)
	{
		LuaSetStackValue(L, 0, table);
		LuaIncStack(L);
		lua_pushnil(L);

 		while (lua_next(L, -2))
		{
			TValue* key = LuaGetStackValue(L, -2);
			TValue* value = LuaGetStackValue(L, -1);

			LuaTypeId keyType = LuaGetTypeId(key);
			LuaTypeId valueType = LuaGetTypeId(value);


			if (valueType == LUA_TFUNCTION && LuaGetFunctionType(value) == LUA_VLCL && keyType == LUA_TSTRING)
			{
				std::string functionName(LuaGetString(key));
				if (std::find(names.begin(), names.end(), functionName) != names.end())
				{
					std::string newName = overidePrefix + functionName;
					TString* newString = LuaNewString(L, newName.c_str());
					TValue* o = LuaGetStackValue(L, 0);
					LuaSetStackValue(L, 0, value);
					LuaIncStack(L);
					lua_setfield(L, -4 , newName.c_str());
 				}
			}
			LuaDecStack(L);
		}
		LuaDecStack(L);
	}


	// --------------------------------------------------------------------------						
	// Function:	ValidateFunctionNames	
	// Description:	checks function names are of allowed fornat
	// Arguments:	lua state, allow class only function names,
	//				disallowed prefixes
	// Returns:		none
	// --------------------------------------------------------------------------
	bool LuaHelperFunctions::ValidateFunctionNames(lua_State* L, bool allowClassOnlyFunctions,
		const std::string &inheritancePrefix, 
		const std::string &messagePrefix,
		const std::string& timerPrefix,
		const std::string& systemPrefix,
		const std::string& staticPrefix,
		const std::string& initializePrefix,
		const std::string& finalizePrefix,
		const std::string& updatePrefix,
		std::string& errorMessage)
	{
		StkId top = LuaGetTopStack(L);
		TValue* globals = LuaGetGlobalsValue(L);
		LuaSetStackValue(L, 0, globals);
		LuaIncStack(L);
		lua_pushnil(L);

		while (lua_next(L, -2))
		{
			TValue* key = LuaGetStackValue(L, -2);
			TValue* value = LuaGetStackValue(L, -1);

			LuaTypeId keyType = LuaGetTypeId(key);
			LuaTypeId valueType = LuaGetTypeId(value);


			if (valueType == LUA_TFUNCTION && LuaGetFunctionType(value) == LUA_VLCL && keyType == LUA_TSTRING)
			{
				std::string functionName(LuaGetString(key));


				//if (functionName.find(inheritancePrefix) != -1)
				//{
				//	errorMessage += functionName + ": Uses double underscore reserved for inheritance.";
				//	break;
				//}
				//else 
				if (functionName == messagePrefix)
				{
					if (allowClassOnlyFunctions)
						errorMessage += functionName + ": Message function must end with a message name";
					else
						errorMessage += functionName + ": Function name reserved for class system.";

					break;
				}
				else if (functionName == timerPrefix)
				{
					if (allowClassOnlyFunctions)
						errorMessage += functionName + ": Update function must end with a update name.";
					else
						errorMessage += functionName + ": Function name reserved for class system.";

					break;
				}
				else if (functionName == systemPrefix)
				{
					if (allowClassOnlyFunctions)
						errorMessage += functionName + ": System function must end with a system name.";
					else
						errorMessage += functionName + ": Function name reserved for agent system.";

					break;
				}
				else if (functionName == staticPrefix)
				{
					if (allowClassOnlyFunctions)
						errorMessage += functionName + ": Static function must end with a system name.";
					else
						errorMessage += functionName + ": Function name reserved for class system.";

					break;
				}
				else if (!allowClassOnlyFunctions && functionName == initializePrefix)
				{
					errorMessage += functionName + ": Function name reserved for class system.";
					break;
				}
				else if (!allowClassOnlyFunctions && functionName == finalizePrefix)
				{
					errorMessage += functionName + ": Function name reserved for class system.";
					break;
				}
				else if (!allowClassOnlyFunctions && functionName == updatePrefix)
				{
					errorMessage += functionName + ": Function name reserved for class system.";
					break;
				}
			}
			LuaDecStack(L);
		}
		LuaDecStack(L);
		
		if (LuaGetTopStack(L) != top)
		{
			while (LuaGetTopStack(L) != top)
				LuaDecStack(L);
			return false;
		}
		return true;
	}
}