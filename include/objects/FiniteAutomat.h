#pragma once
#include "BaseObject.h"
#include <string>
#include <vector>
#include <map>
#include <stack>
#include <set>
#include <iostream>
using namespace std;

struct State {
	int index;
	vector<int> label;
	string identifier;
	bool is_terminal;
	map<char,vector<int> > transitions;
	State();
	State(int index, vector<int> label, string identifier, bool is_terminal, map<char,vector<int> > transitions);
	void set_transition(int, char);
};

class FiniteAutomat : public BaseObject {
private:
	int number_of_states = 0;
	bool is_deterministic = 0;
	int initial_state = 0;
	vector<char> alphabet;
	vector<State> states;

public:
	FiniteAutomat();
	FiniteAutomat(int initial_state, vector<char> alphabet, vector<State> states, bool is_deterministic = false);
	string to_txt() override;
	vector<int> closure(vector<int>);
	FiniteAutomat determinize();
	FiniteAutomat rem_eps();
	FiniteAutomat minimize();
	FiniteAutomat intersection(FiniteAutomat);
	FiniteAutomat uunion(FiniteAutomat);
	FiniteAutomat difference(FiniteAutomat);
	FiniteAutomat complement(FiniteAutomat);
	// и тд
};