#include "Example.h"

void Example::regex_parsing() {
	string reg = "((((a*c)))|(bd|q))";
	Regex r(reg);
	r.pre_order_travers();
	r.clear();
}

void Example::regex_generating() {
	RegexGenerator r1({'a', 'b', 'c'}, 8, 0);
}
