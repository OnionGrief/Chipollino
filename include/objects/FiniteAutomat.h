#pragma once
#include "BaseObject.h"
#include <string>
#include <vector>

class FiniteAutomat: public BaseObject {
private:
	int number_of_states = 0;
	bool is_deterministic = 0;
	int initial_state = 0;
	std::vector<char> alphabet;
	std::vector<bool> is_terminal;
	std::vector<std::vector<std::vector<int>>> transition_matrix;
	std::vector<std::string> state_identifiers; //нужны для проверки на равенство
public:
	FiniteAutomat();
	FiniteAutomat(bool _is_deterministic, int _initial_state, std::vector<char> alphabet, std::vector<bool> _is_terminal, std::vector<std::vector<std::vector<int>>> _transition_matrix);
	std::string to_txt() override;
};