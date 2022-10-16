#include "Example.h"

void Example::regex_parsing() {
	string reg = "((a|b)*c)";
	Regex r;
	if (r.from_string(reg)) {
		cout << "ERROR\n";
		return;
	}
	r.pre_order_travers();
	r.clear();
}

void Example::regex_generating() {
	RegexGenerator r1(8, 0, 0, 3);
	RegexGenerator r3(20, 2, 5, 3);
	RegexGenerator r31(20, 2, 5, 3);
	RegexGenerator r33(20, 2, 5, 3);
	RegexGenerator r34(20, 2, 5, 3);
	RegexGenerator r32(20, 2, 5, 3);
	RegexGenerator r4(20, 0, 2, 3);
	RegexGenerator r5(20, 100, 3, 3);
	RegexGenerator r6(20, 100, 3, 2);
	r6.to_txt();
	RegexGenerator r7(20, 100, 3, 2);
	RegexGenerator r02(35, 7, 2);
}

void Example::random_regex_parsing() {
	for (int i = 0; i < 5; i++) {
		RegexGenerator r6(8, 10, 3, 2);
		string str = r6.to_txt();
		cout <<"\n"<< str << "\n";
		Regex r;
		r.from_string(str);
		r.pre_order_travers();
		r.clear();
	}
}
