#include "Objects/Language.h"

FA_structure::FA_structure(int initial_state, vector<State> states,
						   weak_ptr<Language> language)
	: initial_state(initial_state), states(states), language(language) {}

Regex_structure::Regex_structure(string str, weak_ptr<Language> language)
	: str(str), language(language) {}

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

bool Language::pump_length_cached() {
	return pump_length.has_value();
}

int Language::get_pump_length() {
	return pump_length.value();
}

void Language::set_min_dfa(int initial_state, const vector<State>& states,
						   const shared_ptr<Language>& language) {
	vector<State> renamed_states = states;
	for (int i = 0; i < renamed_states.size(); i++)
		renamed_states[i].identifier = to_string(i);
	min_dfa.emplace(FA_structure(initial_state, renamed_states, language));
}

bool Language::min_dfa_cached() {
	return min_dfa.has_value();
}

FiniteAutomaton Language::get_min_dfa() {
	return FiniteAutomaton(min_dfa->initial_state, min_dfa->states,
						   min_dfa->language.lock());
}

void Language::set_syntactic_monoid(
	TransformationMonoid syntactic_monoid_value) {
	syntactic_monoid.emplace(syntactic_monoid_value);
}

bool Language::syntactic_monoid_cached() {
	return syntactic_monoid.has_value();
}

TransformationMonoid Language::get_syntactic_monoid() {
	return syntactic_monoid.value();
}

void Language::set_nfa_minimum_size(int nfa_minimum_size_value) {
	nfa_minimum_size.emplace(nfa_minimum_size_value);
};

bool Language::nfa_minimum_size_cached() {
	return nfa_minimum_size.has_value();
}

int Language::get_nfa_minimum_size() {
	return nfa_minimum_size.value();
}

bool Language::is_one_unambiguous_flag_cached() {
	return is_one_unambiguous.has_value();
}

void Language::set_one_unambiguous_flag(bool is_one_unambiguous_flag) {
	is_one_unambiguous.emplace(is_one_unambiguous_flag);
}

bool Language::get_one_unambiguous_flag() {
	return is_one_unambiguous.value();
}

bool Language::is_one_unambiguous_regex_cached() {
	return one_unambiguous_regex.has_value();
}

void Language::set_one_unambiguous_regex(string str,
										 const shared_ptr<Language>& language) {
	one_unambiguous_regex.emplace(Regex_structure(str, language));
}

Regex Language::get_one_unambiguous_regex() {
	return Regex(one_unambiguous_regex->str,
				 one_unambiguous_regex->language.lock());
}