 //! /namespace Vector
//! Position or offest in a 3D world.

#ifdef _MSC_VER
#pragma warning( disable : 4786 4503 )
#endif



#include "VectorModule.h"
#include "Vector.h"		
#include "../Arc/Api.h"

namespace shh {

	
	int VectorModule::ourTypeId = -1;

	// --------------------------------------------------------------------------						
	// Function:	RegisterTypes
	// Description:	registers data types to be used by a process
	// Arguments:	alies of module and type, dictionary of configuration vars
	// Returns:		if sucessfull
	// --------------------------------------------------------------------------
	bool VectorModule::RegisterTypes(const std::string& alias, const StringKeyDictionary& sd)
	{
		Vector* example = NULL;
		ourTypeId = Api::LuaRegisterType(example, alias, Stringer, Valuer, true);
		return Module::RegisterTypes(alias, sd);
	}



	// --------------------------------------------------------------------------						
	// Function:	Register
	// Description:	registers functions and variables to be used by a process
	// Arguments:	alies of module and type, dictionary of configuration vars
	// Returns:		if sucessfull
	// --------------------------------------------------------------------------
	bool VectorModule::Register(const std::string& alias, const StringKeyDictionary& sd, Privileges privileges)
	{
		GCPtr<Module> me(this);


		Vector* example = NULL;
	
		Api::OpenNamespace("TypeCheck");
		if (Api::GetImplementation() == Lua)
			Api::LuaRegisterFunction(alias, TypeCheck);
		Api::CloseNamespace();

		Api::RegisterFunction(alias, New0, 1, me);
		Api::RegisterFunction(alias, New1, 2, me);
		Api::RegisterFunction(alias, New3, 4, me);
		Api::RegisterMemberFunction(example, "GetLength", GetLength, 2, me);
		Api::RegisterMemberFunction(example, "GetLengthSquared", GetLengthSquared, 2, me);
		Api::RegisterMemberFunction(example, "Normalise", Normalise, 2, me);
		Api::RegisterMemberFunction(example, "Inverse", Inverse, 2, me);
		Api::RegisterMemberFunction(example, "Dot", Dot, 3, me);
		Api::RegisterMemberFunction(example, "Cross", Cross, 3, me);
		Api::RegisterMemberFunction(example, "Interpolate", Interpolate, 4, me);
		Api::RegisterMemberFunction(example, "Tolerates", Tolerates, 4, me);
		Api::RegisterMemberFunction(example, "GetX", GetX, 2, me);
		Api::RegisterMemberFunction(example, "GetY", GetY, 2, me);
		Api::RegisterMemberFunction(example, "GetZ", GetZ, 2, me);
		Api::RegisterMemberFunction(example, "GetXYZ", GetXYZ, 2, me);

		Api::RegisterMemberFunction(example, "__add", MetaFuncADD, 3, me);
		Api::RegisterMemberFunction(example, "__sub", MetaFuncSUB, 3, me);
		Api::RegisterMemberFunction(example, "__mul", MetaFuncMUL, 3, me);
		Api::RegisterMemberFunction(example, "__div", MetaFuncDIV, 3, me);
		
		Registry::GetRegistry().RegisterFunctions(ourTypeId, NULL, Stringer, Valuer);

		Api::RegisterFunction("TestTable", TestTable, 2, me);

		return Module::Register(alias, sd, privileges);
	}



	// --------------------------------------------------------------------------						
	// Function:	Stringer
	// Description:	converts a value of type used by this module to a string
	//				for printing
	// Arguments:	void pointer to variable of type used by this module
	// Returns:		string
	// --------------------------------------------------------------------------
	std::string VectorModule::Stringer(void const* const data)
	{
		Vector* v = (Vector*)data;
		char temp[50];
		std::string format = Type<Vector>::GetTypeName() + "(%f,%f,%f)";
		_snprintf(temp, 49, format.c_str(), v->myX, v->myY, v->myZ);
		return std::string(temp);
	}



	// --------------------------------------------------------------------------						
	// Function:	Valuer
	// Description:	converts a string to the tyoe used by this module
	// Arguments:	string to convert, pointer to newed data, type of the
	//				newed data
	// Returns:		if successfull
	// --------------------------------------------------------------------------
	bool VectorModule::Valuer(const std::string& format, void*& data, int& type)
	{
		Vector* v = new Vector();
		int start = (int)format.find("(") + 1;
		int finalend = (int)format.find(")") - 1;
		std::string datastring = format.substr(start, finalend);
		int end = (int)datastring.find(",");
		v->myX = std::stof(datastring.substr(0, end - 1));
		datastring = datastring.substr(end + 1, datastring.size());
		end = (int)datastring.find(",");
		v->myY = std::stof(datastring.substr(0, end - 1));
		datastring = datastring.substr(end + 1, datastring.size());
		v->myZ = std::stof(datastring);

		data = v;
		type = Type<Vector>::GetTypeId();
		return true;
	}


	 //! /namespace TypeCheck
	//! /function Vector
	//! /param variable
	//! /returns boolean
	//! Returns whether variable is a Vector
	int VectorModule::TypeCheck(lua_State* L)
	{
		Api::LuaCheckNumArguments(L, 1);
		bool ok = Api::LuaGetArgumentType(L, 1) == ourTypeId;
		//Return(L, ok);
		return 1;
	}

	 //! /namespace Vector
	//! /function Vector
	//! /returns Vector
	//! Returns a newly created vector
	ExecutionState VectorModule::New0(Vector& result)
	{
		return ExecutionOk;
	}


	 //! /namespace Vector
	//! /function Vector
	//! /param vector to copy
	//! /returns Vector
	//! Returns a newly created vector
	ExecutionState VectorModule::New1(Vector& toCopy, Vector& result)
	{
		result = toCopy;
		return ExecutionOk;
	}

	 //! /namespace Vector
	//! /function Vector
	//! /param x
	//! /param y
	//! /param z
	//! /returns Vector
	//! Returns a newly created vector
	ExecutionState VectorModule::New3(float& x, float& y, float& z, Vector& result)
	{
		result.myX = x;
		result.myY = y;
		result.myZ = z;
 		return ExecutionOk;
	}


	 //! /namespace Vector
	//! /function GetLength
	//! /returns length
	//! Returns a newly created vector
	ExecutionState VectorModule::GetLength(Vector& v, float& result)
	{
		result = v.GetLength();
		return ExecutionOk;
	}


	 //! /namespace Vector
	//! /function GetLengthSquared
	//! /returns length
	//! Returns a newly created vector
	ExecutionState VectorModule::GetLengthSquared(Vector& v, float& result)
	{
		result = v.GetLengthSquared();
		return ExecutionOk;
	}


	 //! /namespace Vector
	//! /function Normalise
	//! /returns this
	//! normalises vector
	ExecutionState VectorModule::Normalise(Vector& v, Vector& result)
	{
		result = v;
		result.Normalise();
		return ExecutionOk;
	}


	 //! /namespace Vector
	//! /function Inverse
	//! /returns this
	//! inverts vector
	ExecutionState VectorModule::Inverse(Vector& v, Vector& result)
	{
		result = v;
		result.Inverse();
		return ExecutionOk;
	}

	 //! /namespace Vector
	//! /function Dot
	//! /param other
	//! /returns dot product
	//! calculated dot product
	ExecutionState VectorModule::Dot(Vector& v1, Vector& v2, float& result)
	{
		result = v1.Dot(v2);
		return ExecutionOk;
	}


	 //! /namespace Vector
	//! /function Cross
	//! /param other
	//! /returns cross product
	//! calculated cross product
	ExecutionState VectorModule::Cross(Vector& v1, Vector& v2, Vector& result)
	{
		result.Cross(v1,v2);
		return ExecutionOk;
	}


	 //! /namespace Vector
	//! /function Interpolate
	//! /param vector a
	//! /param vector b
	//! /param range
	//! /returns this
	//! Set this vector between vectors a & b, t: range: 0.0..1.0, 0.0 -> this = a, 1.0 -> this = b.
	ExecutionState VectorModule::Interpolate(Vector& v1, Vector& v2, float& n, Vector& result)
	{
		result.Interpolate(v1, v2, n);
		return ExecutionOk;
	}

	 //! /namespace Vector
	//! /function Tolerates
	//! /param vector a
	//! /param vector b
	//! /param tollerance
	//! /returns boolean
	//! Tests if two vectors are within tollerance of each other
	ExecutionState VectorModule::Tolerates(Vector& v1, Vector& v2, float& t, bool& result)
	{
		result = v1.Equals(v2, t);
		return ExecutionOk;
	}

	ExecutionState VectorModule::GetX(Vector& v, float& result)
	{
		result = v.myX;
		return ExecutionOk;
	}

	ExecutionState VectorModule::GetY(Vector& v, float& result)
	{
		result = v.myY;
		return ExecutionOk;
	}

	ExecutionState VectorModule::GetZ(Vector& v, float& result)
	{
		result = v.myZ;
		return ExecutionOk;
	}


	ExecutionState VectorModule::GetXYZ(Vector& v, float& x, float& y, float& z)
	{
		x = v.myX;
		y = v.myY;
		z = v.myZ;
		return ExecutionOk;
	}

	// **** meta functions ******************************************************************************

	ExecutionState VectorModule::MetaFuncADD(Vector& v1, Vector& v2, Vector& result)
	{
		result = v1;
		result += v2;
		return ExecutionOk;
	}


	ExecutionState VectorModule::MetaFuncSUB(Vector& v1, Vector& v2, Vector& result)
	{
		result = v1;
		v1 -= v2;
		return ExecutionOk;
	}

	ExecutionState VectorModule::MetaFuncMUL(Vector& v, float& n, Vector& result)
	{
		result = v;
		result *= n;
		return ExecutionOk;
	}
	ExecutionState VectorModule::MetaFuncDIV(Vector& v, float& n, Vector& result)
	{
		result = v;
		result /= n;
		return ExecutionOk;
	}

	ExecutionState VectorModule::TestTable(VariantKeyDictionary& dict, VariantKeyDictionary& retVal)
	{
		//NonVariant<Dictionary<Variant>> dict;
		NonVariant<std::string> k(std::string("dave"));
		NonVariant<Vector> v(Vector(1, 2, 3));
		Variant* key = static_cast<Variant*>(&k);
		Variant* value = static_cast<Variant*>(&v);
		retVal.Set(*key, *value);
		//PushLuaVariant(L, &dict);
		return ExecutionOk;
	}


}