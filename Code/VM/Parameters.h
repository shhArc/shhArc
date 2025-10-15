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


#ifndef PARAMETERS_H
#define PARAMETERS_H


#include "../Common/SecureStl.h"
#include "../Common/TypeList.h"
#include "../Common/Dictionary.h"
#include "../Arc/Registry.h"

namespace shh {

	// Compile time type list ///////////////////////////////////////////////

	template< class T >
	const std::type_info& HeadType(int length, const T* = 0)
	{
		static const std::type_info& typeInfo = typeid(typename T::Head);
		if (Length<T>::value == length)
			return typeInfo;

		return HeadType< typename T::Tail >(length, 0);
	}
	const std::type_info& HeadType(int length, NullType*);


	// Parameters /////////////////////////////////////////////////////////////

	template < typename F, typename TL>
	class Parameters
	{

	public:

		typedef TL Arguments;
		typedef F Function;
		enum
		{
			NumArgs = Length<Arguments>::value,
		};

		const unsigned int myFirstReturnArg;

		Parameters(F f, unsigned int r) : myFirstReturnArg(r) {}

		const Registry::OverloadTable::ArgumentTypes GetSendTypeIds() const
		{
			Registry::OverloadTable::ArgumentTypes args;

			args.resize(myFirstReturnArg - 1);
			for (unsigned int a = 0; a < myFirstReturnArg - 1; a++)
			{
				std::string name = std::string(TypeInfo(a).name());
				int id = Registry::GetRegistry().GetTypeId(name);
				if (id == 0)
					RELEASE_ASSERT(false);

				args[a] = id;
			}
			return args;
		}
		virtual unsigned int GetFirstReturnArg() const { return myFirstReturnArg; }

		// this constructor should not be called, used cos template of scope requires it
		// for default constructor though it is never called, hence scope is a friend
		Parameters() : myFirstReturnArg(0) {}


		const std::type_info& TypeInfo(int arg) const
		{
			int length = NumArgs - arg;
			static TL* dummy = 0;
			return HeadType<TL>(length, dummy);
		}

	};

}
#endif