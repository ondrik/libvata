/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    The database of automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>


const char* AUT_TIMBUK_A4 =
	"Ops\n"
	"Automaton anonymous\n"
	"States p q\n"
	"Final States q\n"
	"Transitions\n"
	"a -> p\n"
	"a(p) -> q\n"
	"a(p, q) -> r\n"
	;

const char* AUT_TIMBUK_UNION_1 =
	"Ops\n"
	"Automaton anonymous\n"
	"States\n"
	"Final States r\n"
	"Transitions\n"
	"a -> p\n"
	"b(p) -> q\n"
	"c(p, q) -> r\n"
	;

const char* AUT_TIMBUK_UNION_2 =
	"Ops\n"
	"Automaton anonymous\n"
	"States\n"
	"Final States v\n"
	"Transitions\n"
	"a -> t\n"
	"b(t) -> u\n"
	"c(u, u) -> v\n"
	"c(v, v) -> v\n"
	;

const char* AUT_TIMBUK_UNION_3 =
	"Ops\n"
	"Automaton anonymous\n"
	"States\n"
	"Final States r\n"
	"Transitions\n"
	"a -> s\n"
	"b(s) -> q\n"
	"c(q, q) -> r\n"
	"d(r, r) -> r\n"
	;

const char* AUT_TIMBUK_UNION_12_RESULT =
	"Ops\n"
	"Automaton anonymous\n"
	"States\n"
	"Final States v r\n"
	"Transitions\n"
	"a -> p\n"
	"a -> t\n"
	"b(p) -> q\n"
	"b(t) -> u\n"
	"c(p, q) -> r\n"
	"c(u, u) -> v\n"
	"c(v, v) -> v\n"
	;

const char* AUT_TIMBUK_UNION_13_RESULT =
	"Ops\n"
	"Automaton anonymous\n"
	"States\n"
	"Final States 4 1\n"
	"Transitions\n"
	"a -> 2\n"
	"a -> 6\n"
	"b(2) -> 3\n"
	"b(6) -> 5\n"
	"c(2, 3) -> 1\n"
	"c(5, 5) -> 4\n"
	"d(4, 4) -> 4\n"
	;

const char* AUT_TIMBUK_ISECT_1 =
	"Ops\n"
	"Automaton anonymous\n"
	"States\n"
	"Final States r\n"
	"Transitions\n"
	"a -> p\n"
	"b(p) -> q\n"
	"b(q) -> q\n"
	"c(p, q) -> r\n"
	;

const char* AUT_TIMBUK_ISECT_2 =
	"Ops\n"
	"Automaton anonymous\n"
	"States\n"
	"Final States h\n"
	"Transitions\n"
	"a -> g\n"
	"z -> g\n"
	"b(g) -> f\n"
	"c(g, f) -> h\n"
	"c(g, h) -> h\n"
	;

const char* AUT_TIMBUK_ISECT_12_RESULT =
	"Ops\n"
	"Automaton anonymous\n"
	"States\n"
	"Final States 1\n"
	"Transitions\n"
	"a -> 2\n"
	"b(2) -> 4\n"
	"b(5) -> 4\n"
	"c(2, 4) -> 1\n"
	"c(2, 3) -> 1\n"
	;
