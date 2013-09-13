/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for smart set.
 *
 *****************************************************************************/

#ifndef _VATA_SMART_SET_HH_
#define _VATA_SMART_SET_HH_

// Standard library headers
#include <ostream>
#include <vector>

namespace VATA
{
	namespace Util
	{
		class SmartSet;
	}
}

class VATA::Util::SmartSet
{
public:

	typedef size_t Key;

private:

	struct Element
	{
		Element* next_;
		Key key_;
		size_t count_;

		Element(
			const Key&         key,
			size_t             count = 0) :
			next_(nullptr),
			key_(key),
			count_(count)
		{ }
	};

	GCC_DIAG_OFF(effc++)
	struct Iterator : public std::iterator<std::input_iterator_tag, Key>
	{
	GCC_DIAG_ON(effc++)

		const Element* element_;

		Iterator(const Element* element) :
			element_(element)
		{ }

		Iterator& operator++()
		{
			assert(nullptr != element_);

			element_ = element_->next_;
			return *this;
		}

		Iterator operator++(int) const
		{
			return ++Iterator(element_);
		}

		const Key& operator*()
		{
			assert(nullptr != element_);

			return element_->key_;
		}

		bool operator==(const Iterator& rhs) const
		{
			return element_ == rhs.element_;
		}

		bool operator!=(const Iterator& rhs) const
		{
			return element_ != rhs.element_;
		}
	};

public:

	typedef Iterator iterator;

private:

	Element head_;
	Element* last_;
	size_t size_;

	std::vector<Element*> index_;

protected:

	size_t& insert(const Key& key)
	{
		assert(key < index_.size());

		Element*& prev = index_[key];

		if (nullptr == prev)
		{
			prev = last_;
			prev->next_ = new Element(key);
			last_ = prev->next_;

			++size_;
		}

		assert(key == prev->next_->key_);

		return prev->next_->count_;
	}

	void erase(Element*& prev)
	{
		assert(nullptr != prev);

		--size_;
		Element* el = prev->next_;

		assert(nullptr != el);

		prev->next_ = el->next_;

		if (nullptr != prev->next_)
		{
			assert(prev->next_->key_ < index_.size());
			assert(index_[prev->next_->key_] == el);

			index_[prev->next_->key_] = prev;
		}

		delete el;
		prev = nullptr;
	}

public:

	SmartSet(
		size_t         range = 0) :
		head_(Key(), 0),
		last_(&head_), size_(0),
		index_(range, nullptr)
	{ }

	SmartSet(const SmartSet& s) :
		head_(Key(), 0),
		last_(&head_),
		size_(s.size_),
		index_(s.index_.size(), nullptr)
	{
		for (Element* el = s.head_.next_ ; nullptr != el; el = el->next_)
		{
			index_[el->key_] = last_;
			last_->next_ = new Element(el->key_, el->count_);
			last_ = last_->next_;
		}
	}

	SmartSet& operator=(const SmartSet& s)
	{
		std::fill(index_.begin(), index_.end(), nullptr);
		index_.resize(s.index_.size(), nullptr);
		last_ = &head_;

		for (Element* el = s.head_.next_ ; nullptr != el; el = el->next_)
		{
			index_[el->key_] = last_;
			last_->next_ = new Element(el->key_, el->count_);
			last_ = last_->next_;
		}

		size_ = s.size();

		return *this;
	}

	void assignFlat(const SmartSet& s)
	{
		this->clear();
		assert(nullptr == head_.next_);
		assert(std::all_of(index_.cbegin(), index_.cend(), [](const Element* elem){return nullptr == elem;}));

		index_.resize(s.index_.size(), nullptr);
		last_ = &head_;

		for (Element* el = s.head_.next_ ; nullptr != el; el = el->next_)
		{
			index_[el->key_] = last_;
			last_->next_ = new Element(el->key_, 1);
			last_ = last_->next_;
		}

		size_ = s.size();
	}

	SmartSet::iterator begin() const
	{
		return SmartSet::Iterator(head_.next_);
	}

	SmartSet::iterator end() const
	{
		return SmartSet::Iterator(nullptr);
	}

	bool contains(const Key& key) const
	{
		assert(key < index_.size());

		if (nullptr == index_[key])
		{
			return false;
		}

		assert(index_[key]->next_);
		assert(index_[key]->next_->key_ == key);

		return true;
	}

	size_t count(const Key& key) const
	{
		assert(key < index_.size());

		if (nullptr == index_[key])
		{
			return 0;
		}

		assert(index_[key]->next_);
		assert(index_[key]->next_->key_ == key);

		return index_[key]->next_->count_;
	}

	void init(const Key& key, size_t count)
	{
		if (count > 0)
		{
			this->insert(key) = count;
			return;
		}

		assert(key < index_.size());

		Element*& prev = index_[key];

		if (nullptr != prev)
		{
			this->erase(prev);
		}
	}

	void add(const Key& key)
	{
		++this->insert(key);
	}

	void remove(const Key& key)
	{
		assert(key < index_.size());

		Element*& prev = index_[key];

		if (nullptr == prev)
		{
			return;
		}

		Element* el = prev->next_;

		assert(nullptr != el);
		assert(key == el->key_);

		if (el->count_ == 1)
		{
			this->erase(prev);
		}
		else
		{
			--el->count_;
		}
	}

	void removeStrict(const Key& key)
	{
		assert(key < index_.size());
		Element*& prev = index_[key];
		assert(nullptr != prev);

		Element* el = prev->next_;
		assert(key == el->key_);

		if (el->count_ == 1)
		{
			this->erase(prev);
		}
		else
		{
			--el->count_;
		}
	}

	bool empty() const
	{
		return head_.next_ == nullptr;
	}

	size_t size() const
	{
		return size_;
	}

	void clear()
	{
		for (Element* el = head_.next_; nullptr != el; )
		{
			Element* tmp = el;
			el = el->next_;

			assert(tmp->key_ < index_.size());

			index_[tmp->key_] = nullptr;
			delete tmp;
		}

		last_ = &head_;
		size_ = 0;
	}

	friend std::ostream& operator<<(std::ostream& os, const SmartSet& s)
	{
		os << '{';

		for (const Element* el = s.head_.next_; nullptr != el; el = el->next_)
		{
			os << ' ' << el->key_ << ':' << el->count_;
		}

		return os << " }";
	}
};

#endif
