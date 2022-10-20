#include "Example.h"

void Example::regex_parsing() {
	string reg = "((a|b)*c)";
	Regex r;
	if (r.from_string(reg)) {
		cout << "ERROR\n";
		return;
	}
	r.pre_order_travers();
	r.clear();
}

void Example::arden_test() {
	vector<State> states;
	for (int i = 0; i < 3; i++) {
		State s = {i, {i}, "", false, map<char, vector<int>>()};
		states.push_back(s);
	}

	states[0].set_transition(0, 'a');
	states[0].set_transition(1, 'a');
	states[1].set_transition(1, 'b');
	states[1].set_transition(2, 'a');
	states[1].set_transition(0, 'b');
	states[2].set_transition(1, 'b');
	states[2].is_terminal = true;
	FiniteAutomat NDM(0, {'a', 'b'}, states, false);
	cout << nfa_to_regex(NDM).to_txt();
}