#include "RegexGenerator.h"

RegexGenerator::RegexGenerator() {}

RegexGenerator::RegexGenerator(vector<char> alphabet, int regex_length, int star_num, int star_nesting)
	: alphabet(alphabet), regex_length(regex_length), star_num(star_num), star_nesting(star_nesting) {
	if (regex_length < 1) return;
	generate_regex1();
	cout << res_str << " " << regex_length << "\n";
}

void RegexGenerator::generate_regex() { // <regex> ::= <n-alt-regex> <alt> <regex-without-eps> | <conc-regex> | пусто
	int v = rand() % 3;					// выбираем какую из 3х альтернатив использовать
	if (regex_length < 1) return;
	if (v == 0) {
		if (regex_length > 1) generate_n_alt_regex();
		if (regex_length < 1) return;
		res_str += '|';
		generate_regex1();
	} else if (v == 1) {
		generate_conc_regex();
	}
};

void RegexGenerator::generate_regex1() { // <regex-without-eps> ::= <n-alt-regex> <alt> <regex-without-eps> | <conc-regex1>
	int v = rand() % 2;
	// if (regex_length < 1) return;
	if (v == 0) {
		if (regex_length > 1) generate_n_alt_regex();
		if (regex_length < 1) return;
		res_str += '|';
		generate_regex1();
	} else {
		generate_conc_regex1();
	}
	// cout << res_str << " " << regex_length << "\n";
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
void RegexGenerator::generate_conc_regex1() { // <conc-regex1> ::= <simple-regex1> | <simple-regex><conc-regex1>
	int v = rand() % 2;
	if (v == 0) {
		generate_simple_regex1();
	} else {
		generate_simple_regex();
		if (regex_length < 1) return;
		generate_conc_regex1();
	}
};

void RegexGenerator::generate_simple_regex() { // <simple-regex> ::= <lbr><regex-without-eps><rbr><unary> | <lbr><regex><rbr> | буква <unary>?
	int v = rand() % 2;
	if (v == 0) {
		bool new_nesting = false;
		if (cur_nesting == 0) new_nesting = true;
		int v2 = rand() % 2; //будет ли *
		if (v2 && star_num > 0 && cur_nesting < star_nesting) {
			star_num--;
			cur_nesting++;
		} else
			v2 = 0;

		res_str += '(';
		if (v2)
			generate_regex1();
		else
			generate_regex();
		res_str += ')';

		if (v2) res_str += '*';
		if (new_nesting) cur_nesting = 0;
	} else {
		res_str += rand_symb();
		if (rand() % 2 && star_num > 0 && cur_nesting < star_nesting) {
			res_str += '*';
			star_num--;
			if (cur_nesting > 0) cur_nesting++;
		}
		regex_length--;
	}
};

void RegexGenerator::generate_simple_regex1() { // <simple-regex1> ::= <lbr><regex-without-eps><rbr><unary>? | буква <unary>?
	int v = rand() % 2;
	if (v == 0) {
		bool new_nesting = false;
		if (cur_nesting == 0) new_nesting = true;
		int v2 = rand() % 2; //будет ли *
		if (v2 && star_num > 0 && cur_nesting < star_nesting) {
			star_num--;
			cur_nesting++;
		} else
			v2 = 0;

		res_str += '(';
		generate_regex1();
		res_str += ')';

		if (v2) res_str += '*';
		if (new_nesting) cur_nesting = 0;
	} else {
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
MY GRAMMAR:
<regex> ::= <n-alt-regex> <alt> <regex-without-eps> | <conc-regex> | пусто
<regex-without-eps> ::= <n-alt-regex> <alt> <regex-without-eps> | <conc-regex1>
<n-alt-regex> ::=  <conc-regex> | пусто
<conc-regex> ::= <simple-regex> | <simple-regex><conc-regex>
<conc-regex1> ::= <simple-regex1> | <simple-regex><conc-regex1>
<simple-regex> ::= <lbr><regex-without-eps><rbr><unary> | <lbr><regex><rbr> | буква <unary>?
<simple-regex1> ::= <lbr><regex-without-eps><rbr><unary>? | буква <unary>?
<alt> ::= '|'
<lbr> ::= '('
<rbr> ::= ')'
<unary> ::= '*'
порождает не все возможные: обязывает в конце последовательности альтернатив и конкатенаций иметь непустое слово
*/