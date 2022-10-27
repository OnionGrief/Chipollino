#include "Example.h"
#include <Regex.h>
#include <iostream>
using namespace std;

int main() {
	cout << "Chipollino :-)\n";
	Language* lang;
	lang = new Language();
	Regex r(lang);
	string str = "(ab)*";
	r.from_string(str);
	r.normalize_regex("./../Rules.txt");
	cout << r.to_txt();
	delete lang;
}