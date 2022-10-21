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
	for (int i = 0; i < 8; i++) {
		State s = {i, {i}, "", false, map<char, vector<int>>()};
		states.push_back(s);
	}
	states[0].set_transition(1, 'a');
	states[0].set_transition(4, 'b');
	states[1].set_transition(1, 'a');
	states[1].set_transition(2, 'b');
	states[2].set_transition(1, 'a');
	states[2].set_transition(3, 'b');
	states[3].set_transition(1, 'a');
	states[3].set_transition(3, 'b');
	states[4].set_transition(1, 'a');
	states[4].set_transition(5, 'b');
	states[5].set_transition(6, 'a');
	states[5].set_transition(5, 'b');
	states[6].set_transition(6, 'a');
	states[6].set_transition(7, 'b');
	states[7].set_transition(6, 'a');
	states[7].set_transition(5, 'b');
	states[0].is_terminal = true;
	states[1].is_terminal = true;
	states[2].is_terminal = true;
	states[4].is_terminal = true;
	states[5].is_terminal = true;
	FiniteAutomat NDM(0, {'a', 'b'}, states, false);
	cout << nfa_to_regex(NDM).to_txt() + "\n";
}