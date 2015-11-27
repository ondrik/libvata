/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    File with VarAsgn structure.
 *
 *****************************************************************************/

#ifndef _VATA_VAR_ASGN_HH_
#define _VATA_VAR_ASGN_HH_

// VATA headers
#include <vata/vata.hh>

// Standard library headers
#include <cassert>
#include <string>
#include <vector>
#include <stdexcept>


// insert class into correct namespace
namespace VATA
{
	struct SymbolicVarAsgn;
}


/**
 * @brief   Variable assignment
 *
 * A class that represents assignments to Boolean variables in a compact way.
 * Assigned values can be one of '0', '1' and 'X', where 'X' means <em>don't
 * care</em>.
 */
struct VATA::SymbolicVarAsgn
{
public:   // Public data types

	enum
	{
		ZERO = 0x01,
		ONE = 0x02,
		DONT_CARE = 0x03
	};

	using AssignmentList = std::vector<SymbolicVarAsgn>;

private:  // Private data types

	enum
	{
		BitsPerVariable = 2
	};


	enum
	{
		// needs to be multiple of BitsPerVariable
		BitsInChar = 8
	};

	enum
	{
		DefaultMask = 0x03
	};


private:  // Private data members


	/**
	 * @brief  Number of variables of the assignment
	 *
	 * The number of variables of the assignment, i. e., the index of the
	 * variable with the highest index - 1.
	 */
	size_t variablesCount_;


	/**
	 * @brief  The value of the assignment
	 *
	 * Array of characters representing the value of the assignment.
	 */
	std::vector<char> vars_;


private:  // Private methods

	static size_t numberOfChars(size_t varCount)
	{
		if (varCount == 0)
		{
			return 0;
		}
		else
		{
			return (varCount * BitsPerVariable - 1) / BitsInChar + 1;
		}
	}

	/**
	 * @brief  Gets index of @c char at given variable index
	 *
	 * Returns index of the @c char that holds the value of variable at given
	 * index.
	 *
	 * @see  getIndexInsideChar()
	 *
	 * @param[in]  index  Index of the Boolean variable
	 *
	 * @returns  Index of the @c char in which the variable has value
	 */
	static size_t getIndexOfChar(size_t index)
	{
		return (index * BitsPerVariable) / BitsInChar;
	}


	/**
	 * @brief  Gets index of bit inside a @c char for given variable index
	 *
	 * Returns index of a bit in a @c char that starts the block of bits that
	 * hold the value of variable at given index.
	 *
	 * @see  getIndexOfChar()
	 *
	 * @param[in]  index  Index of the Boolean variable
	 *
	 * @returns  Index of the bit that holds the value of the variable
	 */
	static size_t getIndexInsideChar(size_t index)
	{
		return (index * BitsPerVariable) % BitsInChar;
	}

	static void getAllSymbols(
		SymbolicVarAsgn&                  var,
		std::vector<SymbolicVarAsgn>&     vec,
		size_t                            pos);

public:   // Public methods

	/**
	 * @brief  The copy constructor
	 *
	 * @param[in]  asgn  The copied object
	 */
	SymbolicVarAsgn(const SymbolicVarAsgn& asgn) :
		variablesCount_(asgn.variablesCount_),
		vars_(asgn.vars_)
	{ }

	explicit SymbolicVarAsgn(size_t size) :
		variablesCount_(size),
		vars_(numberOfChars(size))
	{
		for (size_t i = 0; i < size; ++i)
		{	// for each variable
			SetIthVariableValue(i, DONT_CARE);
		}
	}

	SymbolicVarAsgn(size_t size, size_t n) :
		variablesCount_(size),
		vars_(numberOfChars(size))
	{
		for (size_t i = 0; i < size; ++i)
		{	// for each variable
			SetIthVariableValue(i, ((n & (1 << i)) != 0)? ONE : ZERO);
		}
	}

	/**
	 * @brief  Constructor from std::string
	 *
	 * A constructor that creates an instance of the class from a std::string
	 * that desribes the assignments to variable. The string can only hold
	 * values '0', '1' and 'X', where 'X' means <em>don't care</em>.
	 *
	 * @param[in]  value  The string with the value of variables
	 */
	explicit SymbolicVarAsgn(const std::string& value = "");


	/**
	 * @brief  The assignment operator
	 */
	SymbolicVarAsgn& operator=(const SymbolicVarAsgn& rhs)
	{
		if (this != &rhs)
		{
			variablesCount_ = rhs.variablesCount_;
			vars_ = rhs.vars_;
		}

		return *this;
	}

	/**
	 * @brief  Returns value of variable at given index
	 *
	 * Returns the value of variable at given index.
	 *
	 * @param[in]  i  Index of the variable
	 *
	 * @returns  Value of the variable
	 */
	inline char GetIthVariableValue(size_t i) const
	{
		// Assertions
		assert(i < length());

		return (vars_[getIndexOfChar(i)] >> getIndexInsideChar(i)) & DefaultMask;
	}


	inline void SetIthVariableValue(size_t i, char value)
	{
		// Assertions
		assert(i < length());
		assert((value == ZERO) || (value == ONE) || (value == DONT_CARE));

		// prepare the mask
		char mask = (DefaultMask << getIndexInsideChar(i)) ^ static_cast<char>(-1);

		// mask out bits at given position by the mask
		vars_[getIndexOfChar(i)] &= mask;

		// prepare new value of given bits
		value <<= getIndexInsideChar(i);

		// insert the value of given bits
		vars_[getIndexOfChar(i)] |= value;
	}

	void AddVariablesUpTo(size_t maxVariableIndex);


	/**
	 * @brief  The length of the variable assignment
	 *
	 * Returns the length, i.e., the number of variables, of the variable
	 * assignment.
	 *
	 * @returns  The length of the variable assignment
	 */
	inline size_t length() const
	{
		return variablesCount_;
	}


	/**
	 * @brief  Returns string representation
	 *
	 * Transforms the variable assignment to its string representation.
	 *
	 * @see  operator<<()
	 *
	 * @returns  The string representation of the variable assignment
	 */
	std::string ToString() const;


	/**
	 * @brief  Returns all assignments of given variables
	 *
	 * This static class method returns all assignments for all variables with
	 * index lower than the parameter.
	 *
	 * @param[in]  variablesCount  Index denoting that all variables with smaller
	 *                             index should be assigned.
	 *
	 * @returns  The list of all assignments to given variables
	 */
	static AssignmentList GetAllAssignments(size_t variablesCount);


	SymbolicVarAsgn& operator++();

	SymbolicVarAsgn operator++(int)
	{
		SymbolicVarAsgn result(*this);

		++(*this);

		return result;
	}

	void append(const SymbolicVarAsgn& prefix)
	{
		size_t offset = variablesCount_;
		variablesCount_ += prefix.length();

		vars_.resize(numberOfChars(variablesCount_));
		for (size_t i = 0; i < prefix.length(); ++i)
		{
			SetIthVariableValue(offset + i, prefix.GetIthVariableValue(i));
		}
	}


	std::vector<SymbolicVarAsgn> GetVectorOfConcreteSymbols() const;


	/**
	 * @brief  Overloaded << operator
	 *
	 * Overloaded << operator for output stream.
	 *
	 * @see  ToString()
	 *
	 * @param[in]  os    The output stream
	 * @param[in]  asgn  Assignment to the variables
	 *
	 * @returns  Modified output stream
	 */
	friend std::ostream& operator<<(
		std::ostream&                    os,
		const SymbolicVarAsgn&           asgn)
	{
		return (os << asgn.ToString());
	}


	friend bool operator<(
		const SymbolicVarAsgn&     lhs,
		const SymbolicVarAsgn&     rhs)
	{
		if ((lhs.length() < rhs.length()) || rhs.length() < lhs.length())
		{
			return lhs.length() < rhs.length();
		}

		for (size_t i = 0; i < lhs.length(); ++i)
		{
			char lhsIthValue = lhs.GetIthVariableValue(lhs.length() - i - 1);
			char rhsIthValue = rhs.GetIthVariableValue(rhs.length() - i - 1);

			switch (lhsIthValue)
			{
				case SymbolicVarAsgn::ZERO:
					switch (rhsIthValue)
					{
						case SymbolicVarAsgn::ZERO:       continue;     break;
						case SymbolicVarAsgn::ONE:        return true;  break;
						case SymbolicVarAsgn::DONT_CARE:  return true;  break;
						default: assert(false); break;   // fail gracefully
					}
					break;

				case SymbolicVarAsgn::ONE:
					switch (rhsIthValue)
					{
						case SymbolicVarAsgn::ZERO:       return false;  break;
						case SymbolicVarAsgn::ONE:        continue;      break;
						case SymbolicVarAsgn::DONT_CARE:  return false;  break;
						default: assert(false); break;   // fail gracefully
					}
					break;

				case SymbolicVarAsgn::DONT_CARE:
					switch (rhsIthValue)
					{
						case SymbolicVarAsgn::ZERO:       return false;  break;
						case SymbolicVarAsgn::ONE:        return true;   break;
						case SymbolicVarAsgn::DONT_CARE:  continue;      break;
						default: assert(false); break;   // fail gracefully
					}
					break;

					default: assert(false); break;     // fail gracefully
			}
		}

		return false;
	}


public:   // Public static methods

	static SymbolicVarAsgn GetUniversalSymbol()
	{
		return SymbolicVarAsgn(0);
	}
};

#endif
