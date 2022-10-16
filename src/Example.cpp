#include "Example.h"

void Example::regex_parsing() {
	string reg = "((((a*c)))|(bd|q))";
	Regex r;
	r.from_string(reg);
	r.pre_order_travers();
	r.clear();
}
