#include "Tester.h"

void Tester::test(string lang, string r2, int step) {
	vector<double> times;
	vector<bool> belongs;
	Regex r;
	r.from_string(r2);
	// automaton.determinize();
	Regex R;
	R.from_string(lang);
	FiniteAutomaton automaton = R.to_tompson();

	using clock = std::chrono::high_resolution_clock;

	for (int i = 0; i < 13; i++) {
		string w = r.get_iterated_word(i * step);
		// cout << w;
		const auto start = clock::now();
		bool is_belongs = automaton.parsing_by_nfa(w);
		//  parsing_by_regex(lang, w); // падает на больших словах((
		const auto end = clock::now();
		const long long elapsed =
			std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
				.count();
		double time = (double)elapsed / 1000;
		// cout << is_belongs << " " << time << endl;
		times.push_back(time);
		belongs.push_back(is_belongs);
		if (time >= 180) return;
	}
	Logger::init_step("Test");
	Logger::log(lang, r2, step, times, belongs);
	Logger::finish_step();
}

void Tester::test(FiniteAutomaton lang, string r2, int step) {
	vector<double> times;
	vector<bool> belongs;
	Regex r;
	r.from_string(r2);
	using clock = std::chrono::high_resolution_clock;

	for (int i = 0; i < 13; i++) {
		string w = r.get_iterated_word(i * step);
		// cout << w <<endl;
		const auto start = clock::now();
		bool is_belongs = lang.parsing_by_nfa(w);
		const auto end = clock::now();
		const long long elapsed =
			std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
				.count();
		double time = (double)elapsed / 1000;
		times.push_back(time);
		belongs.push_back(is_belongs);
		// cout << is_belongs << " " << time << endl;
		if (time >= 180) return;
	}

	Logger::init_step("Test");
	Logger::log(lang, r2, step, times, belongs);
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