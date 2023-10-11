#include "Tester/Tester.h"

void Tester::test(const Regex& lang, const Regex& regex, int step,
				  iLogTemplate* log) {
	iLogTemplate::Table t;
	iLogTemplate::Plot plot;
	vector<long> steps;
	vector<long> words;
	FiniteAutomaton thompson = lang.to_thompson();
	FiniteAutomaton glushkov = lang.to_glushkov();
	FiniteAutomaton smallNfa = glushkov.reverse().merge_bisimilar().reverse().merge_bisimilar().remove_eps();
	FiniteAutomaton mindfa = smallNfa.minimize().remove_trap_states();
        bool belongs;
	string id; 

	using clock = std::chrono::high_resolution_clock;

	for (int type = 0; type < 4; type++) {
	steps.clear();
	for (int i = 0; i < 13; i++) {
		string word = regex.get_iterated_word(i * step);
		// cout << word;
		const auto start = clock::now();
		int counter;
		switch (type) {
		 case 0: belongs = thompson.parsing_by_nfa(word, counter); id = "Thompson"; break;
		 case 1: belongs = glushkov.parsing_by_nfa(word, counter); id = "Glushkov"; break;
		 case 2: belongs = smallNfa.parsing_by_nfa(word, counter); id = "smallNFA"; break;
		 default : belongs = mindfa.parsing_by_nfa(word, counter); id = "minDFA"; }
		const auto end = clock::now();
		const long long elapsed =
			std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
				.count();
		double time = (double)elapsed / 1000;
		steps.push_back(counter);
		words.push_back(word.length()); 
		if (!type) {t.rows.push_back(to_string(i + 1));
		t.data.push_back(to_string(counter));
		t.data.push_back(to_string(word.length()));
		t.data.push_back(to_string(time));
		 if (belongs)
			t.data.push_back("true");
		 else
			t.data.push_back("false");
		}

		if (time >= 180) break;
	}
	for (int i = 0; i < steps.size(); i++)
		{
			plot.data.push_back(make_pair(make_pair(words[i], int(steps[i])),id));
		}

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
		log->set_parameter("plot", plot);
	}
}

void Tester::test(const FiniteAutomaton& lang, const Regex& regex, int step,
				  iLogTemplate* log) {
	vector<long> words, steps;
	FiniteAutomaton mindfa = lang.minimize().remove_trap_states();
	iLogTemplate::Table t;
	iLogTemplate::Plot plot;
	using clock = std::chrono::high_resolution_clock;
	string id;
	int counter;
	bool belongs;

	for (int type = 0; type < 2; type ++)  {
		steps.clear();
		if (!type) {
			id = "NFA";
			} 
                else
			{ id = "minDFA";} 
		for (int i = 0; i < 13; i++) {
			string word = regex.get_iterated_word(i * step);
			const auto start = clock::now();
			if (!type) {
				belongs = lang.parsing_by_nfa(word, counter);
				} else
				{belongs = mindfa.parsing_by_nfa(word, counter);
				} 
			const auto end = clock::now();
			const long long elapsed =
				std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
					.count();
			double time = (double)elapsed / 1000;
			steps.push_back(counter);
			if (!type) {
				t.rows.push_back(to_string(i + 1));
				t.data.push_back(to_string(counter));
				t.data.push_back(to_string(word.length()));
				t.data.push_back(to_string(time));
				words.push_back(word.length());
			  if (belongs)
				t.data.push_back("true");
	     		  else
				t.data.push_back("false"); 
			}
			if (time >= 180) return;
		}
	for (int i = 0; i < steps.size(); i++)
		{
			plot.data.push_back(make_pair(make_pair(words[i], int(steps[i])),id));
		}
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
		log->set_parameter("plot", plot);
	}
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