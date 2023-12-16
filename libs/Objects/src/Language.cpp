#include "Objects/Language.h"
#include "Objects/TransformationMonoid.h"

using std::cerr;
using std::set;
using std::string;
using std::to_string;
using std::vector;

Language::Regex_model::Regex_model(string str, std::weak_ptr<Language> language)
	: str(str), language(language) {}

const std::string& Language::Regex_model::get_str() const {
	return str;
}

std::shared_ptr<Language> Language::Regex_model::get_language() const {
	return language.lock();
}

Language::Language() {}

Language::Language(set<Symbol> alphabet) : alphabet(alphabet) {}

void Language::enable_retrieving_from_cache() {
	allow_retrieving_from_cache = true;
}

void Language::disable_retrieving_from_cache() {
	allow_retrieving_from_cache = false;
}

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
	if (!allow_retrieving_from_cache)
		return false;
	return pump_length.has_value();
}

void Language::set_pump_length(int pump_length_value) {
	pump_length.emplace(pump_length_value);
}

int Language::get_pump_length() {
	cerr << "INFO: pump_length is obtained from cache \n";
	return pump_length.value();
}

bool Language::is_min_dfa_cached() const {
	if (!allow_retrieving_from_cache)
		return false;
	return min_dfa.has_value();
}

void Language::set_min_dfa(const FiniteAutomaton& fa) {
	vector<FAState> renamed_states = fa.get_states();
	for (int i = 0; i < renamed_states.size(); i++)
		renamed_states[i].identifier = to_string(i);
	min_dfa.emplace(FA_model(fa.get_initial(), renamed_states, fa.get_language()));
}

FiniteAutomaton Language::get_min_dfa() {
	cerr << "INFO: min_dfa is obtained from cache \n";
	return min_dfa->make_fa();
}

bool Language::is_syntactic_monoid_cached() const {
	if (!allow_retrieving_from_cache)
		return false;
	return syntactic_monoid.has_value();
}

void Language::set_syntactic_monoid(TransformationMonoid syntactic_monoid_value) {
	syntactic_monoid.emplace(syntactic_monoid_value);
}

TransformationMonoid Language::get_syntactic_monoid() {
	cerr << "INFO: syntactic_monoid is obtained from cache \n";
	return syntactic_monoid.value();
}

bool Language::is_nfa_minimum_size_cached() const {
	if (!allow_retrieving_from_cache)
		return false;
	return nfa_minimum_size.has_value();
}

void Language::set_nfa_minimum_size(int nfa_minimum_size_value) {
	nfa_minimum_size.emplace(nfa_minimum_size_value);
}

int Language::get_nfa_minimum_size() {
	cerr << "INFO: nfa_minimum_size is obtained from cache \n";
	return nfa_minimum_size.value();
}

bool Language::is_one_unambiguous_flag_cached() const {
	if (!allow_retrieving_from_cache)
		return false;
	return is_one_unambiguous.has_value();
}

void Language::set_one_unambiguous_flag(bool is_one_unambiguous_flag) {
	is_one_unambiguous.emplace(is_one_unambiguous_flag);
}

bool Language::get_one_unambiguous_flag() {
	cerr << "INFO: is_one_unambiguous is obtained from cache \n";
	return is_one_unambiguous.value();
}

bool Language::is_one_unambiguous_regex_cached() const {
	if (!allow_retrieving_from_cache)
		return false;
	return one_unambiguous_regex.has_value();
}

void Language::set_one_unambiguous_regex(string str, const std::shared_ptr<Language>& language) {
	one_unambiguous_regex.emplace(Regex_model(str, language));
}

Regex Language::get_one_unambiguous_regex() {
	cerr << "INFO: one_unambiguous_regex is obtained from cache \n";
	return Regex(one_unambiguous_regex->get_str(), one_unambiguous_regex->get_language());
}