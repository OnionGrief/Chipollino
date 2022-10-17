#include "Example.h"

void Example::regex_parsing() {
	string reg = "a|";
	Regex r;
	if (!r.from_string(reg)) {
		cout << "ERROR\n";
		return;
	}
	// r.from_string(reg);
	cout << r.to_txt();
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
