#include <cstring>
#include <sstream>

#include "Objects/Symbol.h"

using std::cout;
using std::optional;
using std::string;
using std::to_string;
using std::vector;

void Symbol::initialize(const string& s) {
	if (s.empty())
		throw std::invalid_argument(SymbolErrors::EmptyString);

	bool prefix_found = false;
	std::size_t pos = std::string::npos;

	vector<const char*> prefixes = {Epsilon, EmptySet};
	for (const auto& prefix : prefixes) {
		if (s.substr(0, std::strlen(prefix)) == prefix) {
			symbol = prefix;
			pos = std::strlen(prefix);
			prefix_found = true;
			break;
		}
	}

	if (!prefix_found) {
		if (auto memory_string = MemorySymbols::is_memory_string(s); memory_string.has_value()) {
			symbol = memory_string.value();
			pos = memory_string->size();
		} else if (std::isalpha(s[0])) {
			symbol = s[0];
			pos = 1;
		} else {
			throw std::invalid_argument(SymbolErrors::InvalidFormat);
		}
	}

	parse_markup(s, pos);

	value = s;
}

void Symbol::parse_markup(const string& s, size_t pos) {
	auto parse_numbers = [&](char marker, std::vector<int>& numbers) {
		while (pos < s.size() && s[pos] == marker) {
			++pos;
			if (pos >= s.size() || !std::isdigit(s[pos])) {
				throw std::invalid_argument(SymbolErrors::ExpectedNumber);
			}

			int number = 0;
			while (pos < s.size() && std::isdigit(s[pos])) {
				number = number * 10 + (s[pos] - '0');
				++pos;
			}
			numbers.push_back(number);
		}
	};

	parse_numbers(linearize_marker, linearize_numbers);
	parse_numbers(annote_marker, annote_numbers);

	if (pos < s.size())
		throw std::invalid_argument(SymbolErrors::UnexpectedCharacters);
}

Symbol::Symbol(const string& s) {
	initialize(s);
}

Symbol::Symbol(const char* c) {
	if (c == nullptr) {
		throw std::invalid_argument(SymbolErrors::NullString);
	}
	initialize(c);
}

Symbol::Symbol(char c) : symbol(1, c), value(1, c) {}

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
	initialize(s);
	return *this;
}

Symbol& Symbol::operator=(const char* c) {
	if (c == nullptr) {
		throw std::invalid_argument(SymbolErrors::NullString);
	}
	initialize(c);
	return *this;
}

Symbol& Symbol::operator=(char c) {
	symbol = c;
	value = symbol;
	return *this;
}

bool Symbol::empty() const {
	return symbol == "" && annote_numbers.empty() && linearize_numbers.empty() &&
		   !reference.has_value();
}

bool Symbol::operator==(const Symbol& other) const {
	return symbol == other.symbol && annote_numbers == other.annote_numbers &&
		   linearize_numbers == other.linearize_numbers && reference == other.reference;
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

bool Symbol::is_epsilon() const {
	return *this == Symbol::Epsilon;
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

optional<string> MemorySymbols::is_memory_string(const string& s) {
	if (!(s.size() > 1 && (s[0] == CloseChar || s[0] == ResetChar || s[0] == OpenChar)))
		return std::nullopt;

	size_t pos = 1;
	if (pos >= s.size() || !std::isdigit(s[pos]))
		return std::nullopt;

	string memory_string(1, s[0]);
	while (pos < s.length() && std::isdigit(s[pos])) {
		memory_string += s[pos];
		++pos;
	}

	return memory_string;
}

bool MemorySymbols::is_memory_symbol(const Symbol& s) {
	return MemorySymbols::is_memory_string(s.symbol).has_value();
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
	size_t pos = 1;
	if (pos >= s.symbol.size() || !std::isdigit(s.symbol[pos]))
		return 0;

	int number = 0;
	while (pos < s.symbol.size() && std::isdigit(s.symbol[pos])) {
		number = number * 10 + (s.symbol[pos] - '0');
		++pos;
	}

	return number;
}

bool is_special_symbol(const Symbol& s) {
	return s.is_ref() || MemorySymbols::is_memory_symbol(s);
}