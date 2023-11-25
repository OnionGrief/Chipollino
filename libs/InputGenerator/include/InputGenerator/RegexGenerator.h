#pragma once
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "Objects/BaseObject.h"

class RegexGenerator {
  private:
	std::vector<char> alphabet; // TODO: убрать алфавит
	size_t seed_it = 0;			// итерация для рандома

	int regex_length = 0;
	int star_num = 0;
	int cur_regex_length = 0;
	int cur_star_num = 0;
	int star_nesting = 0; // вложенность
	int cur_nesting = 0;
	int neg_chance = 0;
	bool all_alts_are_eps = true;
	std::string res_str = "";
	void generate_regex_();
	void generate_n_alt_regex();
	void generate_conc_regex();
	void generate_simple_regex();
	int generate_alphabet(int);
	char rand_symb();
	void change_seed();

  public:
	/*генератор регулярных выражений, со значениями по умолчанию:
	8 - максимальная длина, 3 - максимальное кол-во звезд,
	2 - максимальная звездная вложенность, 2 - число символов в алфавите*/
	RegexGenerator();
	// генерация регулярок с отрицанием
	explicit RegexGenerator(int neg_chance);
	/*генератор регулярных выражений, параметрирозованных длиной, кол-вом
	итераций Клини и звездной вложенностью*/
	RegexGenerator(int regex_length, int star_num, int star_nesting);
	/*генератор регулярных выражений, параметрирозованных длиной, кол-вом
	итераций Клини, звездной вложенностью и размером алфавита*/
	RegexGenerator(int regex_length, int star_num, int star_nesting, int alphabet_size);
	/*сгенерировать регулярное выражение, параметрирозованное длиной, кол-вом
	итераций Клини, звездной вложенностью и размером алфавита*/
	std::string generate_regex();
	/*сгенерировать регулярное выражение, обрамленное фигурными скобками
	(для интерпретатора)*/
	std::string generate_framed_regex();
	/*запись регулярки в файл*/
	void write_to_file(std::string filename);
	/*установить шанс появления отрицания - чем больше значение, тем реже шанс*/
	void set_neg_chance(int new_neg_chance);
};