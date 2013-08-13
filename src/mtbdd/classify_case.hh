/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    The function for classification of a case of MTBDD traversal
 *
 *****************************************************************************/

#ifndef _VATA_CLASSIFY_CASE_HH_
#define _VATA_CLASSIFY_CASE_HH_

// VATA headers
#include	<vata/vata.hh>

namespace VATA
{
	namespace MTBDDPkg
	{
		const char NODE1MASK = 0x01;  // 00000001
		const char NODE2MASK = 0x02;  // 00000010

		template <class Node1PtrType, class Node2PtrType>
		char classifyCase2(const Node1PtrType& node1,
			const Node2PtrType& node2);
	}
}

template <class Node1PtrType, class Node2PtrType>
inline char VATA::MTBDDPkg::classifyCase2(const Node1PtrType& node1,
	const Node2PtrType& node2)
{
	// Assertions
	assert(!IsNull(node1));
	assert(!IsNull(node2));

	char result = 0x00;

	if (IsInternal(node1))
	{	// node1 is internal
		if (IsLeaf(node2) ||
			(GetVarFromInternal(node1) >= GetVarFromInternal(node2)))
		{
			result |= NODE1MASK;	// branch node1
		}
	}

	if (IsInternal(node2))
	{	// node2 is internal
		if (IsLeaf(node1) ||
			(GetVarFromInternal(node2) >= GetVarFromInternal(node1)))
		{
			result |= NODE2MASK;	// branch node2
		}
	}

	return result;
}
#endif
