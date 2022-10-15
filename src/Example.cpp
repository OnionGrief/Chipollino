#include "Example.h"

void Example::regex_parsing() {
	string reg = "((((a*c)))|(bd|q))";
	Regex r(reg);
	r.pre_order_travers();
	r.clear();
}

void Example::regex_generating() {
	RegexGenerator r1(8, 0, 0, 3); 
	RegexGenerator r3(20, 2, 5, 3);
	RegexGenerator r4(20, 0, 2, 3);
	RegexGenerator r5(20, 100, 3, 3);
	RegexGenerator r6(20, 100, 3, 2);
	RegexGenerator r7(20, 100, 3, 2);
	RegexGenerator r02(35, 7, 2);
}
