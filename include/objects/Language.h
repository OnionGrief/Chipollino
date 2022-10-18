#pragma once
#include "FiniteAutomat.h"
#include "Regex.h"
#include <optional>
#include <vector>
using namespace std;

class Language {
  private:
	vector<string> alphabet;
	optional<int> pump_length;
	optional<FiniteAutomat> min_dfa;
	// классы эквивалентности в минимального дка
	// синтаксический моноид
	// аппроксимации минимальных НКА и регулярок
  public:
	Language();
	Language(vector<string> alphabet);
	// string to_txt();
	const vector<string>& get_alphabet();
	int get_alphabet_size();
	string get_alphabet_letter(int);
	void set_pump_length(int);
	const optional<int>& get_pump_length();
	void set_min_dfa(FiniteAutomat);
	const optional<FiniteAutomat>& get_min_dfa();
	// и тд
};