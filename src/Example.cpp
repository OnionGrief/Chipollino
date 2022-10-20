#include "Example.h"

void Example::regex_parsing() {
	string reg = "((a|)*c)";
	Regex r;
	if (!r.from_string(reg)) {
		cout << "ERROR\n";
		return;
	}
	r.pre_order_travers(); // eps в выводе 0
	r.clear();
}
