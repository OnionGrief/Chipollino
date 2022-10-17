#include "Example.h"
#include <iostream>
using namespace std;

int main() {
	cout << "Chipollino :-)\n";
	vector<State> states;
	for (int i = 0; i < 6; i++) {
		State s = {i, {i}, "", false, map<char, vector <int> >()};
		states.push_back(s);
	}

	states[0].set_transition(5, 'x');
	states[0].set_transition(5, 'y');
	states[0].set_transition(1, '\0');
	states[1].set_transition(2, '\0');
	states[1].set_transition(3, '\0');
	states[1].set_transition(4, '\0');
	states[3].set_transition(3, 'x');
	states[4].set_transition(4, 'y');
	states[4].set_transition(4, 'y');
	states[5].set_transition(5, 'z');
	states[5].set_transition(1, '\0');

	states[2].is_terminal = true;
	states[3].is_terminal = true;
	states[4].is_terminal = true;

	FiniteAutomat NDM(0, {'x', 'y', 'z'}, states, false);
	FiniteAutomat DM = NDM.determinize();
	cout << DM.to_txt();
	FiniteAutomat DM1 = DM.remove_trap_state();
	cout << DM1.to_txt();
	FiniteAutomat DM2 = DM1.add_trap_state();
	cout << DM2.to_txt();
	FiniteAutomat DM3 = DM2.add_trap_state();
	cout << DM3.to_txt();
	FiniteAutomat DM4 = DM3.add_trap_state();
	cout << DM4.to_txt();
	FiniteAutomat DM5 = DM4.remove_trap_state();
	cout << DM5.to_txt();
}