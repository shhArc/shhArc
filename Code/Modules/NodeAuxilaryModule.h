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

#ifndef NODEAUXILARYMODULE_H
#define NODEAUXILARYMODULE_H


#include "../Common/SecureStl.h"
#include "../Arc/Module.h"
#include "../Schema/Node.h"
#include "../Common/Dictionary.h"

namespace shh {

	class NodeAuxilaryModule : public Module
	{
	public:

		virtual bool Register(const std::string& alias, const StringKeyDictionary& sd, Privileges privileges);
		virtual std::string GetName() const { return GetNameStatic(); }
		static std::string GetNameStatic() { return StripName(std::string(typeid(NodeAuxilaryModule).name())); }


		static ExecutionState Destroy(GCPtr<Node>& node, bool& result);
		static ExecutionState DestroyChildNodes(GCPtr<Node>& node);
		static ExecutionState GetChildNodes(GCPtr<Node>& node, VariantKeyDictionary& result);
		static ExecutionState GetNodes(VariantKeyDictionary& dict);
		static ExecutionState CreateInputInterface(GCPtr<Node>& node, std::string& id, unsigned int &size, bool& result);
		static ExecutionState CreateOutputInterface(GCPtr<Node>& node, std::string& id, unsigned int &size, bool& result);
		static ExecutionState CreateEdge(GCPtr<Node>& node, std::string &sourceId, std::string& inputId, std::string& outputId, bool& result);
		static ExecutionState ReadInput(GCPtr<Node>& node, std::string& id, unsigned int& index, double& result);
		static ExecutionState WriteOutput(GCPtr<Node>& node, std::string& id, unsigned int& index, double& value);


	};

};


#endif