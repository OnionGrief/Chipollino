#include "RegexGenerator.h"

RegexGenerator::RegexGenerator() {}

RegexGenerator::RegexGenerator(int regex_length, int star_num, int star_nesting, int alphabet_size)
	: regex_length(regex_length), star_num(star_num), star_nesting(star_nesting), alphabet_size(alphabet_size) {

	if (regex_length < 1) return;
	if (star_nesting < 0) star_nesting = 0;
	if (star_num < 0) star_num = 0;
	
	for (char i = 'a'; i <= 'a' + alphabet_size && i <= 'z'; i++) {
		alphabet.push_back(i);
	}
	for (char i = 'A'; i <= 'A' + alphabet_size - 26 && i <= 'Z'; i++) {
		alphabet.push_back(i);
	}
	all_alts_are_eps = true;
	generate_regex(); // не порождает пустое слово, но так и задумано
	cout << res_str << " " << regex_length << "\n";
}

RegexGenerator::RegexGenerator(int regex_length, int star_num, int star_nesting) 
	: regex_length(regex_length), star_num(star_num), star_nesting(star_nesting) {
	int max_alphabet_size = regex_length > 52 ? 52 : regex_length;
	if (max_alphabet_size) alphabet_size = rand() % max_alphabet_size;
	RegexGenerator::RegexGenerator(regex_length, star_num, star_nesting, alphabet_size);
}

void RegexGenerator::generate_regex() { // <regex> ::= <n-alt-regex> <alt> <regex> | <conc-regex> | пусто
	int v;
	if (all_alts_are_eps) //если нет ни одного не пустого слова то оно не допустимо
		v = rand() % 2;
	else
		v = rand() % 3; // выбираем какую из 3х альтернатив использовать

	if (regex_length < 1) return;
	if (v == 0) {
		if (regex_length > 1) generate_n_alt_regex();
		if (regex_length < 1) return;
		res_str += '|';
		generate_regex();
	} else if (v == 1) {
		generate_conc_regex();
	}
};

void RegexGenerator::generate_n_alt_regex() { // <n-alt-regex> ::=  <conc-regex> | пусто
	int v = rand() % 4;						  // подкрутим вероятность выпадения пустого слова
	if (v) {
		generate_conc_regex();
	}
};

void RegexGenerator::generate_conc_regex() { // <conc-regex> ::= <simple-regex> | <simple-regex><conc-regex>
	int v = rand() % 2;
	if (v == 0) {
		generate_simple_regex();
	} else {
		generate_simple_regex();
		if (regex_length < 1) return;
		generate_conc_regex();
	}
};

void RegexGenerator::generate_simple_regex() { // <simple-regex> ::= <lbr><regex-without-eps><rbr><unary>? | буква <unary>?
	int v = rand() % 2;
	if (v == 0) {
		bool prev_eps_counter = all_alts_are_eps;
		all_alts_are_eps = true; // новый контроллер эпсилонов

		int v2;
		if (star_num) {
			int star_chance = regex_length / star_num; //вероятность выпадения звезды при 2 звездах на 20 букв = 1/10
			if (regex_length > star_num)
				star_chance += star_num / star_nesting; // попытка в зависимость вероятности выпадения звезды от max звездной высоты
			else
				star_chance += regex_length / star_nesting;
			if (star_chance < 2) star_chance += 2;
			v2 = rand() % star_chance; // будет ли *
		} else
			v2 = 1;

		if (!v2 && star_num > 0 && cur_nesting < star_nesting) {
			star_num--;
			cur_nesting++;
		} else
			v2 = 1;

		res_str += '(';
		generate_regex();
		res_str += ')';
		all_alts_are_eps = prev_eps_counter;

		if (!v2) {
			res_str += '*';
			cur_nesting--;
		}
	} else {
		all_alts_are_eps = false;
		res_str += rand_symb();

		int v2;
		if (star_num) {
			int star_chance = regex_length / star_num;
			if (star_chance < 2) star_chance = 2;
			v2 = rand() % star_chance;
		} else
			v2 = 1;

		if (!v2 && star_num > 0 && cur_nesting < star_nesting) {
			res_str += '*';
			star_num--;
		}
		regex_length--;
	}
};

char RegexGenerator::rand_symb() {
	return alphabet[rand() % alphabet.size()];
}

string RegexGenerator::to_txt() {
	return res_str;
}

/*
GRAMMAR:
<regex> ::= <n-alt-regex> <alt> <regex> | <conc-regex> | пусто
<n-alt-regex> ::=  <conc-regex> | пусто
<conc-regex> ::= <simple-regex> | <simple-regex><conc-regex>
<simple-regex> ::= <lbr><regex><rbr><unary>? | буква <unary>?
<alt> ::= '|'
<lbr> ::= '('
<rbr> ::= ')'
<unary> ::= '*'
*/