#include "RegexGenerator.h"

RegexGenerator::RegexGenerator() {}

RegexGenerator::RegexGenerator(vector<char> alphabet, int regex_length, int star_num) : alphabet(alphabet), regex_length(regex_length), star_num(star_num) {
	generate_regex1();
	cout << res_str << "\n";
}

void RegexGenerator::generate_regex() { //<regex> ::= <n-alt-regex> <alt> <regex-without-eps> | <conc-regex> | пусто
	int v = rand() % 3;
	if (v == 0) {
		generate_n_alt_regex();
		res_str += '|';
		generate_regex1();
	} else if (v == 1) {
		generate_conc_regex();
	}
};

void RegexGenerator::generate_regex1() { //<regex-without-eps> ::= <n-alt-regex> <alt> <regex-without-eps> | <conc-regex1>
	int v = rand() % 2;
	if (v == 0) {
		generate_n_alt_regex();
		res_str += '|';
		generate_regex1();
	} else {
		generate_conc_regex();
	}
};

void RegexGenerator::generate_n_alt_regex() { //<n-alt-regex> ::=  <conc-regex> | пусто
	int v = rand() % 2;
	if (v == 0) {
		generate_conc_regex();
	}
};

void RegexGenerator::generate_conc_regex() { //<conc-regex> ::= <simple-regex> | <simple-regex><conc-regex>
	int v = rand() % 2;
	if (v == 0) {
		generate_simple_regex();
	} else {
		generate_simple_regex();
		generate_conc_regex();
	}
};

void RegexGenerator::generate_simple_regex() { //<simple-regex> ::= <lbr><regex><rbr><unary>? | буква <unary>?
	int v = rand() % 2;
	if (v == 0) {
		res_str += '(';
		generate_regex();
		res_str += ')';
		if (rand() % 2) res_str += '*';
	} else {
		res_str += rand_symb();
		if (rand() % 2) res_str += '*';
	}
};

char RegexGenerator::rand_symb() {
	return alphabet[rand() % alphabet.size()];
}

string RegexGenerator::to_txt() {
	return "тут мог быть анекдот";
}