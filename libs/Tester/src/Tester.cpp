#include <chrono>
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <variant>
#include <vector>

#include "Tester/Tester.h"

using std::make_unique;
using std::string;
using std::to_string;
using std::unique_ptr;
using std::vector;

void Tester::test(const ParseDevice& lang, const Regex& regex, int step, iLogTemplate* log) {
	iLogTemplate::Table t;
	iLogTemplate::Plot plot;
	vector<string> labels;
	vector<unique_ptr<AbstractMachine>> machines;
	if (std::holds_alternative<const Regex*>(lang)) {
		auto value = std::get<const Regex*>(lang);
		machines.push_back(make_unique<FiniteAutomaton>(value->to_thompson()));
		labels.emplace_back("Thompson");
		machines.push_back(make_unique<FiniteAutomaton>(value->to_glushkov()));
		labels.emplace_back("Glushkov");
		machines.push_back(
			make_unique<FiniteAutomaton>(dynamic_cast<FiniteAutomaton*>(machines[1].get())
											 ->reverse()
											 .merge_bisimilar()
											 .reverse()
											 .merge_bisimilar()
											 .remove_eps()));
		labels.emplace_back("Small NFA");
		machines.push_back(make_unique<FiniteAutomaton>(
			dynamic_cast<FiniteAutomaton*>(machines[2].get())->minimize().remove_trap_states()));
		labels.emplace_back("Minimal DFA");
	} else if (std::holds_alternative<const FiniteAutomaton*>(lang)) {
		auto value = std::get<const FiniteAutomaton*>(lang);
		machines.push_back(make_unique<FiniteAutomaton>(*value));
		labels.emplace_back("Current FA");
		machines.push_back(make_unique<FiniteAutomaton>(value->minimize().remove_trap_states()));
		labels.emplace_back("Minimal DFA");
	} else if (std::holds_alternative<const BackRefRegex*>(lang)) {
		auto value = std::get<const BackRefRegex*>(lang);
		machines.push_back(make_unique<MemoryFiniteAutomaton>(value->to_mfa()));
		labels.emplace_back("MFA");
		machines.push_back(make_unique<MemoryFiniteAutomaton>(value->to_mfa_additional()));
		labels.emplace_back("Experimental MFA");
	} else if (std::holds_alternative<const MemoryFiniteAutomaton*>(lang)) {
		auto value = std::get<const MemoryFiniteAutomaton*>(lang);
		machines.push_back(make_unique<MemoryFiniteAutomaton>(*value));
		labels.emplace_back("MFA");
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
			auto [count, is_belongs] = machines[type]->parse(word);
			const auto end = clock::now();
			const long long elapsed =
				std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
			double time = (double)elapsed / 1000;
			steps.push_back(count);
			words.push_back(word.length());
			if (!type) {
				t.rows.push_back(to_string(i + 1));
				t.data.push_back(to_string(count));
				t.data.push_back(to_string(word.length()));
				t.data.push_back(to_string(time));
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
		if (std::holds_alternative<const Regex*>(lang)) {
			log->set_parameter("language", *std::get<const Regex*>(lang));
		} else if (std::holds_alternative<const FiniteAutomaton*>(lang)) {
			log->set_parameter("language", *std::get<const FiniteAutomaton*>(lang));
		} else if (std::holds_alternative<const BackRefRegex*>(lang)) {
			log->set_parameter("language", *std::get<const BackRefRegex*>(lang));
		} else if (std::holds_alternative<const MemoryFiniteAutomaton*>(lang)) {
			log->set_parameter("language", *std::get<const MemoryFiniteAutomaton*>(lang));
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