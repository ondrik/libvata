/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    File with VarAsgn implementation.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/sym_var_asgn.hh>


using VATA::SymbolicVarAsgn;


SymbolicVarAsgn::SymbolicVarAsgn(
	const std::string&                      value) :
	variablesCount_(value.length()),
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


void SymbolicVarAsgn::getAllSymbols(
	SymbolicVarAsgn&                    var,
	std::vector<SymbolicVarAsgn>&       vec,
	size_t                              pos)
{
	if (pos == var.length())
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


void SymbolicVarAsgn::AddVariablesUpTo(size_t maxVariableIndex)
{
	size_t newVariablesCount = maxVariableIndex + 1;
	if (newVariablesCount > length())
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


std::string SymbolicVarAsgn::ToString() const
{
	std::string result;

	for (size_t i = 0; i < length(); ++i)
	{	// append all variables to the string
		switch (GetIthVariableValue(i))
		{
			case ZERO:      result += '0'; break;
			case ONE:       result += '1'; break;
			case DONT_CARE: result += 'X'; break;
			default: assert(false); break;   // fail gracefully
		}
	}

	return result;
}


SymbolicVarAsgn::AssignmentList SymbolicVarAsgn::GetAllAssignments(size_t variablesCount)
{
	std::string str;
	for (size_t i = 0; i < variablesCount; ++i)
	{	// for all variables
		str += 'X';
	}

	AssignmentList lst;
	lst.push_back(SymbolicVarAsgn(str));
	return lst;
}


SymbolicVarAsgn& SymbolicVarAsgn::operator++()
{
	for (size_t i = 0; i < length(); ++i)
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
			assert(false);    // fail gracefully
		}
	}

	return *this;
}


std::vector<SymbolicVarAsgn> SymbolicVarAsgn::GetVectorOfConcreteSymbols() const
{
	std::vector<SymbolicVarAsgn> result;

	SymbolicVarAsgn newVar = *this;

	getAllSymbols(newVar, result, 0);

	return result;
}
