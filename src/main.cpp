#include <iostream>
#include "Regex.h"
using namespace std;

int main() {
	cout << "Chipollino :-)\n";
	string reg = "a|b*c";
	Regex r(reg);
	r.pre_order_travers();
	//r.clear();
	cout << "\n";
	FiniteAutomat a;
	a = r.to_tompson(-1);
	cout << a.to_txt();
}