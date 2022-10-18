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

void Example::fa_bisimilar_check() {
	vector<State> states1;
	for (int i = 0; i < 3; i++) {
		State s = {i, {i}, to_string(i), false, map<char, vector<int>>()};
		states1.push_back(s);
	}
	states1[0].set_transition(1, 'a');
	states1[0].set_transition(1, '\0');
	states1[0].set_transition(2, 'b');
	states1[1].set_transition(2, 'a');
	states1[1].set_transition(1, 'b');
	states1[2].set_transition(1, 'a');
	states1[2].set_transition(1, '\0');
	states1[2].set_transition(0, 'b');
	states1[0].is_terminal = true;
	states1[2].is_terminal = true;
	FiniteAutomat fa1(1, {'a', 'b'}, states1, false);

	vector<State> states2;
	for (int i = 0; i < 2; i++) {
		State s = {i, {i}, to_string(i), false, map<char, vector<int>>()};
		states2.push_back(s);
	}
	states2[0].set_transition(1, 'a');
	states2[0].set_transition(1, '\0');
	states2[0].set_transition(0, 'b');
	states2[1].set_transition(0, 'a');
	states2[1].set_transition(1, 'b');
	states2[0].is_terminal = true;
	FiniteAutomat fa2(1, {'a', 'b'}, states2, false);

	cout << FiniteAutomat::bisimilar(fa1, fa2);
	//правильный ответ true
}

void Example::fa_equal_check() {
	vector<State> states1;
	for (int i = 0; i < 4; i++) {
		State s = {i, {i}, to_string(i), false, map<char, vector<int>>()};
		states1.push_back(s);
	}
	states1[0].set_transition(1, 'a');
	states1[0].set_transition(1, 'a');
	states1[0].set_transition(2, 'a');
	states1[1].set_transition(3, 'b');
	states1[2].set_transition(3, 'c');
	FiniteAutomat fa1(0, {'a', 'b', 'c'}, states1, false);

	vector<State> states2;
	for (int i = 0; i < 4; i++) {
		State s = {i, {i}, to_string(i), false, map<char, vector<int>>()};
		states2.push_back(s);
	}
	states2[0].set_transition(1, 'a');
	states2[0].set_transition(1, 'a');
	states2[0].set_transition(2, 'a');
	states2[1].set_transition(3, 'c');
	states2[2].set_transition(3, 'b');
	FiniteAutomat fa2(0, {'a', 'b', 'c'}, states2, false);

	vector<State> states3;
	for (int i = 0; i < 4; i++) {
		State s = {i, {i}, to_string(i), false, map<char, vector<int>>()};
		states3.push_back(s);
	}
	states3[3].set_transition(2, 'a');
	states3[3].set_transition(2, 'a');
	states3[3].set_transition(1, 'a');
	states3[2].set_transition(0, 'b');
	states3[1].set_transition(0, 'c');
	FiniteAutomat fa3(3, {'a', 'b', 'c'}, states3, false);

	cout << FiniteAutomat::equal(fa1, fa1) << endl
		 << FiniteAutomat::equal(fa1, fa2) << endl
		 << FiniteAutomat::equal(fa1, fa3);
	//правильный ответ 1 0 1
}

void Example::fa_merge_bisimilar() {
	vector<State> states1;
	for (int i = 0; i < 3; i++) {
		State s = {i, {i}, to_string(i), false, map<char, vector<int>>()};
		states1.push_back(s);
	}
	states1[0].set_transition(1, 'a');
	states1[0].set_transition(1, '\0');
	states1[0].set_transition(2, 'b');
	states1[1].set_transition(2, 'a');
	states1[1].set_transition(1, 'b');
	states1[2].set_transition(1, 'a');
	states1[2].set_transition(1, '\0');
	states1[2].set_transition(0, 'b');
	states1[0].is_terminal = true;
	states1[2].is_terminal = true;
	FiniteAutomat fa1(1, {'a', 'b'}, states1, false);

	cout << fa1.to_txt();

	FiniteAutomat fa2 = fa1.merge_bisimilar();

	cout << fa2.to_txt();
}
