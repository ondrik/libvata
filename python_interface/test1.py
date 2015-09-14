#!/usr/bin/env python3

AUT1 = """
    Ops
    Automaton A6

    States q5:0 q4:0 q3:0 q2:0 q1:0 q0:0

    Final States q5

    Transitions
    bot0 -> q0
    black -> q1
    bot2(q0,q0) -> q0
    bot2(q0,q0) -> q1
    black(q1,q1) -> q3
    black(q3,q3) -> q2
    black(q3,q3) -> q4
    bot1(q4) -> q5
    red(q3,q3) -> q5
    """

AUT2 = """
    Automaton A7

    States q6:0 q5:0 q4:0 q3:0 q2:0 q1:0 q0:0

    Final States q5

    Transitions
    bot0 -> q0
    black -> q1
    bot2(q0,q0) -> q1
    black(q1,q1) -> q3
    black(q3,q3) -> q4
    bot1(q4) -> q5
    red(q3,q3) -> q5
    bot1(q5) -> q6
    red(q3,q6) -> q5
    """

if __name__ == '__main__':
    aut1 = vata_load_string(AUT1)
    aut2 = vata_load_string(AUT2)

    # union of aut1 and aut2
    aut_union = vata_union(aut1, aut2)

    # assert invariants
    # TODO: maybe keep only one direction of inclusion?
    assert aut1.is_included(aut_union)
    assert aut_union.includes(aut2)
    assert vata_inclusion(aut1, aut_union)
    assert vata_inclusion(aut2, aut_union)

    # complement of aut1
    aut1_cmpl = vata_complement(aut1)
    assert vata_intersection(aut1, aut1_cmpl).is_lang_empty()
    assert vata_union(aut1, aut1_cmpl).is_lang_universal()
