#include "Objects/Language.h"

Language::Language() {}

Language::Language(set<alphabet_symbol> alphabet) : alphabet(alphabet) {}

void Language::set_alphabet(set<alphabet_symbol> _alphabet) {
	alphabet = _alphabet;
}

const set<alphabet_symbol>& Language::get_alphabet() {
	return alphabet;
}

int Language::get_alphabet_size() {
	return alphabet.size();
}

void Language::set_pump_length(int pump_length_value) {
	pump_length.emplace(pump_length_value);
};

const optional<int>& Language::get_pump_length() {
	return pump_length;
}

void Language::set_min_dfa(FiniteAutomaton min_dfa_value) {
	min_dfa.emplace(min_dfa_value);
}

const optional<FiniteAutomaton>& Language::get_min_dfa() {
	return min_dfa;
}