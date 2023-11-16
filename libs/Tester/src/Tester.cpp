#include "Tester/Tester.h"

void Tester::test(const ParseDevice& lang, const Regex& regex, int step, iLogTemplate* log) {
	iLogTemplate::Table t;
	iLogTemplate::Plot plot;
	vector<string> labels;
	vector<FiniteAutomaton> machines;
	if (std::holds_alternative<Regex>(lang)) {
		Regex value = std::get<Regex>(lang);
		machines.push_back(value.to_thompson());
		labels.emplace_back("Thompson");
		machines.push_back(value.to_glushkov());
		labels.emplace_back("Glushkov");
		machines.push_back(
			machines[1].reverse().merge_bisimilar().reverse().merge_bisimilar().remove_eps());
		labels.emplace_back("Small NFA");
		machines.push_back(machines[2].minimize().remove_trap_states());
		labels.emplace_back("Minimal DFA");
	} else if (std::holds_alternative<FiniteAutomaton>(lang)) {
		FiniteAutomaton value = std::get<FiniteAutomaton>(lang);
		machines.push_back(value);
		labels.emplace_back("Current FA");
		machines.push_back(value.minimize().remove_trap_states());
		labels.emplace_back("Minimal DFA");
	}
	/* A counter for parsing objects */
	int obj_types = static_cast<int>(machines.size());

	for (int type = 0; type < obj_types; type++) {
		vector<long> steps;
		vector<int> words;
		for (int i = 0; i < 13; i++) {
			string word = regex.get_iterated_word(i * step);
			using clock = std::chrono::high_resolution_clock;
			const auto start = clock::now();
			auto [count, is_belongs] = machines[type].parsing_by_nfa(word);
			const auto end = clock::now();
			const long long elapsed =
				std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
			double time = (double)elapsed / 1000;
			steps.push_back(count);
			words.push_back(word.length());
			if (!type) {
				t.rows.push_back(std::to_string(i + 1));
				t.data.push_back(std::to_string(count));
				t.data.push_back(std::to_string(word.length()));
				t.data.push_back(std::to_string(time));
				t.data.push_back(is_belongs ? "true" : "false");
			}

			if (time >= 180)
				break;
		}
		for (int i = 0; i < steps.size(); i++) {
			plot.data.push_back({labels[type], words[i], steps[i]});
		}
	}
	t.columns.emplace_back("Шаги");
	t.columns.emplace_back("Длина строки");
	t.columns.emplace_back("Время парсинга");
	t.columns.emplace_back("Принадлежность языку");
	if (log) {
		if (std::holds_alternative<FiniteAutomaton>(lang)) {
			log->set_parameter("language", std::get<FiniteAutomaton>(lang));
		} else {
			log->set_parameter("language", std::get<Regex>(lang));
		}
		log->set_parameter("regex", regex);
		log->set_parameter("step", step);
		log->set_parameter("table", t);
		log->set_parameter("plot", plot);
	}
}

bool Tester::parsing_by_regex(const string& reg, const string& word) {
	std::cmatch match_res;
	std::regex regular(reg);
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