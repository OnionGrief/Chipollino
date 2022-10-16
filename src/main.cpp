#include <iostream>
#include "Regex.h"
using namespace std;

int main()
{
	cout << "Chipollino :-)\n";
	string reg = "(a|b)(a*|ba*|b*)*";
	Regex r(reg);
	r.pre_order_travers();
	// r.clear();
	cout << "\n";
	FiniteAutomat a;
	FiniteAutomat b;
	// a = r.to_tompson(-1);
	// cout << a.to_txt();
	a = r.to_glushkov();

	cout << a.to_txt();
	cout << "------------------------------\n";
	b = r.to_ilieyu();

	cout << b.to_txt();
}