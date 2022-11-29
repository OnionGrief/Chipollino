#include "Interpreter/Interpreter.h"

bool Interpreter::Lexer::eof() {
	return input.pos >= input.str.size();
}

char Interpreter::Lexer::current_symbol() {
	if (input.pos >= 0 && !eof()) {
		return input.str[input.pos];
	}
	return 0;
}

void Interpreter::Lexer::next_symbol() {
	input.pos++;
}

void Interpreter::Lexer::skip_spaces() {
	while (current_symbol() == ' ' || current_symbol() == '\t') {
		next_symbol();
	}
}

bool Interpreter::Lexer::scan_word(string word) {
	skip_spaces();
	int pos_prev = input.pos;
	for (int i = 0; i < word.size(); i++) {
		if (current_symbol() != word[i]) {
			input.pos = pos_prev;
			return false;
		}
		next_symbol();
	}
	return true;
}

string Interpreter::Lexer::scan_until_space() {
	string acc = "";
	skip_spaces();
	while (!eof() && current_symbol() != ' ' && current_symbol() != '\n' &&
		   current_symbol() != '\t' && current_symbol() != '.' &&
		   current_symbol() != '(' && current_symbol() != ')') {

		acc += current_symbol();
		next_symbol();
	}
	return acc;
}

string Interpreter::Lexer::scan_until(char symbol) {
	string acc = "";
	while (!eof() && current_symbol() != symbol) {
		acc += current_symbol();
		next_symbol();
	}
	return acc;
}

Interpreter::Lexem Interpreter::Lexer::scan_equalSign() {
	if (scan_word("=")) {
		return Lexem(Lexem::equalSign);
	}
	return Lexem(Lexem::error);
}

Interpreter::Lexem Interpreter::Lexer::scan_doubleExclamation() {
	if (scan_word("!!")) {
		return Lexem(Lexem::doubleExclamation);
	}
	return Lexem(Lexem::error);
}

Interpreter::Lexem Interpreter::Lexer::scan_parL() {
	if (scan_word("(")) {
		return Lexem(Lexem::parL);
	}
	return Lexem(Lexem::error);
}

Interpreter::Lexem Interpreter::Lexer::scan_parR() {
	if (scan_word(")")) {
		return Lexem(Lexem::parR);
	}
	return Lexem(Lexem::error);
}

Interpreter::Lexem Interpreter::Lexer::scan_dot() {
	if (scan_word(".")) {
		return Lexem(Lexem::dot);
	}
	return Lexem(Lexem::error);
}

Interpreter::Lexem Interpreter::Lexer::scan_number() {
	int pos_prev = input.pos;
	auto is_digit = [](char c) { return c >= '0' && c <= '9'; };
	string acc = "";
	while (!eof() && is_digit(current_symbol())) {
		acc += current_symbol();
		next_symbol();
	}
	if (acc == "") {
		input.pos = pos_prev;
		return Lexem(Lexem::error);
	}
	return Lexem(stoi(acc));
}

Interpreter::Lexem Interpreter::Lexer::scan_name() {
	string name = scan_until_space();
	if (name.size()) {
		return Lexem(Lexem::name, name);
	}
	return Lexem(Lexem::error);
}

Interpreter::Lexem Interpreter::Lexer::scan_stringval() {
	if (scan_word("\"")) {
		string val = scan_until('\"');
		scan_word("\"");
		return Lexem(Lexem::stringval, val);
	}
	return Lexem(Lexem::error);
}

Interpreter::Lexem Interpreter::Lexer::scan_regex() {
	if (scan_word("{")) {
		string val = scan_until('}');
		scan_word("}");
		return Lexem(Lexem::regex, val);
	}
	return Lexem(Lexem::error);
}

Interpreter::Lexem Interpreter::Lexer::scan_lexem() {
	if (Lexem lex = scan_parL(); lex.type) {
		return lex;
	}
	if (Lexem lex = scan_parR(); lex.type) {
		return lex;
	}
	if (Lexem lex = scan_dot(); lex.type) {
		return lex;
	}
	if (Lexem lex = scan_regex(); lex.type) {
		return lex;
	}
	if (Lexem lex = scan_stringval(); lex.type) {
		return lex;
	}
	if (Lexem lex = scan_number(); lex.type) {
		return lex;
	}
	if (Lexem lex = scan_equalSign(); lex.type) {
		return lex;
	}
	if (Lexem lex = scan_doubleExclamation(); lex.type) {
		return lex;
	}
	if (Lexem lex = scan_name(); lex.type) {
		return lex;
	}
	auto logger = parent.init_log();
	logger.log("Lexer: failed to scan \"" +
			   input.str.substr(input.pos, input.str.size()) + "\"");
	return Lexem(Lexem::error);
}

Interpreter::Lexem::Lexem(Type type, string value) : type(type), value(value) {}

Interpreter::Lexem::Lexem(int num) : num(num), type(number) {}

vector<vector<Interpreter::Lexem>> Interpreter::Lexer::load_file(string path) {
	auto logger = parent.init_log();
	logger.log("Lexer: loading file " + path);
	ifstream input_file(path);
	if (!input_file) {
		logger.throw_error("Error: failed to open " + path);
	}
	// Сюда будем записывать строки из лексем
	vector<vector<Lexem>> lexem_lines = {};
	string str = "";
	while (getline(input_file, str)) {
		if (auto lexems = parse_string(str); lexems.size()) {
			lexem_lines.push_back(lexems);
			logger.log("scanned line: " + str);
		}
	}
	logger.log("Lexer: file loaded");
	return lexem_lines;
}

vector<Interpreter::Lexem> Interpreter::Lexer::parse_string(string str) {
	input.str = str;
	input.pos = 0;
	vector<Lexem> lexems;
	skip_spaces();
	while (!eof()) {
		auto lexem = scan_lexem();
		lexems.push_back(lexem);
	}
	return lexems;
}
