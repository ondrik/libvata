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

	size_t _labels;
	size_t _states;
	size_t _transitions;
	std::vector<std::vector<std::vector<size_t> > > _dataPre;
	std::vector<SmartSet> _lPre;

public:

	LTS() : _labels(0), _states(0), _transitions(0), _dataPre(), _lPre() {}
/*
	LTS(const LTS& lts) :
		_labels(lts._labels), _states(lts._states), _transitions(lts._transitions),
		_dataPre(lts._dataPre),
		_lPre(lts._lPre) {}
*/
	void addTransition(size_t q, size_t a, size_t r) {

		if (a >= this->_dataPre.size()) {
			this->_labels = a + 1;
			this->_dataPre.resize(a + 1);
		}
		
		if (r >= this->_dataPre[a].size()) {
			if (r >= this->_states)
				this->_states = r + 1;
			this->_dataPre[a].resize(r + 1);
		}
		
		if (q >= this->_states)
			this->_states = q + 1;
			
		this->_dataPre[a][r].push_back(q);
		
		++this->_transitions;

	}

	void init() {

		this->_lPre.resize(this->_states, SmartSet(this->_labels));

		for (size_t a = 0; a < this->_dataPre.size(); ++a) {
			this->_dataPre[a].resize(this->_states);
			for (size_t r = 0; r < this->_dataPre[a].size(); ++r) {
				assert(r < this->_lPre.size());
				this->_lPre[r].init(a, this->_dataPre[a][r].size());
			}
		}

	}

	void clear() {

		this->_labels = 0;
		this->_states = 0;
		this->_transitions = 0;
		this->_dataPre.clear();
		this->_lPre.clear();

	}

	const std::vector<size_t>& pre(size_t q, size_t a) const {
		assert(a < this->_dataPre.size());
		assert(q < this->_dataPre[a].size());
		return this->_dataPre[a][q];
	}

	const std::vector<std::vector<std::vector<size_t> > >& dataPre() const { return this->_dataPre; }

	void buildPost(size_t label, std::vector<std::vector<size_t> >& dst) const {
		dst.clear();
		dst.resize(this->_states);
		for (size_t i = 0; i < this->_dataPre[label].size(); ++i) {
			for (std::vector<size_t>::const_iterator j = this->_dataPre[label][i].begin(); j != this->_dataPre[label][i].end(); ++j)
				dst[*j].push_back(i);
		}
	}
	
	void buildDelta(std::vector<SmartSet>& delta, std::vector<SmartSet>& delta1) const {
		delta.resize(this->_labels);
		delta1.resize(this->_labels);
		for (size_t a = 0; a < this->_labels; ++a) {
			delta[a] = SmartSet(this->_states);
			delta1[a] = SmartSet(this->_states);
			for (size_t i = 0; i < this->_dataPre[a].size(); ++i) {
				for (std::vector<size_t>::const_iterator j = this->_dataPre[a][i].begin(); j != this->_dataPre[a][i].end(); ++j) {
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

	friend std::ostream& operator<<(std::ostream& os, const LTS& lts) {

		for (size_t i = 0; i < lts._dataPre.size(); ++i) {
			for (size_t j = 0; j < lts._dataPre[i].size(); ++j) {
				for (size_t k = 0; k < lts._dataPre[i][j].size(); ++k)
					os << lts._dataPre[i][j][k] << " --" << i << "--> " << j << std::endl;
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
