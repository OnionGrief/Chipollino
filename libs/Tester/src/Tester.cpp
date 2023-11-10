#include "Tester/Tester.h"

void Tester::test(const ParseDevice& lang, const Regex& regex, int step, iLogTemplate* log) {
	iLogTemplate::Table t;
	iLogTemplate::Plot plot;
	vector<long> steps;
	vector<long> words;
	vector<string> labels;
	vector<FiniteAutomaton> machines;
	/* A counter for parsing objects */
	int obj_types;
	if (holds_alternative<Regex>(lang)) {
		obj_types = 4;
		Regex value = get<Regex>(lang);
		machines.push_back(value.to_thompson());
		labels.push_back("Thompson");
		machines.push_back(value.to_glushkov());
		labels.push_back("Glushkov");
		machines.push_back(
			machines[1].reverse().merge_bisimilar().reverse().merge_bisimilar().remove_eps());
		labels.push_back("Small NFA");
		machines.push_back(machines[2].minimize().remove_trap_states());
		labels.push_back("Minimal DFA");
	} else if (holds_alternative<FiniteAutomaton>(lang)) {
		obj_types = 2;
		FiniteAutomaton value = get<FiniteAutomaton>(lang);
		machines.push_back(value);
		labels.push_back("Current FA");
		machines.push_back(value.minimize().remove_trap_states());
		labels.push_back("Minimal DFA");
	}

	using clock = std::chrono::high_resolution_clock;

	for (int type = 0; type < obj_types; type++) {
		steps.clear();
		for (int i = 0; i < 13; i++) {
			string word = regex.get_iterated_word(i * step);
			// cout << word;
			const auto start = clock::now();
			/*	if (holds_alternative<FiniteAutomaton&>(lang))     */
			auto belongs = machines[type].parsing_by_nfa(word);
			const auto end = clock::now();
			const long long elapsed =
				std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
			double time = (double)elapsed / 1000;
			steps.push_back(belongs.first);
			words.push_back(word.length());
			if (!type) {
				t.rows.push_back(to_string(i + 1));
				t.data.push_back(to_string(belongs.first));
				t.data.push_back(to_string(word.length()));
				t.data.push_back(to_string(time));
				if (belongs.second)
					t.data.push_back("true");
				else
					t.data.push_back("false");
			}

			if (time >= 180) break;
		}
		for (int i = 0; i < steps.size(); i++) {
			plot.data.push_back({labels[type], words[i], steps[i]});
		}
	}
	t.columns.push_back("Шаги");
	t.columns.push_back("Длина строки");
	t.columns.push_back("Время парсинга");
	t.columns.push_back("Принадлежность языку");
	if (log) {
		if (holds_alternative<FiniteAutomaton>(lang)) {
			log->set_parameter("language", get<FiniteAutomaton>(lang));
		} else {
			log->set_parameter("language", get<Regex>(lang));
		}
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