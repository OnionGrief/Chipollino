#include "Tester.h"

void Tester::test(string lang, string r2, int step) {
	vector<word> words;
	Regex r;
	r.from_string(r2);
	Regex R;
	R.from_string(lang);

	for (int i = 0; i < 13; i++) {
		string w = r.get_iterated_word(i * step);
		cout << w;
		if (w.length() > 700) return;
		int start = clock();

		
		bool is_belongs =
			parsing_by_regex(lang, w); // падает на больших словах((
		int end = clock();
		int time = (end - start) / CLOCKS_PER_SEC;
		cout << is_belongs << endl;
		words.push_back({i * step, time, is_belongs});
		if (time >= 180) return;
	}
	tableRegex table = {lang, r2, step, words};

	/*for (int i = 0; i < words.size(); i++) {
		cout << words[i].iterations_num << " " << words[i].time << " "
			 << words[i].is_belongs << endl;
	}*/

	/* место для вызова логгера */
	// my_loger(tableRegex); а мб можно не так
	// my_loger(lang, r2, step, words); без лишней структуры
}

void Tester::test(FiniteAutomaton lang, string r2, int step) {
	vector<word> words;
	Regex r;
	r.from_string(r2);

	for (int i = 0; i < 13; i++) {
		string w = r.get_iterated_word(i * step);
		// cout << w;
		int start = clock();
		bool is_belongs = lang.parsing_by_nfa(w);
		int end = clock();
		int time = (end - start) / CLOCKS_PER_SEC;
		// cout << is_belongs << endl;
		words.push_back({i * step, time, is_belongs});
		if (time >= 180) return;
	}
	tableFA table = {lang, "((ab)*a)*", 2, words};

	/* место для вызова логгера */
	// my_loger(tableFA); а мб можно не так
	// my_loger(lang, r2, step, words); без лишней структуры
}

bool Tester::parsing_by_regex(string reg, string word) {
	cmatch match_res;
	regex regular(reg);
	if (regex_match(word.c_str(), match_res, regular)) {
		// cout << match_res[0].str();
		return true;
	}
	return false;
}