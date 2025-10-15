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

#include "../VM/SoftProcess.h"
#include "Node.h"

namespace shh
{
	IMPLEMENT_MEMORY_MANAGED(Node);


	// --------------------------------------------------------------------------						
	// Function:	Create
	// Description:	creates a node
	// Arguments:	class manager to use, class node is of, process attached to
	//				this node
	// Returns:		node
	// --------------------------------------------------------------------------
	GCPtr<Object> Node::Create(const GCPtr<ClassManager>& classManager, const GCPtr<Class>& objectClass, const GCPtr<Process>& process)
	{
		GCPtr<Node> node(new Node(classManager, objectClass, process));
		GCPtr<Object> object;
		object.DynamicCast(node);
		return object;
	}


	// --------------------------------------------------------------------------						
	// Function:	Node
	// Description:	constructor
	// Arguments:	class manager to use, class node is of, process attached to
	//				this node
	// Returns:		none
	// --------------------------------------------------------------------------
	Node::Node(const GCPtr<ClassManager>& classManager, const GCPtr<Class>& objectClass, const GCPtr<Process>& process) :
		Object(classManager, objectClass, process),
		Schema()
	{
		SetGCMemoryStart(this);
	}

	// --------------------------------------------------------------------------						
	// Function:	~Node
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Node::~Node()
	{ 
		for (Edges::iterator it = myEdges.begin(); it != myEdges.end(); it++)
			it->Destroy();
		myEdges.clear();

	};



	// --------------------------------------------------------------------------						
	// Function:	RequiresConfiguration
	// Description:	flag whether with schema requires configuration on creation
	// Arguments:	none
	// Returns:		flag
	// --------------------------------------------------------------------------
	bool Node::RequiresConfiguration() const
	{ 
		return true; 
	}


	// --------------------------------------------------------------------------						
	// Function:	Configure
	// Description:	use the config to creates interfaces now, and logs edges which
	//				will be created in post initialization
	// Arguments:	configurations
	// Returns:		none
	// --------------------------------------------------------------------------
	bool Node::Configure(const StringKeyDictionary& config) 
	{ 
		StringKeyDictionary inputs;
		inputs = config.Get("inputs", inputs);
		for (StringKeyDictionary::VariablesConstIterator it = inputs.Begin(); it != inputs.End(); it++)
		{
			long size;
			if (it->second->Get(size))
			{
				if (!CreateInputInterface(*it->first, size))
					return false;
			}
			else
			{
				return false;
			}
		}

		StringKeyDictionary outputs;
		outputs = config.Get("outputs", outputs);
		for (StringKeyDictionary::VariablesConstIterator it = outputs.Begin(); it != outputs.End(); it++)
		{
			long size;
			if (it->second->Get(size))
			{
				if (!CreateOutputInterface(*it->first, size))
					return false;
			}
			else
			{
				return false;
			}
		}


		ArrayKeyDictionary edges;
		edges = config.Get("edges", edges);
		for (ArrayKeyDictionary::VariablesConstIterator it = edges.Begin(); it != edges.End(); it++)
		{
			StringKeyDictionary edge;
			if (it->second->Get(edge))
			{
				std::string sourceId;
				sourceId = edge.Get("source", sourceId);
				std::string inputId;
				inputId = edge.Get("input", inputId);
				std::string outputId;
				outputId = edge.Get("output", outputId);
				if (!CreateEdge(sourceId, inputId, outputId))
					return false;
			}
			else
			{
				return false;
			}
		}

		return true; 
	}

	// --------------------------------------------------------------------------						
	// Function:	CreateInputInterface
	// Description:	created an input interface
	// Arguments:	string id to give interface, size of interface
	// Returns:		none
	// --------------------------------------------------------------------------
	bool Node::CreateInputInterface(const std::string &id, unsigned int size)
	{
		if (myInputs.find(id) == myInputs.end())
		{
			myInputs[id] = std::vector<double>();
			myCounters[id] = std::vector<unsigned int>();
			myInputs[id].resize(size);
			myCounters[id].resize(size);
			return true;
		}
		return false;
	}



	// --------------------------------------------------------------------------						
	// Function:	CreateOutputInterface
	// Description:	created an output interface
	// Arguments:	string id to give interface, size of interface
	// Returns:		none
	// --------------------------------------------------------------------------
	bool Node::CreateOutputInterface(const std::string &id, unsigned int size)
	{
		if (myOutputs.find(id) == myOutputs.end())
		{
			myOutputs[id] = std::vector<double>();
			myOutputs[id].resize(size);
			return true;
		}
		return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	ReadInput
	// Description:	reads a value from the input interface
	// Arguments:	string id to give interface, index of element in interface to
	//				read, value returned
	// Returns:		if element exists
	// --------------------------------------------------------------------------
	bool Node::ReadInput(const std::string& id, unsigned int index, double &value)
	{
		Interface::iterator it = myInputs.find(id);
		if (it != myInputs.end())
		{
			if (it->second.size() > index)
			{
				value = it->second[index];
				return true;
			}
		}
		return false;
	}



	// --------------------------------------------------------------------------						
	// Function:	WriteOutput
	// Description:	writes a value from the output interface
	// Arguments:	string id to give interface, index of element in interface to
	//				write, value to write
	// Returns:		if element exists
	// --------------------------------------------------------------------------
	bool Node::WriteOutput(const std::string& id, unsigned int index, double value)
	{
		Interface::iterator it = myOutputs.find(id);
		if (it != myOutputs.end())
		{
			if (it->second.size() > index)
			{
				it->second[index] = value;
				return true;
			}
		}
		return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	CreateEdge
	// Description:	creates an edge between an output interface of a source node 
	//				and an input interface of this node
	// Arguments:	source node id. source node interface id string,
	//				this nodes interface
	// Returns:		if sucessful
	// --------------------------------------------------------------------------
	bool Node::CreateEdge(const std::string& sourceId, const std::string &inputId, const std::string &outputId)
	{
	
		for (unsigned int e = 0; e != myEdges.size(); e++)
		{
			if (myEdges[e]->mySourceId == sourceId && myEdges[e]->myInputId == inputId && myEdges[e]->myOutputId == outputId)
				return false;
		}
		for (unsigned int e = 0; e != myEdgeSpecs.size(); e++)
		{
			if (myEdgeSpecs[e].mySourceId == sourceId && myEdgeSpecs[e].myInputId == inputId && myEdgeSpecs[e].myOutputId == outputId)
				return false;
		}
		Edge::Spec spec;
		spec.mySourceId = sourceId;
		spec.myInputId = inputId;
		spec.myOutputId = outputId;
		myEdgeSpecs.push_back(spec);
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	Initialize
	// Description:	intializes the node after creation
	// Arguments:	owner of this nide, identifier for this noide, configuration
	//				dictionary
	// Returns:		none
	// --------------------------------------------------------------------------
	bool Node::Initialize(const GCPtr<GCObject>& owner, const std::string &id, const StringKeyDictionary& sd)
	{
		const GCPtr<VM>& vm = Scheduler::GetCurrentProcess()->GetVM();
		vm->AddSlaveProcess(myProcess);
		return Object::Initialize(owner, id, sd);
	}


	// --------------------------------------------------------------------------						
	// Function:	PostInitialization
	// Description:	calls after initialization (and soft Initialize function) are
	//				called in order to create edges that were registered to be 
	//				created
	// Arguments:	none
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool Node::PostInitialization()
	{
		for (unsigned int e = 0; e != myEdgeSpecs.size(); e++)
		{
			bool ok = true;

			const Schemas *  schemas = NULL;
			
			std::string schemaId = myEdgeSpecs[e].mySourceId;
			if (schemaId.find("CHILD__") == 0)
			{
				schemaId = schemaId.substr(7);
				schemas = &mySchemas;
			}
			else
			{
				if (myParent.IsValid())
					schemas = &(myParent->GetSchemas());
				else
					ok = false;
			}
			

			

			if (ok)
			{
				for (unsigned int e = 0; e != myEdges.size(); e++)
				{
					if (myEdges[e]->mySourceId == myEdgeSpecs[e].mySourceId && myEdges[e]->myInputId == myEdgeSpecs[e].myInputId && myEdges[e]->myOutputId == myEdgeSpecs[e].myOutputId)
					{
						ok = false;
						break;
					}
				}
				
				if (ok)
				{
					for (int s = 0; s != schemas->size(); s++)
					{
						if ((*schemas)[s]->GetName() == schemaId)
						{
							GCPtr<Node> source;
							source.DynamicCast((*schemas)[s]);
							if (source.IsValid())
							{
								GCPtr<Node> destination(this);
								GCPtr<shh::Edge> edge(new Edge(source, destination, myEdgeSpecs[e].mySourceId, myEdgeSpecs[e].myInputId, myEdgeSpecs[e].myOutputId));
								myEdges.push_back(edge);
							}
						}
					}
				}
				
			}
		}
		myEdgeSpecs.clear();
		
		return Object::PostInitialization();
	}


	// --------------------------------------------------------------------------						
	// Function:	Update
	// Description:	updates the node, clear interfaces and updates edges
	// Arguments:	time to update until, phase of update
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool Node::Update(double until, unsigned int phase)
	{
		if (myProcess->GetState() == ExecutionReady)
		{
			for (Interface::iterator it = myOutputs.begin(); it != myOutputs.end(); it++)
			{
				for (int i = 0; i != it->second.size(); i++)
					it->second[i] = 0.0;
			}

			if (myEdges.size() > 0)
			{
				for (Interface::iterator it = myInputs.begin(); it != myInputs.end(); it++)
				{
					for (int i = 0; i != it->second.size(); i++)
						it->second[i] = 0.0;
				}
				for (Counters::iterator cit = myCounters.begin(); cit != myCounters.end(); cit++)
				{
					for (int i = 0; i != cit->second.size(); i++)
						cit->second[i] = 0;
				}

				Edges::iterator it = myEdges.end();
				it--;
				for (unsigned int e = (unsigned int)myEdges.size(); e > 0; e--)
				{
					Edges::iterator next = it;
					if(next != myEdges.begin())
						next--;
					if (!(*it)->Update())
						myEdges.erase(it);
					if (it != myEdges.begin())
						it--;
				}


				Counters::iterator cit = myCounters.begin();
				for (Interface::iterator it = myInputs.begin(); it != myInputs.end(); it++, cit++)
				{
					for (int i = 0; i != it->second.size(); i++)
						it->second[i] /= cit->second[i];
				}
			}
			return Object::Update(until, phase);
		}
		return false;
	}
}