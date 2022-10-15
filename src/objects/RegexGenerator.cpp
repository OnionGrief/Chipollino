#include "RegexGenerator.h"

RegexGenerator::RegexGenerator() {}

RegexGenerator::RegexGenerator(vector<char> alphabet, int regex_length, int star_num, int star_nesting)
	: alphabet(alphabet), regex_length(regex_length), star_num(star_num), star_nesting(star_nesting) {
	if (regex_length < 1) return;
	all_alts_are_eps = true;
	generate_regex(); // не порождает пустое слово, но так и задумано
	cout << res_str << " " << regex_length << "\n";
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
	int v = rand() % 2;
	if (v == 0) {
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

		bool new_nesting = false;
		if (cur_nesting == 0) new_nesting = true;
		int v2 = rand() % 2; // будет ли *
		if (v2 && star_num > 0 && cur_nesting < star_nesting) {
			star_num--;
			cur_nesting++;
		} else
			v2 = 0;

		res_str += '(';
		generate_regex();
		res_str += ')';
		all_alts_are_eps = prev_eps_counter;

		if (v2) res_str += '*';
		if (new_nesting) cur_nesting = 0;
	} else {
		all_alts_are_eps = false;
		res_str += rand_symb();
		if (rand() % 2 && star_num > 0 && cur_nesting < star_nesting) {
			res_str += '*';
			star_num--;
			if (cur_nesting > 0) cur_nesting++;
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