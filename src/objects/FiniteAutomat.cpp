#include "FiniteAutomat.h"
#include <sstream>
using namespace std;

State::State() : index(0), is_terminal(0), identifier("") {}

State::State(int index, bool is_terminal, string identifier, vector<vector<int>> transitions)
	: index(index), is_terminal(is_terminal), identifier(identifier), transitions(transitions) {}

FiniteAutomat::FiniteAutomat() { }

FiniteAutomat::FiniteAutomat(int initial_state, vector<char> alphabet, vector<State> states, bool is_deterministic)
	: initial_state(initial_state), alphabet(alphabet), states(states), is_deterministic(is_deterministic) {
	number_of_states = (int)states.size();
}

State& FiniteAutomat::get_transition(int i, int j, int k) {
	return states[states[i].transitions[j][k]];
}

string FiniteAutomat::to_txt() {
	stringstream ss;
	ss << "digraph {\n\trankdir = LR\n\tdummy [label = \"\", shape = none]\n\tdummy -> "
		<< states[initial_state].identifier << "\n";

	for (int i = 0; i < number_of_states; i++) {
		for (int j = 0; j <= alphabet.size(); j++) {
			string letter = (j == 0) ? "eps" : string(1, alphabet[j - 1]);
			for (int k = 0; k < states[i].transitions[j].size(); k++) {
				ss << "\t" << states[i].identifier << " -> "
					<< this->get_transition(i, j, k).identifier
					<< " [label = \"" << letter << "\"]\n";
			}
		}
	}

	ss << "}";
	return ss.str();
}