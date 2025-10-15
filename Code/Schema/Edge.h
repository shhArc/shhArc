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

#ifndef EDGE_H
#define EDGE_H

#include "../Common/SecureStl.h"
#include "../Config/GCPtr.h"
#include "../Config/MemoryDefines.h"
#include <vector>
#include <string>

namespace shh
{
	class Node;

	class Edge : public GCObject
	{
		DECLARE_MEMORY_MANAGED(Edge);

		friend class Node;

	public:

		struct Spec
		{
			std::string mySourceId;
			std::string myInputId;
			std::string myOutputId;
		};

		typedef std::vector<Spec> Specs;


		Edge(const GCPtr<Node>& source, const GCPtr<Node>& destination, const std::string &sourceId, const std::string &inputId, const std::string& outputId);
		bool Update();


	protected:

		
		GCPtr<Node> mySource;
		GCPtr<Node> myDestination;
		std::string mySourceId;
		std::string myInputId;
		std::string myOutputId;
		unsigned int mySize;

	};
}

#endif