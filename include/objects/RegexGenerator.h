#pragma once
#include "BaseObject.h"
#include "Regex.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class RegexGenerator : BaseObject {
  private:
	vector<char> alphabet;
	int alphabet_size = 1;
	int regex_length = 0;
	int star_num = 0;
	int star_nesting = 0; //вложенность
	int cur_nesting = 0;
	bool all_alts_are_eps = true;
	Regex res_regex;
	string res_str = "";

  public:
	RegexGenerator();
	RegexGenerator(int, int, int);
	RegexGenerator(int, int, int, int);
	string to_txt() override;

	char rand_symb();
	void generate_regex();
	void generate_n_alt_regex();
	void generate_conc_regex();
	void generate_simple_regex();
};