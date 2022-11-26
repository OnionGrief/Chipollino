#include "Objects/Language.h"

FA_structure::FA_structure(int initial_state, vector<State> states,
						   weak_ptr<Language> language)
	: initial_state(initial_state), states(states), language(language) {}

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

void Language::set_min_dfa(int initial_state, const vector<State>& states,
						   shared_ptr<Language> language) {
	min_dfa.emplace(FA_structure(initial_state, states, language));
}

optional<FiniteAutomaton> Language::get_min_dfa() {
	optional<FiniteAutomaton> min_fa_opt;
	if (min_dfa)
		min_fa_opt.emplace(FiniteAutomaton(
			min_dfa->initial_state, min_dfa->states, min_dfa->language.lock()));
	return min_fa_opt;
}

bool Language::is_one_unambiguity_flag_cached() {
	return is_one_unambiguous.has_value();
}

void Language::set_one_unambiguity_flag(bool is_one_unambiguous_flag) {
	is_one_unambiguous.emplace(is_one_unambiguous_flag);
}

bool Language::get_one_unambiguity_flag() {
	return is_one_unambiguous.value();
}