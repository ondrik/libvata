/*****************************************************************************
 *	VATA Finite Automata Library
 *
 *	Copyright (c) 2013	Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *	Description:
 *	Header file for applying the rules for computing of a normal form
 *	based on preorder.
 *
 *****************************************************************************/

#ifndef _VATA_UTIL_NORMAL_FOR_REL_HH__
#define _VATA_UTIL_NORMAL_FOR_REL_HH__

namespace VATA {
	template <class Rel> class NormalFormRelPreorder;
	template <class Rel> class NormalFormRelSimulation;
}

/*
 * Class is used when the identity is choosen as a preorder
 * and servers for keeping the performance because no other
 * rules can be applied in this case.
 */
template <class Rel>
class VATA::NormalFormRelPreorder {

private: // private data types
	typedef ExplicitFiniteAut ExplicitFA;
	typedef typename ExplicitFA::StateSet StateSet;
	typedef typename ExplicitFA::StateType StateType;

private: // private data members;
	Rel preorder_;

public: // public functions

	NormalFormRelPreorder(Rel preorder) : preorder_(preorder) {}

	/*
	 * It is not possible to apply any additional rules so
	 * the function does nothing
	 */
	void applyRule(StateSet& /*normalForm*/)
	{
		return;
	}
};

/*
 * Class is used for application of a rule for computation
 * of a normal form based on simulation preorder.
 */
template <class Rel>
class VATA::NormalFormRelSimulation {

private: // private data types
	typedef ExplicitFiniteAut ExplicitFA;
	typedef typename ExplicitFA::StateSet StateSet;
	typedef typename ExplicitFA::StateType StateType;

private: // private data members;
	Rel preorder_;

public: // public functions

	NormalFormRelSimulation(Rel preorder) : preorder_(preorder) {}

	/*
	 * All states that are simulated by some state which is allready
	 * in normal form are also added to this normal form.
	 */
	void applyRule(StateSet& normalForm)
	{
		for (auto& state : normalForm)
		{
			for (size_t r=0; r < preorder_.size(); r++) // for all states in simulation
			{
				if (preorder_.get(r,state))
				{
					normalForm.insert(r);
				}
			}
		}
	}
};
#endif
