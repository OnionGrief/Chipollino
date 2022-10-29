#include "Tester.h"

void Tester::test(string r1, string r2, int step) {
	vector<word> words;
	for(int i=0; i<12;i++) {
		words.push_back({
			i, i*100, true
		});
	}
	tableRegex table {
		"(a|ab)*",
		"((ab)*a)*",
		2,
		words
	};

	/*for(int i=0; i<12;i++) {
		cout<<words[i].iterations_num<<" "<<words[i].time<<" "<<words[i].is_belongs<<endl;
	}*/

	/* место для вызова логгера */
	//my_loger(tableRegex); а мб можно не так
	//my_loger(r1, r2, step, words); без лишней структуры
}

void Tester::test(FiniteAutomaton r1, string r2, int step) {
	vector<word> words;
	for(int i=0; i<12;i++) {
		words.push_back({
			i, i*100, true
		});
	}
	FiniteAutomaton FA;
	tableFA table {
		FA,
		"((ab)*a)*",
		2,
		words
	};
}