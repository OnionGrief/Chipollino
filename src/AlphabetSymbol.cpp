#include "AlphabetSymbol.h"

alphabet_symbol epsilon() {
	return '\0';
}
bool is_epsilon(alphabet_symbol as) {
	return as == epsilon();
}
string to_string(alphabet_symbol as) {
	if (is_epsilon(as))
		return "eps";
	else
		return string(1, as);
}