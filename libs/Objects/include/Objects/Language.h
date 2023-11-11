#pragma once
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "AlphabetSymbol.h"
#include "FiniteAutomaton.h"
#include "Regex.h"
#include "TransformationMonoid.h"

// нужна, чтобы хранить weak_ptr на язык
struct FA_structure {
	int initial_state;
	vector<State> states;
	// если не хранить этот указатель,
	// будут созданы разные shared_ptr
	std::weak_ptr<Language> language;

	FA_structure(int initial_state, vector<State> states, std::weak_ptr<Language> language);
};

struct Regex_structure {
	string str;
	std::weak_ptr<Language> language;

	Regex_structure(string str, std::weak_ptr<Language> language);
};

class Language {
  private:
	set<alphabet_symbol> alphabet;
	// регулярка, описывающая язык
	// optional<Regex> regular_expression;
	std::optional<int> pump_length;
	std::optional<FA_structure> min_dfa;
	std::optional<TransformationMonoid> syntactic_monoid;
	// нижняя граница размера НКА для языка
	std::optional<int> nfa_minimum_size;
	// классы эквивалентности минимального дка TODO
	// аппроксимации минимальных НКА и регулярок TODO
	std::optional<bool> is_one_unambiguous;
	std::optional<Regex_structure> one_unambiguous_regex;

  public:
	Language();
	Language(set<alphabet_symbol> alphabet); // NOLINT(runtime/explicit)
	const set<alphabet_symbol>& get_alphabet();
	void set_alphabet(set<alphabet_symbol>);
	int get_alphabet_size();
	// регулярка, описывающая язык
	bool is_regular_expression_cached() const;
	void set_regular_expression(int);
	int get_regular_expression();
	// накачка
	bool is_pump_length_cached() const;
	void set_pump_length(int);
	int get_pump_length();
	// минимальный дка
	bool is_min_dfa_cached() const;
	void set_min_dfa(int initial_state, const vector<State>& states,
					 const std::shared_ptr<Language>& Language);
	FiniteAutomaton get_min_dfa();
	// синтаксический моноид
	bool is_syntactic_monoid_cached() const;
	void set_syntactic_monoid(TransformationMonoid);
	TransformationMonoid get_syntactic_monoid();
	// нижняя граница размера НКА для языка
	bool is_nfa_minimum_size_cached() const;
	void set_nfa_minimum_size(int);
	int get_nfa_minimum_size();
	// 1-однозначная регулярка
	bool is_one_unambiguous_flag_cached() const;
	void set_one_unambiguous_flag(bool);
	bool get_one_unambiguous_flag();
	bool is_one_unambiguous_regex_cached() const;
	void set_one_unambiguous_regex(string, const std::shared_ptr<Language>&);
	Regex get_one_unambiguous_regex();
	//  и тд
};