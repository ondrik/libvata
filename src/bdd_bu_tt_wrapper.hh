/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2013  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Wrapper of a transition table for bottom-up symbolic TA
 *
 *****************************************************************************/

#ifndef _VATA_BDD_BU_TREE_AUT_TT_WRAPPER_HH_
#define _VATA_BDD_BU_TREE_AUT_TT_WRAPPER_HH_

namespace VATA { namespace Util {
template <
	class TMTBDDHandle,
	class TStateSet
	>
class TransTableWrapper
{
private:  // data types

	using MTBDDHandle     = TMTBDDHandle;
	using StateSet        = TStateSet;
	using Table           = VATA::Util::BDDBottomUpTransTable<MTBDDHandle, StateSet>;
	using TablePtr        = std::shared_ptr<Table>;
	using StateTuple      = typename Table::StateTuple;

public:   // data types

	using TransMTBDD      = typename Table::MTBDD;

private:  // data types

	using key_type    = typename Table::TupleMap::key_type;
	using mapped_type = typename Table::TupleMap::mapped_type;
	using value_type  = typename Table::TupleMap::value_type;


	template <class TupleMapIterator>
	class generic_iterator
	{
	private:  // data members

		bool isNullary_;
		const TransTableWrapper& tableWrap_;
		TupleMapIterator itTupleMap_;

	public:   // methods

		bool operator==(const generic_iterator& rhs) const
		{
			bool match = (isNullary_ == rhs.isNullary_);
			return match && (isNullary_ || (itTupleMap_ == rhs.itTupleMap_));
		}

		bool operator!=(const generic_iterator& rhs) const
		{
			return !operator==(rhs);
		}

		generic_iterator& operator++()
		{
			if (isNullary_)
			{
				isNullary_ = false;
			}
			else
			{
				++itTupleMap_;
			}

			return *this;
		}

	protected:// methods

		generic_iterator(
			const TransTableWrapper&      tableWrap,
			bool                          isBegin) :
			isNullary_(isBegin),
			tableWrap_(tableWrap),
			itTupleMap_(isBegin? tableWrap.table_->GetTupleMap().begin() :
				tableWrap.table_->GetTupleMap().end())
		{ }

#if 0
		inline value_type refOp()
		{
			if (isNullary_)
			{
				assert(false);
			}
			else
			{
				return *itTupleMap_;
			}
		}
#endif

		value_type constRefOp() const
		{
			if (isNullary_)
			{
				return std::make_pair(StateTuple(), tableWrap_.nullaryMtbdd_);
			}
			else
			{
				return *itTupleMap_;
			}
		}

	};

public:   // data types

#if 0
	GCC_DIAG_OFF(effc++)
	class iterator :
		public generic_iterator<typename Table::TupleMap::iterator>
	{
	GCC_DIAG_ON(effc++)

	public:   // methods

		iterator(const typename Table::TupleMap::iterator& iter, bool isNullary) :
			generic_iterator(iter, isNullary)
		{ }

		inline value_type& operator*()
		{
			return refOp();
		}
	};
#endif

	GCC_DIAG_OFF(effc++)
	class const_iterator :
		public generic_iterator<typename Table::TupleMap::const_iterator>
	{
	GCC_DIAG_ON(effc++)

	public:   // methods

		const_iterator(
			const TransTableWrapper&       tableWrap,
			bool                           isBegin) :
			generic_iterator<typename Table::TupleMap::const_iterator>(tableWrap, isBegin)
		{ }

		value_type operator*() const
		{
			return this->constRefOp();
		}
	};


private:  // data members

	TablePtr table_;
	TransMTBDD nullaryMtbdd_;

public:   // methods

	TransTableWrapper() :
		table_(new Table),
		nullaryMtbdd_(StateSet())
	{ }

	explicit TransTableWrapper(TablePtr& table) :
		table_(table),
		nullaryMtbdd_(StateSet())
	{ }

	TransTableWrapper(
		TablePtr&                 table,
		const TransMTBDD&         nullaryMtbdd) :
		table_(table),
		nullaryMtbdd_(nullaryMtbdd)
	{ }

	const TransMTBDD& GetMtbdd(
		const StateTuple&         children) const
	{
		if (children.empty())
		{
			return nullaryMtbdd_;
		}
		else
		{
			return table_->GetMtbdd(children);
		}
	}

	void SetMtbdd(
		const StateTuple&              children,
		const TransMTBDD&              mtbdd)
	{
		if (children.empty())
		{
			nullaryMtbdd_ = mtbdd;;
		}
		else
		{
			table_->SetMtbdd(children, mtbdd);
		}
	}

	TablePtr GetTable() const
	{
		return table_;
	}

	bool unique() const
	{
		return table_.unique();
	}

	size_t size() const
	{
		return table_->size();
	}

	const_iterator begin() const
	{
		return const_iterator(*this, true);
	}

	const_iterator cbegin() const
	{
		return begin();
	}

	const_iterator end() const
	{
		return const_iterator(*this, false);
	}

	const_iterator cend() const
	{
		return end();
	}
};
}} // namespace

#endif
