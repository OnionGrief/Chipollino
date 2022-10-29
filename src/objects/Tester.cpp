#include "Tester.h"

void Tester::test(string lang, string r2, int step) {
	vector<word> words;
	vector<double> times;
	vector<bool> booleans;
	Regex r;
	r.from_string(r2);
	FiniteAutomaton automaton = r.to_tompson();
	Regex R;
	R.from_string(lang);
	using clock = std::chrono::high_resolution_clock;

	for (int i = 0; i < 13; i++) {
		string w = r.get_iterated_word(i * step);
		cout << w;

		const auto start = clock::now();
		bool is_belongs = automaton.parsing_by_nfa(w);
		// parsing_by_regex(lang, w); // падает на больших словах((
		const auto end = clock::now();
		const double elapsed =
			std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
				.count()*1e-7;
		// int time = (int) elapsed;
		// int time = (end - start); // ((clock_t)100);
		cout << " " << elapsed << endl;
		// cout << is_belongs << endl;
		times.push_back(elapsed);
		booleans.push_back(is_belongs);
		if (elapsed >= 180) return;
	}
	/*Logger::init_step("Test");
	Logger::log(lang, r2, step, times, booleans);
	Logger::finish_step();*/
}

void Tester::test(FiniteAutomaton lang, string r2, int step) {
	vector<word> words;
	vector<double> times;
	vector<bool> booleans;
	Regex r;
	r.from_string(r2);
	using clock = std::chrono::high_resolution_clock;

	for (int i = 0; i < 13; i++) {
		string w = r.get_iterated_word(i * step);
		// cout << w <<endl;
		const auto start = clock::now();
		bool is_belongs = lang.parsing_by_nfa(w);
		const auto end = clock::now();
		cout<<std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
		
		const double elapsed =
			std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
				.count()*1e-6;
		times.push_back(elapsed);
		booleans.push_back(is_belongs);
		// int time = (end - start); // CLOCKS_PER_SEC;
		cout << is_belongs << " " << elapsed << endl;
		//words.push_back({i * step, time, is_belongs});
		if (elapsed >= 180) return;
	}

	/* место для вызова логгера */
	// my_loger(tableFA); а мб можно не так
	// my_loger(lang, r2, step, words); без лишней структуры
	Logger::init_step("Test");
	Logger::log(lang, r2, step, times, booleans);
	Logger::finish_step();
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