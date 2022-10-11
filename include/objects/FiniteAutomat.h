#pragma once
#include "BaseObject.h"
#include <string>
#include <vector>
using namespace std;

struct State {
	int index;
	bool is_terminal;
	string identifier;
	vector<vector<int>> transitions;
	State();
	State(int index, bool is_terminal, string identifier, vector<vector<int>> transitions);
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
	State& get_transition(int i, int j, int k);
	string to_txt() override;
	FiniteAutomat determinize();
	FiniteAutomat rem_eps();
	FiniteAutomat minimize();
	FiniteAutomat merge_bisimilar();
	static bool equiv(const FiniteAutomat&, const FiniteAutomat&);
	static bool bisimilar(const FiniteAutomat&, const FiniteAutomat&);
	// и тд
};