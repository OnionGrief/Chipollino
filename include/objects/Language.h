#pragma once
#include "AlphabetSymbol.h"
#include "FiniteAutomaton.h"
#include "Regex.h"
#include <optional>
#include <vector>

class Language {
  private:
	vector<alphabet_symbol> alphabet;
	optional<int> pump_length;
	optional<FiniteAutomaton> min_dfa;
	// классы эквивалентности в минимального дка TODO
	// синтаксический моноид TODO
	// аппроксимации минимальных НКА и регулярок TODO
  public:
	Language();
	Language(vector<alphabet_symbol> alphabet);
	// string to_txt();
	const vector<alphabet_symbol>& get_alphabet();
	void set_alphabet(vector<alphabet_symbol>);
	int get_alphabet_size();
	alphabet_symbol get_alphabet_symbol(int);
	void set_pump_length(int);
	const optional<int>& get_pump_length();
	void set_min_dfa(FiniteAutomaton);
	const optional<FiniteAutomaton>& get_min_dfa();
	// и тд
};