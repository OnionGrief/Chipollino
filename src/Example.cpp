#include "Example.h"

void Example::determinize() {
	vector<State> states;
	for (int i = 0; i < 6; i++) {
		State s = {
			i, {i}, to_string(i), false, map<alphabet_symbol, set<int>>()};
		states.push_back(s);
	}

	states[0].set_transition(5, "x");
	states[0].set_transition(5, "y");
	states[0].set_transition(1, "eps");
	states[1].set_transition(2, "eps");
	states[1].set_transition(3, "eps");
	states[1].set_transition(4, "eps");
	states[3].set_transition(3, "x");
	states[4].set_transition(4, "y");
	states[4].set_transition(4, "y");
	states[5].set_transition(5, "z");
	states[5].set_transition(1, "eps");

	states[2].is_terminal = true;
	states[3].is_terminal = true;
	states[4].is_terminal = true;

	FiniteAutomaton nfa(0, states, {"x", "y", "z"});
	cout << nfa.determinize().to_txt();
}

void Example::remove_eps() {
	vector<State> states;
	for (int i = 0; i < 3; i++) {
		State s = {
			i, {i}, to_string(i), false, map<alphabet_symbol, set<int>>()};
		states.push_back(s);
	}

	states[0].set_transition(0, "0");
	states[0].set_transition(1, "eps");
	states[1].set_transition(1, "1");
	states[1].set_transition(2, "eps");
	states[2].set_transition(2, "0");
	states[2].set_transition(2, "1");

	states[2].is_terminal = true;

	FiniteAutomaton nfa(0, states, {"0", "1"});
	cout << nfa.remove_eps().to_txt();
}

void Example::minimize() {
	vector<State> states;
	for (int i = 0; i < 8; i++) {
		State s = {
			i, {i}, to_string(i), false, map<alphabet_symbol, set<int>>()};
		states.push_back(s);
	}

	states[0].set_transition(7, "0");
	states[0].set_transition(1, "1");
	states[1].set_transition(0, "1");
	states[1].set_transition(7, "0");
	states[2].set_transition(4, "0");
	states[2].set_transition(5, "1");
	states[3].set_transition(4, "0");
	states[3].set_transition(5, "1");
	states[4].set_transition(5, "0");
	states[4].set_transition(6, "1");
	states[5].set_transition(5, "0");
	states[5].set_transition(5, "1");
	states[6].set_transition(6, "0");
	states[6].set_transition(5, "1");
	states[7].set_transition(2, "0");
	states[7].set_transition(2, "1");

	states[5].is_terminal = true;
	states[6].is_terminal = true;

	FiniteAutomaton nfa(0, states, {"0", "1"});
	cout << nfa.minimize().to_txt();
}

void Example::intersection() {
	vector<State> states1;
	for (int i = 0; i < 3; i++) {
		State s = {
			i, {i}, to_string(i), false, map<alphabet_symbol, set<int>>()};
		states1.push_back(s);
	}
	vector<State> states2;
	for (int i = 0; i < 3; i++) {
		State s = {
			i, {i}, to_string(i), false, map<alphabet_symbol, set<int>>()};
		states2.push_back(s);
	}

	states1[0].set_transition(0, "b");
	states1[0].set_transition(1, "a");
	states1[1].set_transition(1, "b");
	states1[1].set_transition(2, "a");
	states1[2].set_transition(2, "a");
	states1[2].set_transition(2, "b");

	states1[1].is_terminal = true;

	states2[0].set_transition(0, "a");
	states2[0].set_transition(1, "b");
	states2[1].set_transition(1, "a");
	states2[1].set_transition(2, "b");
	states2[2].set_transition(2, "a");
	states2[2].set_transition(2, "b");

	states2[1].is_terminal = true;

	FiniteAutomaton dfa1 = FiniteAutomaton(0, states1, {"a", "b"});
	FiniteAutomaton dfa2 = FiniteAutomaton(0, states2, {"a", "b"});

	cout << FiniteAutomaton::intersection(dfa1, dfa2).to_txt();
}

void Example::regex_parsing() {
	string regl = "a(bbb*aaa*)*bb*|aaa*(bbb*aaa*)*|b(aaa*bbb*)*aa*|";
	string regr = "bbb*(aaa*bbb*)*"; //"((a|)*c)";
	regl = regl + regr;
	// egl = "a()a))";			  // regl + regr;
	// regl = "a1456|b244444444444";
	//  regl = "(ab|b)*ba"; //"bbb*(aaa*bbb*)*";
	Regex r;
	if (!r.from_string(regl)) {
		cout << "ERROR\n";
		return;
	}
	cout << r.to_txt() << "\n";
	// cout << r.pump_length() << "\n";
	FiniteAutomaton a;
	FiniteAutomaton b;
	FiniteAutomaton c;
	FiniteAutomaton d;

	cout << "to_tompson ------------------------------\n";
	c = r.to_tompson(); // to_tompson(-1);
	cout << c.to_txt();

	cout << "to_glushkov ------------------------------\n";
	a = r.to_glushkov();
	cout << a.to_txt();
	cout << "to_ilieyu  ------------------------------\n";
	b = r.to_ilieyu();
	cout << b.to_txt();

	// FiniteAutomaton d;
	cout << "to_antimirov  ------------------------------\n";
	d = r.to_antimirov();
	// cout << d.to_txt();
	//  cout << r.deannote().to_txt();

	//  cout << FiniteAutomaton::equal(b.minimize(), c.minimize()) << endl;

	cout << a.nfa_to_regex().to_txt();
}

void Example::parsing_nfa() {
	string regl = "a(bbb*aaa*)*bb*|aaa*(bbb*aaa*)*|b(aaa*bbb*)*aa*|";
	string regr = "bbb*(aaa*bbb*)*"; //"((a|)*c)";
	regl = regl + regr;
	regl = "a*bc*"; //"bbb*(aaa*bbb*)*";
	Regex r;
	if (!r.from_string(regl)) {
		cout << "ERROR\n";
		return;
	}

	FiniteAutomaton a;
	FiniteAutomaton b;
	FiniteAutomaton c;

	cout << "to_tompson ------------------------------\n";
	c = r.to_ilieyu(); // to_tompson(-1);
	cout << c.to_txt();
	cout << "Parsing: aaaaaaaaaaaaaaaaaaabccccc\n";
	cout << c.parsing_by_nfa("aaaaaaaaaaaaaaaaaaabccccc")
		 << endl; // true если распознал слово
}

void Example::regex_generating() {
	cout << RegexGenerator(20, 100, 3, 3).generate_regex() << "\n";
	cout << RegexGenerator().generate_regex() << "\n";
	cout << RegexGenerator(9, 2, 2).generate_regex() << "\n";
}

void Example::tasks_generating() {
	TasksGenerator TG;
	cout << "\n" << TG.generate_task(10, 3, false, false); // корректные задачи
	// cout << "\n" << TG.generate_task(15, 1, false, false);
	cout << "\n" << TG.generate_task(3, 6, true, false); // для стат. тайпчека
	cout << "\n" << TG.generate_task(5, 6, false, true); // для динам. тайпчека
}

void Example::random_regex_parsing() {
	RegexGenerator r1(8, 10, 3, 2);
	for (int i = 0; i < 5; i++) {
		string str = r1.generate_regex();
		cout << "\n" << str << "\n";
		Regex r;
		if (!r.from_string(str)) {
			cout << "ERROR\n";
			return;
		}
		cout << r.to_txt() << endl;
	}
}

void Example::parsing_regex(string str) {
	cout << str << endl;
	Regex r;
	if (!r.from_string(str)) {
		cout << "ERROR\n";
		return;
	}
	r.from_string(str);
	cout << r.to_txt() << endl;
}

void Example::fa_bisimilar_check() {
	vector<State> states1;
	for (int i = 0; i < 3; i++) {
		State s = {
			i, {i}, to_string(i), false, map<alphabet_symbol, set<int>>()};
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
		State s = {
			i, {i}, to_string(i), false, map<alphabet_symbol, set<int>>()};
		states2.push_back(s);
	}
	states2[0].set_transition(1, "a");
	states2[0].set_transition(1, "eps");
	states2[0].set_transition(0, "b");
	states2[1].set_transition(0, "a");
	states2[1].set_transition(1, "b");
	states2[0].is_terminal = true;
	FiniteAutomaton fa2(1, states2, {"a", "b"});

	cout << FiniteAutomaton::bisimilar(fa1, fa2) << endl;
	//правильный ответ true
}

void Example::fa_equal_check() {
	vector<State> states1;
	for (int i = 0; i < 6; i++) {
		State s = {
			i, {i}, to_string(i), false, map<alphabet_symbol, set<int>>()};
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
		State s = {
			i, {i}, to_string(i), false, map<alphabet_symbol, set<int>>()};
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
		State s = {
			i, {i}, to_string(i), false, map<alphabet_symbol, set<int>>()};
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

	cout << FiniteAutomaton::equal(fa1, fa1) << endl
		 << FiniteAutomaton::equal(fa1, fa2) << endl
		 << FiniteAutomaton::equal(fa1, fa3) << endl;
	//правильный ответ 1 0 1
}

void Example::fa_merge_bisimilar() {
	vector<State> states1;
	for (int i = 0; i < 3; i++) {
		State s = {
			i, {i}, to_string(i), false, map<alphabet_symbol, set<int>>()};
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

	cout << fa1.to_txt();

	FiniteAutomaton fa2 = fa1.merge_bisimilar();

	cout << fa2.to_txt();
}

void Example::fa_equivalent_check() {
	vector<State> states1;
	for (int i = 0; i < 3; i++) {
		State s = {
			i, {i}, to_string(i), false, map<alphabet_symbol, set<int>>()};
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
		State s = {
			i, {i}, to_string(i), false, map<alphabet_symbol, set<int>>()};
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

	cout << FiniteAutomaton::equivalent(fa1, fa2);
}

void Example::transformation_monoid_example() {
	vector<State> states;
	for (int i = 0; i < 4; i++) {
		State s = {
			i, {i}, to_string(i), false, map<alphabet_symbol, set<int>>()};
		states.push_back(s);
	}
	states[0].set_transition(1, "a");
	states[0].set_transition(0, "b");
	states[1].set_transition(1, "a");
	states[1].set_transition(2, "b");
	states[1].set_transition(1, "c");
	states[2].set_transition(2, "c");
	states[2].set_transition(1, "a");
	states[2].set_transition(2, "b");
	states[1].is_terminal = true;
	states[2].is_terminal = true;
	FiniteAutomaton fa1(0, states, {"a", "b", "c"});
	// cout << fa1.to_txt();
	TransformationMonoid a(&fa1);
	cout << a.get_Equalence_Classes_Txt(); //вывод эквивалентных классов
	// cout << a.get_Rewriting_Rules_Txt(); //Вывод правил переписывания
	// cout << a.is_minimality() << "\n";
	// cout << a.to_Txt_MyhillNerode();
	//  cout << a.get_Equalence_Classes_Txt(); /*
	/*vector<Term> cur = a.get_Equalence_Classes();
	cout << cur[1].name << "\n";
	vector<TermDouble> temp = a.get_Equalence_Classes_VWV(cur[1]);
	for (int i = 0; i < temp.size(); i++) {
		cout << temp[i].first.name << " " << temp[i].second.name << "\n";
	}*/
	return;
}

void Example::fa_subset_check() {
	vector<State> states1;
	for (int i = 0; i < 4; i++) {
		State s = {
			i, {i}, to_string(i), false, map<alphabet_symbol, set<int>>()};
		states1.push_back(s);
	}
	states1[0].set_transition(1, "a");
	states1[0].set_transition(1, "b");
	states1[0].set_transition(2, "a");
	states1[1].set_transition(3, "b");
	states1[2].set_transition(3, "c");
	states1[3].is_terminal = true;
	FiniteAutomaton fa1(0, states1, {"a", "b", "c"});

	vector<State> states2;
	for (int i = 0; i < 3; i++) {
		State s = {
			i, {i}, to_string(i), false, map<alphabet_symbol, set<int>>()};
		states2.push_back(s);
	}
	states2[0].set_transition(1, "b");
	states2[1].set_transition(2, "b");
	states2[2].is_terminal = true;
	FiniteAutomaton fa2(0, states2, {"a", "b", "c"});

	cout << fa1.subset(fa2) << endl;
}

void Example::normalize_regex() {
	string regl = "a(bbb*aaa*)*bb*|aaa*(bbb*aaa*)*|b(aaa*bbb*)*aa*|";
	string regr = "bbb*(aaa*bbb*)*"; //"((a|)*c)";
	regl = regl + regr;
	// regl = "abc"; //"bbb*(aaa*bbb*)*";
	Regex r;
	if (!r.from_string(regl)) {
		cout << "ERROR\n";
		return;
	}
	r.normalize_regex("./../temp/Rules.txt");
	cout << r.to_txt();
}
void Example::to_image() {
	vector<State> states1;
	for (int i = 0; i < 3; i++) {
		State s = {i, {i}, to_string(i), false, map<string, set<int>>()};
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

	string s1 = fa1.to_txt();

	FiniteAutomaton fa2 = fa1.merge_bisimilar();

	string s2 = fa2.to_txt();

	AutomatonToImage::to_image(s1, 1);
	AutomatonToImage::to_image(s2, 2);
}

void Example::step() {
	vector<State> states1;
	for (int i = 0; i < 3; i++) {
		State s = {
			i, {i}, to_string(i), false, map<alphabet_symbol, set<int>>()};
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

	// string f1 = fa1.to_txt();

	FiniteAutomaton fa2 = fa1.merge_bisimilar();
	// string f2 = fa2.to_txt();
	FiniteAutomaton fa3 = fa2.remove_eps();
	// string f3 = fa3.to_txt();
	string s = "merge\\_bisimilar";
	Logger::activate();
	Logger::init();
	Logger::init_step(s);
	Logger::log("Автомат1", "Автомат2", fa1, fa2);
	Logger::finish_step();
	s = "skip";
	Logger::activate();
	Logger::init_step(s);
	Logger::log("Автомат1", "Автомат2", fa1, fa1);
	Logger::finish_step();
	s = "remove\\_eps";
	Logger::init_step(s);
	Logger::log("Автомат1", "Автомат2", fa2, fa3);
	Logger::finish_step();
	Logger::finish();
	Logger::deactivate();
}

void Example::tester() {
	Regex r;
	r.from_string("ab(ab|a)*ababa");
	FiniteAutomaton dfa1 = r.to_tompson();
	// cout << dfa1.parsing_by_nfa("abaaabaaababa");
	// cout << dfa1.to_txt();
	Regex r1;
	r1.from_string("((ab)*a)*");
	Regex r2;
	r2.from_string("ab(ab|a)*ababa");
	Tester::test(dfa1, r1, 1);
	Tester::test(r2, r1, 1);
}

void Example::step_interection() {
	vector<State> states1;
	for (int i = 0; i < 3; i++) {
		State s = {
			i, {i}, to_string(i), false, map<alphabet_symbol, set<int>>()};
		states1.push_back(s);
	}
	vector<State> states2;
	for (int i = 0; i < 3; i++) {
		State s = {
			i, {i}, to_string(i), false, map<alphabet_symbol, set<int>>()};
		states2.push_back(s);
	}

	states1[0].set_transition(0, "b");
	states1[0].set_transition(1, "a");
	states1[1].set_transition(1, "b");
	states1[1].set_transition(2, "a");
	states1[2].set_transition(2, "a");
	states1[2].set_transition(2, "b");

	states1[1].is_terminal = true;

	states2[0].set_transition(0, "a");
	states2[0].set_transition(1, "b");
	states2[1].set_transition(1, "a");
	states2[1].set_transition(2, "b");
	states2[2].set_transition(2, "a");
	states2[2].set_transition(2, "b");

	states2[1].is_terminal = true;

	FiniteAutomaton dfa1 = FiniteAutomaton(0, states1, {"a", "b"});
	FiniteAutomaton dfa2 = FiniteAutomaton(0, states2, {"a", "b"});

	// string f1 = dfa1.to_txt();
	// string f2 = dfa2.to_txt();
	FiniteAutomaton dfa3 = FiniteAutomaton::intersection(dfa1, dfa2);
	string s = "interection";
	Logger::activate();
	Logger::init();
	Logger::init_step(s);
	Logger::log("Автомат1", "Автомат2", "Пересечение автоматов", dfa1, dfa2,
				dfa3);
	Logger::finish_step();
	Logger::finish();
	Logger::deactivate();
}

void Example::arden_test() {
	vector<State> states;
	for (int i = 0; i < 8; i++) {
		State s = {
			i, {i}, to_string(i), false, map<alphabet_symbol, set<int>>()};
		states.push_back(s);
	}
	states[0].set_transition(1, "a");
	states[0].set_transition(4, "b");
	states[1].set_transition(1, "a");
	states[1].set_transition(2, "b");
	states[2].set_transition(1, "a");
	states[2].set_transition(3, "b");
	states[3].set_transition(1, "a");
	states[3].set_transition(3, "b");
	states[4].set_transition(1, "a");
	states[4].set_transition(5, "b");
	states[5].set_transition(6, "a");
	states[5].set_transition(5, "b");
	states[6].set_transition(6, "a");
	states[6].set_transition(7, "b");
	states[7].set_transition(6, "a");
	states[7].set_transition(5, "b");
	states[0].is_terminal = true;
	states[1].is_terminal = true;
	states[2].is_terminal = true;
	states[4].is_terminal = true;
	states[5].is_terminal = true;

	FiniteAutomaton NDM(0, states, {"a", "b"});
	cout << NDM.nfa_to_regex().to_txt() + "\n";
}

void Example::table() {
	vector<State> states1;
	for (int i = 0; i < 3; i++) {
		State s = {
			i, {i}, to_string(i), false, map<alphabet_symbol, set<int>>()};
		states1.push_back(s);
	}
	vector<State> states2;
	for (int i = 0; i < 3; i++) {
		State s = {
			i, {i}, to_string(i), false, map<alphabet_symbol, set<int>>()};
		states2.push_back(s);
	}

	states1[0].set_transition(0, "b");
	states1[0].set_transition(1, "a");
	states1[1].set_transition(1, "b");
	states1[1].set_transition(2, "a");
	states1[2].set_transition(2, "a");
	states1[2].set_transition(2, "b");

	states1[1].is_terminal = true;

	states2[0].set_transition(0, "a");
	states2[0].set_transition(1, "b");
	states2[1].set_transition(1, "a");
	states2[1].set_transition(2, "b");
	states2[2].set_transition(2, "a");
	states2[2].set_transition(2, "b");

	states2[1].is_terminal = true;

	FiniteAutomaton dfa1 = FiniteAutomaton(0, states1, {"a", "b"});
	FiniteAutomaton dfa2 = FiniteAutomaton(0, states2, {"a", "b"});

	// string f1 = dfa1.to_txt();
	// string f2 = dfa2.to_txt();
	FiniteAutomaton dfa3 = FiniteAutomaton::intersection(dfa1, dfa2);

	// vector<Tester::word> words;
	// for(int i=0; i<12;i++) {
	// 	words.push_back({
	// 		i, i*100, true
	// 	});
	// }
	// string s = "test";
	Logger::activate();
	Logger::init();
	// Logger::init_step(s);
	tester();
	// Logger::finish_step();
	Logger::finish();
	Logger::deactivate();
}

void Example::fa_semdet_check() {
	vector<State> states;
	for (int i = 0; i < 4; i++) {
		State s = {
			i, {i}, to_string(i), false, map<alphabet_symbol, set<int>>()};
		states.push_back(s);
	}
	states[0].set_transition(1, "a");
	states[1].set_transition(1, "a");
	states[1].set_transition(2, "b");
	states[2].set_transition(1, "a");
	states[2].set_transition(3, "b");
	states[3].set_transition(1, "a");
	states[3].set_transition(3, "b");
	states[0].is_terminal = true;
	states[1].is_terminal = true;
	states[2].is_terminal = true;

	FiniteAutomaton NDM(0, states, {"a", "b"});

	auto sdet = NDM.semdet();

	cout << "Semdet?: " << sdet << "\n";
}