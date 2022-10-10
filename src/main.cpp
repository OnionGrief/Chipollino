#include <iostream>
#include <Regex.h>
#include <FiniteAutomat.h>
using namespace std;

int main() {
	cout << "Chipollino :-)\n";
	string reg = "((((a*c)))|(bd|q))";
	Regex r(reg);
	r.pre_order_travers();
	r.clear();

	//determinize testing
	vector<State> states;
	for (int i = 0; i < 5; i++) {
		State s = {i, {i}, "", false, map<char, vector <int> >()};
		states.push_back(s);
	}

	states[0].set_transition(1, 'a');
	states[0].set_transition(2, 'a');
	states[1].set_transition(3, 'b');
	states[3].set_transition(1, '\0');
	states[3].set_transition(3, 'a');
	states[2].set_transition(4, 'c');
	states[4].set_transition(4, 'a');
	states[4].set_transition(4, 'b');

	states[3].is_terminal = true;
	states[4].is_terminal = true;

	FiniteAutomat NDM(0, {'a', 'b', 'c'}, states, false);
	FiniteAutomat DM = NDM.determinize();
	cout << DM.to_txt();

	//rem_eps testing
	vector<State> states1;
	for (int i = 0; i < 3; i++) {
		State s = {i, {i}, "", false, map<char, vector <int> >()};
		states1.push_back(s);
	}

	states1[0].set_transition(0, '0');
	states1[0].set_transition(1, '\0');
	states1[1].set_transition(1, '1');
	states1[1].set_transition(2, '\0');
	states1[2].set_transition(2, '0');
	states1[2].set_transition(2, '1');

	states1[2].is_terminal = true;

	FiniteAutomat NDM1(0, {'0', '1'}, states1, false);
	FiniteAutomat ENDM1 = NDM1.rem_eps();
	cout << ENDM1.to_txt();

	//intersection, union, difference & complement testing
	vector<State> states2;
	for (int i = 0; i < 3; i++) {
		State s = {i, {i}, "", false, map<char, vector <int> >()};
		states2.push_back(s);
	}
	vector<State> states3;
	for (int i = 0; i < 3; i++) {
		State s = {i, {i}, "", false, map<char, vector <int> >()};
		states3.push_back(s);
	}

	states2[0].set_transition(0, 'b');
	states2[0].set_transition(1, 'a');
	states2[1].set_transition(1, 'b');
	states2[1].set_transition(2, 'a');
	states2[2].set_transition(2, 'a');
	states2[2].set_transition(2, 'b');

	states2[1].is_terminal = true;

	states3[0].set_transition(0, 'a');
	states3[0].set_transition(1, 'b');
	states3[1].set_transition(1, 'a');
	states3[1].set_transition(2, 'b');
	states3[2].set_transition(2, 'a');
	states3[2].set_transition(2, 'b');

	states3[1].is_terminal = true;

	FiniteAutomat DM1 = FiniteAutomat(0, {'a', 'b'}, states2, false);
	FiniteAutomat DM2 = FiniteAutomat(0, {'a', 'b'}, states3, false);

	FiniteAutomat DM3 = DM1.intersection(DM2);
	FiniteAutomat DM4 = DM1.uunion(DM2);
	FiniteAutomat DM5 = DM1.difference(DM2);
	FiniteAutomat DM6 = DM.complement();

	cout << DM3.to_txt();
	cout << DM4.to_txt();
	cout << DM5.to_txt();
	cout << DM6.to_txt();
}