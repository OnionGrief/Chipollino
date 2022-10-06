#include <iostream>
#include <Regex.h>
using namespace std;

int main() {
	cout << "Chipollino :-)\n";
	string reg = "((((a*c)))|(bd|q))";
	Regex r(reg);
	r.preOrderTravers(&r);
	r.clear();
}