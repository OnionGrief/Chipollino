#pragma once
#include "FiniteAutomat.h"
#include "Regex.h"
#include <optional>
#include <vector>
using namespace std;

using alphabet_symbol = char;
alphabet_symbol epsilon();
bool is_epsilon(alphabet_symbol as);

class Language {
  private:
	vector<alphabet_symbol> alphabet;
	optional<int> pump_length;
	optional<FiniteAutomat> min_dfa;
	// классы эквивалентности в минимального дка
	// синтаксический моноид
	// аппроксимации минимальных НКА и регулярок
  public:
	Language();
	Language(vector<alphabet_symbol> alphabet);
	// string to_txt();
	const vector<alphabet_symbol>& get_alphabet();
	int get_alphabet_size();
	alphabet_symbol get_alphabet_symbol(int);
	void set_pump_length(int);
	const optional<int>& get_pump_length();
	void set_min_dfa(FiniteAutomat);
	const optional<FiniteAutomat>& get_min_dfa();
	// и тд
};