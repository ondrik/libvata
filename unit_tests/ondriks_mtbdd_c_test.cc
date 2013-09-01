/*****************************************************************************
 *
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Test suite for OndriksMTBDD class with leaves as chars.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/sym_var_asgn.hh>
#include <vata/util/convert.hh>

#include "../src/mtbdd/apply1func.hh"
#include "../src/mtbdd/apply2func.hh"
#include "../src/mtbdd/apply3func.hh"
#include "../src/mtbdd/ondriks_mtbdd.hh"

using VATA::MTBDDPkg::OndriksMTBDD;
using VATA::MTBDDPkg::Apply1Functor;
using VATA::MTBDDPkg::Apply2Functor;
using VATA::MTBDDPkg::Apply3Functor;
using VATA::Util::Convert;


// Boost headers
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE OndriksMTBDDc
#include <boost/test/unit_test.hpp>
#include <boost/random/mersenne_twister.hpp>

// testing headers
#include "formula_parser.hh"
#include "log_fixture.hh"


/******************************************************************************
 *                                  Constants                                 *
 ******************************************************************************/


/**
 * Formulae for standard test cases to be stored in the MTBDD
 */
const char* const STANDARD_TEST_CASES[] =
{
	"~x0 * ~x1 *  x2 *  x3 =  3",
	"~x0 *  x1 * ~x2 * ~x3 =  4",
	" x0 * ~x1 * ~x2 *  x3 =  9",
	" x0 *  x1 *  x2 * ~x3 = 14",
	" x0 *  x1 *  x2 * ~x3 = 14",
	" x0 *  x1 *  x2 *  x3 = 15"
};


/**
 * Number of formulae for standard test cases in the MTBDD
 */
const unsigned STANDARD_TEST_CASES_SIZE =
	sizeof(STANDARD_TEST_CASES) / sizeof(const char* const);


/**
 * Formulae for standard test cases to be found not present in the MTBDD
 */
const char* const STANDARD_FAIL_CASES[] =
{
	"~x0 * ~x1 * ~x2 *  x3 =  1",
	"~x0 * ~x1 *  x2 * ~x3 =  2",
	"~x0 *  x1 * ~x2 *  x3 =  5",
	"~x0 *  x1 *  x2 * ~x3 =  6",
	"~x0 *  x1 *  x2 *  x3 =  7",
	" x0 * ~x1 * ~x2 * ~x3 =  8",
	" x0 * ~x1 *  x2 * ~x3 = 10",
	" x0 * ~x1 *  x2 *  x3 = 11",
	" x0 *  x1 * ~x2 * ~x3 = 12",
	" x0 *  x1 * ~x2 *  x3 = 13"
};

const unsigned char DEFAULT_DATA_VALUE = 0;

const unsigned VAR_COUNT = 64;

/**
 * Number of formulae for standard test cases that are to be not found in the
 * MTBDD
 */
const unsigned STANDARD_FAIL_CASES_SIZE =
	sizeof(STANDARD_FAIL_CASES) / sizeof(const char* const);


/**
 * Formulae for standard test cases with trimmed variables
 */
const char* const TRIMMED_STANDARD_TEST_CASES[] =
{
	"~x1 *  x3 = 12",
	" x1 * ~x3 = 18",
	" x1 *  x3 = 15"
};


/**
 * Number of formulae for standard test cases with trimmed variables.
 */
const unsigned TRIMMED_STANDARD_TEST_CASES_SIZE =
	sizeof(TRIMMED_STANDARD_TEST_CASES) / sizeof(const char* const);


/**
 * Number of variables of the MTBDD
 */
const unsigned NUM_VARIABLES = 64;

/**
 * The seed of the pseudorandom number generator
 */
const unsigned PRNG_SEED = 781436;

/**
 * Number of cases for large test formula test
 */
const unsigned LARGE_TEST_FORMULA_CASES = 200;


/******************************************************************************
 *                                  Fixtures                                  *
 ******************************************************************************/

/**
 * @brief  OndriksMTBDD char test fixture
 *
 * Fixture for test of OndriksMTBDD with characters as leaves.
 */
class OndriksMTBDDCharFixture : public LogFixture
{
private:  // private constants

public:   // public types

	/**
	 * @brief  Root type
	 *
	 * The type of MTBDD root
	 */
	typedef unsigned char RootType;

	/**
	 * @brief  Leaf type
	 *
	 * The type of MTBDD leaf
	 */
	typedef unsigned char DataType;


	/**
	 * @brief  List of test cases
	 *
	 * List of strings with formulae for test cases.
	 */
	typedef std::vector<std::string> ListOfTestCasesType;

	/**
	 * @brief  Variable assignment
	 *
	 * Type for variable assignment used in the test.
	 */
	typedef VATA::SymbolicVarAsgn VarAsgn;

	/**
	 * @brief  Ondrik's MTBDD type
	 *
	 * Type for Ondrik's MTBDD type
	 */
	typedef OndriksMTBDD<DataType> MTBDD;

	/**
	 * @brief  Dictionary of variables
	 *
	 * Type for dictionary of variables mapping variable name to index.
	 */
	typedef std::map<std::string, unsigned> VariableNameDictionary;

private:  // private data members


	/**
	 * Counter of known variables.
	 */
	unsigned varCounter_;


	/**
	 * Dictionary mapping variables' names to variable indices.
	 */
	VariableNameDictionary varDict_;

private:  // private methods

	/**
	 * @brief  Copy constructor
	 *
	 * Copy constructor.
	 *
	 * @param[in]  fixture  Input fixture
	 */
	OndriksMTBDDCharFixture(
		const OndriksMTBDDCharFixture& fixture);


	/**
	 * @brief  Assignment operator
	 *
	 * Assignment operator.
	 *
	 * @param[in]  rhs  Assigned value
	 *
	 * @returns  The new value
	 */
	OndriksMTBDDCharFixture& operator=(
		const OndriksMTBDDCharFixture& rhs);

public:   // public methods

	/**
	 * @brief  Default constructor
	 *
	 * Default constructor.
	 */
	OndriksMTBDDCharFixture()
		: varCounter_(0), varDict_()
	{ }

protected:// protected methods

	/**
	 * @brief  Loads standard tests
	 *
	 * A routine that loads passed structures with standard tests.
	 *
	 * @param[out]  testCases    Reference to list of test cases that is to be
	 *                           filled
	 * @param[out]  failedCases  Reference to list of failing test cases that is
	 *                           to be filled
	 */
	static void loadStandardTests(ListOfTestCasesType& testCases,
		ListOfTestCasesType& failedCases)
	{
		// formulae that we wish to store in the BDD
		for (size_t i = 0; i < STANDARD_TEST_CASES_SIZE; ++i)
		{	// load test cases
			testCases.push_back(STANDARD_TEST_CASES[i]);
		}

		// formulae that we want to check that are not in the BDD
		for (unsigned i = 0; i < STANDARD_FAIL_CASES_SIZE; ++i)
		{	// load test cases
			failedCases.push_back(STANDARD_FAIL_CASES[i]);
		}
	}


	/**
	 * @brief  Translates variable name to variable index
	 *
	 * This method translates variable name to corresponding variable index
	 * using variable dictionary. In case the variable name is not in the
	 * dictionary, it is assigned new free index and the pair is inserted in the
	 * dictionary.
	 *
	 * @param[in]  varName  Variable name
	 *
	 * @returns  Variable index
	 */
	size_t translateVarNameToIndex(const std::string& varName)
	{
		VariableNameDictionary::const_iterator itDict;
		if ((itDict = varDict_.find(varName)) == varDict_.end())
		{	// in case the variable was not found in the dictionary
			itDict = varDict_.insert(std::make_pair(varName, varCounter_++)).first;
		}

		return itDict->second;
	}


	/**
	 * @brief  Translates list of variables to variable assignment
	 *
	 * This method takes a list of variables and their values (either normal or
	 * complemented) and transforms it into the variable assignment structure.
	 *
	 * @param[in]  varList  List of variables and their values
	 *
	 * @returns  Variable assignment data structure
	 */
	VarAsgn varListToAsgn(const FormulaParser::VariableListType& varList)
	{
		VarAsgn asgn(NUM_VARIABLES);

		for (FormulaParser::VariableListType::const_iterator itVar =
			varList.begin(); itVar != varList.end(); ++itVar)
		{	// for each variable in the list, change the corresponding assignment
			size_t index = translateVarNameToIndex(itVar->first);
			asgn.SetIthVariableValue(index, (itVar->second)?
				VarAsgn::ONE : VarAsgn::ZERO);
		}

		return asgn;
	}


	/**
	 * @brief  Creates MTBDDs for test cases
	 *
	 * This method creates in given MTBDD new root with leaves according to the
	 * set of test cases.
	 *
	 * @param[in]  bdd        An MTBDD
	 * @param[in]  testCases  Set of test cases to be inserted in the MTBDD
	 *
	 * @returns  New root with leaves according to the test cases
	 */
	MTBDD createMTBDDForTestCases(const ListOfTestCasesType& testCases)
	{
		GCC_DIAG_OFF(effc++)
		class CopyApply2Functor :
			public Apply2Functor<CopyApply2Functor, DataType, DataType, DataType>
		{
		GCC_DIAG_ON(effc++)

		public:

			inline DataType ApplyOperation(const DataType& data1, const DataType& data2)
			{
				if (data2 == getMTBDD2().GetDefaultValue())
				{
					return data1;
				}
				else
				{
					return data2;
				}
			}
		};

		CopyApply2Functor copyFunc;

		MTBDD bdd(VarAsgn(VAR_COUNT), DEFAULT_DATA_VALUE, DEFAULT_DATA_VALUE);

		for (ListOfTestCasesType::const_iterator itTests = testCases.begin();
			itTests != testCases.end(); ++itTests)
		{
			FormulaParser::ParserResultUnsignedType prsRes =
				FormulaParser::ParseExpressionUnsigned(*itTests);
			DataType leafValue = static_cast<DataType>(prsRes.first);
			VarAsgn asgn = varListToAsgn(prsRes.second);

			MTBDD tmp(asgn, leafValue, DEFAULT_DATA_VALUE);
			bdd = copyFunc(bdd, tmp);
		}

		return bdd;
	}
};


/******************************************************************************
 *                              Start of testing                              *
 ******************************************************************************/


BOOST_FIXTURE_TEST_SUITE(suite, OndriksMTBDDCharFixture)

BOOST_AUTO_TEST_CASE(setters_and_getters_test)
{
	// load test cases
	ListOfTestCasesType testCases;
	ListOfTestCasesType failedCases;
	loadStandardTests(testCases, failedCases);

	MTBDD bdd = createMTBDDForTestCases(testCases);

	for (unsigned i = 0; i < testCases.size(); ++i)
	{	// test that the test cases have been stored properly
		FormulaParser::ParserResultUnsignedType prsRes =
			FormulaParser::ParseExpressionUnsigned(testCases[i]);
		DataType leafValue = static_cast<DataType>(prsRes.first);
		VarAsgn asgn = varListToAsgn(prsRes.second);

		BOOST_CHECK_MESSAGE(bdd.GetValue(asgn) == leafValue,
			testCases[i] + " != " + Convert::ToString(bdd.GetValue(asgn)));

		for (ListOfTestCasesType::const_iterator itFailed = failedCases.begin();
			itFailed != failedCases.end(); ++itFailed)
		{	// for every test case that should fail
			FormulaParser::ParserResultUnsignedType prsFailedRes =
				FormulaParser::ParseExpressionUnsigned(*itFailed);
			VarAsgn failedAsgn = varListToAsgn(prsFailedRes.second);

			BOOST_CHECK_MESSAGE(bdd.GetValue(failedAsgn) == bdd.GetDefaultValue(),
				testCases[i] + " == " + Convert::ToString(bdd.GetValue(failedAsgn)));
		}
	}
}

BOOST_AUTO_TEST_CASE(large_diagram_test)
{
	boost::mt19937 prnGen(PRNG_SEED);

	// formulae that we wish to store in the BDD
	ListOfTestCasesType testCases;

	for (unsigned i = 0; i < LARGE_TEST_FORMULA_CASES; ++i)
	{	// generate test cases
		std::string formula;

		for (unsigned j = 0; j < NUM_VARIABLES; ++j)
		{
			if (prnGen() % 4 != 0)
			{
				formula += (formula.empty()? "" : " * ") +
					Convert::ToString((prnGen() % 2 == 0)? " " : "~")
					+ "x" + Convert::ToString(j);
			}
		}

		DataType randomNum;
		while ((randomNum = prnGen()) == 0) ;   // generate non-zero random number

		formula += " = " + Convert::ToString(static_cast<unsigned>(randomNum));

		testCases.push_back(formula);
	}

	// formulae that we want to check that are not in the BDD
	ListOfTestCasesType failedCases;

	for (unsigned i = 0; i < LARGE_TEST_FORMULA_CASES; ++i)
	{	// generate failed test cases
		std::string formula;

		for (unsigned j = 0; j < NUM_VARIABLES; ++j)
		{
			if (prnGen() % 31 != 0)
			{
				formula += (formula.empty()? "" : " * ") +
					Convert::ToString((prnGen() % 2 == 0)? " " : "~")
					+ "x" + Convert::ToString(j);
			}
		}

		formula += " = " + Convert::ToString(static_cast<unsigned>(1));

		failedCases.push_back(formula);
	}

	MTBDD bdd = createMTBDDForTestCases(testCases);

	for (ListOfTestCasesType::const_iterator itTests = testCases.begin();
		itTests != testCases.end(); ++itTests)
	{	// test that the test cases have been stored properly
		#if DEBUG
			BOOST_TEST_MESSAGE("Finding stored " + *itTests);
		#endif
		FormulaParser::ParserResultUnsignedType prsRes =
			FormulaParser::ParseExpressionUnsigned(*itTests);
		DataType leafValue = static_cast<DataType>(prsRes.first);
		VarAsgn asgn = varListToAsgn(prsRes.second);

		BOOST_CHECK_MESSAGE(bdd.GetValue(asgn) == leafValue,
			*itTests + " != " + Convert::ToString(bdd.GetValue(asgn)));
	}

	for (ListOfTestCasesType::const_iterator itFailed = failedCases.begin();
		itFailed != failedCases.end(); ++itFailed)
	{	// for every test case that should fail
		#if DEBUG
			BOOST_TEST_MESSAGE("Finding failed " + *itFailed);
		#endif
		FormulaParser::ParserResultUnsignedType prsFailedRes =
			FormulaParser::ParseExpressionUnsigned(*itFailed);
		VarAsgn asgn = varListToAsgn(prsFailedRes.second);

		BOOST_CHECK_MESSAGE(bdd.GetValue(asgn) == bdd.GetDefaultValue(),
			*itFailed + " == " + Convert::ToString(bdd.GetValue(asgn)));
	}
}


BOOST_AUTO_TEST_CASE(no_variables_formula)
{
	const char* const TEST_VALUE = " = 42";

	FormulaParser::ParserResultUnsignedType prsRes =
		FormulaParser::ParseExpressionUnsigned(TEST_VALUE);
	DataType leafValue = static_cast<DataType>(prsRes.first);
	VarAsgn asgn = varListToAsgn(prsRes.second);
	MTBDD bdd(asgn, leafValue, DEFAULT_DATA_VALUE);

	BOOST_CHECK_MESSAGE(bdd.GetValue(asgn) == leafValue,
		Convert::ToString(TEST_VALUE) + " != " +
		Convert::ToString(bdd.GetValue(asgn)));
}


BOOST_AUTO_TEST_CASE(multiple_independent_bdds)
{
	// load test cases for the first BDD
	ListOfTestCasesType testCases1;
	ListOfTestCasesType failedCases1;
	loadStandardTests(testCases1, failedCases1);

	// load test cases for the second BDD
	ListOfTestCasesType testCases2;
	ListOfTestCasesType failedCases2;
	loadStandardTests(failedCases2, testCases2);

	MTBDD bdd1 = createMTBDDForTestCases(testCases1);
	MTBDD bdd2 = createMTBDDForTestCases(testCases2);

	for (ListOfTestCasesType::const_iterator itTests = testCases1.begin();
		itTests != testCases1.end(); ++itTests)
	{	// test that the test cases have been stored properly
		#if DEBUG
			BOOST_TEST_MESSAGE("Finding stored " + *itTests);
		#endif
		FormulaParser::ParserResultUnsignedType prsRes =
			FormulaParser::ParseExpressionUnsigned(*itTests);
		DataType leafValue = static_cast<DataType>(prsRes.first);
		VarAsgn asgn = varListToAsgn(prsRes.second);

		BOOST_CHECK_MESSAGE(bdd1.GetValue(asgn) == leafValue,
			*itTests + " != " + Convert::ToString(bdd2.GetValue(asgn)));
	}

	for (ListOfTestCasesType::const_iterator itTests = testCases2.begin();
		itTests != testCases2.end(); ++itTests)
	{	// test that the test cases have been stored properly
		#if DEBUG
			BOOST_TEST_MESSAGE("Finding stored " + *itTests);
		#endif
		FormulaParser::ParserResultUnsignedType prsRes =
			FormulaParser::ParseExpressionUnsigned(*itTests);
		DataType leafValue = static_cast<DataType>(prsRes.first);
		VarAsgn asgn = varListToAsgn(prsRes.second);

		BOOST_CHECK_MESSAGE(bdd2.GetValue(asgn) == leafValue,
			*itTests + " != " + Convert::ToString(bdd2.GetValue(asgn)));
	}

	for (ListOfTestCasesType::const_iterator itFailed = failedCases1.begin();
		itFailed != failedCases1.end(); ++itFailed)
	{	// for every test case that should fail
		#if DEBUG
			BOOST_TEST_MESSAGE("Finding failed " + *itFailed);
		#endif
		FormulaParser::ParserResultUnsignedType prsFailedRes =
			FormulaParser::ParseExpressionUnsigned(*itFailed);
		VarAsgn asgn = varListToAsgn(prsFailedRes.second);

		BOOST_CHECK_MESSAGE(bdd1.GetValue(asgn) == bdd1.GetDefaultValue(),
			*itFailed + " == " + Convert::ToString(bdd1.GetValue(asgn)));
	}

	for (ListOfTestCasesType::const_iterator itFailed = failedCases2.begin();
		itFailed != failedCases2.end(); ++itFailed)
	{	// for every test case that should fail
		#if DEBUG
			BOOST_TEST_MESSAGE("Finding failed " + *itFailed);
		#endif
		FormulaParser::ParserResultUnsignedType prsFailedRes =
			FormulaParser::ParseExpressionUnsigned(*itFailed);
		VarAsgn asgn = varListToAsgn(prsFailedRes.second);

		BOOST_CHECK_MESSAGE(bdd2.GetValue(asgn) == bdd2.GetDefaultValue(),
			*itFailed + " == " + Convert::ToString(bdd2.GetValue(asgn)));
	}
}


BOOST_AUTO_TEST_CASE(monadic_apply)
{
	// load test cases
	ListOfTestCasesType testCases;
	ListOfTestCasesType failedCases;
	loadStandardTests(testCases, failedCases);

	MTBDD bdd = createMTBDDForTestCases(testCases);

	// apply functor that squares values in leaves
	GCC_DIAG_OFF(effc++)
	class SquareMonadicApplyFunctor :
		public Apply1Functor<SquareMonadicApplyFunctor, DataType, DataType>
	{
	GCC_DIAG_ON(effc++)

	public:

		inline DataType ApplyOperation(const DataType& val)
		{
			return val * val;
		}
	};

	SquareMonadicApplyFunctor func;

	MTBDD squaredBdd = func(bdd);

	for (ListOfTestCasesType::const_iterator itTests = testCases.begin();
		itTests != testCases.end(); ++itTests)
	{	// test that the test cases have been stored properly
		#if DEBUG
			BOOST_TEST_MESSAGE("Finding stored " + *itTests);
		#endif
		FormulaParser::ParserResultUnsignedType prsRes =
			FormulaParser::ParseExpressionUnsigned(*itTests);
		DataType leafValue = static_cast<DataType>(prsRes.first);
		leafValue *= leafValue;
		VarAsgn asgn = varListToAsgn(prsRes.second);

		BOOST_CHECK_MESSAGE(squaredBdd.GetValue(asgn) == leafValue,
			*itTests + " != " + Convert::ToString(squaredBdd.GetValue(asgn)));
	}
}


BOOST_AUTO_TEST_CASE(apply)
{
	// load test cases
	ListOfTestCasesType testCases;
	ListOfTestCasesType failedCases;
	loadStandardTests(testCases, failedCases);

	MTBDD bdd = createMTBDDForTestCases(testCases);

	// apply functor that squares values in leaves
	GCC_DIAG_OFF(effc++)
	class TimesApply2Functor :
		public Apply2Functor<TimesApply2Functor, DataType, DataType, DataType>
	{
	GCC_DIAG_ON(effc++)

	public:

		inline DataType ApplyOperation(const DataType& lhs, const DataType& rhs)
		{
			return lhs * rhs;
		}
	};

	TimesApply2Functor func;

	MTBDD timesBdd = func(bdd, bdd);

	for (ListOfTestCasesType::const_iterator itTests = testCases.begin();
		itTests != testCases.end(); ++itTests)
	{	// test that the test cases have been stored properly
		#if DEBUG
			BOOST_TEST_MESSAGE("Finding stored " + *itTests);
		#endif
		FormulaParser::ParserResultUnsignedType prsRes =
			FormulaParser::ParseExpressionUnsigned(*itTests);
		DataType leafValue = static_cast<DataType>(prsRes.first);
		leafValue *= leafValue;
		VarAsgn asgn = varListToAsgn(prsRes.second);

		BOOST_CHECK_MESSAGE(timesBdd.GetValue(asgn) == leafValue,
			*itTests + " != " + Convert::ToString(timesBdd.GetValue(asgn)));
	}
}


BOOST_AUTO_TEST_CASE(ternary_apply)
{
	// load test cases
	ListOfTestCasesType testCases;
	ListOfTestCasesType failedCases;
	loadStandardTests(testCases, failedCases);

	MTBDD bdd = createMTBDDForTestCases(testCases);

	// apply functor that squares values in leaves
	GCC_DIAG_OFF(effc++)
	class TimesApplyFunctor :
		public Apply3Functor<TimesApplyFunctor, DataType, DataType, DataType,
		DataType>
	{
	GCC_DIAG_ON(effc++)

	public:

		inline DataType ApplyOperation(
			const DataType& lhs, const DataType& mhs, const DataType& rhs)
		{
			return lhs * mhs * rhs;
		}
	};

	TimesApplyFunctor func;

	MTBDD timesBdd = func(bdd, bdd, bdd);

	for (ListOfTestCasesType::const_iterator itTests = testCases.begin();
		itTests != testCases.end(); ++itTests)
	{	// test that the test cases have been stored properly
		#if DEBUG
			BOOST_TEST_MESSAGE("Finding stored " + *itTests);
		#endif
		FormulaParser::ParserResultUnsignedType prsRes =
			FormulaParser::ParseExpressionUnsigned(*itTests);
		DataType leafValue = static_cast<DataType>(prsRes.first);
		leafValue = leafValue * leafValue * leafValue;
		VarAsgn asgn = varListToAsgn(prsRes.second);

		BOOST_CHECK_MESSAGE(timesBdd.GetValue(asgn) == leafValue,
			*itTests + " != " + Convert::ToString(timesBdd.GetValue(asgn)));
	}
}


// BOOST_AUTO_TEST_CASE(variable_renaming)
// {
// 	ASMTBDDCC* bdd = new CuddMTBDDCC();
// 	bdd->SetBottomValue(0);
// 
// 	for (unsigned i = 0; i < NUM_VARIABLES; ++i)
// 	{	// fill the table of variables
// 		translateVarNameToIndex("x" + Convert::ToString(i));
// 	}
// 
// 	// load test cases
// 	ListOfTestCasesType testCases;
// 	ListOfTestCasesType failedCases;
// 	loadStandardTests(testCases, failedCases);
// 
// 	RootType root = createMTBDDForTestCases(bdd, testCases);
// 
// 	class MovingUpVariableRenamingFunctor
// 		: public ASMTBDDCC::AbstractVariableRenamingFunctorType
// 	{
// 	public:
// 
// 		virtual ASMTBDDCC::VariableType operator()(const ASMTBDDCC::VariableType& var)
// 		{
// 			if (var < NUM_VARIABLES/2)
// 			{
// 				return var + NUM_VARIABLES/2;
// 			}
// 			else
// 			{
// 				return var;
// 			}
// 		}
// 	};
// 
// 	MovingUpVariableRenamingFunctor funcMovingUp;
// 	RootType renamedRoot = bdd->RenameVariables(root, &funcMovingUp);
// 
// 	class MovingDownVariableRenamingFunctor
// 		: public ASMTBDDCC::AbstractVariableRenamingFunctorType
// 	{
// 	public:
// 
// 		virtual ASMTBDDCC::VariableType operator()(const ASMTBDDCC::VariableType& var)
// 		{
// 			if ((var >= NUM_VARIABLES/2) && (var < NUM_VARIABLES))
// 			{
// 				return 2 * (var - NUM_VARIABLES/2) + 1;
// 			}
// 			else
// 			{
// 				return var;
// 			}
// 		}
// 	};
// 
// 	MovingDownVariableRenamingFunctor funcMovingDown;
// 	renamedRoot = bdd->RenameVariables(renamedRoot, &funcMovingDown);
// 
// 
// 	ListOfTestCasesType renamedTestCases;
// 	for (ListOfTestCasesType::const_iterator itTests = testCases.begin();
// 		itTests != testCases.end(); ++itTests)
// 	{	// for each test case create new with properly renamed variables
// 		std::string renamedCase = *itTests;
// 
// 		for (unsigned i = 0; i < NUM_VARIABLES/2; ++i)
// 		{	// double the name of each variable, 
// 			boost::algorithm::replace_all(renamedCase, "x" + Convert::ToString(i) + " ",
// 				"x" + Convert::ToString(i+NUM_VARIABLES/2) + " ");
// 		}
// 
// 		for (unsigned i = NUM_VARIABLES/2; i < NUM_VARIABLES; ++i)
// 		{	// for each variable
// 			boost::algorithm::replace_all(renamedCase, "x" + Convert::ToString(i) + " ",
// 				"x" + Convert::ToString(2*(i-static_cast<int>(NUM_VARIABLES)/2)+1) + " ");
// 		}
// 
// 		renamedTestCases.push_back(renamedCase);
// 	}
// 
// 	for (ListOfTestCasesType::const_iterator itTests = renamedTestCases.begin();
// 		itTests != renamedTestCases.end(); ++itTests)
// 	{	// test that the test cases have been stored properly
// #if DEBUG
// 		BOOST_TEST_MESSAGE("Finding stored " + *itTests);
// #endif
// 		FormulaParser::ParserResultUnsignedType prsRes =
// 			FormulaParser::ParseExpressionUnsigned(*itTests);
// 		LeafType leafValue = static_cast<LeafType>(prsRes.first);
// 		VariableAssignment asgn = varListToAsgn(prsRes.second);
// 
// 		ASMTBDDCC::LeafContainer res;
// 		res.push_back(&leafValue);
// 
// 		BOOST_CHECK_MESSAGE(
// 			compareTwoLeafContainers(bdd->GetValue(renamedRoot, asgn), res), *itTests
// 			+ " != " + leafContainerToString(bdd->GetValue(renamedRoot, asgn)));
// 	}
// 
// 
// 	delete bdd;
// }
// 
// BOOST_AUTO_TEST_CASE(variable_trimming)
// {
// 	ASMTBDDCC* bdd = new CuddMTBDDCC();
// 	bdd->SetBottomValue(0);
// 
// 	for (unsigned i = 0; i < NUM_VARIABLES; ++i)
// 	{	// fill the table of variables
// 		translateVarNameToIndex("x" + Convert::ToString(i));
// 	}
// 
// 	// load test cases
// 	ListOfTestCasesType testCases;
// 	ListOfTestCasesType failedCases;
// 	loadStandardTests(testCases, failedCases);
// 
// 	RootType root = createMTBDDForTestCases(bdd, testCases);
// 
// 	class OddVariablePredicateFunctorType
// 		: public ASMTBDDCC::AbstractVariablePredicateFunctorType
// 	{
// 	public:
// 
// 		virtual bool operator()(const ASMTBDDCC::VariableType& var)
// 		{
// 			return var % 2 == 0;
// 		}
// 	};
// 
// 	OddVariablePredicateFunctorType oddPred;
// 
// 	class AdditionApplyFunctorType: public ASMTBDDCC::AbstractApplyFunctorType
// 	{
// 	public:
// 		virtual LeafType operator()(const LeafType& lhs, const LeafType& rhs)
// 		{
// 			return lhs + rhs;
// 		}
// 	};
// 
// 	AdditionApplyFunctorType addApply;
// 
// 	RootType trimmedRoot = bdd->TrimVariables(root, &oddPred, &addApply);
// 
// 	ListOfTestCasesType trimmedTestCases;
// 	// formulae that we wish to check
// 	for (size_t i = 0; i < TRIMMED_STANDARD_TEST_CASES_SIZE; ++i)
// 	{	// load test cases
// 		trimmedTestCases.push_back(TRIMMED_STANDARD_TEST_CASES[i]);
// 	}
// 
// 	for (ListOfTestCasesType::const_iterator itTests = trimmedTestCases.begin();
// 		itTests != trimmedTestCases.end(); ++itTests)
// 	{	// test that the test cases have been stored properly
// #if DEBUG
// 		BOOST_TEST_MESSAGE("Finding stored " + *itTests);
// #endif
// 		FormulaParser::ParserResultUnsignedType prsRes =
// 			FormulaParser::ParseExpressionUnsigned(*itTests);
// 		LeafType leafValue = static_cast<LeafType>(prsRes.first);
// 		VariableAssignment asgn = varListToAsgn(prsRes.second);
// 
// 		ASMTBDDCC::LeafContainer res;
// 		res.push_back(&leafValue);
// 
// 		BOOST_CHECK_MESSAGE(
// 			compareTwoLeafContainers(bdd->GetValue(trimmedRoot, asgn), res), *itTests
// 			+ " != " + leafContainerToString(bdd->GetValue(trimmedRoot, asgn)));
// 	}
// 	delete bdd;
// }

//BOOST_AUTO_TEST_CASE(serialization)
//{
//	ASMTBDDCC* bdd = new CuddMTBDDCC();
//
//	// load test cases
//	ListOfTestCasesType testCases;
//	ListOfTestCasesType failedCases;
//	loadStandardTests(testCases, failedCases);
//
//	RootType root = createMTBDDForTestCases(bdd, testCases);
//
//	std::string str = bdd->Serialize();
//
//	BOOST_TEST_MESSAGE("BDD:\n" + str);
//
//	delete bdd;
//}

BOOST_AUTO_TEST_SUITE_END()

