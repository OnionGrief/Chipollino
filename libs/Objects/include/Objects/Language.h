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
	// если не хранить этот указатель,
	// будут созданы разные shared_ptr
	weak_ptr<Language> language;

	FA_structure(int initial_state, vector<State> states,
				 weak_ptr<Language> language);
};

class Language {
  private:
	set<alphabet_symbol> alphabet;
	// регулярка, описывающая язык
	// optional<Regex> regular_expression;
	optional<int> pump_length;
	optional<FA_structure> min_dfa;
	optional<TransformationMonoid> syntactic_monoid;
	// нижняя граница размера НКА для языка
	optional<int> nfa_minimum_size;
	// классы эквивалентности минимального дка TODO
	// аппроксимации минимальных НКА и регулярок TODO
  public:
	Language();
	Language(set<alphabet_symbol> alphabet);
	const set<alphabet_symbol>& get_alphabet();
	void set_alphabet(set<alphabet_symbol>);
	int get_alphabet_size();
	// регулярка, описывающая язык
	void set_regular_expression(int);
	bool regular_expression_cached();
	int get_regular_expression();
	// накачка
	void set_pump_length(int);
	bool pump_length_cached();
	int get_pump_length();
	//минимальный дка
	void set_min_dfa(int initial_state, const vector<State>& states,
					 shared_ptr<Language> Language);
	bool min_dfa_cached();
	FiniteAutomaton get_min_dfa();
	// синтаксический моноид
	void set_syntactic_monoid(TransformationMonoid);
	bool syntactic_monoid_cached();
	TransformationMonoid get_syntactic_monoid();
	// нижняя граница размера НКА для языка
	void set_nfa_minimum_size(int);
	bool nfa_minimum_size_cached();
	int get_nfa_minimum_size();
	//  и тд
};