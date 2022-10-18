#include "Example.h"
#include "Language.h"
#include <iostream>
using namespace std;

int main() {
	cout << "Chipollino :-)\n";

	Language l({"a", "b", "c"});
	cout << l.get_alphabet_size() << endl;
	const optional<int>& a = l.get_pump_length();
	l.set_pump_length(5);
	if (a) cout << *a;
}