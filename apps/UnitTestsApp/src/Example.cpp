#include "UnitTestsApp/Example.h"

using std::cout;
using std::endl;
using std::map;
using std::set;
using std::string;
using std::vector;

void Example::determinize() {
	vector<FAState> states;
	for (int i = 0; i < 6; i++) {
		states.emplace_back(i, set<int>({i}), std::to_string(i), false, FAState::Transitions());
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
	vector<FAState> states;
	for (int i = 0; i < 3; i++) {
		states.emplace_back(i, set<int>({i}), std::to_string(i), false, FAState::Transitions());
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
	vector<FAState> states;
	for (int i = 0; i < 8; i++) {
		states.emplace_back(i, set<int>({i}), std::to_string(i), false, FAState::Transitions());
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
	vector<FAState> states1;
	for (int i = 0; i < 3; i++) {
		states1.emplace_back(i, set<int>({i}), std::to_string(i), false, FAState::Transitions());
	}
	vector<FAState> states2;
	for (int i = 0; i < 3; i++) {
		states2.emplace_back(i, set<int>({i}), std::to_string(i), false, FAState::Transitions());
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
	string regr = "(a*a*)*"; //"((a|)*c)";
	regl = regl + regr;
	// egl = "a()a))";			  // regl + regr;
	// regl = "a1456|b244444444444";
	//  regl = "(ab|b)*ba"; //"bbb*(aaa*bbb*)*";
	Regex r(regr);
	cout << r.to_txt() << "\n";
	// cout << r.pump_length() << "\n";
	FiniteAutomaton a;
	FiniteAutomaton b;
	FiniteAutomaton c;
	FiniteAutomaton d;

	// cout << "to_tompson ------------------------------\n";
	c = r.to_thompson(); // to_tompson(-1);
	// cout << c.to_txt();

	// cout << "to_glushkov ------------------------------\n";
	a = r.to_glushkov();
	// cout << a.to_txt();
	// cout << "to_ilieyu  ------------------------------\n";
	b = r.to_ilieyu();
	// cout << b.to_txt();

	// FiniteAutomaton d;
	// cout << "to_antimirov  ------------------------------\n";
	d = r.to_antimirov();
	// cout << d.to_txt();
	//   cout << r.deannote().to_txt();

	//  cout << FiniteAutomaton::equal(b.minimize(), c.minimize()) << endl;

	// cout << a.to_regex().to_txt();
	FiniteAutomaton r1 = r.linearize().to_glushkov();
	cout << r1.to_txt() << "\n";
	cout << "-----------------\n";
	cout << r1.minimize().to_txt() << "\n";
}

void Example::parsing_nfa() {
	string regl = "a(bbb*aaa*)*bb*|aaa*(bbb*aaa*)*|b(aaa*bbb*)*aa*|";
	string regr = "bbb*(aaa*bbb*)*"; //"((a|)*c)";
	regl = regl + regr;
	regl = "a*bc*"; //"bbb*(aaa*bbb*)*";
	Regex r(regl);

	FiniteAutomaton a;
	FiniteAutomaton b;
	FiniteAutomaton c;

	cout << "to_thompson ------------------------------\n";
	c = r.to_ilieyu(); // to_thompson(-1);
	cout << c.to_txt();
	cout << "Parsing: aaaaaaaaaaaaaaaaaaabccccc\n";
	cout << c.parse("aaaaaaaaaaaaaaaaaaabccccc").second << endl; // true если распознал слово
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

void Example::parsing_regex(string str) {
	cout << str << endl;
	Regex r(str);
	cout << r.to_txt() << endl;
}

void Example::transformation_monoid_example() {
	FiniteAutomaton fa = Regex("(ba)*bc").to_ilieyu();
	vector<FAState> states1;
	for (int i = 0; i < 3; i++) {
		states1.emplace_back(i, set<int>({i}), std::to_string(i), false, FAState::Transitions());
	}
	// states1[0].set_transition(0, "b");
	states1[0].set_transition(1, "a");
	states1[1].set_transition(0, "b");
	states1[0].set_transition(1, "c");
	states1[1].set_transition(2, "c");
	states1[2].is_terminal = true;
	// states1[2].set_transition(2, "a");
	// states1[2].set_transition(2, "b");
	FiniteAutomaton dfa1 = FiniteAutomaton(0, states1, {"a", "b", "c"});
	cout << "-----\n";
	cout << dfa1.to_txt();
	cout << "-----\n";
	TransformationMonoid a(dfa1);
	cout << a.get_equalence_classes_txt() << endl;
	cout << a.get_rewriting_rules_txt() << endl;
	a.class_card();
	a.class_length();
	a.is_minimal();
	a.get_classes_number_MyhillNerode();
}

void Example::fa_subset_check() {
	vector<FAState> states1;
	for (int i = 0; i < 4; i++) {
		states1.emplace_back(i, set<int>({i}), std::to_string(i), false, FAState::Transitions());
	}
	states1[0].set_transition(1, "a");
	states1[0].set_transition(1, "b");
	states1[0].set_transition(2, "a");
	states1[1].set_transition(3, "b");
	states1[2].set_transition(3, "c");
	states1[3].is_terminal = true;
	FiniteAutomaton fa1(0, states1, {"a", "b", "c"});

	vector<FAState> states2;
	for (int i = 0; i < 3; i++) {
		states2.emplace_back(i, set<int>({i}), std::to_string(i), false, FAState::Transitions());
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
	Regex r(regl);

	cout << "\nNormalize\nBefore: " << r.to_txt() << "\n";
	r.normalize_regex({{Regex("a"), Regex("b")}});
	cout << "After: " << r.to_txt() << "\n";
}
void Example::to_image() {
	vector<FAState> states1;
	for (int i = 0; i < 3; i++) {
		states1.emplace_back(i, set<int>({i}), std::to_string(i), false, FAState::Transitions());
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

	AutomatonToImage::to_image(s1);
	AutomatonToImage::to_image(s2);
}

void Example::step() {
	vector<FAState> states1;
	for (int i = 0; i < 3; i++) {
		states1.emplace_back(i, set<int>({i}), std::to_string(i), false, FAState::Transitions());
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
	s = "skip";
	s = "remove\\_eps";
}

void Example::tester() {
	Regex r("ab(ab|a)*ababa");
	FiniteAutomaton dfa1 = r.to_thompson();
	// cout << dfa1.parse("abaaabaaababa");
	// cout << dfa1.to_txt();
	Regex r1("((ab)*a)*");
	Regex r2("ab(ab|a)*ababa");
	Tester::test(dfa1, r1, 1);
	Tester::test(r2, r1, 1);
}

void Example::step_interection() {
	vector<FAState> states1;
	for (int i = 0; i < 3; i++) {
		states1.emplace_back(i, set<int>({i}), std::to_string(i), false, FAState::Transitions());
	}
	vector<FAState> states2;
	for (int i = 0; i < 3; i++) {
		states2.emplace_back(i, set<int>({i}), std::to_string(i), false, FAState::Transitions());
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
}

void Example::arden_example() {

	Regex r1("b((b(b|)ab*))*");
	// cout << r1.to_thompson().to_txt();
	Regex temp = r1.to_thompson().to_regex();
	// cout << temp.to_txt() << "\n";
	//  cout << temp.to_thompson().to_txt();
}

void Example::table() {
	vector<string> r;
	vector<string> c;
	vector<string> data;
	for (int i = 0; i < 3; i++) {
		c.push_back("c");
		r.push_back("r");
		for (int j = 0; j < 3; j++) {
			data.push_back(std::to_string(i + j));
		}
	}
	LogTemplate::Table t;
	t.columns = c;
	t.data = data;
	t.rows = r;
	LogTemplate log_template;
	Logger tex_logger;
	log_template.set_parameter("table", t);
	log_template.load_tex_template("determinize");
	tex_logger.add_log(log_template);
	tex_logger.render_to_file("./resources/report.tex");
}

void Example::fa_semdet_check() {
	vector<FAState> states;
	for (int i = 0; i < 4; i++) {
		states.emplace_back(i, set<int>({i}), std::to_string(i), false, FAState::Transitions());
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

void Example::classes_number_GlaisterShallit() {
	Regex r("abc");
	r.pump_length();
	FiniteAutomaton fa = r.to_glushkov();
	cout << fa.get_classes_number_GlaisterShallit() << endl;
	fa.is_nfa_minimal();
}

void Example::all_examples() {
	// determinize();
	// remove_eps();
	// minimize();
	// intersection();
	// regex_parsing();
	// regex_generating();
	// random_regex_parsing();
	// tasks_generating();
	// parsing_regex("b(ab)*b");
	// transformation_monoid_example();
	normalize_regex();
	// step();
	parsing_nfa();
	fa_subset_check();
	arden_example();
	// to_image();
	// tester();
	// step_interection();
	// table();
	fa_semdet_check();
	// logger_test();
	fa_to_pgrammar();
	// Regex("abaa").pump_length();
	cout << "all the examlples are successful" << endl;
}

void Example::logger_test() {
	LogTemplate log_template;
	Logger tex_logger;
	// log_template.load_tex_template("glushkov-long");
	// Regex("abaa").pump_length(&log_template);
	// Regex("(a|b)*b").to_glushkov(&log_template);
	// tex_logger.add_log(log_template);
	// log_template.load_tex_template("determinize-short");
	// Regex("(a|b)*b").to_glushkov().determinize(&log_template);
	// tex_logger.add_log(log_template);
	// log_template.load_tex_template("tomson-long");
	// Regex("(a|b)*b").to_tompson(&log_template);
	// tex_logger.add_log(log_template);
	// log_template.load_tex_template("follow-long");
	// Regex("(a|b)*b").to_ilieyu(&log_template);
	// tex_logger.add_log(log_template);
	// log_template.load_tex_template("antimirov-long");
	// Regex("(a|b)*b").to_antimirov(&log_template);
	// tex_logger.add_log(log_template);
	log_template.load_tex_template("Test1");
	Regex r("ab(ab|a)*ababa");
	FiniteAutomaton dfa1 = r.to_thompson();
	Regex r1("((ab)*a)*");
	Regex r2("ab(ab|a)*ababa");
	Tester::test(dfa1, r1, 1, &log_template);
	tex_logger.add_log(log_template);
	Tester::test(r2, r1, 1, &log_template);
	tex_logger.add_log(log_template);
	tex_logger.render_to_file("./resources/report.tex");
}

void Example::testing_with_generator(int regex_length, int star_num, int star_nesting,
									 int alphabet_size,
									 const std::function<void(string&)>& check_function) {
	RegexGenerator RG(regex_length, star_num, star_nesting, alphabet_size);
	for (int i = 0; i < 100; i++) {
		string s = RG.generate_regex();
		cout << "> " << i << endl;
		check_function(s);
	}
}

void Example::arden_lemma_testing() {
	testing_with_generator(3, 3, 3, 3, [](string& s) {
		Regex r1(s);
		cout << s << endl;
		FiniteAutomaton fa1 = Regex(s).to_glushkov();
		cout << fa1.size() << endl;
		int a1 = fa1.ambiguity();
		cout << "A " << a1 << endl;

		Regex r2 = fa1.to_regex();
		cout << r2.to_txt() << endl;
		FiniteAutomaton fa2 = r2.to_glushkov();
		cout << fa2.size() << endl;
		int a2 = fa2.ambiguity();
		cout << "A " << a2 << endl;

		cout << "Eq " << Regex::equivalent(r1, r2) << endl;
		assert(a1 == a2);
	});
}

void Example::fa_to_pgrammar() {
	FiniteAutomaton a1 = Regex("(c,1(ab*a|b*)*d,1)|(c,2(ba*b|a*)*d,25)")
							 .to_glushkov()
							 .merge_bisimilar(); // Regex("b*a(a|c)*b(b|c)*").to_ilieyu();
	// cout << a1.to_txt();

	vector<FAState> states1;
	for (int i = 0; i < 5; i++) {
		states1.emplace_back(i, set<int>({i}), std::to_string(i), false, FAState::Transitions());
	}
	// states1[0].set_transition(0, "b");
	states1[4].set_transition(1, "a");
	// states1[0].set_transition(1, "c");
	//  states1[1].set_transition(1, "a");
	//  states1[1].set_transition(1, "c");
	states1[1].set_transition(2, "b");
	states1[1].set_transition(4, "c");
	// states1[2].set_transition(2, "c");
	states1[2].set_transition(2, "b");
	states1[2].set_transition(2, "c");
	states1[2].is_terminal = true;
	states1[0].set_transition(4, "c");
	states1[3].set_transition(0, "a");
	states1[3].set_transition(0, "b");
	states1[4].is_terminal = true;
	FiniteAutomaton dfa1 = FiniteAutomaton(3, states1, {"a", "b", "c"});

	PrefixGrammar g;
	FiniteAutomaton test = a1.annote();
	cout << dfa1.to_txt();

	g.fa_to_prefix_grammar(dfa1);
	cout << "+++++++++++++++++++++++++++++" << endl;
	cout << g.pg_to_txt();
	cout << "+++++++++++++++++++++++++++++" << endl;

	cout << g.prefix_grammar_to_automaton().to_txt();

	g.fa_to_prefix_grammar_TM(dfa1);
	cout << "+++++++++++++++++++++++++++++" << endl;
	cout << g.pg_to_txt();
	cout << "+++++++++++++++++++++++++++++" << endl;
	cout << g.prefix_grammar_to_automaton().to_txt();
}