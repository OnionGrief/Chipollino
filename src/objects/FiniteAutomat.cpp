#include "FiniteAutomat.h"
#include <sstream>

FiniteAutomat::FiniteAutomat() { }
	
FiniteAutomat::FiniteAutomat(bool _is_deterministic, int _initial_state, std::vector<char> _alphabet, std::vector<bool> _is_terminal, std::vector<std::vector<std::vector<int>>> _transition_matrix)
	:  is_deterministic(_is_deterministic), initial_state(_initial_state), alphabet(_alphabet), is_terminal(is_terminal), transition_matrix(_transition_matrix) {
	number_of_states = (int)transition_matrix.size();
	state_identifiers.resize(number_of_states, "");
}

std::string FiniteAutomat::to_txt() {
    std::stringstream ss;
	ss << "digraph {\n\trankdir = LR\n\tdummy [label = \"\", shape = none]\n\tdummy -> " << initial_state << "\n";
	for(int i = 0; i < number_of_states; i++) {
		for(int j = 0; j < alphabet.size(); j++) {
			for(int k = 0; k < transition_matrix[i][j].size(); k++){
				ss << "\t" << i << " -> " << transition_matrix[i][j][k] << " [label = \"" << alphabet[j] << "\"]\n";
			}
		}
	}
	ss << "}";
	return ss.str();
}