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
	int regex_length = 0;
	int star_num = 0;
	int star_nesting = 0; //вложенность
	int cur_nesting = 0;
	Regex res_regex;
	string res_str = "";

  public:
	RegexGenerator();
	RegexGenerator(vector<char>, int, int, int);
	string to_txt() override;

	char rand_symb();
	void generate_regex();
	void generate_regex1(); // without epsilon
	void generate_n_alt_regex();
	void generate_conc_regex();
	void generate_conc_regex1();
	void generate_simple_regex();
	void generate_simple_regex1();
};