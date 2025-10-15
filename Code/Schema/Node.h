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

#ifndef NODE_H
#define NODE_H

#include "../Common/SecureStl.h"
#include "../Config/MemoryDefines.h"
#include "../VM/Object.h"
#include "../VM/Message.h"
#include "Edge.h"
#include "Schema.h"
#include <vector>
#include <map>
#include <string>

#ifdef _MSC_VER
#pragma warning( disable : 4250)
#endif

namespace shh
{
	class Process;
	class Class;
	class ClassManager;

	class Node : public Object, public Schema
	{
		DECLARE_MEMORY_MANAGED(Node);
		friend class Edge;

	public:

		typedef std::map<std::string, std::vector<double>> Interface;
		typedef std::map<std::string, std::vector<unsigned int>> Counters;
		typedef std::vector<GCPtr<Edge>> Edges;

		static GCPtr<Object> Create(const GCPtr<ClassManager>& classManager, const GCPtr<Class>& objectClass, const GCPtr<Process>& process);

		Node(const GCPtr<ClassManager>& classManager, const GCPtr<Class>& objectClass, const GCPtr<Process>& process);
		virtual ~Node();

		virtual bool RequiresConfiguration() const;
		virtual bool Configure(const StringKeyDictionary& config);
		bool CreateInputInterface(const std::string &id, unsigned int size);
		bool CreateOutputInterface(const std::string &id, unsigned int size);
		bool ReadInput(const std::string& id, unsigned int index, double& value);
		bool WriteOutput(const std::string& id, unsigned int index, double value);
		bool CreateEdge(const std::string &sourceId, const std::string& inputId, const std::string& outputId);

		virtual bool Initialize(const GCPtr<GCObject>& owner, const std::string& id, const StringKeyDictionary& sd);
		virtual bool PostInitialization();
		virtual bool Update(double until, unsigned int phase);



	protected:

		Edge::Specs myEdgeSpecs;

		Edges myEdges;

		Interface myInputs;
		Interface myOutputs;
		Counters myCounters;
		
	};
}

#endif