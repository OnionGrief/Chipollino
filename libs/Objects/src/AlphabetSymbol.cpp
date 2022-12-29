#include "Objects/AlphabetSymbol.h"

alphabet_symbol::alphabet_symbol(){};
alphabet_symbol::alphabet_symbol(const string& s) : value(s){};
alphabet_symbol::alphabet_symbol(const char* c) : value(c){};
alphabet_symbol::alphabet_symbol(char c) : value(string(1, c)){};

alphabet_symbol::alphabet_symbol(const alphabet_symbol& other)
	: value(other.value), annote_numbers(other.annote_numbers),
	  linearize_numbers(other.linearize_numbers){};

const alphabet_symbol& alphabet_symbol::operator=(const string& s) {
	value = s;
	return *this;
}

const alphabet_symbol& alphabet_symbol::operator=(const char* c) {
	value = c;
	return *this;
}

const alphabet_symbol& alphabet_symbol::operator=(char c) {
	value = c;
	return *this;
}

const alphabet_symbol& alphabet_symbol::operator=(
	const alphabet_symbol& other) {
	value = other.value;
	annote_numbers = other.annote_numbers;
	linearize_numbers = other.linearize_numbers;
	return *this;
}

alphabet_symbol alphabet_symbol::epsilon() {
	return "eps";
}

bool alphabet_symbol::operator==(const alphabet_symbol& other) const {
	return value == other.value && annote_numbers == other.annote_numbers &&
		   linearize_numbers == other.linearize_numbers;
}

bool alphabet_symbol::operator!=(const alphabet_symbol& other) const {
	return !(*this == other);
}

bool alphabet_symbol::operator<(const alphabet_symbol& other) const {
	return annote_numbers < other.linearize_numbers &&
		   linearize_numbers < other.linearize_numbers && value < other.value;
}

// alphabet_symbol alphabet_symbol::operator+(const alphabet_symbol& other)
// const { 	return value + other.value;
// }

// alphabet_symbol alphabet_symbol::operator+(const string& s) const {
// 	return value + s;
// }

bool alphabet_symbol::is_epsilon() const {
	return *this == alphabet_symbol::epsilon() && annote_numbers.empty() &&
		   linearize_numbers.empty();
}

alphabet_symbol::operator string() const {
	string res = value;
	for (const auto& i : annote_numbers)
		res += i;
	for (const auto& i : linearize_numbers)
		res += i;
	return res;
}

alphabet_symbol alphabet_symbol::remove_numbers() {
	string str_without_numbers;
	for (auto c : value) {
		if (c >= '0' && c <= '9') break;
		str_without_numbers += c;
	}
	return str_without_numbers;
}

string alphabet_symbol::vector_to_str(const vector<alphabet_symbol>& in) {
	string out = "";
	for (const auto& i : in)
		out += i.value;
	return out;
}

ostream& operator<<(ostream& os, const alphabet_symbol& as) {
	return os << (string)as;
}

void alphabet_symbol::annote(int num) {
	annote_numbers.push_back("|" + to_string(num));
}
void alphabet_symbol::linearize(int num) {
	linearize_numbers.push_back("|" + to_string(num));
}
void alphabet_symbol::deannote() {
	if (!annote_numbers.empty()) annote_numbers.pop_back();
}
void alphabet_symbol::delinearize() {
	if (!linearize_numbers.empty()) linearize_numbers.pop_back();
}