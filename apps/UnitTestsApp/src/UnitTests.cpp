#include "UnitTestsApp/UnitTests.h"
#include "AutomatonToImage/AutomatonToImage.h"
#include "Interpreter/Interpreter.h"
#include "Objects/AlgExpression.h"
#include "Objects/BackRefRegex.h"
#include "Objects/FiniteAutomaton.h"
#include "Objects/Grammar.h"
#include "Objects/Language.h"
#include "Objects/MemoryFiniteAutomaton.h"
#include "Objects/Regex.h"
#include "Objects/TransformationMonoid.h"
#include "Tester/Tester.h"

using std::cout;
using std::map;
using std::set;
using std::string;
using std::unordered_map;
using std::unordered_set;
using std::vector;

TEST(TestParseString, FromString) {
	struct Test {
		string regex_str;
		bool want_err;
		bool bref;
		int lexemes_len = 0;
	};

	vector<Test> tests = {
		// тесты на ссылки и захваты памяти
		{"[]", true, true},
		{"[]:", true, true},
		{"[a]", true, true},
		{"[a]:", true, true},
		{"[[a]:1", true, true},
		{"a]:1", true, true},
		{"[a]:1", false, true, 3},
		{"&", true, true},
		{"&1", false, true, 1},
		{"[b[a]:1]:1", true, true},
		{"[b[a]:1&1]:2&2", false, true, 11},
		// тесты на отрицание
		{"^a", false, false, 2},
		{"a^|b", true, false},
		{"d^*^b", true, false},
		{"d|^|b", true, false},
		{"a^", false, false, 4},	 // a . ^ eps
		{"a|(c^)", false, false, 8}, // a | ( c . ^ eps )
		{"[b[a]:1&1]:2&2^a", true, true},
		{"[b[a]:1&1]:2&2^a", true, false},
	};

	for (const auto& t : tests) {
		std::stringstream message;
		message << "Case: " << t.regex_str << ", WantErr: " << t.want_err;
		SCOPED_TRACE(message.str());

		bool allow_ref = false;
		bool allow_negation = true;
		if (t.bref) {
			allow_ref = true;
			allow_negation = false;
		}
		vector<UnitTests::Lexeme> l =
			UnitTests::parse_string(t.regex_str, allow_ref, allow_negation);
		ASSERT_FALSE(l.empty());

		if (t.want_err) {
			ASSERT_EQ(UnitTests::LexemeType::error, l[0].type);
		} else {
			ASSERT_NE(UnitTests::LexemeType::error, l[0].type);
			ASSERT_EQ(t.lexemes_len, l.size());
			// TODO: добавить проверку содержимого l
		}
	}
}

TEST(TestNegativeRegex, Thompson) {
	vector<FAState> states;
	for (int i = 0; i < 9; i++) {
		states.emplace_back(i, set<int>{i}, std::to_string(i), false, FAState::Transitions());
	}

	states[0].add_transition(1, Symbol::Epsilon);
	states[0].add_transition(5, Symbol::Epsilon);

	states[1].add_transition(4, Symbol::Epsilon);
	states[1].add_transition(2, Symbol('a'));
	states[1].add_transition(3, Symbol('b'));
	states[1].add_transition(3, Symbol('c'));
	states[1].add_transition(4, Symbol::Epsilon);

	states[2].add_transition(3, Symbol('a'));
	states[2].add_transition(3, Symbol('b'));
	states[2].add_transition(3, Symbol('c'));

	states[3].add_transition(3, Symbol('a'));
	states[3].add_transition(3, Symbol('b'));
	states[3].add_transition(3, Symbol('c'));
	states[3].add_transition(4, Symbol::Epsilon);

	states[4].add_transition(7, Symbol::Epsilon);

	states[7].add_transition(8, Symbol('c'));

	states[5].add_transition(6, Symbol('b'));

	states[6].add_transition(7, Symbol::Epsilon);

	states[8].is_terminal = true;
	FiniteAutomaton fa(0, states, {Symbol('a'), Symbol('b'), Symbol('c')});

	ASSERT_TRUE(FiniteAutomaton::equal(fa, Regex("(^a|b)c").to_thompson()));
}

TEST(TestNegativeRegex, Antimirov) {
	vector<FAState> states;
	for (int i = 0; i < 5; i++) {
		states.emplace_back(i, set<int>{i}, std::to_string(i), false, FAState::Transitions());
	}

	states[0].add_transition(1, Symbol('a'));
	states[0].add_transition(2, Symbol('b'));
	states[0].add_transition(3, Symbol('b'));
	states[0].add_transition(2, Symbol('c'));
	states[0].add_transition(4, Symbol('c'));

	states[1].add_transition(2, Symbol('a'));
	states[1].add_transition(2, Symbol('b'));
	states[1].add_transition(2, Symbol('c'));

	states[2].add_transition(2, Symbol('a'));
	states[2].add_transition(2, Symbol('b'));
	states[2].add_transition(2, Symbol('c'));
	states[2].add_transition(4, Symbol('c'));

	states[3].add_transition(4, Symbol('c'));

	states[4].is_terminal = true;
	FiniteAutomaton fa(0, states, {Symbol('a'), Symbol('b'), Symbol('c')});

	ASSERT_TRUE(FiniteAutomaton::equal(fa, Regex("(^a|b)c").to_antimirov()));
}

TEST(TestEqual, FA_Equal) {
	vector<FAState> states1;
	for (int i = 0; i < 6; i++) {
		states1.emplace_back(i, set<int>({i}), std::to_string(i), false, FAState::Transitions());
	}
	states1[0].add_transition(1, Symbol('b'));
	states1[0].add_transition(2, Symbol('b'));
	states1[0].add_transition(5, Symbol('c'));
	states1[1].add_transition(3, Symbol('a'));
	states1[1].add_transition(4, Symbol('c'));
	states1[2].add_transition(4, Symbol('a'));
	states1[5].add_transition(4, Symbol('a'));
	states1[3].is_terminal = true;
	states1[4].is_terminal = true;
	FiniteAutomaton fa1(0, states1, {Symbol('a'), Symbol('b'), Symbol('c')});

	vector<FAState> states2;
	for (int i = 0; i < 6; i++) {
		states2.emplace_back(i, set<int>({i}), std::to_string(i), false, FAState::Transitions());
	}
	states2[0].add_transition(1, Symbol('b'));
	states2[0].add_transition(2, Symbol('b'));
	states2[0].add_transition(5, Symbol('c'));
	states2[1].add_transition(3, Symbol('a'));
	states2[1].add_transition(3, Symbol('c'));
	states2[2].add_transition(4, Symbol('a'));
	states2[5].add_transition(4, Symbol('a'));
	states2[3].is_terminal = true;
	states2[4].is_terminal = true;
	FiniteAutomaton fa2(0, states2, {Symbol('a'), Symbol('b'), Symbol('c')});

	vector<FAState> states3;
	for (int i = 0; i < 6; i++) {
		states3.emplace_back(i, set<int>({i}), std::to_string(i), false, FAState::Transitions());
	}
	states3[5].add_transition(4, Symbol('b'));
	states3[5].add_transition(3, Symbol('b'));
	states3[5].add_transition(0, Symbol('c'));
	states3[4].add_transition(2, Symbol('a'));
	states3[4].add_transition(1, Symbol('c'));
	states3[3].add_transition(1, Symbol('a'));
	states3[0].add_transition(1, Symbol('a'));
	states3[2].is_terminal = true;
	states3[1].is_terminal = true;
	FiniteAutomaton fa3(5, states3, {Symbol('a'), Symbol('b'), Symbol('c')});

	ASSERT_TRUE(FiniteAutomaton::equal(fa1, fa1));
	ASSERT_TRUE(!FiniteAutomaton::equal(fa1, fa2));
	ASSERT_TRUE(FiniteAutomaton::equal(fa1, fa3));
	ASSERT_TRUE(FiniteAutomaton::equal(Regex("(aab|aab)*").to_thompson().remove_eps(),
									   Regex("(aab|aab)*").to_glushkov()));
	ASSERT_TRUE(FiniteAutomaton::equal(Regex("a(a)*ab(bb)*baa").to_thompson().remove_eps(),
									   Regex("a(a)*ab(bb)*baa").to_glushkov()));
	ASSERT_TRUE(FiniteAutomaton::equal(
		Regex("a(bbb*aaa*)*bb*|aaa*(bbb*aaa*)*|b(aaa*bbb*)*aa*|bbb*(aaa*bbb*)*")
			.to_thompson()
			.remove_eps(),
		Regex("a(bbb*aaa*)*bb*|aaa*(bbb*aaa*)*|b(aaa*bbb*)*aa*|bbb*(aaa*bbb*)*").to_glushkov()));
}

TEST(TestEquivalent, FA_Equivalent) {
	vector<FAState> states1;
	for (int i = 0; i < 3; i++) {
		states1.emplace_back(i, set<int>({i}), std::to_string(i), false, FAState::Transitions());
	}
	states1[0].add_transition(0, Symbol('c'));
	states1[0].add_transition(1, Symbol('c'));
	states1[1].add_transition(2, Symbol('c'));
	states1[1].add_transition(0, Symbol('c'));
	states1[2].add_transition(1, Symbol('c'));
	states1[2].add_transition(2, Symbol('c'));
	states1[0].is_terminal = true;
	FiniteAutomaton fa1(0, states1, {Symbol('c'), Symbol('c')});

	vector<FAState> states2;
	for (int i = 0; i < 4; i++) {
		states2.emplace_back(i, set<int>({i}), std::to_string(i), false, FAState::Transitions());
	}
	states2[0].add_transition(0, Symbol('c'));
	states2[0].add_transition(1, Symbol('c'));
	states2[1].add_transition(2, Symbol('c'));
	states2[1].add_transition(0, Symbol('c'));
	states2[2].add_transition(3, Symbol('c'));
	states2[2].add_transition(2, Symbol('c'));
	states2[3].add_transition(2, Symbol('c'));
	states2[3].add_transition(0, Symbol('c'));
	states2[0].is_terminal = true;
	FiniteAutomaton fa2(0, states2, {Symbol('c'), Symbol('c')});

	ASSERT_TRUE(FiniteAutomaton::equivalent(fa1, fa2));
}

TEST(TestBisimilar, FA_Bisimilar) {
	vector<FAState> states1;
	for (int i = 0; i < 3; i++) {
		states1.emplace_back(i, set<int>({i}), std::to_string(i), false, FAState::Transitions());
	}
	states1[0].add_transition(1, Symbol('a'));
	states1[0].add_transition(1, Symbol::Epsilon);
	states1[0].add_transition(2, Symbol('b'));
	states1[1].add_transition(2, Symbol('a'));
	states1[1].add_transition(1, Symbol('b'));
	states1[2].add_transition(1, Symbol('a'));
	states1[2].add_transition(1, Symbol::Epsilon);
	states1[2].add_transition(0, Symbol('b'));
	states1[0].is_terminal = true;
	states1[2].is_terminal = true;
	FiniteAutomaton fa1(1, states1, {Symbol('a'), Symbol('b')});

	vector<FAState> states2;
	for (int i = 0; i < 2; i++) {
		states2.emplace_back(i, set<int>({i}), std::to_string(i), false, FAState::Transitions());
	}
	states2[0].add_transition(1, Symbol('a'));
	states2[0].add_transition(1, Symbol::Epsilon);
	states2[0].add_transition(0, Symbol('b'));
	states2[1].add_transition(0, Symbol('a'));
	states2[1].add_transition(1, Symbol('b'));
	states2[0].is_terminal = true;
	FiniteAutomaton fa2(1, states2, {Symbol('a'), Symbol('b')});

	ASSERT_TRUE(FiniteAutomaton::bisimilar(fa1, fa2));
}

TEST(TestBisimilar, FA_MergeBisimilar) {
	FiniteAutomaton fa = Regex("(a|b)*b").to_glushkov();
	FiniteAutomaton fa1 = fa.merge_bisimilar();

	ASSERT_TRUE(FiniteAutomaton::equal(Regex("(a|b)*b").to_ilieyu(), fa1));

	vector<FAState> states2;
	for (int i = 0; i < 3; i++) {
		states2.emplace_back(i, set<int>({i}), std::to_string(i), false, FAState::Transitions());
	}
	states2[0].add_transition(1, Symbol('a'));
	states2[0].add_transition(1, Symbol::Epsilon);
	states2[0].add_transition(2, Symbol('b'));
	states2[1].add_transition(2, Symbol('a'));
	states2[1].add_transition(1, Symbol('b'));
	states2[2].add_transition(1, Symbol('a'));
	states2[2].add_transition(1, Symbol::Epsilon);
	states2[2].add_transition(0, Symbol('b'));
	states2[0].is_terminal = true;
	states2[2].is_terminal = true;
	FiniteAutomaton fa2(1, states2, {Symbol('a'), Symbol('b')});

	vector<FAState> states3;
	for (int i = 0; i < 2; i++) {
		states3.emplace_back(i, set<int>({i}), std::to_string(i), false, FAState::Transitions());
	}
	states3[0].add_transition(0, Symbol('b'));
	states3[0].add_transition(1, Symbol('a'));
	states3[1].add_transition(0, Symbol::Epsilon);
	states3[1].add_transition(0, Symbol('a'));
	states3[1].add_transition(1, Symbol('b'));
	states3[1].is_terminal = true;
	FiniteAutomaton fa3(0, states3, {Symbol('a'), Symbol('b')});

	ASSERT_TRUE(FiniteAutomaton::equal(fa2.merge_bisimilar(), fa3));
}

TEST(TestSubset, Regex_Subset) {
	Regex r1("a*baa");
	Regex r2("abaa");

	ASSERT_TRUE(r1.subset(r2));
	ASSERT_TRUE(!r2.subset(r1));
	ASSERT_TRUE(!Regex("ab*").subset(Regex("a*b*")));
}

TEST(TestEqual, Regex_Equal) {
	Regex r1("a(bbb*aaa*)*bb*|aaa*(bbb*aaa*)*");
	Regex r2("aaa*(bbb*aaa*)*|a(bbb*aaa*)*bb*");
	Regex r3("a(bbb*aaa*)*bb*|aaa*(bbb*aaa*)");

	ASSERT_TRUE(Regex::equal(r1, r2));
	ASSERT_TRUE(!Regex::equal(r1.linearize(), r1));
	ASSERT_TRUE(Regex::equal(r1.linearize(), r1.linearize()));
	ASSERT_TRUE(!Regex::equal(r1, r3));
}

TEST(TestRemoveTrap, FASizeEquality) {
	Regex r1("ca*a(b|c)*");
	Regex r2("caa*b*");
	Regex r3("(a|b)*a(a|b)");
	FiniteAutomaton fa1 = r1.to_glushkov().determinize().complement();
	FiniteAutomaton fa2 = r2.to_glushkov().determinize();
	FiniteAutomaton fa3 = r3.to_glushkov().determinize();
	FiniteAutomaton fa4 = FiniteAutomaton::intersection(fa1, fa2).remove_trap_states();

	ASSERT_EQ((fa2.size() - fa2.remove_trap_states().size()),
			  1); // В норме детерминизация добавляет одну ловушку
	ASSERT_EQ(fa4.size(), 1); // Кейс, когда осталось несколько ловушек, и они коллапсируют в
							  //  одну, чтобы не получился пустой автомат.
	ASSERT_EQ(fa3.size(), fa3.remove_trap_states().size()); // Кейс, когда ловушек нет.
}

TEST(TestEquivalent, Regex_Equivalence) {
	auto test_equivalence = [](const string& rgx_str) {
		Regex r1(rgx_str), r2(rgx_str);
		ASSERT_TRUE(Regex::equivalent(r1, r2.to_thompson().to_regex()));
		ASSERT_TRUE(Regex::equivalent(r1, r2.to_glushkov().to_regex()));
		ASSERT_TRUE(Regex::equivalent(r1, r2.to_ilieyu().to_regex()));
		ASSERT_TRUE(Regex::equivalent(r1, r2.to_antimirov().to_regex()));
	};

	test_equivalence("a");
	test_equivalence("a*");
	test_equivalence("(ab)*a");
	test_equivalence("a(a)*ab(bb)*baa");
	test_equivalence("(b)*(b)");
	test_equivalence("a*|");
	test_equivalence("|b((b((a)*)(a(|(a))))*)");
	test_equivalence("(((a*)))(((a(b|)|a)*||b))");
	test_equivalence("((b(((ba|b)|||(b))*)))");
	test_equivalence("(((((a*)((a*)|bb)(((|||((b)))))))))");
}

TEST(TestPumpLength, PumpLengthValues) {
	ASSERT_EQ(Regex("abaa").pump_length(), 5);
}

TEST(TestPrefixGrammar, PrefixGrammarBuilding) {
	vector<FAState> states1;
	for (int i = 0; i < 5; i++) {
		states1.emplace_back(i, set<int>({i}), std::to_string(i), false, FAState::Transitions());
	}

	states1[4].add_transition(1, Symbol('a'));
	states1[1].add_transition(2, Symbol('b'));
	states1[1].add_transition(4, Symbol('c'));
	states1[2].add_transition(2, Symbol('b'));
	states1[2].add_transition(2, Symbol('c'));
	states1[2].is_terminal = true;
	states1[0].add_transition(4, Symbol('c'));
	states1[3].add_transition(0, Symbol('a'));
	states1[3].add_transition(0, Symbol('b'));
	states1[4].is_terminal = true;
	FiniteAutomaton dfa1 = FiniteAutomaton(3, states1, {Symbol('a'), Symbol('b'), Symbol('c')});

	PrefixGrammar g;

	// cout << "1\n";
	g.fa_to_prefix_grammar(dfa1);
	// cout << "2\n";
	ASSERT_TRUE(FiniteAutomaton::equivalent(dfa1, g.prefix_grammar_to_automaton()));
	// cout << "3\n";
	g.fa_to_prefix_grammar_TM(dfa1);
	// cout << "4\n";
	ASSERT_TRUE(FiniteAutomaton::equivalent(dfa1, g.prefix_grammar_to_automaton()));
}

TEST(TestIsOneUnambigous, IsOneUnambigousWorks) {
	Regex r1("(a|b)*a");
	Regex r2("(a|b)*(ac|bd)");
	Regex r3("(a|b)*a(a|b)");
	Regex r4("(c(a|b)*c)*");
	Regex r5("a(bbb*aaa*)*bb*|aaa*(bbb*aaa*)*|b(aaa*bbb*)*aa*|");

	// ok
	ASSERT_TRUE(r1.to_glushkov().is_one_unambiguous());
	// doesn't fulfill the orbit property
	ASSERT_TRUE(!r2.to_glushkov().is_one_unambiguous());
	// consists of a single orbit, but neither a nor b is consistent
	ASSERT_TRUE(!r3.to_glushkov().is_one_unambiguous());
	// ok
	ASSERT_TRUE(r4.to_glushkov().is_one_unambiguous());
	// doesn't fulfill the orbit property
	ASSERT_TRUE(!r5.to_glushkov().is_one_unambiguous());
}

TEST(TestGetOneUnambigous, GetOneUnambigousWorks) {
	auto check_one_unambiguous = [](const string& rgx_str, bool expected_res) {
		ASSERT_TRUE(Regex(rgx_str).get_one_unambiguous_regex().is_one_unambiguous() ==
					expected_res);
	};
	// ok
	check_one_unambiguous("(a|b)*a", true);
	// doesn't fulfill the orbit property
	check_one_unambiguous("(a|b)*(ac|bd)", false);
	// consists of a single orbit, but neither a nor b is consistent
	check_one_unambiguous("(a|b)*a(a|b)", false);
	// ok
	check_one_unambiguous("(c(a|b)*c)*", true);
	// doesn't fulfill the orbit property
	check_one_unambiguous("a(bbb*aaa*)*bb*|aaa*(bbb*aaa*)*|b(aaa*bbb*)*aa*|", false);
}

TEST(TestInterpreter, RunLineTest) {
	Interpreter interpreter;
	interpreter.set_log_mode(Interpreter::LogMode::nothing);
	ASSERT_TRUE(!interpreter.run_line("A =	 Annote (Glushkova {a})"));
	ASSERT_TRUE(interpreter.run_line("  N1 =	(   (   Glushkov ({ab|a})    ))      "));
	ASSERT_TRUE(interpreter.run_line(" N2 =  (Annote N1)"));
	ASSERT_TRUE(!interpreter.run_line("N2 =  (Glushkov N1)"));
	ASSERT_TRUE(!interpreter.run_line("Equiv N1 N3"));
	ASSERT_TRUE(interpreter.run_line("  Equiv ((  N1)) (   (Reverse   .Reverse (N2) !!		))"));
	ASSERT_TRUE(interpreter.run_line("Test (Glushkov {a*}) {a*} 1"));

	ASSERT_TRUE(interpreter.run_line("A = Annote.Glushkov.DeAnnote {a}"));
	ASSERT_TRUE(interpreter.run_line("B = Annote (Glushkov.DeAnnote {a})"));
	ASSERT_TRUE(interpreter.run_line("B = Annote (Glushkov(DeAnnote {a}))"));
	ASSERT_TRUE(interpreter.run_line("A = Annote   .Glushkov.   DeAnnote {a} !!  "));
	ASSERT_TRUE(interpreter.run_line("B = Annote (Glushkov.DeAnnote {a}) !!   "));
	ASSERT_TRUE(interpreter.run_line("B = Annote (   Glushkov(DeAnnote {a})) !! "));
	ASSERT_TRUE(interpreter.run_line("B = Annote (Glushkov {a} !!)"));
	ASSERT_TRUE(interpreter.run_line("B = Annote (Glushkov(DeAnnote {a} !!) !!) !!"));

	// Arrays
	ASSERT_TRUE(interpreter.run_line("A = []"));
	ASSERT_TRUE(interpreter.run_line("A = [[] []]"));
	ASSERT_TRUE(interpreter.run_line("A = [{a} {b}]"));
	ASSERT_TRUE(interpreter.run_line("A = [[(([{a}]))] [{a} []]]"));
	ASSERT_TRUE(!interpreter.run_line("A = [[(([{a}])] [{a} []]]"));
	ASSERT_TRUE(!interpreter.run_line("A = [[([{a}]))] [{a} []]]"));
	ASSERT_TRUE(!interpreter.run_line("A = [[(([{a}]))] [{a} []]"));
	ASSERT_TRUE(!interpreter.run_line("A = [[(([a}]))] [{a} (Glushkov(DeAnnote {a} !!) !!) []]]"));

	// Normalize
	ASSERT_TRUE(interpreter.run_line("A = Normalize {abc} [[{a} {b}]]"));
	ASSERT_TRUE(!interpreter.run_line("A = Normalize {abc} [[{a} []]]"));
}

TEST(TestTransformationMonoid, IsMinimal) {
	FiniteAutomaton fa1 = Regex("a*b*c*").to_thompson().minimize();
	TransformationMonoid tm1(fa1);
	ASSERT_EQ(tm1.class_card(), 7);
	ASSERT_EQ(tm1.class_length(), 2);
	ASSERT_TRUE(tm1.is_minimal());
	ASSERT_EQ(tm1.get_classes_number_MyhillNerode(), 3);

	vector<FAState> states;
	for (int i = 0; i < 5; i++) {
		states.emplace_back(i, set<int>({i}), std::to_string(i), false, FAState::Transitions());
	}
	states[0].add_transition(1, Symbol('a'));
	states[1].add_transition(2, Symbol('c'));
	states[2].add_transition(3, Symbol('a'));
	states[3].add_transition(2, Symbol('c'));
	states[3].add_transition(4, Symbol('b'));
	states[4].add_transition(4, Symbol('b'));
	states[4].add_transition(4, Symbol('c'));
	states[4].is_terminal = true;
	FiniteAutomaton fa2(0, states, {Symbol('a'), Symbol('b'), Symbol('c')});
	TransformationMonoid tm2(fa2);
	ASSERT_EQ(tm2.class_card(), 12);
	ASSERT_EQ(tm2.class_length(), 4);
	ASSERT_EQ(tm2.is_minimal(), 1);
	ASSERT_EQ(tm2.get_classes_number_MyhillNerode(), 5);

	FiniteAutomaton fa3 = Regex("ab|b").to_glushkov().minimize();
	TransformationMonoid tm3(fa3);
	ASSERT_TRUE(tm3.is_minimal());

	FiniteAutomaton fa4 = Regex("a").to_glushkov().minimize();
	TransformationMonoid tm4(fa4);
	ASSERT_TRUE(tm4.is_minimal());

	FiniteAutomaton fa5 = Regex("b*a*").to_thompson().minimize();
	TransformationMonoid tm5(fa5);
	ASSERT_TRUE(tm5.is_minimal());
}

TEST(TestGlaisterShallit, GetClassesNumber) {
	auto check_classes_number = [](const string& rgx_str, int num) {
		ASSERT_TRUE(Regex(rgx_str).to_glushkov().get_classes_number_GlaisterShallit() == num);
	};
	check_classes_number("abc", 4);
	check_classes_number("a*b*c*", 3);
	check_classes_number("aa*bb*cc*", 4);
	check_classes_number("ab|abc", 4);
	check_classes_number("a(a|b)*(a|b)", 3);
	check_classes_number("a((a|b)*)*(b|c)", 3);
	check_classes_number("a(b|c)(a|b)(b|c)", 5);
	check_classes_number("abc|bca", 6);
	check_classes_number("abc|bbc", 4);
}

TEST(TestToMFA, ToMfa) {
	vector<MFAState> states = BackRefRegex("[a|b]:1*").to_mfa().get_states();
	ASSERT_EQ(states.size(), 5);
	ASSERT_EQ(states[0],
			  MFAState(0,
					   "0",
					   false,
					   {{Symbol('a'), {MFATransition(1, {1}, {})}},
						{Symbol('b'), {MFATransition(2, {1}, {})}},
						{Symbol::Epsilon, {MFATransition(4)}}}));
	ASSERT_EQ(states[1], MFAState(1, "1", false, {{Symbol::Epsilon, {MFATransition(3, {}, {1})}}}));
	ASSERT_EQ(states[2], MFAState(2, "2", false, {{Symbol::Epsilon, {MFATransition(3, {}, {1})}}}));
	ASSERT_EQ(states[3],
			  MFAState(3,
					   "3",
					   true,
					   {{Symbol('a'), {MFATransition(1, {1}, {})}},
						{Symbol('b'), {MFATransition(2, {1}, {})}}}));
	ASSERT_EQ(states[4], MFAState(4, "4", true, {}));

	states = BackRefRegex("([&2]:1[&1a]:2)*").to_mfa().get_states();
	ASSERT_EQ(states.size(), 7);
	ASSERT_EQ(states[0],
			  MFAState(0,
					   "0",
					   false,
					   {{Symbol::Ref(2), {MFATransition(1, {1}, {})}},
						{Symbol::Epsilon, {MFATransition(6)}}}));
	ASSERT_EQ(states[1], MFAState(1, "1", false, {{Symbol::Epsilon, {MFATransition(2, {}, {1})}}}));
	ASSERT_EQ(states[2], MFAState(2, "2", false, {{Symbol::Ref(1), {MFATransition(3, {2}, {})}}}));
	ASSERT_EQ(states[3], MFAState(3, "3", false, {{Symbol('a'), {MFATransition(4)}}}));
	ASSERT_EQ(states[4], MFAState(4, "4", false, {{Symbol::Epsilon, {MFATransition(5, {}, {2})}}}));
	ASSERT_EQ(states[5], MFAState(5, "5", true, {{Symbol::Ref(2), {MFATransition(1, {1}, {})}}}));
	ASSERT_EQ(states[6], MFAState(6, "6", true, {}));
}

TEST(TestToMFA, ToMfaAdditional) {
	vector<MFAState> states = BackRefRegex("[a|b]:1*c").to_mfa_additional().get_states();
	ASSERT_EQ(states.size(), 4);
	ASSERT_EQ(states[0],
			  MFAState(0,
					   "S",
					   false,
					   {{Symbol('a'), {MFATransition(1, {1}, {})}},
						{Symbol('b'), {MFATransition(2, {1}, {})}},
						{Symbol('c'), {MFATransition(3)}}}));
	ASSERT_EQ(states[1],
			  MFAState(1,
					   "a.0",
					   false,
					   {{Symbol('a'), {MFATransition(1, {1}, {})}},
						{Symbol('b'), {MFATransition(2, {1}, {})}},
						{Symbol('c'), {MFATransition(3, {}, {1})}}}));
	ASSERT_EQ(states[2],
			  MFAState(2,
					   "b.1",
					   false,
					   {{Symbol('a'), {MFATransition(1, {1}, {})}},
						{Symbol('b'), {MFATransition(2, {1}, {})}},
						{Symbol('c'), {MFATransition(3, {}, {1})}}}));
	ASSERT_EQ(states[3], MFAState(3, "c.2", true, {}));

	states = BackRefRegex("([&2]:1[&1a]:2)*c").to_mfa_additional().get_states();
	ASSERT_EQ(states.size(), 5);
	ASSERT_EQ(states[0],
			  MFAState(0,
					   "S",
					   false,
					   {{Symbol::Ref(2), {MFATransition(1, {1}, {})}},
						{Symbol('c'), {MFATransition(4)}}}));
	ASSERT_EQ(states[1],
			  MFAState(1, "&2.0", false, {{Symbol::Ref(1), {MFATransition(2, {2}, {1})}}}));
	ASSERT_EQ(states[2], MFAState(2, "&1.1", false, {{Symbol('a'), {MFATransition(3)}}}));
	ASSERT_EQ(states[3],
			  MFAState(3,
					   "a.2",
					   false,
					   {{Symbol::Ref(2), {MFATransition(1, {1}, {2})}},
						{Symbol('c'), {MFATransition(4, {}, {2})}}}));
	ASSERT_EQ(states[4], MFAState(4, "c.3", true, {}));
}

TEST(TestLanguage, Caching) {
	Language::enable_retrieving_from_cache();

	Regex r("abaa");
	std::shared_ptr<Language> lang(r.get_language());
	// pump_length
	r.pump_length();
	ASSERT_TRUE(lang->is_pump_length_cached());
	ASSERT_EQ(lang->get_pump_length(), 5);
	// min_dfa
	FiniteAutomaton fa = r.to_glushkov();
	fa.minimize();
	ASSERT_TRUE(lang->is_min_dfa_cached());
	ASSERT_EQ(lang, lang->get_min_dfa().get_language());
	// syntactic_monoid
	fa.get_syntactic_monoid();
	ASSERT_TRUE(lang->is_syntactic_monoid_cached());
	// nfa_minimum_size
	fa.get_classes_number_GlaisterShallit();
	ASSERT_TRUE(lang->is_nfa_minimum_size_cached());
	ASSERT_EQ(lang->get_nfa_minimum_size(), 5);
	// is_one_unambiguous
	fa.is_one_unambiguous();
	ASSERT_TRUE(lang->is_one_unambiguous_flag_cached());
	ASSERT_EQ(lang->get_one_unambiguous_flag(), true);
	// one_unambiguous_regex
	r.get_one_unambiguous_regex();
	ASSERT_TRUE(lang->is_one_unambiguous_regex_cached());
	ASSERT_EQ(lang, lang->get_one_unambiguous_regex().get_language());

	Language::disable_retrieving_from_cache();
}

TEST(TestIsDeterministic, FA_IsDeterministic) {
	ASSERT_TRUE(Regex("ab|c").to_glushkov().is_deterministic());
	ASSERT_FALSE(Regex("ab|ac").to_glushkov().is_deterministic());
	ASSERT_FALSE(Regex("a*").to_thompson().is_deterministic());
}

TEST(TestIsDeterministic, MFA_IsDeterministic) {
	ASSERT_TRUE(BackRefRegex("ab|c").to_mfa().is_deterministic());
	ASSERT_FALSE(BackRefRegex("ab|ac").to_mfa().is_deterministic());
	ASSERT_FALSE(BackRefRegex("ab|c|&1").to_mfa().is_deterministic());
	ASSERT_TRUE(BackRefRegex("[a]:1").to_mfa().is_deterministic());
}

TEST(TestRemoveEps, MFA_RemoveEps) {
	MemoryFiniteAutomaton mfa = BackRefRegex("[[a]:1]:2&1").to_mfa();
	mfa = mfa.remove_eps();
	vector<MFAState> states = mfa.get_states();
	ASSERT_EQ(states.size(), 3);
	ASSERT_EQ(states[0], MFAState(0, "0", false, {{Symbol('a'), {MFATransition(1, {1, 2}, {})}}}));
	ASSERT_EQ(states[1],
			  MFAState(1, "1, 2, 3", false, {{Symbol::Ref(1), {MFATransition(2, {}, {1, 2})}}}));
	ASSERT_EQ(states[2], MFAState(2, "4", true, {}));
}

TEST(TestIsAcreg, BRegex_IsAcreg) {
	ASSERT_TRUE(BackRefRegex("([&2b]:1&1[a*]:1[b*&1]:2)*").is_acreg());
	ASSERT_FALSE(BackRefRegex("([a*]:1[&2b]:1&1[b*&1]:2)*").is_acreg());
	ASSERT_FALSE(BackRefRegex("([&2]:1([&1]:2|[a]:2))*").is_acreg());
}

TEST(TestParsing, MFA_parse) {
	using Test = std::tuple<bool, string, string, bool>;
	vector<Test> tests = {
		{true, "[a*]:1&1", "aaa", false},
		{true, "[a*]:1&1", "aaaa", true},
		{true, "[a*]:1&1[b|c]:2*&2", "abcb", false},
		{true, "[a*]:1&1[b|c]:2*&2", "bcb", false},
		{true, "[a*]:1&1[b|c]:2*&2", "bcc", true},
		{true, "[a*]:1&1[b|c]:2*&2", "aaaabcc", true},
		{true, "[a*]:1&1[b|c]:1*&1", "bcc", true},
		{true, "[a*]:1&1[b|c]:1*&1", "aaaabcc", true},
		{true, "[[a*]:1b&1]:2&2", "aabaaaba", false},
		{true, "[[a*]:1b&1]:2&2", "aabaaaabaa", true},
		{true, "[[a*]:1b&1]:2&2", "aabaaaabaa", true},
		{true, "([&2]:1[&1a]:2)*", "aaaaaaaa", false},
		{true, "([&2]:1[&1a]:2)*", "aaaaaaaaa", true},
		{false, "[b]:1[a*]:1&1", "bb", false},
		{false, "[b]:1[a*]:1&1", "b", true},
		{true, "[b]:1a[a*]:1&1", "ba", true},
		{false, "(&1[b]:1[a*]:1)*", "bb", true},
	};

	for_each(tests.begin(), tests.end(), [](const Test& test) {
		auto [test_rem_eps, rgx_str, str, expected_res] = test;

		std::stringstream message;
		message << "Regex: " << rgx_str << ", Str: " << str << ", Res: " << expected_res;
		SCOPED_TRACE(message.str());

		MemoryFiniteAutomaton mfa(BackRefRegex(rgx_str).to_mfa()),
			mfa_add(BackRefRegex(rgx_str).to_mfa_additional());
		ASSERT_EQ(mfa.parse(str).second, expected_res);
		ASSERT_EQ(mfa.parse_additional(str).second, expected_res);
		ASSERT_EQ(mfa_add.parse(str).second, expected_res);
		ASSERT_EQ(mfa_add.parse_additional(str).second, expected_res);

		if (test_rem_eps) {
			MemoryFiniteAutomaton rem_eps_mfa = mfa.remove_eps();
			ASSERT_EQ(rem_eps_mfa.parse(str).second, expected_res);
			ASSERT_EQ(rem_eps_mfa.parse_additional(str).second, expected_res);
		}
	});
}

TEST(TestParsing, MFA_equivalence) {
	using Test = std::tuple<bool, string>;
	vector<Test> tests = {
		{true, "[a*]:1&1"},
		{false, "(a*bc*)*d"},
		{true, "[[a*]:1b&1]:2&2"},
		{true, "([&2]:1[&1a]:2)*"},
		{false, "[b]:1[a*]:1&1"},
		{true, "[b]:1a[a*]:1&1"},
		{false, "(&1[b]:1[a*]:1)*"},
		{true, "[a*]:1&1[b|c]:2*&2"},
		{true, "[a*]:1&1[b|c]:1*&1"},
		{false, "[[|b]:2*]:1*a&1&2"},
		{false, "(([[b*]:1|]:2|)&1&1&2)*"},
		{false, "(a[[b|]:1|]:2&1&1)*"},
	};

	for_each(tests.begin(), tests.end(), [](const Test& test) {
		auto [test_rem_eps, rgx_str] = test;
		SCOPED_TRACE("Regex: " + rgx_str);

		MemoryFiniteAutomaton mfa = BackRefRegex(rgx_str).to_mfa();
		vector<MemoryFiniteAutomaton> MFAs = {BackRefRegex(rgx_str).to_mfa_additional()};
		if (test_rem_eps) {
			MFAs.emplace_back(mfa.remove_eps());
		}

		int MAX_LEN = mfa.size();
		auto base_test_set = mfa.generate_test_set(MAX_LEN);
		unordered_map<string, int> base_parsing_res;
		for (const auto& mutated_word : base_test_set.second) {
			auto res = mfa.parse(mutated_word);
			base_parsing_res[mutated_word] = res.second;
		}

		for (auto& cur_mfa : MFAs) {
			auto test_set = cur_mfa.generate_test_set(MAX_LEN);
			ASSERT_EQ(base_test_set.first, test_set.first);

			for (const auto& [mutated_word, res] : base_parsing_res) {
				auto test_res = mfa.parse(mutated_word);
				ASSERT_EQ(test_res.second, res);
			}
		}
	});
}

TEST(TestReverse, BRegex_Reverse) {
	ASSERT_TRUE(BackRefRegex::equal(BackRefRegex("([a*b]:1&1|b&1)").reverse(),
									BackRefRegex("[ba*]:1&1|&1b")));
	ASSERT_EQ(BackRefRegex("(cb)*(c[ca*b]:1&1b(c&1b)*)*").reverse().to_txt(),
			  "((b[ba*c]:1c)*b&1&1c)*(bc)*");
}

TEST(TestBisimilar, MFA_Bisimilar) {
	using Test = std::tuple<string, string, bool>;
	vector<Test> tests = {
		{"[aa*]:1a&1", "a[a*a]:1&1", true},
		{"[a*]:1a*&1", "a*[a*]:1&1", false},
		{"[ab]:2cab&2", "abc[ab]:2&2", true},
		{"[a|b]:1c(a|b)&1", "(a|b)c[a|b]:1&1", false},
		{"[a]:1*&1", "[a*]:1*&1", false},
		{"[a*]:1&1", "[a*]:1a*&1", false},
		{"[a*a*|]:1&1", "[a*]:1&1", true},
		{"[a|a]:1*&1", "[a]:1*[a]:1*&1", true},
		{"[a]:1*[a*]:1&1", "[a|]:1*&1", false},
	};

	for_each(tests.begin(), tests.end(), [](const Test& test) {
		auto [rgx1, rgx2, expected_res] = test;
		SCOPED_TRACE(rgx1 + " " + rgx2);
		ASSERT_EQ(MemoryFiniteAutomaton::bisimilar(BackRefRegex(rgx1).to_mfa_additional(),
												   BackRefRegex(rgx2).to_mfa_additional()),
				  expected_res);
	});

	ASSERT_FALSE(
		MemoryFiniteAutomaton::bisimilar(BackRefRegex("[[a*]:1]:2a*&1").to_mfa_additional(),
										 BackRefRegex("a*[a*]:1&1").to_mfa_additional())
			.has_value());
}

TEST(TestBisimilar, MFA_MergeBisimilar) {
	vector<MFAState> states =
		BackRefRegex("ab*[b*]:1*&1").to_mfa_additional().merge_bisimilar().get_states();
	ASSERT_EQ(states.size(), 4);
	ASSERT_EQ(states[0],
			  MFAState(0,
					   "a.0, b.1",
					   false,
					   {{Symbol('b'), {MFATransition(0), MFATransition(1, {1}, {})}},
						{Symbol::Ref(1), {MFATransition(3), MFATransition(3, {}, {}, {1})}}}));
	ASSERT_EQ(
		states[1],
		MFAState(1,
				 "b.2",
				 false,
				 {{Symbol('b'), {MFATransition(1), MFATransition(1, {1}, {})}},
				  {Symbol::Ref(1), {MFATransition(3, {}, {1}), MFATransition(3, {}, {}, {1})}}}));
	ASSERT_EQ(states[2], MFAState(2, "S", false, {{Symbol('a'), {MFATransition(0)}}}));
	ASSERT_EQ(states[3], MFAState(3, "&1.3", true, {}));

	states =
		BackRefRegex("([a*]:1|[a*a*]:1)*&1").to_mfa_additional().merge_bisimilar().get_states();
	ASSERT_EQ(states.size(), 3);
	ASSERT_EQ(states[0],
			  MFAState(0,
					   "S",
					   false,
					   {{Symbol('a'), {MFATransition(1, {1}, {})}},
						{Symbol::Ref(1), {MFATransition(2)}}}));
	ASSERT_EQ(
		states[1],
		MFAState(1,
				 "a.0, a.1, a.2",
				 false,
				 {{Symbol('a'), {MFATransition(1), MFATransition(1, {1}, {})}},
				  {Symbol::Ref(1), {MFATransition(2, {}, {1}), MFATransition(2, {}, {}, {1})}}}));
	ASSERT_EQ(states[2], MFAState(2, "&1.3", true, {}));

	states =
		BackRefRegex("(b|[&2*]:1*[a|&1]:2&2)*").to_mfa_additional().merge_bisimilar().get_states();
	ASSERT_EQ(states.size(), 4);
	ASSERT_EQ(
		states[0],
		MFAState(0,
				 "&2.1",
				 false,
				 {{Symbol::Ref(2), {MFATransition(0), MFATransition(0, {1}, {})}},
				  {Symbol('a'), {MFATransition(1, {2}, {1}), MFATransition(1, {2}, {}, {1})}},
				  {Symbol::Ref(1), {MFATransition(1, {2}, {1}), MFATransition(1, {2}, {}, {1})}}}));
	ASSERT_EQ(states[1],
			  MFAState(1, "a.2, &1.3", false, {{Symbol::Ref(2), {MFATransition(2, {}, {2})}}}));
	ASSERT_EQ(
		states[2],
		MFAState(2,
				 "b.0, &2.4",
				 true,
				 {{Symbol('b'), {MFATransition(2)}},
				  {Symbol::Ref(2), {MFATransition(0, {1}, {})}},
				  {Symbol('a'), {MFATransition(1, {2}, {}), MFATransition(1, {2}, {}, {1})}},
				  {Symbol::Ref(1), {MFATransition(1, {2}, {}), MFATransition(1, {2}, {}, {1})}}}));
	ASSERT_EQ(states[3],
			  MFAState(3,
					   "S",
					   true,
					   {{Symbol('b'), {MFATransition(2)}},
						{Symbol::Ref(2), {MFATransition(0, {1}, {})}},
						{Symbol('a'), {MFATransition(1, {2}, {})}},
						{Symbol::Ref(1), {MFATransition(1, {2}, {})}}}));
}

TEST(TestAmbiguity, AmbiguityValues) {
	enum AutomatonType {
		thompson,
		glushkov,
		ilieyu
	};
	using Test = std::tuple<int, string, AutomatonType, FiniteAutomaton::AmbiguityValue>;
	vector<Test> tests = {
		//{0, "(a*)*", thompson, FiniteAutomaton::exponentially_ambiguous},
		{1, "a*a*", glushkov, FiniteAutomaton::polynomially_ambigious},
		{2, "abc", thompson, FiniteAutomaton::unambigious},
		//{3, "b|a", thompson, FiniteAutomaton::almost_unambigious},
		{4, "(aa|aa)*", glushkov, FiniteAutomaton::exponentially_ambiguous},
		{5, "(aab|aab)*", glushkov, FiniteAutomaton::exponentially_ambiguous},
		{6, "a*a*((a)*)*", glushkov, FiniteAutomaton::polynomially_ambigious},
		//{7, "a*a*((a)*)*", thompson,
		// FiniteAutomaton::exponentially_ambiguous},
		//{8, "a*(b*)*", thompson, FiniteAutomaton::exponentially_ambiguous},
		//{9, "a*((ab)*)*", thompson, FiniteAutomaton::exponentially_ambiguous},
		{10, "(aa|aa)(aa|bb)*|a(ba)*", glushkov, FiniteAutomaton::almost_unambigious},
		{11, "(aaa)*(a|)(a|)", ilieyu, FiniteAutomaton::almost_unambigious},
		{12, "(a|)(ab|aaa|baa)*(a|)", glushkov, FiniteAutomaton::almost_unambigious},
		{13, "(a|b|c)*(d|d)*(a|b|c|d)*", glushkov, FiniteAutomaton::almost_unambigious},
		{14, "(ac*|ad*)*", glushkov, FiniteAutomaton::exponentially_ambiguous},
		{15, "(a|b|c)*(a|b|c|d)(a|b|c)*|(a|b)*ca*", glushkov, FiniteAutomaton::almost_unambigious},
		{16, "(a|b|c)*(a|b|c|d)(a|b|c)*|(ac*|ad*)*", glushkov, FiniteAutomaton::almost_unambigious},
		{17,
		 "(ab)*ab(ab)*|(ac)*(ac)*|(d|c)*", // (abab)*abab(abab)*|(aac)*(aac)*|(b|d|c)*
		 glushkov,
		 FiniteAutomaton::almost_unambigious},
		{18, "(abab)*abab(abab)*|(aac)*(aac)*", glushkov, FiniteAutomaton::polynomially_ambigious},
		{19,
		 "(ab)*ab(ab)*", // (abab)*abab(abab)*
		 glushkov,
		 FiniteAutomaton::polynomially_ambigious},
		{20, "(ab)*ab(ab)*|(ac)*(ac)*", glushkov, FiniteAutomaton::polynomially_ambigious},
		// {21, "(a|b)*(f*)*q", thompson,
		//  FiniteAutomaton::exponentially_ambiguous},
		{22,
		 "((bb*c|c)c*b|bb*b|b)(b|(c|bb*c)c*b|bb*b)*",
		 glushkov,
		 FiniteAutomaton::exponentially_ambiguous},
	};

	for_each(tests.begin(), tests.end(), [](const Test& test) {
		auto [test_number, reg_string, type, expected_res] = test;
		// cout << test_number << endl;
		switch (type) {
		case thompson:
			ASSERT_TRUE(Regex(reg_string).to_thompson().ambiguity() == expected_res);
			break;
		case glushkov:
			ASSERT_TRUE(Regex(reg_string).to_glushkov().ambiguity() == expected_res);
			break;
		case ilieyu:
			ASSERT_TRUE(Regex(reg_string).to_ilieyu().ambiguity() == expected_res);
			break;
		}
	});
}