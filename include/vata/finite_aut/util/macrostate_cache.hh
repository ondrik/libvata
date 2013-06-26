/*****************************************************************************
 *	VATA Finite Automata Library
 *
 *	Copyright (c) 2013	Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *	Description:
 *	Header file for cache of macrostates.
 *
 *****************************************************************************/


#ifndef UTIL_MACROSTATE_CACHE_
#define UTIL_MACROSTATE_CACHE_

// VATA headers
#include <vata/vata.hh>

// Standard library headers
#include <unordered_map>

namespace VATA {
	template<class Aut> class MacroStateCache;
}

/*
 * Cache for caching macro state
 */
template<class Aut>
class VATA::MacroStateCache {
private:
	typedef typename Aut::StateSet StateSet;
	typedef std::list<StateSet> SetList;
	typedef std::unordered_map<size_t,SetList> CacheMap;

	CacheMap cacheMap;
public:
	MacroStateCache() : cacheMap(){}

	// Function inserts a new element to macrostate cache, when
	// the element is already presented it will return pointer to it
	StateSet& insert(size_t key, StateSet& value) {
		auto areEqual = [] (StateSet& lss, StateSet& rss) -> bool {
		 if (lss.size() != rss.size()) {
			 return false;
		 }
		 if (!lss.size() || !rss.size()) {
			 return false;
		 }
		 for (auto& ls : lss) {
			 if (!rss.count(ls)) {
				 return false;
			 }
		 }

			return true;
	 };

		auto iter = cacheMap.find(key);
		if (iter == cacheMap.end()) { // new value
			auto& list = cacheMap.insert(std::make_pair(key,SetList())).first->second;
			list.push_back(StateSet(value));
			return list.back();
		}
		else {
			for (auto& set : iter->second) { // set already cached
				if (areEqual(set,value)) {
					return set;
				}
			}
			iter->second.push_back(StateSet(value));
			return iter->second.back();
		}
	}

};

#endif
