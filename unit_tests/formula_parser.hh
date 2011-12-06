/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for FormulaParser; static class that parses formulae.
 *
 *****************************************************************************/

#ifndef _VATA_FORMULA_PARSER_HH_
#define _VATA_FORMULA_PARSER_HH_

// standard library headers
#include <string>
#include <vector>

// Boost headers
#include <boost/algorithm/string.hpp>


/**
 * @brief   Parser of formulae
 *
 * Static class that is used to parse some types of formulae.
 */
class FormulaParser
{
public:   // public types

	/**
	 * @brief  Type for variable occurence
	 *
	 * Type that holds name of a variable and information whether it occurs in
	 * positive or negative.
	 */
	typedef std::pair<std::string, bool> VariableOccurenceType;

	/**
	 * @brief  List of variable occurences
	 *
	 * List of variable occurences of type VariableOccurenceType.
	 */
	typedef std::vector<VariableOccurenceType> VariableListType;

	/**
	 * @brief  Parser result type for Boolean
	 *
	 * Result type of the parser of formulae with Boolean value (the value is
	 * always TRUE)
	 */
	typedef VariableListType ParserResultBooleanType;

	/**
	 * @brief  Parser result type for unsigned
	 *
	 * Result type of the parser of formulae with unsigned value.
	 */
	typedef std::pair<unsigned, VariableListType> ParserResultUnsignedType;

	/**
	 * @brief  Parser result type for vector of unsigneds
	 *
	 * Result type of the parser of formulae with vector of unsigned values.
	 */
	typedef std::pair<std::vector<unsigned>, VariableListType>
		ParserResultUnsignedVecType;

private:  // private methods

	/**
	 * @brief  Private default constructor
	 *
	 * Default constructor which is private to disable creating an instance
	 * of the class.
	 */
	FormulaParser();


	/**
	 * @brief  Private copy constructor
	 *
	 * Copy constructor which is private to disable creating an instance
	 * of the class.
	 *
	 * @param[in]  formulaParser  The instance to be copied
	 */
	FormulaParser(const FormulaParser& formulaParser);


	/**
	 * @brief  Private assignment operator
	 *
	 * Assignment operator which is private to disable creating an instance
	 * of the class.
	 *
	 * @param[in]  formulaParser  The instance to be copied
	 *
	 * @returns  The resulting object
	 */
	FormulaParser& operator=(const FormulaParser& formulaParser);


	/**
	 * @brief  Private destructor
	 *
	 * Private destructor.
	 */
	~FormulaParser();


public:   // public methods

	/**
	 * @brief  Parser of formulae with Boolean result for test cases
	 *
	 * Parser of formulae with Boolean result for test cases. An example of the
	 * format of a formula follows:
	 *                         x1 * x2 * ~x3 * x4
	 *
	 * "x1", "x2", "x3", "x4": Boolean variables of the MTBDD (can be arbitrary
	 *                         strings)
	 * "~": denotes that the Boolean variable that follows is complemented 
	 * "*": times symbol
	 *
	 * @param[in]  input  The input string
	 * 
	 * @returns  Parser result
	 */
	static ParserResultBooleanType ParseExpressionBoolean(std::string input);


	/**
	 * @brief  Parser of formulae with unsigned result for test cases
	 *
	 * Parser of formulae with unsigned result for test cases. An example of the
	 * format of a formula follows:
	 *                       x1 * x2 * ~x3 * x4 = 5
	 *
	 * "x1", "x2", "x3", "x4": Boolean variables of the MTBDD (can be arbitrary
	 *                         strings)
	 * "~": denotes that the Boolean variable that follows is complemented 
	 * "*": times symbol
	 * "5": value that is saved to the leaf
	 *
	 * @param[in]  input  The input string
	 * 
	 * @returns  Parser result
	 */
	static ParserResultUnsignedType ParseExpressionUnsigned(std::string input);


	/**
	 * @brief  Parser of formulae with unsigneds vector result for test cases
	 *
	 * Parser of formulae with unsigneds vector result for test cases. An
	 * example of the format of a formula follows:
	 *                         x1 * x2 * ~x3 * x4 = { 1 , 2 }
	 *
	 * "x1", "x2", "x3", "x4": Boolean variables of the MTBDD (can be arbitrary
	 *                         strings)
	 * "~": denotes that the Boolean variable that follows is complemented 
	 * "*": times symbol
	 * "{": start of result set
	 * "}": end of result set
	 * "1", "2": elements of the result set
	 *
	 * @param[in]  input  The input string
	 * 
	 * @returns  Parser result
	 */
	static ParserResultUnsignedVecType ParseExpressionUnsignedVec(
		std::string input);


	/**
	 * @brief  Transforms parser result to string
	 *
	 * Transforms parser result to string.
	 *
	 * @param[in]  prsRes  Parser result
	 *
	 * @returns  String that represents the parser result
	 */
	static std::string parserResultToString(
		const ParserResultUnsignedType& prsRes);


	/**
	 * @copybrief  parserResultToString()
	 *
	 * @copydetails  parserResultToString()
	 */
	static std::string parserResultToString(
		const ParserResultUnsignedVecType& prsRes);
};

#endif
