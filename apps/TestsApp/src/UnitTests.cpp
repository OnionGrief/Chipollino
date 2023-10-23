#include "AutomatonToImage/AutomatonToImage.h"
#include "InputGenerator/RegexGenerator.h"
#include "InputGenerator/TasksGenerator.h"
#include "Interpreter/Interpreter.h"
#include "Objects/FiniteAutomaton.h"
#include "Objects/Grammar.h"
#include "Objects/Language.h"
#include "Objects/Regex.h"
#include "Objects/TransformationMonoid.h"
#include "Objects/iLogTemplate.h"
#include "Tester/Tester.h"
#include <cassert>
#include <functional>
#include <iostream>
#include "gtest/gtest.h"

TEST(TestCaseName, GlaisterShallit) {
	auto check_classes_number = [](string rgx_str) {
		return (Regex(rgx_str).to_glushkov().get_classes_number_GlaisterShallit());
	};
	EXPECT_EQ(1, 1);
	EXPECT_EQ(check_classes_number("abs"), 4);
	EXPECT_EQ(check_classes_number("a*b*c*"), 3);
	EXPECT_EQ(check_classes_number("aa*bb*cc*"), 4);
	EXPECT_EQ(check_classes_number("ab|abc"), 4);
	EXPECT_EQ(check_classes_number("a(b|c)(a|b)(b|c)"), 5);
}



TEST(TestCaseName, Test_random_regex_parsing) {
	RegexGenerator rg(15, 10, 5, 3);
	for (int i = 0; i < 30; i++) {
		string str = rg.generate_regex();
		Regex r1(str);
		string r1_str = r1.to_txt();
		Regex r2(r1_str);
		EXPECT_EQ(true, Regex::equivalent(r1, r2));
	}
}


TEST(TestCaseName, Test_test_fa_equal) {
	vector<State> states1;
	for (int i = 0; i < 6; i++) {
		State s = {i, {i}, to_string(i), false, map<alphabet_symbol, set<int>>()};
		states1.push_back(s);
	}
	states1[0].set_transition(1, "b");
	states1[0].set_transition(2, "b");
	states1[0].set_transition(5, "c");
	states1[1].set_transition(3, "a");
	states1[1].set_transition(4, "c");
	states1[2].set_transition(4, "a");
	states1[5].set_transition(4, "a");
	states1[3].is_terminal = true;
	states1[4].is_terminal = true;
	FiniteAutomaton fa1(0, states1, {"a", "b", "c"});

	vector<State> states2;
	for (int i = 0; i < 6; i++) {
		State s = {i, {i}, to_string(i), false, map<alphabet_symbol, set<int>>()};
		states2.push_back(s);
	}
	states2[0].set_transition(1, "b");
	states2[0].set_transition(2, "b");
	states2[0].set_transition(5, "c");
	states2[1].set_transition(3, "a");
	states2[1].set_transition(3, "c");
	states2[2].set_transition(4, "a");
	states2[5].set_transition(4, "a");
	states2[3].is_terminal = true;
	states2[4].is_terminal = true;
	FiniteAutomaton fa2(0, states2, {"a", "b", "c"});

	vector<State> states3;
	for (int i = 0; i < 6; i++) {
		State s = {i, {i}, to_string(i), false, map<alphabet_symbol, set<int>>()};
		states3.push_back(s);
	}
	states3[5].set_transition(4, "b");
	states3[5].set_transition(3, "b");
	states3[5].set_transition(0, "c");
	states3[4].set_transition(2, "a");
	states3[4].set_transition(1, "c");
	states3[3].set_transition(1, "a");
	states3[0].set_transition(1, "a");
	states3[2].is_terminal = true;
	states3[1].is_terminal = true;
	FiniteAutomaton fa3(5, states3, {"a", "b", "c"});

	EXPECT_TRUE(FiniteAutomaton::equal(fa1, fa1));
	EXPECT_TRUE(!FiniteAutomaton::equal(fa1, fa2));
	EXPECT_TRUE(FiniteAutomaton::equal(fa1, fa3));
	EXPECT_TRUE(FiniteAutomaton::equal(Regex("(aab|aab)*").to_thompson().remove_eps(),
								  Regex("(aab|aab)*").to_glushkov()));
	EXPECT_TRUE(FiniteAutomaton::equal(Regex("a(a)*ab(bb)*baa").to_thompson().remove_eps(),
								  Regex("a(a)*ab(bb)*baa").to_glushkov()));
	EXPECT_TRUE(FiniteAutomaton::equal(
		Regex("a(bbb*aaa*)*bb*|aaa*(bbb*aaa*)*|b(aaa*bbb*)*aa*|bbb*(aaa*bbb*)*")
			.to_thompson()
			.remove_eps(),
		Regex("a(bbb*aaa*)*bb*|aaa*(bbb*aaa*)*|b(aaa*bbb*)*aa*|bbb*(aaa*bbb*)*").to_glushkov()));
}
