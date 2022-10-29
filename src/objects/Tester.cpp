#include "Tester.h"

void Tester::test(string lang, string r2, int step) {
	vector<word> words;
	vector<int> times;
	vector<bool> booleans;
	Regex r;
	r.from_string(r2);
	FiniteAutomaton automaton = r.to_tompson();
	Regex R;
	R.from_string(lang);

	for (int i = 0; i < 13; i++) {
		string w = r.get_iterated_word(i * step);
		// cout << w;
		int start = clock();
		bool is_belongs = automaton.parsing_by_nfa(w);
		// parsing_by_regex(lang, w); // падает на больших словах((
		int end = clock();
		int time = (end - start); // ((clock_t)100);
		// cout << " " << time << endl;
		// cout << is_belongs << endl;
		times.push_back(time);
		booleans.push_back(is_belongs);
		if (time >= 180) return;
	}
	Logger::init_step("Test");
	Logger::log(lang, r2, step, times, booleans);
	Logger::finish_step();
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
		int time = (end - start); // CLOCKS_PER_SEC;
		// cout << is_belongs << endl;
		words.push_back({i * step, time, is_belongs});
		if (time >= 180) return;
	}

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