#include "Example.h"
#include <iostream>
using namespace std;

int main() {
	cout << "Chipollino :-)\n";
	vector<State> states1;
	for (int i = 0; i < 6; i++) {
		State s = {i, {i}, "", false, map<char, vector <int> >()};
		states1.push_back(s);
	}

	states1[0].set_transition(1, '0');
	states1[0].set_transition(2, '1');
	states1[1].set_transition(0, '0');
	states1[1].set_transition(3, '1');
	states1[2].set_transition(4, '0');
	states1[2].set_transition(5, '1');
	states1[3].set_transition(4, '0');
	states1[3].set_transition(5, '1');
	states1[4].set_transition(4, '0');
	states1[4].set_transition(5, '1');
	states1[5].set_transition(5, '0');
	states1[5].set_transition(5, '1');

	states1[2].is_terminal = true;
	states1[3].is_terminal = true;
	states1[4].is_terminal = true;

	FiniteAutomat DM6(0, {'0', '1'}, states1, false);
	cout << DM6.to_txt();
	FiniteAutomat DM7 = DM6.minimize();
	cout << DM7.to_txt();
}