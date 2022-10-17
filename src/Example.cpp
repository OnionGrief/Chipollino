#include "Example.h"

void Example::regex_parsing() {
	string reg = "(a|b)(a*|ba*|b*)*";
	Regex r;
	r.from_string(reg);
	r.pre_order_travers();
	// r.clear();
	cout << "\n";
	FiniteAutomat a;
	FiniteAutomat b;
	FiniteAutomat c;
	cout << "to_tompson ------------------------------\n";
	c = r.to_tompson(-1);
	cout << c.to_txt();
	cout << "to_glushkov ------------------------------\n";
	// a = r.to_tompson(-1);
	// cout << a.to_txt();
	a = r.to_glushkov();

	cout << a.to_txt();
	cout << "to_ilieyu  ------------------------------\n";
	b = r.to_ilieyu();

	cout << b.to_txt();
}
