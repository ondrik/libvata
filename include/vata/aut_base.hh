/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for base classes of automata.
 *
 *****************************************************************************/

#ifndef _VATA_AUT_BASE_HH_
#define _VATA_AUT_BASE_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/notimpl_except.hh>
#include <vata/parsing/abstr_parser.hh>
#include <vata/serialization/abstr_serializer.hh>
#include <vata/symbolic.hh>

// Utilities
#include <vata/util/binary_relation.hh>
#include <vata/util/two_way_dict.hh>
#include <vata/util/transl_weak.hh>
#include <vata/util/transl_strict.hh>


namespace VATA
{
	class AutBase;
	class TreeAutBase;
	class SymbolicTreeAutBase;

	template <class Automaton>
	Automaton GetCandidateTree(const Automaton&)
	{
		throw NotImplementedException(__func__);
	}

	/**
	 * @brief  Generic procedure for checking equivalence of automata
	 *
	 * To be used when a specific implementation is not available
	 */
	template <class Automaton>
	bool CheckEquivalence(const Automaton& lhs, const Automaton& rhs)
	{
		return CheckInclusion(lhs, rhs) && CheckInclusion(rhs, lhs);
	}
}


/**
 * @brief  Base class for automata
 *
 * This is the base class for automata. Contains  mostly static definitions, no
 * virtual methods are present (so that the overhead of virtual method table
 * would not propagate.
 */
class VATA::AutBase
{
public:   // data types

	using StateType      = size_t;
	using StateHT        = std::unordered_set<StateType>;
	using FinalStateSet  = std::unordered_set<StateType>;

	using AutDescription             = VATA::Util::AutDescription;
	using StateDict                  = Util::TwoWayDict<std::string, StateType>;
	using StringToStateTranslWeak    = Util::TranslatorWeak<StateDict>;
	using StringToStateTranslStrict  = Util::TranslatorStrict<StateDict>;
	using StateBackTranslStrict      = Util::TranslatorStrict<StateDict::MapBwdType>;

	using StateToStateMap         = std::unordered_map<StateType, StateType>;
	using StateToStateTranslWeak  = Util::TranslatorWeak<StateToStateMap>;
	using StateToStateTranslStrict= Util::TranslatorStrict<StateToStateMap>;
	using StateToStringConvFunc   = std::function<std::string(const StateType&)>;

	using StatePair = std::pair<StateType, StateType>;

	using ProductTranslMap =
		std::unordered_map<StatePair, StateType, boost::hash<StatePair>>;

	using StateBinaryRelation = Util::BinaryRelation;
	using StateDiscontBinaryRelation = Util::DiscontBinaryRelation;

protected:// methods

	AutBase() { }

public:   // methods

	template <class Automaton>
	static StateType SanitizeAutsForInclusion(
		Automaton&     smaller,
		Automaton&     bigger)
	{
		StateType stateCnt = 0;
		StateToStateMap stateMap;
		StateToStateTranslWeak stateTrans(stateMap,
			[&stateCnt](const StateType&){return stateCnt++;});

		Automaton tmpAut = smaller.RemoveUselessStates();
		Automaton newSmaller = tmpAut.ReindexStates(stateTrans);

		tmpAut = bigger.RemoveUselessStates();
		stateMap.clear();
		Automaton newBigger = tmpAut.ReindexStates(stateTrans);

		smaller = newSmaller;
		bigger = newBigger;

		return stateCnt;
	}
};

class VATA::TreeAutBase : public AutBase
{
protected:// methods

	TreeAutBase() { }

public:   // data types

	using StateTuple     = std::vector<StateType>;

protected:// data types

	template <
		class TSymbol>
	class TTransition
	{
	private:  // data types

		using SymbolType = TSymbol;

	private:  // data members

		StateType parent_;
		SymbolType symbol_;
		StateTuple children_;

	public:

		TTransition() :
			parent_(),
			symbol_(),
			children_()
		{ }

		TTransition(
			const StateType&      parent,
			const SymbolType&     symbol,
			const StateTuple&     children) :
			parent_(parent),
			symbol_(symbol),
			children_(children)
		{ }

		const StateType&  GetParent()           const { return parent_;   }
		const SymbolType& GetSymbol()           const { return symbol_;   }
		const StateTuple& GetChildren()         const { return children_; }
        size_t            GetChildrenSize()     const { return children_.size();}
        size_t            GetNthChildren(int n) const { return children_[n];}

		bool operator<(const TTransition& rhs) const
		{
			if (parent_ < rhs.parent_)
			{
				return true;
			}
			else if (parent_ == rhs.parent_)
			{
				if (symbol_ < rhs.symbol_)
				{
					return true;
				}
				else if (symbol_ == rhs.symbol_)
				{
					if (children_.size() != rhs.children_.size())
					{
						return children_.size() < rhs.children_.size();
					}
					else
					{
						for (size_t i = 0; i < children_.size(); ++i)
						{
							if (children_[i] != rhs.children_[i])
							{
								return children_[i] < rhs.children_[i];
							}
						}
					}
				}
			}

			return false;
		}


		bool operator==(const TTransition& rhs) const
		{
			if ((parent_ == rhs.parent_) && (symbol_ == rhs.symbol_))
			{
				if (children_.size() == rhs.children_.size())
				{
					size_t i;
					for (i = 0; i < children_.size(); ++i)
					{
						if (children_[i] != rhs.children_[i])
						{
							break;
						}
					}

					if (children_.size() == i)
					{
						return true;
					}
				}
			}

			return false;
		}

		bool operator!=(const TTransition& rhs) const
		{
			return !(*this == rhs);
		}

		friend std::ostream& operator<<(
			std::ostream&              os,
			const TTransition&         trans)
		{
			os << trans.GetSymbol();

			os << "(";

			for (auto it = trans.GetChildren().cbegin();
				it != trans.GetChildren().cend(); ++it)
			{
				if (it != trans.GetChildren().cbegin())
				{
					os << ", ";
				}

				os << *it;
			}

			os << ") -> ";
			os << trans.GetParent();

			return os;
		}
	};
};


class VATA::SymbolicTreeAutBase :
	public TreeAutBase,
	public Symbolic
{
public:   // data types

	using StringSymbolType = std::string;

	using SymbolDict                      = Util::TwoWayDict<std::string, SymbolType>;
	using StringSymbolToSymbolTranslStrict= Util::TranslatorStrict<SymbolDict>;
	using StringSymbolToSymbolTranslWeak  = Util::TranslatorWeak<SymbolDict>;
	using SymbolBackTranslStrict          =
		Util::TranslatorStrict<typename SymbolDict::MapBwdType>;

	// TODO: merge with ExplicitTreeAut
	class AbstractAlphabet
	{
	public:  // data types

		using FwdTranslator    = VATA::Util::AbstractTranslator<StringSymbolType, SymbolType>;
		using FwdTranslatorPtr = std::unique_ptr<FwdTranslator>;
		using BwdTranslator    = VATA::Util::AbstractTranslator<SymbolType, StringSymbolType>;
		using BwdTranslatorPtr = std::unique_ptr<BwdTranslator>;

	public:  // methods

		virtual FwdTranslatorPtr GetSymbolTransl() = 0;
		virtual BwdTranslatorPtr GetSymbolBackTransl() = 0;
		virtual SymbolDict& GetSymbolDict() = 0;

		virtual ~AbstractAlphabet()
		{ }
	};


	class OnTheFlyAlphabet : public AbstractAlphabet
	{
	private:  // data members

		SymbolDict symbolDict_{};
		SymbolType nextSymbol_;

	public:   // methods

		OnTheFlyAlphabet() :
			nextSymbol_(Symbolic::GetZeroSymbol())
		{ }

		virtual FwdTranslatorPtr GetSymbolTransl() override
		{
			FwdTranslator* fwdTransl = new
				StringSymbolToSymbolTranslWeak{symbolDict_,
				[&](const StringSymbolType&){return nextSymbol_++;}};
			return FwdTranslatorPtr(fwdTransl);
		}

		virtual BwdTranslatorPtr GetSymbolBackTransl() override
		{
			throw NotImplementedException(__func__);
		}

		virtual SymbolDict& GetSymbolDict() override
		{
			return symbolDict_;
		}
	};

	class DirectAlphabet : public AbstractAlphabet
	{
	public:

		virtual FwdTranslatorPtr GetSymbolTransl() override
		{
			throw NotImplementedException(__func__);
		}

		virtual BwdTranslatorPtr GetSymbolBackTransl() override
		{
			throw NotImplementedException(__func__);
		}

		virtual SymbolDict& GetSymbolDict() override
		{
			throw NotImplementedException(__func__);
		}
	};

	using AlphabetType = std::shared_ptr<AbstractAlphabet>;

	using Transition = TreeAutBase::TTransition<SymbolType>;

protected:// methods

	SymbolicTreeAutBase() { }

public:   // methods

	static const StringSymbolType& ToStringSymbolType(
		const std::string&         str,
		size_t                     /* rank */)
	{
		return str;
	}
};

#endif
