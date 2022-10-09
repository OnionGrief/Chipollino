#include "Interpreter.h"

bool Interpreter::eof() {
	return input.pos >= input.str.size();
}

char Interpreter::current_symbol() {
	if (input.pos > 0 && !eof()) {
		return input.str[input.pos];
	}
	return 0;
}

void Interpreter::next_symbol() {
	input.pos++;
}

void Interpreter::skip_spaces() {
	while (current_symbol() == ' ') {
		next_symbol();
	}
}

bool Interpreter::scan_word(string word) {
	skip_spaces();
	int pos_prev = input.pos;
	for (int i = 0; i < word.size(); i++) {
		next_symbol();
		if (current_symbol() != word[i]) {
			input.pos = pos_prev;
			return false;
		}
	}
	return true;
}

string Interpreter::scan_until_space() {
	string acc = "";
	while (
		current_symbol() != ' ' && 
		current_symbol() != '\n' &&
		!eof()) {

		acc += current_symbol();
		next_symbol();
	}
	return acc;
}

Interpreter::Lexem Interpreter::scan_equalSign() {
	if (scan_word("=")) {
		return Lexem(Lexem::equalSign);
	}
	return Lexem(Lexem::error);
}

Interpreter::Lexem Interpreter::scan_doubleExclamation() {
	if (scan_word("!!")) {
		return Lexem(Lexem::doubleExclamation);
	}
	return Lexem(Lexem::error);
}

Interpreter::Lexem Interpreter::scan_function() {
	for (const auto& function : functions) {
		if (scan_word(function)) {
			return Lexem(Lexem::function, function);
		}
	}
	return Lexem(Lexem::error);
}

Interpreter::Lexem Interpreter::scan_object() {
	if (Lexem number = scan_number(); number.type) {
		return number;
	}
	if (Lexem regex = scan_regex(); regex.type) {
		return regex;
	}
	if (Lexem id = scan_id_rvalue(); id.type) {
		return id;
	}
	return Lexem(Lexem::error);
}

Interpreter::Lexem Interpreter::scan_id_lvalue() {
	// TODO: сделать проверки на корректность имени, чтобы не
	// начиналось с цифры, не было коллизий с именами функций
	string id_name = scan_until_space();
	ids.insert(id_name);
	return Lexem(Lexem::id, id_name);
}

Interpreter::Lexem Interpreter::scan_id_rvalue() {
	for (const auto& id : ids) {
		if (scan_word(id)) {
			return Lexem(Lexem::id, id);
		}
	}
	return Lexem(Lexem::error);
}


Interpreter::Lexem Interpreter::scan_regex() {
	// TODO: сохранять regex куда-нибудь
	string word = scan_until_space();
	auto regex = Regex(word);
	if (regex.is_error()) {
		return Lexem(Lexem::error);
	}
	return Lexem(Lexem::regex, word);
}

Interpreter::Lexem Interpreter::scan_number() {
	int pos_prev = input.pos;
	auto is_digit = [](char c) {
		return c >= '0' && c <= '9';
	};
	string acc = "";
	while (!eof() && is_digit(current_symbol())) {
		acc += current_symbol();
	}
	if (acc == "") {
		input.pos = pos_prev;
		return Lexem(Lexem::error);
	}
	return Lexem(stoi(acc));
}

Interpreter::Lexem Interpreter::scan_predicate() {
	for (const auto& predicate : predicates) {
		if (scan_word(predicate)) {
			return Lexem(Lexem::predicate, predicate);
		}
	}
	return Lexem(Lexem::error);
}

Interpreter::Lexem Interpreter::scan_test() {
	if (scan_word("Test")) {
		return Lexem(Lexem::test);
	}
	return Lexem(Lexem::error);
}

Interpreter::Lexem::Lexem(Type type, string value) : type(type), value(value) {}

Interpreter::Lexem::Lexem(int num) : num(num), type(number) {}
