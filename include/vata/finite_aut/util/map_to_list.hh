/*****************************************************************************
 *	VATA Finite Automata Library
 *
 *	Copyright (c) 2013	Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *	Description:
 *	Header file for mapping key to list of values.
 *
 *****************************************************************************/


#ifndef UTIL_MAP_TO_LIST_
#define UTIL_MAP_TO_LIST_

// Standard library headers
#include <unordered_map>

// VATA headers
#include <vata/vata.hh>

namespace VATA {
	template <class Key,class ListValue> class MapToList;
}

/*
 * Class for mapping pointer to macro state
 * to list of pointers to macro states
 */
template <class Key, class ListValue>
class VATA::MapToList {
public:
	typedef std::unordered_set<ListValue> VList;
	typedef typename std::unordered_map<Key,VList> ListMap;

	ListMap map;

	MapToList() : map() {}

	/*
	 * Function adds value v to list mapped by key k.
	 */
	inline void add(Key k, ListValue v) {
		auto iter = map.find(k);
		if (iter == map.end()) {
			map.insert(std::make_pair(k,VList())).first->second.insert(v);
		}
		else {
			iter->second.insert(v);
		}
	}


	/*
	 * Checks whether for the given key k exists
	 * value v in its list of pointers.
	 */
	inline bool contains(Key k, ListValue v) {
		auto iter = map.find(k);
		if (iter == map.end()) {
			return false;
		}
		else {
			return iter->second.count(v);
		}
	}

	// Checke whether map contains key k
	inline bool containsKey(Key k) {
		return map.find(k) != map.end();
	}

	// Get list to which is key k mapped
	inline VList* getList(Key k) {
		if (containsKey(k)) {
			return &map.find(k)->second;
		}
		else {
			return nullptr;
		}
	}
};

#endif
