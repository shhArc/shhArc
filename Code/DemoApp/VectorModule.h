//--------------------------------------------------------------------
// Filename:    VectorModule
// Class:       VectorModule
// Purpose:     vm namespaceed vector functions and data type
// Description: Wraps up engine vector class for use in scripts
//
// Usage:		register class with vm or agent manager
//				world or governor for use in respective systems
//
// History:
// 2002 DKB Initial version
//--------------------------------------------------------------------

#ifndef VECTORMODULE_H
#define VECTORMODULE_H


#include "../Common/SecureStl.h"
#include "../Arc/Module.h"
#include "../Common/Dictionary.h"
#include <Lua/src/lstate.h>

namespace shh {

	class Vector;

	class VectorModule : public Module
	{
	public:

		virtual bool RegisterTypes(const std::string& alias, const StringKeyDictionary& sd);
		virtual bool Register(const std::string& alias, const StringKeyDictionary& sd, Privileges privileges);
		virtual std::string GetName() const { return GetNameStatic(); }
		static std::string GetNameStatic() { return StripName(std::string(typeid(VectorModule).name())); }

		static std::string Stringer(void const* const data);
		static bool Valuer(const std::string& format, void*& data, int& type);

		static int TypeCheck(lua_State* L);
		static ExecutionState New0(Vector& result);
		static ExecutionState New1(Vector& toCopy, Vector& result);
		static ExecutionState New3(float& x, float& y, float& z, Vector& result);
		static ExecutionState GetLength(Vector& v, float& result);
		static ExecutionState GetLengthSquared(Vector& v, float& result);
		static ExecutionState Normalise(Vector& v, Vector& result);
		static ExecutionState Inverse(Vector& v, Vector& result);
		static ExecutionState Dot(Vector& v1, Vector& v2, float& result);
		static ExecutionState Cross(Vector& v1, Vector& v2, Vector& result);
		static ExecutionState Interpolate(Vector& v1, Vector& v2, float& n, Vector& result);
		static ExecutionState Tolerates(Vector& v1, Vector& v2, float& t, bool& result);
		static ExecutionState GetX(Vector& v, float& result);
		static ExecutionState GetY(Vector& v, float& result);
		static ExecutionState GetZ(Vector& v, float& result);
		static ExecutionState GetXYZ(Vector& v, float& x, float& y, float& z);
		static ExecutionState MetaFuncADD(Vector& v1, Vector& v2, Vector& result);
		static ExecutionState MetaFuncSUB(Vector& v1, Vector& v2, Vector& result);
		static ExecutionState MetaFuncMUL(Vector& v, float& n, Vector& result);
		static ExecutionState MetaFuncDIV(Vector& v, float& n, Vector& result);
		
		static ExecutionState TestTable(VariantKeyDictionary& dict, VariantKeyDictionary& retVal);

	private:

		static int ourTypeId;

	};
}


#endif	//VECTORMODULE_H