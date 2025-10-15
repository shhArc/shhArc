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

#include "../Common/Debug.h"
#include "Edge.h"
#include "Node.h"


namespace shh
{
	IMPLEMENT_MEMORY_MANAGED(Edge);

	// --------------------------------------------------------------------------						
	// Function:	Edge
	// Description:	constructor
	// Arguments:	source node, destination node, id index used to locate this
	//				edge source node, input interface id, output interface id
	//				interface id
	// Returns:		none
	// --------------------------------------------------------------------------
	Edge::Edge(const GCPtr<Node> &source, const GCPtr<Node> &destination, const std::string &sourceId, const std::string &inputId, const std::string &outputId) :
		mySource(source),
		myDestination(destination),
		mySourceId(sourceId),
		myInputId(inputId),
		myOutputId(outputId)
	{
		SetGCMemoryStart(this);
		mySize = (unsigned int)myDestination->myOutputs[outputId].size();
		if (mySource->myInputs[inputId].size() < mySize)
		{
			mySize = (unsigned int)mySource->myInputs[inputId].size();
			std::string error = "Edge source size " + std::to_string(mySource->myInputs[inputId].size()) + " is bigger than destination size " + std::to_string(myDestination->myOutputs[outputId].size()) + ".\n";
			ERROR_TRACE(error.c_str());
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	Update
	// Description:	transfters the data from input to outpit
	// Arguments:	none
	// Returns:		if source and destination exists (and was updated
	// --------------------------------------------------------------------------
	bool Edge::Update()
	{
		if(!mySource.IsValid())
			return false;

		Node::Interface::iterator source = mySource->myOutputs.find(myInputId);
		if (source == mySource->myOutputs.end())
			return false;

		Node::Interface::iterator dest = myDestination->myInputs.find(myOutputId);
		if (dest == myDestination->myInputs.end())
			return false;

		Node::Counters::iterator counter = myDestination->myCounters.find(myOutputId);


		for (unsigned int idx = 0; idx < mySize; idx++)
		{
			dest->second[idx] += source->second[idx];
			counter->second[idx]++;
		}
		return true;
	}
}