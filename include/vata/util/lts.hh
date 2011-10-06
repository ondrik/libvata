/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for LTS definition.
 *
 *****************************************************************************/

#ifndef _VATA_LTS_HH_
#define _VATA_LTS_HH_

#include <vata/util/convert.hh>
#include <vata/util/smart_set.hh>

namespace VATA { namespace Util { class LTS; } }

class VATA::Util::LTS {

//	size_t _labels;
	size_t states_;
	size_t transitions_;
//	std::vector<std::vector<std::vector<size_t> > > _dataPre;
	std::vector<
		std::pair<
			std::vector<std::vector<size_t>>,
			std::vector<std::vector<size_t>>
		>
	> data_;
	std::vector<SmartSet> lPre_;

public:

	LTS() : /*_labels(0),*/ states_(0), transitions_(0), data_(), lPre_() {}
/*
	LTS(const LTS& lts) :
		_labels(lts._labels), _states(lts._states), _transitions(lts._transitions),
		_dataPre(lts._dataPre),
		_lPre(lts._lPre) {}
*/
	void addTransition(size_t q, size_t a, size_t r) {

		if (a >= this->data_.size())
			this->data_.resize(a + 1);

		if (q >= this->data_[a].first.size()) {
			if (q >= this->states_)
				this->states_ = q + 1;
			this->data_[a].first.resize(q + 1);
		}

		if (r >= this->data_[a].second.size()) {
			if (r >= this->states_)
				this->states_ = r + 1;
			this->data_[a].second.resize(r + 1);
		}
		
		this->data_[a].first[q].push_back(r);
		this->data_[a].second[r].push_back(q);
		
		++this->transitions_;

	}

	void init() {

		this->lPre_.resize(this->states_, SmartSet(this->data_.size()));

		for (size_t a = 0; a < this->data_.size(); ++a) {

			this->data_[a].first.resize(this->states_);
			this->data_[a].second.resize(this->states_);

			for (size_t r = 0; r < this->states_; ++r)
				this->lPre_[r].init(a, this->data_[a].second[r].size());

		}

	}

	void clear() {

		this->data_.clear();
		this->lPre_.clear();
		this->states_ = 0;
		this->transitions_ = 0;

	}
/*
	const std::vector<size_t>& pre(size_t q, size_t a) const {
		assert(a < this->_dataPre.size());
		assert(q < this->_dataPre[a].size());
		return this->_dataPre[a][q];
	}
*/

	const std::vector<std::vector<size_t>>& post(size_t a) const {

		assert(a < this->data_.size());

		return this->data_[a].first;

	}

	const std::vector<std::vector<size_t>>& pre(size_t a) const {

		assert(a < this->data_.size());

		return this->data_[a].second;

	}

//	const std::vector<std::vector<std::vector<size_t> > >& dataPre() const { return this->_dataPre; }
/*
	void buildPost(size_t label, std::vector<std::vector<size_t> >& dst) const {
		dst.clear();
		dst.resize(this->_states);
		for (size_t i = 0; i < this->_dataPre[label].size(); ++i) {
			for (std::vector<size_t>::const_iterator j = this->_dataPre[label][i].begin(); j != this->_dataPre[label][i].end(); ++j)
				dst[*j].push_back(i);
		}
	}
*/	
	void buildDelta(std::vector<SmartSet>& delta, std::vector<SmartSet>& delta1) const {

		delta.resize(this->data_.size(), SmartSet(this->states_));
		delta1.resize(this->data_.size(), SmartSet(this->states_));

		for (size_t a = 0; a < this->data_.size(); ++a) {

			for (size_t q = 0; q < this->data_[a].first.size(); ++q) {

				for (auto& r : this->data_[a].first[q]) {

					delta[a].add(r);
					delta1[a].add(q);

				}

			}

		}

	}

	const std::vector<SmartSet>& lPre() const { return this->lPre_; }
	
	size_t labels() const { return this->data_.size(); }

	size_t states() const { return this->states_; }

	friend std::ostream& operator<<(std::ostream& os, const LTS& lts) {

		for (size_t a = 0; a < lts.data_.size(); ++a) {

			for (size_t q = 0; q < lts.data_[a].second.size(); ++q) {

				for (auto& r : lts.data_[a].first[q])
					os << q << " --" << a << "--> " << r << std::endl;

			}

		}

		return os;

	}

};

/*
class LTS2 {

	size_t _labels;
	size_t _states;
//	size_t _transitions;
	std::vector<std::vector<size_t> > _dataPre;
	std::vector<SmartSet> _lPre;

public:

	LTS2(size_t labels = 0, size_t states = 0) :
		_labels(labels), _states(states), //_transitions(0),
		_dataPre(labels*states),
		_lPre(states, SmartSet(labels)) {}

	void addTransition(size_t q, size_t a, size_t r) {
		assert(q < this->_states);
		assert(a < this->_labels);
		assert(r < this->_states);
		this->_dataPre[a*this->_states + r].push_back(q);
		this->_lPre[r].add(a);
//		++this->_transitions;
	}

	const std::vector<size_t>& pre(size_t q, size_t a) const {
		return this->_dataPre[a*this->_states + q];
	}

//	const std::vector<std::vector<size_t> >& dataPre() const { return this->_dataPre; }

	void buildPost(size_t a, std::vector<std::vector<size_t> >& dst) const {
		dst.clear();
		dst.resize(this->_states);
		for (size_t i = 0; i < this->_states; ++i) {
			for (std::vector<size_t>::const_iterator j = this->_dataPre[a*this->_states + i].begin(); j != this->_dataPre[a*this->_states + i].end(); ++j)
				dst[*j].push_back(i);
		}
	}
	
	void buildDelta(std::vector<SmartSet>& delta, std::vector<SmartSet>& delta1) const {
		delta.resize(this->_labels);
		delta1.resize(this->_labels);
		for (size_t a = 0; a < this->_labels; ++a) {
			delta[a] = SmartSet(this->_states);
			delta1[a] = SmartSet(this->_states);
			for (size_t i = 0; i < this->_states; ++i) {
				for (std::vector<size_t>::const_iterator j = this->_dataPre[a*this->_states + i].begin(); j != this->_dataPre[this->_states + i].end(); ++j) {
					delta[a].add(i);
					delta1[a].add(*j);
				}
			}
		}
	}

	const std::vector<SmartSet>& lPre() const {
		return this->_lPre;
	}
	
	size_t labels() const {
		return this->_labels;
	}

	size_t states() const {
		return this->_states;
	}

	friend std::ostream& operator<<(std::ostream& os, const LTS2& lts) {
		os << "states: " << lts._states << ", labels: " << lts._labels << std::endl;
		for (size_t i = 0; i < lts._labels; ++i) {
			for (size_t j = 0; j < lts._states; ++j) {
				for (size_t k = 0; k < lts._dataPre[i*lts._states + j].size(); ++k)
					os << lts._dataPre[i*lts._states + j][k] << " -" << i << "-> " << j << std::endl;
			}
		}
	}

};

std::ostream& operator<<(std::ostream& os, const LTS2& lts);
*/
#endif
