#include "InputGenerator/RegexGenerator.h"

using std::ofstream;
using std::string;
using std::to_string;

RegexGenerator::RegexGenerator() : RegexGenerator::RegexGenerator(8, 3, 2, 2) {}

RegexGenerator::RegexGenerator(int neg_chance_) : RegexGenerator::RegexGenerator() {
	neg_chance = neg_chance_;
}

RegexGenerator::RegexGenerator(int regex_length, int star_num, int star_nesting, int alphabet_size)
	: regex_length(regex_length), star_num(star_num), star_nesting(star_nesting) {

	if (regex_length < 1)
		return;
	if (star_nesting < 0)
		star_nesting = 0;
	if (star_num < 0)
		star_num = 0;
	if (alphabet_size < 1)
		alphabet_size = 1;

	for (char i = 'a'; i < 'a' + alphabet_size && i <= 'z'; i++) {
		alphabet.push_back(i);
	}
	for (char i = 'A'; i < 'A' + alphabet_size - 26 && i <= 'Z'; i++) {
		alphabet.push_back(i);
	}
}

RegexGenerator::RegexGenerator(int regex_length, int cur_star_num, int star_nesting)
	: RegexGenerator::RegexGenerator(regex_length, cur_star_num, star_nesting,
									 generate_alphabet(regex_length)) {}

void RegexGenerator::change_seed() {
	seed_it++;
	srand((size_t)time(nullptr) + seed_it + rand());
}

// рандомное число в диапозоне [0; n]
int rand_num(int n) {
	return n > 0 ? rand() % n : 0;
}

bool check_probability(int percentage) {
	if (rand_num(100) < percentage)
		return true;
	return false;
}

void RegexGenerator::write_to_file(string filename) {
	ofstream out(filename, std::ios::app);
	if (out.is_open())
		out << res_str << "\n";
	out.close();
}

int RegexGenerator::generate_alphabet(int regex_length) {
	change_seed();
	int max_alphabet_size = regex_length > 52 ? 52 : regex_length;
	int alphabet_size = 0;
	if (max_alphabet_size)
		alphabet_size = rand_num(max_alphabet_size);
	return alphabet_size + 1;
}

string RegexGenerator::generate_regex() {
	change_seed();
	all_alts_are_eps = true;
	cur_nesting = 0;
	res_str = "";
	cur_regex_length = regex_length;
	cur_star_num = star_num;
	generate_regex_(); // не порождает пустое слово, но так и задумано
	return res_str;
}

string RegexGenerator::generate_brefregex(int cells_num_, int mem_writer_chance_, int ref_chance_) {
	cells_num = cells_num_;
	mem_writer_chance = mem_writer_chance_;
	ref_chance = ref_chance_;
	is_backref = true;
	neg_chance = 0;
	string brefregex = generate_regex();
	is_backref = false;
	return brefregex;
}

void RegexGenerator::generate_regex_() { // <regex> ::= <n-alt-regex> <alt>
										 // <regex> | <conc-regex> | пусто
	int v;
	if (all_alts_are_eps) // если нет ни одного не пустого слова то оно не
						  // допустимо
		v = rand() % 2;
	else
		v = rand() % 3; // выбираем какую из 3х альтернатив использовать

	if (cur_regex_length < 1)
		return;
	if (v == 0) {
		if (cur_regex_length > 1)
			generate_n_alt_regex();
		if (cur_regex_length < 1)
			return;
		res_str += '|';
		generate_regex_();
	} else if (v == 1) {
		generate_conc_regex();
	}
}

void RegexGenerator::generate_n_alt_regex() { // <n-alt-regex> ::=  <conc-regex>
											  // | пусто
	if (check_probability(75)) {
		generate_conc_regex();
	}
}

void RegexGenerator::generate_conc_regex() { // <conc-regex> ::= <simple-regex>
											 // | <simple-regex><conc-regex>
	if (check_probability(50)) {
		generate_simple_regex();
	} else {
		generate_simple_regex();
		if (cur_regex_length < 1)
			return;
		generate_conc_regex();
	}
}

void RegexGenerator::generate_simple_regex() { // <simple-regex> ::= <neg>? буква <star>? |
											   //        <neg>? <lbr><regex><rbr> <star>?
	if (check_probability(40)) {
		bool prev_eps_counter = all_alts_are_eps;
		all_alts_are_eps = true; // новый контроллер эпсилонов

		int v2;
		if (cur_star_num) {
			int star_chance = cur_regex_length / cur_star_num; // вероятность выпадения звезды при
															   // 2 звездах на 20 букв = 1/10
			if (cur_regex_length > cur_star_num)
				star_chance +=
					cur_star_num / star_nesting; // попытка в зависимость вероятности выпадения
												 // звезды от max звездной высоты
			else
				star_chance += cur_regex_length / star_nesting;
			if (star_chance < 2)
				star_chance += 2;
			v2 = rand() % star_chance; // будет ли *
		} else {
			v2 = 1;
		}

		if (!v2 && cur_star_num > 0 && cur_nesting < star_nesting) {
			cur_star_num--;
			cur_nesting++;
		} else {
			v2 = 1;
		}

		if (!is_backref && check_probability(neg_chance))
			res_str += '^';

		if (is_backref && (cells_num > in_memory_writer.size()) &&
			check_probability(mem_writer_chance)) {

			int ref_num = rand_num(cells_num) + 1;
			// запрещено [[]:1]:1, но допускается [[]:2]:1
			while (in_memory_writer.count(ref_num) != 0)
				ref_num = rand_num(cells_num) + 1;
			in_memory_writer.insert(ref_num);

			res_str += "[";
			generate_regex_();
			res_str += "]:" + to_string(ref_num);

			in_memory_writer.erase(ref_num);
		} else {
			res_str += '(';
			generate_regex_();
			res_str += ')';
		}

		all_alts_are_eps = prev_eps_counter;

		if (!v2) {
			res_str += '*';
			cur_nesting--;
		}
	} else {
		all_alts_are_eps = false;

		if (!is_backref && check_probability(neg_chance))
			res_str += '^';

		if (is_backref && check_probability(ref_chance) && (cells_num > in_memory_writer.size())) {
			int ref_num = rand_num(cells_num) + 1;
			// запрещено [&1]:1, но допускается [&2]:1
			while (in_memory_writer.count(ref_num) != 0)
				ref_num = rand_num(cells_num) + 1;
			res_str += "&" + to_string(ref_num);
		} else {
			res_str += rand_symb();
		}

		int v2;
		if (cur_star_num) {
			int star_chance = cur_regex_length / cur_star_num;
			if (star_chance < 2)
				star_chance = 2;
			v2 = rand() % star_chance;
		} else {
			v2 = 1;
		}

		if (!v2 && cur_star_num > 0 && cur_nesting < star_nesting) {
			res_str += '*';
			cur_star_num--;
		}
		cur_regex_length--;
	}
}

char RegexGenerator::rand_symb() {
	return alphabet[rand_num(alphabet.size())];
}

void RegexGenerator::set_neg_chance(int new_neg_chance) {
	neg_chance = 0 < new_neg_chance ? new_neg_chance : 0;
	neg_chance = neg_chance < 100 ? neg_chance : 100;
}
/*
GRAMMAR:
<regex> ::= <n-alt-regex> <alt> <regex> | <conc-regex> | пусто
<n-alt-regex> ::=  <conc-regex> | пусто
<conc-regex> ::= <simple-regex> | <simple-regex><conc-regex>
<simple-regex> ::= <neg>? ( <lbr><regex><rbr> | буква | [ <regex> ] : <num> |  & <num> ) <star>?
<alt> ::= '|'
<lbr> ::= '('
<rbr> ::= ')'
<star> ::= '*'
<neg> ::= '^'
*/