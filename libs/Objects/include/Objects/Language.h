#pragma once
#include "AlphabetSymbol.h"
#include "FiniteAutomaton.h"
#include "Regex.h"
#include "TransformationMonoid.h"
#include <optional>
#include <vector>

// нужна, чтобы хранить weak_ptr на язык
struct FA_structure {
	int initial_state;
	vector<State> states;
	weak_ptr<Language> language;

	FA_structure(int initial_state, vector<State> states,
				 weak_ptr<Language> language);
};

class Language {
  private:
	set<alphabet_symbol> alphabet;
	optional<int> pump_length;
	optional<FA_structure> min_dfa;
	optional<TransformationMonoid> syntactic_monoid;
	// нижняя граница размера НКА для языка
	optional<int> nfa_minimum_size;
	// классы эквивалентности минимального дка TODO
	// синтаксический моноид TODO
	// аппроксимации минимальных НКА и регулярок TODO
  public:
	Language();
	Language(set<alphabet_symbol> alphabet);
	const set<alphabet_symbol>& get_alphabet();
	void set_alphabet(set<alphabet_symbol>);
	int get_alphabet_size();
	void set_pump_length(int);
	const optional<int>& get_pump_length();
	void set_min_dfa(int initial_state, const vector<State>& states,
					 shared_ptr<Language> Language);
	optional<FiniteAutomaton> get_min_dfa();
	void set_syntactic_monoid(TransformationMonoid);
	const optional<TransformationMonoid>& get_syntactic_monoid();
	void set_nfa_minimum_size(int);
	const optional<int>& get_nfa_minimum_size();
	// и тд
};