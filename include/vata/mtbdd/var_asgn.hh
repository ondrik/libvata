/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    File with VarAsgn structure.
 *
 *****************************************************************************/

#ifndef _VATA_VARIABLE_ASSIGNMENT_HH_
#define _VATA_VARIABLE_ASSIGNMENT_HH_

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
	namespace MTBDDPkg
	{
		struct VarAsgn;
	}
}


/**
 * @brief   Variable assignment
 *
 * A class that represents assignments to Boolean variables in a compact way.
 * Assigned values can be one of '0', '1' and 'X', where 'X' means <em>don't
 * care</em>.
 */
struct VATA::MTBDDPkg::VarAsgn
{
public:   // Public data types

	enum
	{
		ZERO = 0x01,
		ONE = 0x02,
		DONT_CARE = 0x03
	};

	typedef std::vector<VarAsgn> AssignmentList;

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

	static inline size_t numberOfChars(size_t varCount)
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
	static inline size_t getIndexOfChar(size_t index)
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
	static inline size_t getIndexInsideChar(size_t index)
	{
		return (index * BitsPerVariable) % BitsInChar;
	}

	static void getAllSymbols(VarAsgn& var,
		std::vector<VarAsgn>& vec, size_t pos);

public:   // Public methods

	explicit VarAsgn(size_t size) :
		variablesCount_(size),
		vars_(numberOfChars(size))
	{
		for (size_t i = 0; i < size; ++i)
		{	// for each variable
			SetIthVariableValue(i, DONT_CARE);
		}
	}

	VarAsgn(size_t size, size_t n) :
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
	explicit VarAsgn(const std::string& value);


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
	 * index lvoer than te parameter.
	 *
	 * @param[in]  variablesCount  Index denoting that all variables with smaller
	 *                             index should be assigned.
	 *
	 * @returns  The list of all assignments to given variables
	 */
	static AssignmentList GetAllAssignments(size_t variablesCount);


	VarAsgn& operator++();

	inline VarAsgn operator++(int)
	{
		VarAsgn result(*this);

		++(*this);

		return result;
	}

	inline void append(const VarAsgn& prefix)
	{
		size_t offset = variablesCount_;
		variablesCount_ += prefix.length();

		vars_.resize(numberOfChars(variablesCount_));
		for (size_t i = 0; i < prefix.length(); ++i)
		{
			SetIthVariableValue(offset + i, prefix.GetIthVariableValue(i));
		}
	}


	std::vector<VarAsgn> GetVectorOfConcreteSymbols() const;


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
	friend std::ostream& operator<<(std::ostream& os,
		const VarAsgn& asgn)
	{
		return (os << asgn.ToString());
	}

	friend bool operator<(const VarAsgn& lhs,
		const VarAsgn& rhs)
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
				case ZERO:
					switch (rhsIthValue)
					{
						case ZERO: continue; break;
						case ONE: return true; break;
						case DONT_CARE: return true; break;
						default: assert(false); break;   // fail gracefully
					}
					break;

				case ONE:
					switch (rhsIthValue)
					{
						case ZERO: return false; break;
						case ONE: continue; break;
						case DONT_CARE: return false; break;
						default: assert(false); break;   // fail gracefully
					}
					break;

				case DONT_CARE:
					switch (rhsIthValue)
					{
						case ZERO: return false; break;
						case ONE: return true; break;
						case DONT_CARE: continue; break;
						default: assert(false); break;   // fail gracefully
					}
					break;

					default: assert(false); break;     // fail gracefully
			}
		}

		return false;
	}

public:   // Public static methods

	static VarAsgn GetUniversalSymbol()
	{
		return VarAsgn(0);
	}
};

#endif
