#include "Objects/AlphabetSymbol.h"

alphabet_symbol epsilon() {
	return "eps";
}

bool is_epsilon(alphabet_symbol as) {
	return as == epsilon();
}

string to_string(alphabet_symbol as) {
	return as;
}

alphabet_symbol char_to_alphabet_symbol(char c) {
	return string(1, c);
}

alphabet_symbol remove_numbers(alphabet_symbol symb) {
	string str_without_numbers;
	for (auto c : to_string(symb)) {
		if (c >= '0' && c <= '9') break;
		str_without_numbers += c;
	}
	return str_without_numbers;
}