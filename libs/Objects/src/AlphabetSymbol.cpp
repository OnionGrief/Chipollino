#include "Objects/AlphabetSymbol.h"

alphabet_symbol::alphabet_symbol(){};
alphabet_symbol::alphabet_symbol(const string& s) : symbol(s), value(s){};
alphabet_symbol::alphabet_symbol(const char* c) : symbol(c), value(c){};
alphabet_symbol::alphabet_symbol(char c)
	: symbol(string(1, c)), value(string(1, c)){};

alphabet_symbol::alphabet_symbol(const alphabet_symbol& other)
	: symbol(other.symbol), annote_numbers(other.annote_numbers),
	  linearize_numbers(other.linearize_numbers), value(other.value){};

void alphabet_symbol::update_value() {
	value = symbol;
	for (const auto& i : annote_numbers)
		value += i;
	for (const auto& i : linearize_numbers)
		value += i;
}

const alphabet_symbol& alphabet_symbol::operator=(const string& s) {
	symbol = s;
	value = symbol;
	return *this;
}

const alphabet_symbol& alphabet_symbol::operator=(const char* c) {
	symbol = c;
	value = symbol;
	return *this;
}

const alphabet_symbol& alphabet_symbol::operator=(char c) {
	symbol = c;
	value = symbol;
	return *this;
}

const alphabet_symbol& alphabet_symbol::operator=(
	const alphabet_symbol& other) {
	symbol = other.symbol;
	annote_numbers = other.annote_numbers;
	linearize_numbers = other.linearize_numbers;
	value = other.value;
	return *this;
}

alphabet_symbol alphabet_symbol::epsilon() {
	return "eps";
}

bool alphabet_symbol::operator==(const alphabet_symbol& other) const {
	return symbol == other.symbol && annote_numbers == other.annote_numbers &&
		   linearize_numbers == other.linearize_numbers;
}

bool alphabet_symbol::operator!=(const alphabet_symbol& other) const {
	return !(*this == other);
}

bool alphabet_symbol::operator<(const alphabet_symbol& other) const {
	return value < other.value;
}

bool alphabet_symbol::is_epsilon() const {
	return *this == alphabet_symbol::epsilon();
}

alphabet_symbol::operator string() const {
	return value;
}

string alphabet_symbol::vector_to_str(const vector<alphabet_symbol>& in) {
	string out = "";
	for (const auto& i : in)
		out += i;
	return out;
}

ostream& operator<<(ostream& os, const alphabet_symbol& as) {
	return os << (string)as;
}

void alphabet_symbol::annote(int num) {
	annote_numbers.push_back("." + to_string(num));
	update_value();
}

void alphabet_symbol::linearize(int num) {
	linearize_numbers.push_back("." + to_string(num));
	update_value();
}

void alphabet_symbol::deannote() {
	if (!annote_numbers.empty()) {
		annote_numbers.pop_back();
		update_value();
	}
}

void alphabet_symbol::delinearize() {
	if (!linearize_numbers.empty()) {
		linearize_numbers.pop_back();
		update_value();
	}
}

bool alphabet_symbol::is_annotated() const {
	return !annote_numbers.empty();
}

bool alphabet_symbol::is_linearize() const {
	return !linearize_numbers.empty();
}