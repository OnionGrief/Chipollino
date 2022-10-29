#pragma once
#include "BaseObject.h"
#include <iostream>
#include <string>
#include <time.h>
#include <vector>
using namespace std;

class RegexGenerator {
  private:
	vector<char> alphabet; // TODO: убрать алфавит

	int regex_length = 0;
	int star_num = 0;
	int cur_regex_length = 0;
	int cur_star_num = 0;
	int star_nesting = 0; //вложенность
	int cur_nesting = 0;
	bool all_alts_are_eps = true;
	string res_str = "";
	void generate_regex_();
	void generate_n_alt_regex();
	void generate_conc_regex();
	void generate_simple_regex();
	int generate_alphabet(int);
	char rand_symb();

  public:
	/*генератор регулярных выражений, со значениями по умолчанию:
	8 - максимальная длина, 3 - максимальное кол-во звезд,
	2 - максимальная звездная вложенность, 2 - число символов в алфавите*/
	RegexGenerator();
	/*генератор регулярных выражений, параметрирозованных длиной, кол-вом
	итераций Клини и звездной вложенностью*/
	RegexGenerator(int regex_length, int star_num, int star_nesting);
	/*генератор регулярных выражений, параметрирозованных длиной, кол-вом
	итераций Клини, звездной вложенностью и размером алфавита*/
	RegexGenerator(int regex_length, int star_num, int star_nesting,
				   int alphabet_size);
	/*сгенерировать регулярное выражение, параметрирозованное длиной, кол-вом
	итераций Клини, звездной вложенностью и размером алфавита*/
	string generate_regex();
};