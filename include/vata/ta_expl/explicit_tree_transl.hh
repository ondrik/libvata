/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Tree automata to LTS translation for explicitly represented tree automata.
 *
 *****************************************************************************/

#ifndef _VATA_EXPLICIT_TREE_TRANSL_HH_
#define _VATA_EXPLICIT_TREE_TRANSL_HH_

#include <unordered_map>

#include <boost/functional/hash.hpp>

#include <vata/vata.hh>
#include <vata/explicit_lts.hh>
#include <vata/ta_expl/explicit_tree_aut.hh>
#include <vata/util/transl_strict.hh>
#include <vata/util/transl_weak.hh>
#include <vata/util/convert.hh>

namespace VATA {

	template <class SymbolType, class Index = Util::IdentityTranslator<AutBase::StateType>>
	ExplicitLTS TranslateDownward(const ExplicitTreeAut<SymbolType>& aut,
		const Index& stateIndex = Index());

	template <
		class SymbolType,
		class Rel,
		class Index = Util::IdentityTranslator<AutBase::StateType>
	>
	ExplicitLTS TranslateUpward(const ExplicitTreeAut<SymbolType>& aut,
		std::vector<std::vector<size_t>>& partition, Util::BinaryRelation& relation,
		const Rel& param, const Index& stateIndex = Index());

}

template <class SymbolType, class Index>
VATA::ExplicitLTS VATA::TranslateDownward(const ExplicitTreeAut<SymbolType>& aut,
	const Index& stateIndex) {

	typedef Explicit::StateTuple StateTuple;

	std::unordered_map<SymbolType, size_t> symbolMap;
	std::unordered_map<const StateTuple*, size_t> lhsMap;

	size_t symbolCnt = 0;
	Util::TranslatorWeak2<std::unordered_map<SymbolType, size_t>>
		symbolTranslator(symbolMap, [&symbolCnt](const SymbolType&){ return symbolCnt++; });

	assert(aut.transitions_);

	size_t lhsCnt = aut.transitions_->size();
	Util::TranslatorWeak2<std::unordered_map<const StateTuple*, size_t>>
		lhsTranslator(lhsMap, [&lhsCnt](const StateTuple*){ return lhsCnt++; });

	ExplicitLTS result;

  /*
   * Iterate through all transitions and adds them
   * to the LTS.
   */
	for (auto& stateClusterPair : *aut.transitions_) {

		assert(stateClusterPair.second);

		size_t state = stateIndex[stateClusterPair.first];

		for (auto& symbolTupleSetPair : *stateClusterPair.second) {

			assert(symbolTupleSetPair.second);

			size_t symbol = symbolTranslator(symbolTupleSetPair.first);

			for (auto& tuple : *symbolTupleSetPair.second) {

				assert(tuple);

				if (tuple->size() == 1) { // a(p) -> q
					// inline lhs of size 1 >:-)
					result.addTransition(state, symbol, tuple->front());
				} else { // a(p,r) -> q
					result.addTransition(
						stateClusterPair.first, symbol, lhsTranslator(tuple.get())
					);
				}

			}

		}

	}

	for (auto& tupleIndexPair : lhsMap) {

		assert(tupleIndexPair.first);

		size_t i = 0;

		for (auto& state : *tupleIndexPair.first) {

			result.addTransition(tupleIndexPair.second, symbolMap.size() + i, stateIndex[state]);

			++i;

		}

	}

	result.init();

	return result;

}

template <class SymbolType, class Rel, class Index>
VATA::ExplicitLTS VATA::TranslateUpward(const ExplicitTreeAut<SymbolType>& aut,
	std::vector<std::vector<size_t>>& partition, Util::BinaryRelation& relation,
	const Rel& param, const Index& stateIndex) {

	typedef Explicit::StateTuple StateTuple;

	struct Env {

		StateTuple children_;
		size_t index_;
		size_t symbol_;
		size_t state_;

		Env(const StateTuple& children, size_t index, size_t symbol, size_t state)
			: children_(), index_(index), symbol_(symbol), state_(state) {

			this->children_.insert(
				this->children_.end(), children.begin(), children.begin() + index
			);

			this->children_.insert(
				this->children_.end(), children.begin() + index + 1, children.end()
			);

		}

		bool lessThan(const Env& env, const Rel& rel) const {

			if (this->children_.size() != env.children_.size()) return false;
			if (this->index_ != env.index_) return false;
			if (this->symbol_ != env.symbol_) return false;

			for (size_t i = 0; i < this->children_.size(); ++i) {

				if (!rel.get(this->children_[i], env.children_[i]))
					return false;

			}

			return true;

		}

		bool equal(const Env& env, const Rel& rel) const {

			if (this->children_.size() != env.children_.size()) return false;
			if (this->index_ != env.index_) return false;
			if (this->symbol_ != env.symbol_) return false;

			for (size_t i = 0; i < this->children_.size(); ++i) {

				if (!rel.sym(this->children_[i], env.children_[i]))
					return false;

			}

			return true;

		}

		bool operator==(const Env& rhs) const {

			return (this->children_.size() == rhs.children_.size()) &&
				(this->index_ == rhs.index_) && (this->symbol_ == rhs.symbol_) &&
				(this->children_ == rhs.children_);

		}

	};

	struct env_hash {

		size_t operator()(const Env& env) const {

			size_t seed = 0;
			boost::hash_combine(seed, env.children_);
			boost::hash_combine(seed, env.index_);
			boost::hash_combine(seed, env.symbol_);
			boost::hash_combine(seed, env.state_);
			return seed;

		}

	};


	assert(aut.transitions_);
//	assert(aut.transitions_->size() == param.size());

	size_t symbolCnt = 0;
	size_t stateCnt = aut.transitions_->size() + 1; // leaf state

	std::unordered_map<SymbolType, size_t> symbolMap;
	std::unordered_map<Env, size_t, env_hash> envMap;

	size_t base = ((0 < aut.finalStates_.size()) &&
		(aut.finalStates_.size() < aut.transitions_->size())) ? 3 : 2;

	partition.clear();
	partition.resize(base);

	std::vector<const Env*> head;

	VATA::Util::TranslatorWeak2<std::unordered_map<SymbolType, size_t>>
		symbolTranslator(symbolMap, [&symbolCnt](const SymbolType&){ return symbolCnt++; });

	VATA::Util::TranslatorWeak2<std::unordered_map<Env, size_t, env_hash>>
		envTranslator(
			envMap,
			[&base, &stateCnt, &head, &partition, &param](const Env& env) -> size_t {

				for (size_t i = 0; i < head.size(); ++i) {

					assert(head[i]);

					if (!head[i]->equal(env, param))
						continue;

					partition[base + i].push_back(stateCnt);

					return stateCnt++;

				}

				head.push_back(&env);

				partition.push_back(std::vector<size_t>(1, stateCnt));

				return stateCnt++;

			}
	);

	for (auto& stateClusterPair : *aut.transitions_) {

		assert(stateClusterPair.second);
		assert(stateIndex[stateClusterPair.first] < aut.transitions_->size());

		partition[
			aut.IsFinalState(stateClusterPair.first)?(0):(base - 2)
		].push_back(stateIndex[stateClusterPair.first]);

		for (auto& symbolTupleSetPair : *stateClusterPair.second) {

			assert(symbolTupleSetPair.second);

			symbolTranslator(symbolTupleSetPair.first);

		}

	}

	partition[base - 1].push_back(aut.transitions_->size()); // leaf state

	ExplicitLTS result;

	for (auto& stateClusterPair : *aut.transitions_) {

		assert(stateClusterPair.second);

		size_t state = stateIndex[stateClusterPair.first];

		for (auto& symbolTupleSetPair : *stateClusterPair.second) {

			assert(symbolTupleSetPair.second);

			size_t symbol = symbolTranslator(symbolTupleSetPair.first);

			for (auto& tuple : *symbolTupleSetPair.second) {

				assert(tuple);

				if (tuple->empty()) {
					// take care of leaves
					result.addTransition(aut.transitions_->size(), symbol, state);
					continue;
				}

				if (tuple->size() == 1) {
					// inline lhs of size 1 >:-)
					result.addTransition(stateIndex[(*tuple)[0]], symbol, state);
					continue;
				}

				for (size_t i = 0; i < tuple->size(); ++i) {

					result.addTransition(
						stateIndex[(*tuple)[i]],
						symbolCnt,
						envTranslator(Env(*tuple, i, symbol, state))
					);

				}

			}

		}

	}

	for (auto& envIndexPair : envMap) {

		result.addTransition(
			envIndexPair.second, envIndexPair.first.symbol_, stateIndex[envIndexPair.first.state_]
		);

	}

	relation.resize(partition.size());
	relation.reset(false);

	// 0 accepting, 1 non-accepting, 2 .. environments
	relation.set(0, 0, true);

	if (base == 3) {

		relation.set(1, 0, true);
		relation.set(1, 1, true);

	}

	relation.set(base - 1, base - 1, true); // reflexivity of leaf state

	for (size_t i = 0; i < head.size(); ++i) {

		assert(head[i]);
		assert(head[i]->lessThan(*head[i], param));

		for (size_t j = 0; j < head.size(); ++j) {

			assert(head[j]);
			if (head[i]->lessThan(*head[j], param))
				relation.set(base + i, base + j, true);

		}

	}

  result.init();

	return result;

}

#endif
