/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Test suite for BDD based tree automaton
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>

using VATA::BDDTreeAut;
using VATA::Util::Convert;
using VATA::Parsing::TimbukParser;
using VATA::Serialization::TimbukSerializer;

// Boost headers
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE BDDTreeAut
#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string.hpp>

// testing headers
#include "log_fixture.hh"


/******************************************************************************
 *                                  Constants                                 *
 ******************************************************************************/

const char* AUT_A1 =
	"Ops \n"
	"Automaton A1\n"
	"States\n"
	"Final States\n"
	"Transitions\n";

const char* AUT_A2 =
	"Ops \n"
	"Automaton A2\n"
	"States\n"
	"Final States\n"
	"Transitions\n";



/******************************************************************************
 *                                  Fixtures                                  *
 ******************************************************************************/

/**
 * @brief  BDDTreeAut testing fixture
 *
 * Fixture for test of BDDTreeAut with characters as leaves.
 */
class BDDTreeAutFixture : public LogFixture
{

protected:  // methods

	bool areAutomataEqual(const std::string& lhs, const std::string& rhs) const
	{
		typedef std::vector<std::string> StringVector;

		StringVector splitLhs;

		boost::split(splitLhs, lhs, boost::algorithm::is_any_of("\n"),
			boost::algorithm::token_compress_on);

		return false;
	}
};


/******************************************************************************
 *                              Start of testing                              *
 ******************************************************************************/


BOOST_FIXTURE_TEST_SUITE(suite, BDDTreeAutFixture)

BOOST_AUTO_TEST_CASE(import_export)
{
	TimbukParser<BDDTreeAut> parser;
	BDDTreeAut* aut1 = new BDDTreeAut("");

	parser.LoadAut(*aut1, AUT_A1);

	TimbukSerializer<BDDTreeAut> serializer;

	std::string aut1Out = serializer.Serialize(*aut1);

	BOOST_CHECK_MESSAGE(areAutomataEqual(AUT_A1, aut1Out),
		"Expected serialization output is: \n" + std::string(AUT_A1) +
		"\n\ngot:\n" + aut1Out);

	delete aut1;
}

BOOST_AUTO_TEST_SUITE_END()

