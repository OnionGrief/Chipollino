#include "Example.h"
#include <iostream>
using namespace std;

int main() {
	auto regl = "(ab|b)*ba"; //"bbb*(aaa*bbb*)*";
	Language* lang;
	lang = new Language();
	Regex r(lang);
	if (!r.from_string(regl)) {
		cout << "ERROR\n";
		return 0;
	}
	Regex s2(lang);
	if (!s2.from_string("b")) {
		cout << "ERROR\n";
		return 0;
	}
	vector<Regex> t;
	r.partial_symbol_derevative(s2, t); // regex a* symbol a
	for (int i = 0; i < t.size(); i++) {
		cout << t[i].to_txt() << "\n"; // вывод пустой
	}

	delete lang;
	cout << "Chipollino :-)\n";
}