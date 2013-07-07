/*****************************************************************************
 *	VATA Finite Automata Library
 *
 *	Copyright (c) 2013	Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *	Description:
 *	Product state set for congruence algorithm.
 *
 *****************************************************************************/


#ifndef UTIL_PRODUCT_STATESET_
#define UTIL_PRODUCT_STATESET_

// VATA headers
#include <vata/vata.hh>

// Standard library headers
#include <vector>

namespace VATA {
	template <class StateSet, class ProductState>
		class ProductStateSet;
	template <class StateSet, class ProductState>
		class ProductStateSetBreadth;
	template <class StateSet, class ProductState>
		class ProductStateSetDepth;
}

/*
 * Cache for caching macro state
 */
GCC_DIAG_OFF(effc++)
template<class StateSet,class ProductState>
class VATA::ProductStateSet : public std::vector<ProductState>
{
GCC_DIAG_ON(effc++)
private: //data types
	typedef StateSet SmallerElementType;
	typedef StateSet BiggerElementType;

public:
		bool get(SmallerElementType& smaller, BiggerElementType& bigger) {
			if (this->size() == 0) {
				return false;
			}

			auto& nextPair = this->back();
			smaller = *nextPair.first;
			bigger = *nextPair.second;
			this->pop_back();

			return true;
		}
};

GCC_DIAG_OFF(effc++)
template<class StateSet,class ProductState>
class VATA::ProductStateSetBreadth :
	public VATA::ProductStateSet<StateSet,ProductState>
{
GCC_DIAG_ON(effc++)
		public:
			void add(StateSet& smaller, StateSet& bigger) {
				this->insert(this->begin(),std::make_pair(&smaller,&bigger));
			}
};

GCC_DIAG_OFF(effc++)
template<class StateSet,class ProductState>
class VATA::ProductStateSetDepth :
	public VATA::ProductStateSet<StateSet,ProductState>
{
GCC_DIAG_ON(effc++)
		public:
			void add(StateSet& smaller, StateSet& bigger) {
				this->push_back(std::make_pair(&smaller,&bigger));
			}
};

#endif
