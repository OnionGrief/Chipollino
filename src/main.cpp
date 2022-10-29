#include "Example.h"
#include <iostream>
using namespace std;

int main() {
	cout << "Chipollino :-)\n";
	vector<State> states1;
	for (int i = 0; i < 4; i++) {
		State s = {
			i, {i}, to_string(i), false, map<alphabet_symbol, set<int>>()};
		states1.push_back(s);
	}
	states1[0].set_transition(1, "a");
	states1[0].set_transition(2, "a");
	states1[1].set_transition(3, "b");
	states1[2].set_transition(3, "b");
	states1[3].set_transition(3, "a");
	states1[3].set_transition(0, "b");
	states1[3].is_terminal = true;
	FiniteAutomaton fa1(0, states1, {"a", "b"});

	cout << fa1.to_txt() << endl;
	cout << fa1.minimize().to_txt() << endl;

	fa1.ambiguity();

	vector<State> states2;
	for (int i = 0; i < 4; i++) {
		State s = {
			i, {i}, to_string(i), false, map<alphabet_symbol, set<int>>()};
		states2.push_back(s);
	}
	states2[0].set_transition(1, "a");
	states2[0].set_transition(2, "a");
	states2[1].set_transition(3, "b");
	states2[2].set_transition(3, "b");
	states2[2].set_transition(1, "b");
	states2[2].set_transition(0, "b");
	states2[3].set_transition(3, "a");
	states2[3].is_terminal = true;
	FiniteAutomaton fa2(0, states2, {"a", "b"});

	fa2.ambiguity();
}