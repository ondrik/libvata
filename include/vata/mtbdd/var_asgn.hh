/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    File with VariableAssignment structure.
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
		struct VariableAssignment;
	}
}


/**
 * @brief   Variable assignment
 * @author  Ondra Lengal <ilengal@fit.vutbr.cz>
 * @date    2011
 *
 * A class that represents assignments to Boolean variables in a compact way.
 * Assigned values can be one of '0', '1' and 'X', where 'X' means <em>don't
 * care</em>.
 */
struct VATA::MTBDDPkg::VariableAssignment
{
public:   // Public data types


	enum
	{
		ZERO = 0x01,
		ONE = 0x02,
		DONT_CARE = 0x03
	};

	typedef std::vector<VariableAssignment> AssignmentList;

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

	static void getAllSymbols(VariableAssignment& var,
		std::vector<VariableAssignment>& vec, size_t pos)
	{
		if (pos == var.VariablesCount())
		{
			vec.push_back(var);
		}
		else
		{
			if (var.GetIthVariableValue(pos) == DONT_CARE)
			{	// in case we fork
				var.SetIthVariableValue(pos, ZERO);
				getAllSymbols(var, vec, pos + 1);
				var.SetIthVariableValue(pos, ONE);
				getAllSymbols(var, vec, pos + 1);
			}
			else
			{
				getAllSymbols(var, vec, pos + 1);
			}
		}
	}

public:   // Public methods

	explicit VariableAssignment(size_t size)
		: variablesCount_(size),
			vars_(numberOfChars(size))
	{
		for (size_t i = 0; i < size; ++i)
		{	// for each variable
			SetIthVariableValue(i, DONT_CARE);
		}
	}

	VariableAssignment(size_t size, size_t n)
		: variablesCount_(size),
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
	explicit VariableAssignment(const std::string& value)
		: variablesCount_(value.length()),
			vars_(numberOfChars(value.length()))
	{
		for (size_t i = 0; i < value.length(); ++i)
		{	// load the string into the array of variables
			char val = 0;
			switch (value[i])
			{
				case '0': val = ZERO;      break;
				case '1': val = ONE;       break;
				case 'X': val = DONT_CARE; break;
				default: throw std::runtime_error("Invalid input value!");
			}

			// prepare the mask
			char mask = (DefaultMask << getIndexInsideChar(i)) ^ static_cast<char>(-1);

			// mask out bits at given position by the mask
			vars_[getIndexOfChar(i)] &= mask;

			// prepare new value of given bits
			val <<= getIndexInsideChar(i);

			// insert the value of given bits
			vars_[getIndexOfChar(i)] |= val;
		}
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
		assert(i < VariablesCount());

		return (vars_[getIndexOfChar(i)] >> getIndexInsideChar(i)) & DefaultMask;
	}


	void SetIthVariableValue(size_t i, char value)
	{
		// Assertions
		assert(i < VariablesCount());

		switch (value)
		{
			case ZERO:      break;
			case ONE:       break;
			case DONT_CARE: break;
			default:        throw std::runtime_error("Invalid input value!");
		}

		// prepare the mask
		char mask = (DefaultMask << getIndexInsideChar(i)) ^ static_cast<char>(-1);

		// mask out bits at given position by the mask
		vars_[getIndexOfChar(i)] &= mask;

		// prepare new value of given bits
		value <<= getIndexInsideChar(i);

		// insert the value of given bits
		vars_[getIndexOfChar(i)] |= value;
	}

	void AddVariablesUpTo(size_t maxVariableIndex)
	{
		size_t newVariablesCount = maxVariableIndex + 1;
		if (newVariablesCount > VariablesCount())
		{
			size_t oldVariablesCount = variablesCount_;
			variablesCount_ = newVariablesCount;
			vars_.resize(numberOfChars(newVariablesCount));

			for (size_t i = oldVariablesCount; i < newVariablesCount; ++i)
			{
				SetIthVariableValue(i, DONT_CARE);
			}
		}
	}


	/**
	 * @brief  The number of variables
	 *
	 * Returns the number of variables of the variable assignment.
	 *
	 * @returns  The number of variables of the variable assignment
	 */
	inline size_t VariablesCount() const
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
	std::string ToString() const
	{
		std::string result;

		for (size_t i = 0; i < VariablesCount(); ++i)
		{	// append all variables to the string
			switch (GetIthVariableValue(i))
			{
				case ZERO:      result += '0'; break;
				case ONE:       result += '1'; break;
				case DONT_CARE: result += 'X'; break;
				default: throw std::runtime_error("Invalid bit value!");
			}
		}

		return result;
	}


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
	static AssignmentList GetAllAssignments(size_t variablesCount)
	{
		std::string str;
		for (size_t i = 0; i < variablesCount; ++i)
		{	// for all variables
			str += 'X';
		}

		AssignmentList lst;
		lst.push_back(VariableAssignment(str));
		return lst;
	}


	VariableAssignment& operator++()
	{
		for (size_t i = 0; i < VariablesCount(); ++i)
		{	// for each variable
			char value = GetIthVariableValue(i);
			if (value == ZERO)
			{	// in case we can stop here
				SetIthVariableValue(i, ONE);
				return *this;
			}
			else if (value == ONE)
			{	// we change to zero and continue to search zero
				SetIthVariableValue(i, ZERO);
			}
			else
			{	// otherwise
				throw std::runtime_error(
					"An attempt to increment assignment with invalid states");
			}
		}

		return *this;
	}

	inline std::vector<VariableAssignment> GetVectorOfConcreteSymbols() const
	{
		std::vector<VariableAssignment> result;

		VariableAssignment newVar = *this;

		getAllSymbols(newVar, result, 0);

		return result;
	}


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
		const VariableAssignment& asgn)
	{
		return (os << asgn.ToString());
	}

	friend bool operator<(const VariableAssignment& lhs,
		const VariableAssignment& rhs)
	{
		if ((lhs.VariablesCount() < rhs.VariablesCount()) ||
			rhs.VariablesCount() < lhs.VariablesCount())
		{
			return lhs.VariablesCount() < rhs.VariablesCount();
		}

		for (size_t i = 0; i < lhs.VariablesCount(); ++i)
		{
			char lhsIthValue = lhs.GetIthVariableValue(lhs.VariablesCount() - i - 1);
			char rhsIthValue = rhs.GetIthVariableValue(rhs.VariablesCount() - i - 1);

			switch (lhsIthValue)
			{
				case ZERO:
					switch (rhsIthValue)
					{
						case ZERO: continue; break;
						case ONE: return true; break;
						case DONT_CARE: return true; break;
						default: throw std::runtime_error("Invalid variable assignment value");
							break;
					}
					break;

				case ONE:
					switch (rhsIthValue)
					{
						case ZERO: return false; break;
						case ONE: continue; break;
						case DONT_CARE: return false; break;
						default: throw std::runtime_error("Invalid variable assignment value");
							break;
					}
					break;

				case DONT_CARE:
					switch (rhsIthValue)
					{
						case ZERO: return false; break;
						case ONE: return true; break;
						case DONT_CARE: continue; break;
						default: throw std::runtime_error("Invalid variable assignment value");
							break;
					}
					break;

				default: throw std::runtime_error("Invalid variable assignment value");
					break;
			}
		}

		return false;
	}

public:   // Public static methods

	static VariableAssignment GetUniversalSymbol()
	{
		return VariableAssignment(0);
	}
};

#endif
