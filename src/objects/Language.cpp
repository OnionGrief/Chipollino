#include "Language.h"

Language::Language() {}

Language::Language(vector<string> alphabet) : alphabet(alphabet) {}

const vector<string>& Language::get_alphabet() {
	return alphabet;
}

int Language::get_alphabet_size() {
	return alphabet.size();
}

string Language::get_alphabet_letter(int ind) {
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