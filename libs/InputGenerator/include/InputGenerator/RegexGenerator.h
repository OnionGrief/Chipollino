#pragma once
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

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

	// for BackRefRegex:
	bool is_backref = false;
	int cells_num = 0;
	// вероятность появления memoryWriter [..]:1
	int mem_writer_chance = 0;
	// вер-ть появления ссылки &1
	int ref_chance = 0;
	// не допускает вложенности [[..]:1]:2 и [&1]:1.
	// хранит номера текущих memoryWriters, вектор пустой - если находимся вне [..]:1
	std::vector<int> in_memory_writer;

	// для проверки на отсутствие (|||)
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
	/*сгенерировать регулярное выражение с обратными ссылками. Аргументы:
	cells_num - кол-ов ячеек памяти;
	mem_writer_chance - вероятность появления [..]:1 (в %) вместо скобок (..);
	ref_chance - шанс появления ссылки &1 (в %) вместо буквы. */
	std::string generate_brefregex(int cells_num = 3, int mem_writer_chance = 30,
								   int ref_chance = 30);
	/*запись регулярки в файл*/
	void write_to_file(std::string filename);
	/*установить шанс появления отрицания - чем больше значение, тем реже шанс*/
	void set_neg_chance(int new_neg_chance);
};