#include <iostream>
#include <vector>

#include "FiniteAutomat.h"
#include "TransformationMonoid.h"
using namespace std;

int main() {
	vector<State> states;
	for (int i = 0; i < 3; i++) {
		State s = {i, {i}, "", false, map<char, vector<int>>()};
		states.push_back(s);
	}

	states[0].set_transition(1, 'a');
	states[0].set_transition(0, 'b');

	states[1].set_transition(1, 'a');
	states[1].set_transition(2, 'b');
	states[1].set_transition(1, 'c');

	states[2].set_transition(1, 'a');
	states[2].set_transition(2, 'b');
	states[2].set_transition(2, 'c');

	states[2].is_terminal = true;

	FiniteAutomat NDM(0, {'a', 'b', 'c'}, states, false);
	// cout << NDM.to_txt();
	TransformationMonoid a(&NDM);
	// cout << a.get_Equalence_Classes_Txt();
	// cout << a.get_Equalence_Classes_Txt(); /*
	vector<Term> cur = a.get_Equalence_Classes();
	cout << cur[1].name << "\n";
	vector<TermDouble> temp = a.get_Equalence_Classes_VWV(cur[1]);
	for (int i = 0; i < temp.size(); i++) {
		cout << temp[i].first.name << " " << temp[i].second.name << "\n";
	}
}