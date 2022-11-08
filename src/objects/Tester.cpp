#include "Tester.h"

void Tester::test(const Regex& lang, const Regex& regex, int step) {
	vector<double> times;
	vector<int> lengths;
	vector<bool> belongs;
	// automaton.determinize();
	FiniteAutomaton automaton = lang.to_glushkov();

	using clock = std::chrono::high_resolution_clock;

	for (int i = 0; i < 13; i++) {
		string word = regex.get_iterated_word(i * step);
		// cout << word;
		const auto start = clock::now();
		bool is_belongs = automaton.parsing_by_nfa(word);
		//  parsing_by_regex(lang, word); // падает на больших словах((
		const auto end = clock::now();
		const long long elapsed =
			std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
				.count();
		double time = (double)elapsed / 1000;
		// cout << is_belongs << " " << time << endl;
		times.push_back(time);
		lengths.push_back(word.length());
		belongs.push_back(is_belongs);
		if (time >= 180) return;
	}
	Logger::init_step("Test");
	Logger::log("Язык, основанный на регулярке", lang.to_txt());
	Logger::log("Слова порождаются регуляркой", regex.to_txt());
	Logger::log("Шаг итерации", to_string(step));
	Logger::log(step, lengths, times, belongs);
	Logger::finish_step();
}

void Tester::test(const FiniteAutomaton& lang, const Regex& regex, int step) {
	vector<double> times;
	vector<int> lengths;
	vector<bool> belongs;
	using clock = std::chrono::high_resolution_clock;

	for (int i = 0; i < 13; i++) {
		string word = regex.get_iterated_word(i * step);
		// cout << word <<endl;
		const auto start = clock::now();
		bool is_belongs = lang.parsing_by_nfa(word);
		const auto end = clock::now();
		const long long elapsed =
			std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
				.count();
		double time = (double)elapsed / 1000;
		times.push_back(time);
		lengths.push_back(word.length());
		belongs.push_back(is_belongs);
		// cout << is_belongs << " " << time << endl;
		if (time >= 180) return;
	}

	Logger::init_step("Test");
	Logger::log("Автомат", lang);
	Logger::log("Слова порождаются регуляркой", regex.to_txt());
	Logger::log("Шаг итерации", to_string(step));
	Logger::log(step, lengths, times, belongs);
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