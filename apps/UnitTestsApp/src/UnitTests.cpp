#include "UnitTestsApp/UnitTests.h"

TEST(ParseStringTest, Test_regex_lexer) {
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

TEST(TestCaseName, Test_random_regex_parsing) {
	RegexGenerator rg(15, 10, 5, 3);
	for (int i = 0; i < 30; i++) {
		string str = rg.generate_regex();
		Regex r1(str);
		string r1_str = r1.to_txt();
		Regex r2(r1_str);
		ASSERT_EQ(true, Regex::equivalent(r1, r2));
	}
}

TEST(TestCaseName, Test_fa_equal) {
	vector<State> states1;
	for (int i = 0; i < 6; i++) {
		State s = {i, {i}, std::to_string(i), false, map<alphabet_symbol, set<int>>()};
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
		State s = {i, {i}, std::to_string(i), false, map<alphabet_symbol, set<int>>()};
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
		State s = {i, {i}, std::to_string(i), false, map<alphabet_symbol, set<int>>()};
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

TEST(TestCaseName, Test_fa_equiv) {
	vector<State> states1;
	for (int i = 0; i < 3; i++) {
		State s = {i, {i}, std::to_string(i), false, map<alphabet_symbol, set<int>>()};
		states1.push_back(s);
	}
	states1[0].set_transition(0, "c");
	states1[0].set_transition(1, "d");
	states1[1].set_transition(2, "c");
	states1[1].set_transition(0, "d");
	states1[2].set_transition(1, "c");
	states1[2].set_transition(2, "d");
	states1[0].is_terminal = true;
	FiniteAutomaton fa1(0, states1, {"c", "d"});

	vector<State> states2;
	for (int i = 0; i < 4; i++) {
		State s = {i, {i}, std::to_string(i), false, map<alphabet_symbol, set<int>>()};
		states2.push_back(s);
	}
	states2[0].set_transition(0, "c");
	states2[0].set_transition(1, "d");
	states2[1].set_transition(2, "c");
	states2[1].set_transition(0, "d");
	states2[2].set_transition(3, "c");
	states2[2].set_transition(2, "d");
	states2[3].set_transition(2, "c");
	states2[3].set_transition(0, "d");
	states2[0].is_terminal = true;
	FiniteAutomaton fa2(0, states2, {"c", "d"});

	ASSERT_TRUE(FiniteAutomaton::equivalent(fa1, fa2));
}

TEST(TestCaseName, Test_bisimilar) {
	vector<State> states1;
	for (int i = 0; i < 3; i++) {
		State s = {i, {i}, std::to_string(i), false, map<alphabet_symbol, set<int>>()};
		states1.push_back(s);
	}
	states1[0].set_transition(1, "a");
	states1[0].set_transition(1, "eps");
	states1[0].set_transition(2, "b");
	states1[1].set_transition(2, "a");
	states1[1].set_transition(1, "b");
	states1[2].set_transition(1, "a");
	states1[2].set_transition(1, "eps");
	states1[2].set_transition(0, "b");
	states1[0].is_terminal = true;
	states1[2].is_terminal = true;
	FiniteAutomaton fa1(1, states1, {"a", "b"});

	vector<State> states2;
	for (int i = 0; i < 2; i++) {
		State s = {i, {i}, std::to_string(i), false, map<alphabet_symbol, set<int>>()};
		states2.push_back(s);
	}
	states2[0].set_transition(1, "a");
	states2[0].set_transition(1, "eps");
	states2[0].set_transition(0, "b");
	states2[1].set_transition(0, "a");
	states2[1].set_transition(1, "b");
	states2[0].is_terminal = true;
	FiniteAutomaton fa2(1, states2, {"a", "b"});

	ASSERT_TRUE(FiniteAutomaton::bisimilar(fa1, fa2));
}

TEST(TestCaseName, Test_merge_bisimilar) {
	FiniteAutomaton fa = Regex("(a|b)*b").to_glushkov();
	FiniteAutomaton fa1 = fa.merge_bisimilar();

	vector<State> states2;
	for (int i = 0; i < 3; i++) {
		State s = {i, {i}, std::to_string(i), false, map<alphabet_symbol, set<int>>()};
		states2.push_back(s);
	}
	states2[0].set_transition(1, "a");
	states2[0].set_transition(1, "eps");
	states2[0].set_transition(2, "b");
	states2[1].set_transition(2, "a");
	states2[1].set_transition(1, "b");
	states2[2].set_transition(1, "a");
	states2[2].set_transition(1, "eps");
	states2[2].set_transition(0, "b");
	states2[0].is_terminal = true;
	states2[2].is_terminal = true;
	FiniteAutomaton fa2(1, states2, {"a", "b"});

	vector<State> states3;
	for (int i = 0; i < 2; i++) {
		State s = {i, {i}, std::to_string(i), false, map<alphabet_symbol, set<int>>()};
		states3.push_back(s);
	}
	states3[0].set_transition(0, "b");
	states3[0].set_transition(1, "a");
	states3[1].set_transition(0, "eps");
	states3[1].set_transition(0, "a");
	states3[1].set_transition(1, "b");
	states3[1].is_terminal = true;
	FiniteAutomaton fa3(0, states3, {"a", "b"});

	ASSERT_TRUE(FiniteAutomaton::equal(Regex("(a|b)*b").to_ilieyu(), fa1));
	ASSERT_TRUE(FiniteAutomaton::equal(fa2.merge_bisimilar(), fa3));
}

TEST(TestCaseName, Test_regex_subset) {
	Regex r1("a*baa");
	Regex r2("abaa");

	ASSERT_TRUE(r1.subset(r2));
	ASSERT_TRUE(!r2.subset(r1));
	ASSERT_TRUE(!Regex("ab*").subset(Regex("a*b*")));
}

TEST(TestCaseName, Test_regex_equal) {
	Regex r1("a(bbb*aaa*)*bb*|aaa*(bbb*aaa*)*");
	Regex r2("aaa*(bbb*aaa*)*|a(bbb*aaa*)*bb*");
	Regex r3("a(bbb*aaa*)*bb*|aaa*(bbb*aaa*)");

	ASSERT_TRUE(Regex::equal(r1, r2));
	ASSERT_TRUE(!Regex::equal(r1.linearize(), r1));
	ASSERT_TRUE(Regex::equal(r1.linearize(), r1.linearize()));
	ASSERT_TRUE(!Regex::equal(r1, r3));
}

TEST(TestCaseName, Test_ambiguity) {
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

TEST(TestCaseName, Test_remove_trap) {
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

TEST(TestCaseName, Test_arden) {
	auto test_equivalence = [](const string& rgx_str) {
		Regex reg(rgx_str);
		ASSERT_TRUE(Regex::equivalent(reg, reg.to_thompson().to_regex()));
		ASSERT_TRUE(Regex::equivalent(reg, reg.to_glushkov().to_regex()));
		ASSERT_TRUE(Regex::equivalent(reg, reg.to_ilieyu().to_regex()));
		ASSERT_TRUE(Regex::equivalent(reg, reg.to_antimirov().to_regex()));
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

TEST(TestCaseName, Test_pump_length) {
	ASSERT_EQ(Regex("abaa").pump_length(), 5);
}

TEST(TestCaseName, Test_fa_to_pgrammar) {
	// cout << "fa to grammar\n";
	vector<State> states1;
	for (int i = 0; i < 5; i++) {
		State s = {i, {i}, std::to_string(i), false, map<alphabet_symbol, set<int>>()};
		states1.push_back(s);
	}

	states1[4].set_transition(1, "a");
	states1[1].set_transition(2, "b");
	states1[1].set_transition(4, "c");
	states1[2].set_transition(2, "b");
	states1[2].set_transition(2, "c");
	states1[2].is_terminal = true;
	states1[0].set_transition(4, "c");
	states1[3].set_transition(0, "a");
	states1[3].set_transition(0, "b");
	states1[4].is_terminal = true;
	FiniteAutomaton dfa1 = FiniteAutomaton(3, states1, {"a", "b", "c"});

	Grammar g;

	// cout << "1\n";
	g.fa_to_prefix_grammar(dfa1);
	// cout << "2\n";
	ASSERT_TRUE(FiniteAutomaton::equivalent(dfa1, g.prefix_grammar_to_automaton()));
	// cout << "3\n";
	g.fa_to_prefix_grammar_TM(dfa1);
	// cout << "4\n";
	ASSERT_TRUE(FiniteAutomaton::equivalent(dfa1, g.prefix_grammar_to_automaton()));
}

TEST(TestCaseName, Test_is_one_unambiguous) {
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

TEST(TestCaseName, Test_get_one_unambiguous_regex) {
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

TEST(TestCaseName, test_interpreter) {
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

TEST(TestCaseName, Test_TransformationMonoid) {
	FiniteAutomaton fa1 = Regex("a*b*c*").to_thompson().minimize();
	TransformationMonoid tm1(fa1);
	ASSERT_EQ(tm1.class_card(), 7);
	ASSERT_EQ(tm1.class_length(), 2);
	ASSERT_TRUE(tm1.is_minimal());
	ASSERT_EQ(tm1.get_classes_number_MyhillNerode(), 3);

	vector<State> states;
	for (int i = 0; i < 5; i++) {
		State s = {i, {i}, std::to_string(i), false, map<alphabet_symbol, set<int>>()};
		states.push_back(s);
	}
	states[0].set_transition(1, "a");
	states[1].set_transition(2, "c");
	states[2].set_transition(3, "a");
	states[3].set_transition(2, "c");
	states[3].set_transition(4, "b");
	states[4].set_transition(4, "b");
	states[4].set_transition(4, "c");
	states[4].is_terminal = true;
	FiniteAutomaton fa2(0, states, {"a", "b", "c"});
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

TEST(TestCaseName, Test_GlaisterShallit) {
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