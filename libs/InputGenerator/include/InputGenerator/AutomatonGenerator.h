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

class AutomatonGeneratorBuilder {
public:
    int initial_ = 0;
    int states_number_ = 10;
    int max_edges_number_ = 0;
    int edges_number_ = 0;
    int colors_ = 4;
    int colors_tries_ = 10;
    int terminal_probability_ = 20;
    int epsilon_probability_ = 10;
    int ref_probability_ = 50;
    int seed_it_ = 0;
    int memory_cells_number_ = 0;
    std::vector<char> alphabet_;

    AutomatonGeneratorBuilder& setInitial(int initial) {
        initial_ = initial;
        return *this;
    }

    AutomatonGeneratorBuilder& setStatesNumber(int states_number) {
        states_number_ = states_number;
        return *this;
    }

    AutomatonGeneratorBuilder& setMaxEdgesNumber(int max_edges_number) {
        max_edges_number_ = max_edges_number;
        return *this;
    }

    AutomatonGeneratorBuilder& setEdgesNumber(int edges_number) {
        edges_number_ = edges_number;
        return *this;
    }

    AutomatonGeneratorBuilder& setColors(int colors) {
        colors_ = colors;
        return *this;
    }

    AutomatonGeneratorBuilder& setColorsTries(int colors_tries) {
        colors_tries_ = colors_tries;
        return *this;
    }

    AutomatonGeneratorBuilder& setTerminalProbability(int terminal_probability) {
        terminal_probability_ = terminal_probability;
        return *this;
    }

    AutomatonGeneratorBuilder& setEpsilonProbability(int epsilon_probability) {
        epsilon_probability_ = epsilon_probability;
        return *this;
    }

    AutomatonGeneratorBuilder& setRefProbability(int ref_probability) {
        ref_probability_ = ref_probability;
        return *this;
    }

    AutomatonGeneratorBuilder& setSeedIt(int seed_it) {
        seed_it_ = seed_it;
        return *this;
    }

    AutomatonGeneratorBuilder& setMemoryCellsNumber(int memory_cells_number) {
        memory_cells_number_ = memory_cells_number;
        return *this;
    }

    AutomatonGeneratorBuilder& setAlphabet(const std::vector<char>& alphabet) {
        alphabet_ = alphabet;
        return *this;
    }

    AutomatonGenerator build() {
        return AutomatonGenerator(*this);
    }
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

	bool coloring_MFA_transition(int beg, FAtransition& trans, // NOLINT(runtime/references)
								 int color);

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

    AutomatonGenerator(const AutomatonGeneratorBuilder& builder)
        : initial(builder.initial_),
          states_number(builder.states_number_),
          max_edges_number(builder.max_edges_number_),
          edges_number(builder.edges_number_),
          colors(builder.colors_),
          colors_tries(builder.colors_tries_),
          terminal_probability(builder.terminal_probability_),
          epsilon_probability(builder.epsilon_probability_),
          ref_probability(builder.ref_probability_),
          seed_it(builder.seed_it_),
          memory_cells_number(builder.memory_cells_number_),
          alphabet(builder.alphabet_) {}

	void write_to_file(const std::string& filename);

    int getInitial() const {
        return initial;
    }

    int getStatesNumber() const {
        return states_number;
    }

    int getMaxEdgesNumber() const {
        return max_edges_number;
    }

    int getEdgesNumber() const {
        return edges_number;
    }

    int getColors() const {
        return colors;
    }

    int getColorsTries() const {
        return colors_tries;
    }

    int getTerminalProbability() const {
        return terminal_probability;
    }

    int getEpsilonProbability() const {
        return epsilon_probability;
    }

    int getRefProbability() const {
        return ref_probability;
    }

    int getSeedIt() const {
        return seed_it;
    }

    int getMemoryCellsNumber() const {
        return memory_cells_number;
    }

    const std::vector<char>& getAlphabet() const {
        return alphabet;
    }
};