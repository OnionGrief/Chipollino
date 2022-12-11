#include "Objects/AlphabetSymbol.h"

alphabet_symbol::alphabet_symbol(){};
alphabet_symbol::alphabet_symbol(const string& s) : value(s){};
alphabet_symbol::alphabet_symbol(const char* c) : value(c){};
alphabet_symbol::alphabet_symbol(char c) : value(string(1, c)){};

alphabet_symbol::alphabet_symbol(const alphabet_symbol& other) {
	value = other.value;
};

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
	return *this;
}

alphabet_symbol alphabet_symbol::epsilon() {
	return "eps";
}

bool alphabet_symbol::operator==(const alphabet_symbol& other) const {
	return value == other.value;
}

bool alphabet_symbol::operator!=(const alphabet_symbol& other) const {
	return value != other.value;
}

bool alphabet_symbol::operator<(const alphabet_symbol& other) const {
	return value < other.value;
}

alphabet_symbol alphabet_symbol::operator+(const alphabet_symbol& b) const {
	return value + b.value;
}

bool alphabet_symbol::is_epsilon() const {
	return *this == alphabet_symbol::epsilon();
}

alphabet_symbol::operator string() const {
	return value;
}

alphabet_symbol alphabet_symbol::remove_numbers() {
	string str_without_numbers;
	for (auto c : value) {
		if (c >= '0' && c <= '9') break;
		str_without_numbers += c;
	}
	return str_without_numbers;
}

int alphabet_symbol::size() const {
	return value.size();
}

ostream& operator<<(ostream& os, const alphabet_symbol& as) {
	return os << as.value;
}