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
#ifdef _MSC_VER
#pragma warning( disable : 4786 4503 )
#endif


#include "../Arc/Api.h"
#include "NodeAuxilaryModule.h"	

//! /type Node
//! Schema Node functions 

namespace shh {


	// --------------------------------------------------------------------------						
	// Function:	Register
	// Description:	registers functions and variables to be used by a process
	// Arguments:	alies of module and type, dictionary of configuration vars
	// Returns:		if sucessfull
	// --------------------------------------------------------------------------
	bool NodeAuxilaryModule::Register(const std::string& alias, const StringKeyDictionary& sd, Privileges privileges)
	{
		GCPtr<Module> me(this);


		GCPtr<Node>* example = NULL;

		Api::OpenNamespace("shh");
		Api::RegisterFunction("DestroyNode", Destroy, 2, me);
		Api::RegisterFunction("DestroyChildNodes", DestroyChildNodes, 2, me);
		Api::RegisterFunction("GetChildNodes", GetChildNodes, 2, me);
		Api::CloseNamespace();

		Api::RegisterMemberFunction(example, "Destroy", Destroy, 2, me);
		Api::RegisterMemberFunction(example, "DestroyChildNodes", DestroyChildNodes, 2, me);
		Api::RegisterMemberFunction(example, "GetChildNodes", GetChildNodes, 2, me);
		Api::RegisterMemberFunction(example, "CreateInputInterface", CreateInputInterface, 4, me);
		Api::RegisterMemberFunction(example, "CreateOutputInterface", CreateOutputInterface, 4, me);
		Api::RegisterMemberFunction(example, "CreateEdge", CreateEdge, 5, me);
		Api::RegisterMemberFunction(example, "ReadInput", ReadInput, 4, me);
		Api::RegisterMemberFunction(example, "WriteOutput", WriteOutput, 5, me);
		
		return Module::Register(alias, sd, privileges);
	}


	//! /member Node
	//! /function Destroy
	//! /privilege Agent
	//! /privilege Node
	//! /returns boolean
	//! Destroys a Node and all its child nodes and returns true if successful.
	ExecutionState NodeAuxilaryModule::Destroy(GCPtr<Node>& node, bool& result)
	{
		result = false;
		GCPtr<Object> object = Scheduler::GetCurrentProcess()->GetObject();
		GCPtr<Node> caller;
		caller.DynamicCast(object);
		if (caller.IsValid())
		{
			if (caller == node || caller == node->GetParent())
			{
				node.Destroy();
				result = true;
			}
		}
		return ExecutionOk;
	}


	//! /member Node
	//! /function DestroyChildNodes
	//! /privilege Agent
	//! /privilege Node
	//! Destroys all child nodes and returns true if successful.
	ExecutionState NodeAuxilaryModule::DestroyChildNodes(GCPtr<Node>& node)
	{
		GCPtr<Object> object = Scheduler::GetCurrentProcess()->GetObject();
		GCPtr<Node> caller;
		caller.DynamicCast(object);
		if (caller.IsValid())
		{
			if (caller == node || caller == node->GetParent())
			{
				Schema::Schemas schemas = node->GetSchemas();
				for (Schema::Schemas::const_iterator it = schemas.begin(); it != schemas.end(); it++)
				{
					GCPtr<Node> node;
					node.DynamicCast(*it);
					if (node.IsValid())
						node.Destroy();
				}
			}
		}
		return ExecutionOk;
	}


	//! /member Node
	//! /function GetChildNodes
	//! /privilege Agent
	//! /privilege Node
	//! /return table_of_nodes
	//! Returns a table of all child nodes.
	ExecutionState NodeAuxilaryModule::GetChildNodes(GCPtr<Node>& node, VariantKeyDictionary& result)
	{
		GCPtr<Object> object = Scheduler::GetCurrentProcess()->GetObject();
		GCPtr<Node> caller;
		caller.DynamicCast(object);
		if (caller.IsValid())
		{
			if (caller == node || caller == node->GetParent())
			{
				unsigned int index = 1;
				const Schema::Schemas& schemas = node->GetSchemas();
				for (Schema::Schemas::const_iterator it = schemas.begin(); it != schemas.end(); it++)
				{
					GCPtr<Node> node;
					node.DynamicCast(*it);
					if (node.IsValid())
					{
						NonVariant<double> key((double)index);
						result.Set(key, node);
						index++;
					}
				}
			}
		}

		return ExecutionOk;
	}


	//! /member Node
	//! /function CreateInputInterface
	//! /param string interface_id
	//! /param integer size_of_iterface
	//! /returns boolean
	//! Creates an input interface of the given name with the given size, returns if successful.
	ExecutionState NodeAuxilaryModule::CreateInputInterface(GCPtr<Node>& node, std::string& id, unsigned int &size, bool& result)
	{
		result = false;
		GCPtr<Object> object = Scheduler::GetCurrentProcess()->GetObject();
		GCPtr<Node> caller;
		caller.DynamicCast(object);
		if (caller.IsValid())
		{
			if (caller == node)
				result = node->CreateInputInterface(id, size);
		}
		return ExecutionOk;
	}


	//! /member Node
	//! /function CreateOutputInterface
	//! /param string interface_id
	//! /param integer size_of_iterface
	//! /returns boolean
	//! Creates an output interface of the given name with the given size, returns if successful.
	ExecutionState NodeAuxilaryModule::CreateOutputInterface(GCPtr<Node>& node, std::string& id, unsigned int &size, bool& result)
	{
		result = false;
		GCPtr<Object> object = Scheduler::GetCurrentProcess()->GetObject();
		GCPtr<Node> caller;
		caller.DynamicCast(object);
		if (caller.IsValid())
		{
			if (caller == node)
				result = node->CreateOutputInterface(id, size);
		}
		return ExecutionOk;
	}


	//! /member Node
	//! /function CreateEdge
	//! /param integer id_index_of_source_node
	//! /param string source_node_interface_id
	//! /param string destination_node_interface_id
	//! /returns boolean
	//! Creates an edge from the interface of the given name in the source node to
	//! the interface of the given name in this node.
	//! A negative source index selects a node from my siblings, and positive index
	//! selects a node from my children.
	//! Return bool if successful.
	ExecutionState NodeAuxilaryModule::CreateEdge(GCPtr<Node>& node, std::string &sourceId, std::string& inputId, std::string& outputId, bool& result)
	{
		result = false;
		GCPtr<Object> object = Scheduler::GetCurrentProcess()->GetObject();
		GCPtr<Node> caller;
		caller.DynamicCast(object);
		if (caller.IsValid())
		{
			if (caller == node || caller == node->GetParent())
				result = node->CreateEdge(sourceId, inputId, outputId);
		}
		return ExecutionOk;
	}



	//! /member Node
	//! /function ReadInput
	//! /param string input_interface_id
	//! /param integer variable_within_interface
	//! /return double
	//! Returns the variable value of a given index in the input interface of the given name.
	ExecutionState NodeAuxilaryModule::ReadInput(GCPtr<Node>& node, std::string& id, unsigned int& index, double& result)
	{
		result = 0.0;
		GCPtr<Object> object = Scheduler::GetCurrentProcess()->GetObject();
		GCPtr<Node> caller;
		caller.DynamicCast(object);
		if (caller.IsValid())
		{
			if (caller == node)
				node->ReadInput(id, index, result);
		}
		return ExecutionOk;
	}


	//! /member Node
	//! /function WriteOutput
	//! /param string output_interface_id
	//! /param integer variable_within_interface
	//! /param double value_of_variable
	//! Writes the variable value to a given index in the output interface of the given name.
	ExecutionState NodeAuxilaryModule::WriteOutput(GCPtr<Node>& node, std::string& id, unsigned int& index, double& value)
	{
		GCPtr<Object> object = Scheduler::GetCurrentProcess()->GetObject();
		GCPtr<Node> caller;
		caller.DynamicCast(object);
		if (caller.IsValid())
		{
			if (caller == node)
				node->WriteOutput(id, index, value);
		}
		return ExecutionOk;
	}

}

