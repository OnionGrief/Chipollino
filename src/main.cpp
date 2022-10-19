#include "Example.h"
#include "arden.h"
#include <iostream>
using namespace std;

int main() {
	vector<State> states;
	for (int i = 0; i < 3; i++) {
		State s = {i, {i}, "", false, map<char, vector<int>>()};
		states.push_back(s);
	}

	states[0].set_transition(0, 'a');
	states[0].set_transition(1, 'a');

	states[1].set_transition(1, 'b');
	states[1].set_transition(2, 'a');

	states[1].set_transition(2, 'b');
	states[1].set_transition(0, 'b');

	states[2].set_transition(1, 'b');

	states[2].is_terminal = true;

	FiniteAutomat NDM(0, {'a', 'b'}, states, false);
	// cout << NDM.to_txt();
	string reg = "((a|b)*c)";
	Regex r;
	r = nfa_to_regex(NDM);
	// cout << r.to_txt();
}