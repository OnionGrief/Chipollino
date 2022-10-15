#include "Example.h"

void Example::regex_parsing() {
	string reg = "((((a*c)))|(bd|q))";
	Regex r(reg);
	r.pre_order_travers();
	r.clear();
}

void Example::regex_generating() {
	RegexGenerator r1({'a', 'b', 'c'}, 8, 0, 0);
	RegexGenerator r2({'a', 'b', 'c'}, 8, 5, 0);
	RegexGenerator r3({'a', 'b', 'c'}, 20, 2, 5);
	RegexGenerator r4({'a', 'b', 'c'}, 20, 0, 2);
	RegexGenerator r5({'a', 'b', 'c'}, 20, 100, 3);
	RegexGenerator r6({'a', 'b'}, 20, 100, 3);
	RegexGenerator r7({'a', 'b'}, 20, 100, 3);
}
