#include "Tester/Tester.h"

void Tester::test(const Regex& lang, const Regex& regex, int step,
				  iLogTemplate* log) {
	// Logger::init_step("Test");
	// vector<double> times;
	// vector<int> lengths;
	// vector<bool> belongs;
	iLogTemplate::Table t;
	// vector<string> data;
	// vector<string> row;
	FiniteAutomaton automaton = lang.to_glushkov();

	using clock = std::chrono::high_resolution_clock;

	for (int i = 0; i < 13; i++) {
		string word = regex.get_iterated_word(i * step);
		// cout << word;
		const auto start = clock::now();
		bool is_belongs = automaton.parsing_by_nfa(word);
		const auto end = clock::now();
		const long long elapsed =
			std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
				.count();
		double time = (double)elapsed / 1000;
		// cout << is_belongs << " " << time << endl;
		// times.push_back(time);
		// lengths.push_back(word.length());
		// belongs.push_back(is_belongs);
		t.rows.push_back(to_string(i + 1));
		t.data.push_back(to_string(step));
		t.data.push_back(to_string(word.length()));
		t.data.push_back(to_string(time));
		if (is_belongs)
			t.data.push_back("true");
		else
			t.data.push_back("false");

		if (time >= 180) break;
	}

	// vector<string> col;
	t.columns.push_back("Количество итераций");
	t.columns.push_back("Длина строки");
	t.columns.push_back("Время парсинга");
	t.columns.push_back("Принадлежность языку");
	if (log) {
		log->set_parameter("language", lang);
		log->set_parameter("regex", regex);
		log->set_parameter("step", step);
		log->set_parameter("table", t);
	}
	// Logger::log("Язык, основанный на регулярке", lang.to_txt());
	// Logger::log("Слова порождаются регуляркой", regex.to_txt());
	// Logger::log("Шаг итерации", to_string(step));
	// Logger::log(step, lengths, times, belongs);
	// Logger::finish_step();
}

void Tester::test(const FiniteAutomaton& lang, const Regex& regex, int step,
				  iLogTemplate* log) {
	// Logger::init_step("Test");
	// vector<double> times;
	// vector<int> lengths;
	// vector<bool> belongs;
	iLogTemplate::Table t;
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
		// times.push_back(time);
		// lengths.push_back(word.length());
		// belongs.push_back(is_belongs);
		t.rows.push_back(to_string(i + 1));
		t.data.push_back(to_string(step));
		t.data.push_back(to_string(word.length()));
		t.data.push_back(to_string(time));
		if (is_belongs)
			t.data.push_back("true");
		else
			t.data.push_back("false");
		// cout << is_belongs << " " << time << endl;
		if (time >= 180) return;
	}

	t.columns.push_back("Количество итераций");
	t.columns.push_back("Длина строки");
	t.columns.push_back("Время парсинга");
	t.columns.push_back("Принадлежность языку");
	if (log) {
		log->set_parameter("language", lang);
		log->set_parameter("regex", regex);
		log->set_parameter("step", step);
		log->set_parameter("table", t);
	}
	// Logger::log("Автомат", lang);
	// Logger::log("Слова порождаются регуляркой", regex.to_txt());
	// Logger::log("Шаг итерации", to_string(step));
	// Logger::log(step, lengths, times, belongs);
	// Logger::finish_step();
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

/*
Продам шары для ночного катания по полу.
Б/у. Работают без нареканий.
Причина продажи: переезд на первый этаж.
*/