#include "Language.h"

alphabet_symbol epsilon() {
	return '\0';
}
bool is_epsilon(alphabet_symbol as) {
	return as == epsilon();
}

Language::Language() {}

Language::Language(vector<alphabet_symbol> alphabet) : alphabet(alphabet) {}

const vector<alphabet_symbol>& Language::get_alphabet() {
	return alphabet;
}

int Language::get_alphabet_size() {
	return alphabet.size();
}

alphabet_symbol Language::get_alphabet_symbol(int ind) {
	return alphabet[ind];
}

void Language::set_pump_length(int pump_length_value) {
	pump_length.emplace(pump_length_value);
};

const optional<int>& Language::get_pump_length() {
	return pump_length;
}

void Language::set_min_dfa(FiniteAutomat min_dfa_value) {
	min_dfa.emplace(min_dfa_value);
}

const optional<FiniteAutomat>& Language::get_min_dfa() {
	return min_dfa;
}