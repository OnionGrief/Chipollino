#pragma once

#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "AutomataParser/Lexer.h"
#include "AutomataParser/Parser.h"

enum class FA_type {
	MFA,
	NFA,
	DFA
};

class AutomatonGenerator {
  private:
	struct FAtransition {
		int end;

		Symbol symbol;

		// MFA fields
		std::unordered_set<int> close;
		std::unordered_set<int> open;

		// PDA fields
		Symbol pop;
		std::vector<Symbol> push;
	};

	struct StateDescription {
		int index;

		bool terminal;
		bool initial;

		StateDescription(int index, bool terminal, bool initial)
			: index(index), terminal(terminal), initial(initial) {}
	};

	int initial = 0, states_number = 10;
	int max_edges_number, edges_number;
	// количество ячеек, количество попыток создать открытие ячейки
	int colors = 4, colors_tries = 10;
	int terminal_probability = 20;
	int epsilon_probability = 10;
	int ref_probability = 50;

	int seed_it, memory_cells_number;
	std::vector<char> alphabet;

	void change_seed();

	// возвращает true с вероятностью percentage%
	bool dice_throwing(int percentage);

	void generate_symbol(int beg, FAtransition& trans); // NOLINT(runtime/references)

	std::vector<StateDescription> stateDescriptions;
	std::vector<std::vector<FAtransition>> graph;
	std::vector<std::vector<int>> regraph;

	std::map<int, bool> finality_coloring;
	std::map<int, std::map<int, int>> MFA_coloring;

	void add_terminality();

	bool coloring_MFA_transition(int beg, FAtransition& trans,
								 int color); // NOLINT(runtime/references)

	void generate_graph();

	std::queue<std::string> TERMINAL, STRING, NUMBER, LETTER, DIGIT, STACK_SYMBOLS;
	int cur_state = 0, cur_transition = 0;

	std::map<std::string, lexy_ascii_child::children_range::iterator> rewriting_rules;
	std::set<std::string> attributes;

	std::map<std::string, std::function<bool()>> parse_func = {
		// transition --> stmt (MFA? memory_lists : EPS) (PDA? stack_actions : EPS);
		{"atribute",
		 [=, this]() {
			 auto transition = rewriting_rules["atribute"];
			 auto res = parse_alternative(*transition);
			 if (res) {
				 output << "\n";
			 }
			 return res;
		 }},
		{"states",
		 [=, this]() {
			 for (auto& stateDescription : stateDescriptions) {
				 if (stateDescription.initial || stateDescription.terminal) {
					 STRING.push(std::to_string(stateDescription.index));
					 if (stateDescription.terminal)
						 TERMINAL.emplace("terminal");
					 if (stateDescription.initial)
						 TERMINAL.emplace("initial_state");
					 TERMINAL.emplace(";");
				 }
			 }
			 TERMINAL.emplace("...");
			 auto transition = rewriting_rules["states"];
			 auto res = parse_alternative(*transition);
			 if (res) {
				 output << "\n";
			 }
			 return res;
		 }},
		{"state_description",
		 [=, this]() {
			 auto transition = rewriting_rules["state_description"];
			 auto res = parse_alternative(*transition);
			 if (res) {
				 output << "\n";
			 }
			 return res;
		 }},
		{"transition",
		 [=, this]() {
			 while (cur_state < graph.size() && cur_transition >= graph[cur_state].size()) {
				 cur_state++;
				 cur_transition = 0;
			 }
			 if (cur_state < graph.size()) {
				 // stmt
				 STRING.push(std::to_string(cur_state));
				 STRING.push(std::to_string(graph[cur_state][cur_transition].end));
				 if (graph[cur_state][cur_transition].symbol.is_epsilon()) {
					 TERMINAL.emplace("eps");
				 } else {
					 if (graph[cur_state][cur_transition].symbol.is_ref()) {
						 TERMINAL.emplace("&");
						 NUMBER.push(
							 std::to_string(graph[cur_state][cur_transition].symbol.get_ref()));
					 } else {
						 LETTER.push(graph[cur_state][cur_transition].symbol);
					 }
				 }

				 // memory_lists
				 for (auto cell : graph[cur_state][cur_transition].open) {
					 NUMBER.push(std::to_string(cell));
					 TERMINAL.emplace("o");
				 }
				 for (auto cell : graph[cur_state][cur_transition].close) {
					 NUMBER.push(std::to_string(cell));
					 TERMINAL.emplace("c");
				 }

				 // stack_actions
				 STACK_SYMBOLS.push(graph[cur_state][cur_transition].pop);
				 for (const auto& sym : graph[cur_state][cur_transition].push)
					 STACK_SYMBOLS.push(sym);

				 TERMINAL.emplace(";");

				 cur_transition++;
			 }

			 auto transition = rewriting_rules["transition"];
			 auto res = parse_alternative(*transition);
			 if (res)
				 output << "\n";
			 return res;
		 }},
	};

	void generate_alphabet(int max_alphabet_size);

	void parse_attribute(lexy_ascii_child ref);

	bool parse_transition(const std::string& name);

	bool parse_nonterminal(lexy_ascii_child ref);

	bool parse_reserved(const std::string& res_case);

	bool parse_terminal(lexy_ascii_child ref);

	bool parse_alternative(lexy_ascii_child ref);

  public:
	std::stringstream output;

	AutomatonGenerator(FA_type type = FA_type::NFA, int n = 10,
					   const std::string& grammar_file = GrammarPath);

	void write_to_file(const std::string& filename);

	void set_terminal_probability(int elem);

	void set_states_number(int n);
};