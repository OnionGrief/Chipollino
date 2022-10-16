#include <iostream>
#include <Regex.h>
using namespace std;

int main() {
	cout << "Chipollino :-)\n";
	string reg = "((a|b)*c)";
	Regex r;
	if (r.from_string(reg)) {
		cout << "ERROR\n";
		return 0;
	}
	r.pre_order_travers();
	r.clear();
}