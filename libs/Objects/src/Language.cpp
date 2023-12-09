#include "Objects/Language.h"
#include "Objects/TransformationMonoid.h"

using std::set;
using std::string;
using std::to_string;
using std::vector;

Language::Regex_model::Regex_model(string str, std::weak_ptr<Language> language)
	: str(str), language(language) {}

Language::Language() {}

Language::Language(set<Symbol> alphabet) : alphabet(alphabet) {}

void Language::set_alphabet(set<Symbol> _alphabet) {
	alphabet = _alphabet;
}

const set<Symbol>& Language::get_alphabet() {
	return alphabet;
}

int Language::get_alphabet_size() {
	return alphabet.size();
}

bool Language::is_pump_length_cached() const {
	return pump_length.has_value();
}

void Language::set_pump_length(int pump_length_value) {
	pump_length.emplace(pump_length_value);
}

int Language::get_pump_length() {
	return pump_length.value();
}

bool Language::is_min_dfa_cached() const {
	return min_dfa.has_value();
}

void Language::set_min_dfa(int initial_state, const vector<FAState>& states,
						   const std::shared_ptr<Language>& language) {
	vector<FAState> renamed_states = states;
	for (int i = 0; i < renamed_states.size(); i++)
		renamed_states[i].identifier = to_string(i);
	min_dfa.emplace(FA_model(initial_state, renamed_states, language));
}

FiniteAutomaton Language::get_min_dfa() {
	return min_dfa->make_fa();
}

bool Language::is_syntactic_monoid_cached() const {
	return syntactic_monoid.has_value();
}

void Language::set_syntactic_monoid(TransformationMonoid syntactic_monoid_value) {
	syntactic_monoid.emplace(syntactic_monoid_value);
}

TransformationMonoid Language::get_syntactic_monoid() {
	return syntactic_monoid.value();
}

bool Language::is_nfa_minimum_size_cached() const {
	return nfa_minimum_size.has_value();
}

void Language::set_nfa_minimum_size(int nfa_minimum_size_value) {
	nfa_minimum_size.emplace(nfa_minimum_size_value);
}

int Language::get_nfa_minimum_size() {
	return nfa_minimum_size.value();
}

bool Language::is_one_unambiguous_flag_cached() const {
	return is_one_unambiguous.has_value();
}

void Language::set_one_unambiguous_flag(bool is_one_unambiguous_flag) {
	is_one_unambiguous.emplace(is_one_unambiguous_flag);
}

bool Language::get_one_unambiguous_flag() {
	return is_one_unambiguous.value();
}

bool Language::is_one_unambiguous_regex_cached() const {
	return one_unambiguous_regex.has_value();
}

void Language::set_one_unambiguous_regex(string str, const std::shared_ptr<Language>& language) {
	one_unambiguous_regex.emplace(Regex_model(str, language));
}

Regex Language::get_one_unambiguous_regex() {
	return Regex(one_unambiguous_regex->str, one_unambiguous_regex->language.lock());
}