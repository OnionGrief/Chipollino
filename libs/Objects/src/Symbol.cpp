#include <sstream>

#include "Objects/Symbol.h"

using std::cout;
using std::string;
using std::to_string;
using std::vector;

Symbol::Symbol(const string& s) : symbol(s), value(s) {}
Symbol::Symbol(const char* c) : symbol(c), value(c) {}
Symbol::Symbol(char c) : symbol(string(1, c)), value(string(1, c)) {}

Symbol Symbol::Ref(int number) {
	Symbol s;
	s.reference = number;
	s.symbol = to_string(number);
	s.value = '&' + s.symbol;
	return s;
}

void Symbol::update_value() {
	std::stringstream ss;
	if (reference.has_value())
		ss << '&';
	ss << symbol;

	for (const auto& i : annote_numbers)
		ss << annote_marker << i;

	for (const auto& i : linearize_numbers)
		ss << linearize_marker << i;

	value = ss.str();
}

Symbol& Symbol::operator=(const string& s) {
	symbol = s;
	value = symbol;
	return *this;
}

Symbol& Symbol::operator=(const char* c) {
	symbol = c;
	value = symbol;
	return *this;
}

Symbol& Symbol::operator=(char c) {
	symbol = c;
	value = symbol;
	return *this;
}

bool Symbol::is_epsilon() const {
	return *this == Symbol::Epsilon;
}

bool Symbol::operator==(const Symbol& other) const {
	return symbol == other.symbol && annote_numbers == other.annote_numbers &&
		   linearize_numbers == other.linearize_numbers;
}

bool Symbol::operator==(char c) const {
	return symbol.size() == 1 && symbol[0] == c;
}

bool Symbol::operator!=(const Symbol& other) const {
	return !(*this == other);
}

bool Symbol::operator<(const Symbol& other) const {
	return value < other.value;
}

Symbol::operator string() const {
	return value;
}

string Symbol::vector_to_str(const vector<Symbol>& in) {
	string out;
	for (const auto& i : in)
		out += i;
	return out;
}

std::ostream& operator<<(std::ostream& os, const Symbol& as) {
	return os << (string)as;
}

void Symbol::annote(int num) {
	annote_numbers.push_back(num);
	update_value();
}

void Symbol::linearize(int num) {
	linearize_numbers.push_back(num);
	update_value();
}

void Symbol::deannote() {
	if (!annote_numbers.empty()) {
		annote_numbers.pop_back();
		update_value();
	}
}

void Symbol::delinearize() {
	if (!linearize_numbers.empty()) {
		linearize_numbers.pop_back();
		update_value();
	}
}

bool Symbol::is_annotated() const {
	return !annote_numbers.empty();
}

bool Symbol::is_linearized() const {
	return !linearize_numbers.empty();
}

bool Symbol::is_ref() const {
	return reference.has_value();
}

int Symbol::get_ref() const {
	return reference.value();
}

int Symbol::last_linearization_number() const {
	if (!linearize_numbers.empty())
		return linearize_numbers.back();
	else
		return 0;
}

std::size_t Symbol::Hasher::operator()(const Symbol& s) const {
	std::hash<string> string_hash;
	std::size_t hash = string_hash(s.value);
	return hash;
}

Symbol MemorySymbols::Close(int number) {
	return {CloseChar + std::to_string(number)};
}

Symbol MemorySymbols::Reset(int number) {
	return {ResetChar + std::to_string(number)};
}

Symbol MemorySymbols::Open(int number) {
	return {OpenChar + std::to_string(number)};
}

bool MemorySymbols::is_memory_symbol(const Symbol& s) {
	return s.symbol.size() > 1 &&
		   (s.symbol[0] == CloseChar || s.symbol[0] == ResetChar || s.symbol[0] == OpenChar);
}

bool MemorySymbols::is_memory_char(char c) {
	return c == CloseChar || c == ResetChar || c == OpenChar;
}

bool MemorySymbols::is_close(const Symbol& s) {
	return s.symbol.size() > 1 && s.symbol[0] == CloseChar;
}

bool MemorySymbols::is_reset(const Symbol& s) {
	return s.symbol.size() > 1 && s.symbol[0] == ResetChar;
}

bool MemorySymbols::is_open(const Symbol& s) {
	return s.symbol.size() > 1 && s.symbol[0] == OpenChar;
}

int MemorySymbols::get_cell_number(const Symbol& s) {
	string number_str = s.symbol.substr(1);
	if (number_str.empty()) {
		return 0;
	}

	int number = stoi(number_str);
	return number;
}

bool is_special_symbol(const Symbol& s) {
	return s.is_ref() || MemorySymbols::is_memory_symbol(s);
}