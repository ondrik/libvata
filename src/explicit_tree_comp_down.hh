/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2012  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Downward complementation for explicitly represented tree automata.
 *
 *****************************************************************************/

#ifndef _VATA_EXPLICIT_TREE_COMP_DOWN_HH_
#define _VATA_EXPLICIT_TREE_COMP_DOWN_HH_

#include <vector>
#include <unordered_set>
#include <unordered_map>

#include <vata/ta_expl/explicit_tree_aut.hh>
#include <vata/util/antichain1c.hh>
#include <vata/util/transl_strict.hh>

namespace VATA { class ExplicitDownwardComplementation; }

class VATA::ExplicitDownwardComplementation
{
	typedef std::vector<const VATA::ExplicitTreeAutCore::StateTuple*> TupleList;
	typedef std::vector<TupleList> IndexedTupleList;
	typedef std::vector<IndexedTupleList> DoubleIndexedTupleList;

	typedef VATA::Util::Convert Convert;

private:

	class ChoiceFunction
	{
		std::vector<size_t> data_;
		size_t arity_;

	public:

		ChoiceFunction() :
			data_(),
			arity_()
		{ }

		void swap(ChoiceFunction& rhs)
		{
			std::swap(data_, rhs.data_);
			std::swap(arity_, rhs.arity_);
		}

		void init(size_t size, size_t arity)
		{
			assert(size);

			data_ = std::vector<size_t>(size, 0);
			arity_ = arity;
		}

		bool next()
		{
			// move to the next choice function
			size_t index = 0;

			while (++data_[index] == arity_)
			{
				data_[index] = 0; // reset this counter

				++index;          // move to the next counter

				if (index == data_.size())
				{
					// if we drop out from the n-tuple
					return false;
				}
			}

			return true;
		}

		const size_t& operator[](size_t index)
		{
			assert(index < data_.size());

			return data_[index];
		}

		size_t size() const
		{
			return data_.size();
		}

		const size_t& arity() const
		{
			return arity_;
		}
	};

	template <
		class Aut,
		class SymbolIndex>
	static void topDownIndex(
		DoubleIndexedTupleList&      topDownIndex,
		SymbolIndex&                 symbolIndex,
		const Aut&                   aut)
	{
		for (auto& stateClusterPair : *aut.transitions_)
		{
			assert(stateClusterPair.second);

			if (stateClusterPair.first >= topDownIndex.size())
			{
				topDownIndex.resize(stateClusterPair.first + 1);
			}

			auto& indexedTupleList = topDownIndex[stateClusterPair.first];

			for (auto& symbolTupleSetPair : *stateClusterPair.second)
			{
				assert(symbolTupleSetPair.second);
				assert(symbolTupleSetPair.second->size());

				const auto& symbol = symbolIndex[symbolTupleSetPair.first];

				if (symbol >= indexedTupleList.size())
				{
					indexedTupleList.resize(symbol + 1);
				}

				auto& tupleList = indexedTupleList[symbol];

				for (auto& tuple : *symbolTupleSetPair.second)
				{
					assert(tuple);

					tupleList.push_back(tuple.get());
				}
			}
		}
	}

public:

	template <
		class Aut,
		class Dict,
		class Rel>
	static void Compute(
		Aut&                     dst,
		const Aut&               src,
		const Dict&              alphabet,
		const Rel&               preorder)
	{
		typedef std::vector<VATA::Explicit::StateType> StateSet;
		typedef typename VATA::Util::Antichain1C<VATA::Explicit::StateType> Antichain1C;
		typedef VATA::Explicit::StateTuple StateTuple;
		typedef std::unordered_map<StateSet, size_t, boost::hash<StateSet>> StateCache;
		typedef const StateCache::value_type* StateCachePtr;

		DoubleIndexedTupleList transitionIndex;

		std::unordered_map<typename Aut::SymbolType, size_t> symbolMap;

		std::vector<size_t> ranks;

		size_t maxRank = 0;

		for (auto& symbolRankPair : alphabet)
		{
			// assert the symbol has not been processed
			assert(symbolMap.end() == symbolMap.find(symbolRankPair.first));

			symbolMap.insert(std::make_pair(symbolRankPair.first, ranks.size()));

			ranks.push_back(symbolRankPair.second);

			if (maxRank < symbolRankPair.second)
			{
				maxRank = symbolRankPair.second;
			}
		}

		Util::TranslatorStrict<
			std::unordered_map<typename Aut::SymbolType, size_t>
		> symbolTranslator(symbolMap);

		ExplicitDownwardComplementation::topDownIndex(transitionIndex, symbolTranslator, src);

		std::vector<std::vector<size_t>> ind, inv;

		preorder.buildIndex(ind, inv);

		std::unordered_set<const StateTuple*> tupleSet;

		std::vector<const StateTuple*> W;

		std::vector<Antichain1C> post((maxRank == 0)? 1 : maxRank);

		for (auto state : src.finalStates_)
		{
			assert(state < ind.size());

			if (post[0].contains(ind[state]))
			{
				continue;
			}

			assert(state < inv.size());

			post[0].refine(inv[state]);
			post[0].insert(state);
		}

		StateSet tmp = StateSet(post[0].data().begin(), post[0].data().end());

		post[0].clear();

		std::sort(tmp.begin(), tmp.end());

		StateCache stateCache;

		auto R = &*stateCache.insert(std::make_pair(tmp, 0)).first;

		std::unordered_set<StateCachePtr> todo;

		todo.insert(R);

		ChoiceFunction choiceFunction;

		dst.SetStateFinal(0);

		while (todo.size())
		{
			const auto P = *todo.begin();

			todo.erase(todo.begin());

			for (auto symbolIndexPair : symbolMap)
			{
				tupleSet.clear();

				W.clear();

				for (auto& state : P->first)
				{
					if (transitionIndex.size() <= state)
					{
						continue;
					}

					auto& biggerCluster = transitionIndex[state];

					if (biggerCluster.size() <= symbolIndexPair.second)
					{
						continue;
					}

					for (auto& tuple : biggerCluster[symbolIndexPair.second])
					{
						if (tupleSet.insert(tuple).second)
						{
							W.push_back(tuple);
						}
					}
				}

				assert(symbolIndexPair.second < ranks.size());

				if (W.empty())
				{
					if (ranks[symbolIndexPair.second] == 0)
					{
						dst.AddTransition(StateTuple(), symbolIndexPair.first, P->second);

						continue;
					}

					auto p = stateCache.insert(std::make_pair(StateSet(), stateCache.size()));

					if (p.second)
					{
						todo.insert(&*p.first);
					}

					dst.AddTransition(
						StateTuple(ranks[symbolIndexPair.second], p.first->second),
						symbolIndexPair.first,
						P->second
					);

					continue;
				}

				if (ranks[symbolIndexPair.second] == 0)
				{
					continue;
				}

				choiceFunction.init(W.size(), /* arity */ ranks[symbolIndexPair.second]);

				do
				{
					// we loop for each choice function
					for (size_t i = 0; i < choiceFunction.size(); ++i)
					{
						auto choice = choiceFunction[i];

						assert(choice < W[i]->size());

						auto state = (*W[i])[choice];

						assert(state < ind.size());

						if (post[choice].contains(ind[state]))
						{
							continue;
						}

						assert(state < inv.size());

						post[choice].refine(inv[state]);
						post[choice].insert(state);
					}

					StateTuple stateTuple(choiceFunction.arity());

					for (size_t i = 0; i < choiceFunction.arity(); ++i)
					{
						tmp = StateSet(post[i].data().begin(), post[i].data().end());

						post[i].clear();

						std::sort(tmp.begin(), tmp.end());

						auto p = stateCache.insert(std::make_pair(tmp, stateCache.size()));

						if (p.second)
						{
							todo.insert(&*p.first);
						}

						stateTuple[i] = p.first->second;
					}

					dst.AddTransition(stateTuple, symbolIndexPair.first, P->second);

				} while (choiceFunction.next());
			}
		}
	}
};


template <
	class Dict,
	class Rel>
ExplicitTreeAutCore ExplicitTreeAutCore::ComplementWithPreorder(
	const Dict&                            alphabet,
	const Rel&                             preorder) const
{
	ExplicitTreeAutCore res;

	VATA::ExplicitDownwardComplementation::Compute(res, *this, alphabet, preorder);

	return res.RemoveUselessStates();
}


template <class Dict>
ExplicitTreeAutCore ExplicitTreeAutCore::Complement(
	const Dict&                           alphabet) const
{
	typedef AutBase::StateType StateType;
	typedef std::unordered_map<StateType, StateType> StateDict;

	StateDict stateDict;

	size_t stateCnt = 0;
	Util::TranslatorWeak<StateDict> stateTranslator(
		stateDict, [&stateCnt](const StateType&){ return stateCnt++; }
	);

	aut.BuildStateIndex(stateTranslator);

	return this->ComplementWithPreorder(
		*this,
		alphabet,
		Util::Identity(stateCnt)
		/* ComputeDownwardSimulation(
			aut, stateDict.size(), Util::TranslatorStrict<StateDict>(stateDict)
		)*/
	);
}


#endif
