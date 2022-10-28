#include "AlphabetSymbol.h"

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