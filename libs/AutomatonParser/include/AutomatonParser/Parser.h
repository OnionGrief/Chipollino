#pragma once

#include <functional>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#include <lexy/callback/string.hpp>
#include <lexy/lexeme.hpp>

#define lexy_ascii_child lexy::_pt_node<lexy::_bra, void>

#include "Lexer.h"
#include "Objects/FiniteAutomaton.h"
#include "Objects/MemoryFiniteAutomaton.h"
#include "Objects/Symbol.h"

const char GrammarPath[] = "./config/automaton_parser/grammar.txt";

class Parser {
  private:
	struct FAtransition {
		std::string beg;
		std::string end;

		Symbol symbol;

		// MFA fields
		std::unordered_set<int> close;
		std::unordered_set<int> open;
		std::unordered_set<int> reset;

		// PDA fields
		Symbol pop;
		std::vector<Symbol> push;
	};

	std::string cur_state, cur_label;
	bool init, term, was_labeled;
	std::set<std::string> states;
	std::set<std::string> final_states;
	std::map<std::string, std::string> labels;
	std::string initial;

	std::vector<FAtransition> FAtransitions;
	std::queue<std::string*> nodes;
	std::queue<Symbol*> symbols;

	std::string STRING;
	int NUMBER;
	std::string TERMINAL;
	char DIGIT;
	std::string LETTER;

	std::map<std::string, std::function<bool()>> parse_func = {
		{"state_description",
		 [=, this]() {
			 nodes.push(&cur_state);
			 init = false;
			 term = false;
			 was_labeled = false;

			 auto transition = rewriting_rules["state_description"];
			 auto res = parse_alternative(*transition);

			 if (res) {
				 states.insert(cur_state);
				 if (was_labeled)
					 labels[cur_state] = STRING;
				 if (init)
					 initial = cur_state;
				 if (term)
					 final_states.insert(cur_state);
			 }

			 return res;
		 }},
		{"label",
		 [=, this]() {
			 was_labeled = true;

			 auto transition = rewriting_rules["label"];
			 return parse_alternative(*transition);
		 }},
		{"initial_state",
		 [=, this]() {
			 init = true;

			 auto transition = rewriting_rules["initial_state"];
			 return parse_alternative(*transition);
		 }},
		{"final",
		 [=, this]() {
			 auto transition = rewriting_rules["final"];
			 bool res = parse_alternative(*transition);
			 if (res && TERMINAL == "final")
				 term = true;
			 return res;
		 }},
		{"transition",
		 [=, this]() {
			 FAtransitions.resize(FAtransitions.size() + 1);
			 auto transition = rewriting_rules["transition"];
			 auto res = parse_alternative(*transition);
			 if (!res) {
				 FAtransitions.pop_back();
			 }
			 return res;
		 }},
		{"stmt",
		 [=, this]() {
			 nodes.push(&FAtransitions.back().beg);
			 nodes.push(&FAtransitions.back().end);
			 symbols.push(&FAtransitions.back().symbol);
			 if (attributes.count("PDA"))
				 symbols.push(&FAtransitions.back().pop);
			 auto transition = rewriting_rules["stmt"];
			 return parse_alternative(*transition);
		 }},
		{"node_id",
		 [=, this]() {
			 auto transition = rewriting_rules["node_id"];
			 auto res = parse_alternative(*transition);
			 if (!nodes.empty()) {
				 *nodes.front() = STRING;
				 nodes.pop();
			 }
			 return res;
		 }},
		{"symbol",
		 [=, this]() {
			 DIGIT = 0;
			 LETTER = "";
			 TERMINAL = "";
			 NUMBER = -1;
			 auto transition = rewriting_rules["symbol"];
			 auto res = parse_alternative(*transition);
			 if (!symbols.empty()) {
				 if (DIGIT != 0) {
					 std::string r;
					 r.push_back(DIGIT);
					 *symbols.front() = r;
				 }
				 if (LETTER != "") {
					 std::string r;
					 r += LETTER;
					 *symbols.front() = r;
				 }
				 if (TERMINAL == "eps") {
					 *symbols.front() = Symbol::Epsilon;
				 }
				 if (NUMBER != -1) {
					 *symbols.front() = Symbol::Ref(NUMBER);
				 }
				 symbols.pop();
			 }
			 return res;
		 }},
		{"memory_cell",
		 [=, this]() {
			 TERMINAL = "";
			 auto transition = rewriting_rules["memory_cell"];
			 auto res = parse_alternative(*transition);
			 if (TERMINAL == "o") {
				 FAtransitions.back().open.insert(NUMBER);
			 }
			 if (TERMINAL == "c") {
				 FAtransitions.back().close.insert(NUMBER);
			 }
			 if (TERMINAL == "r") {
				 FAtransitions.back().reset.insert(NUMBER);
			 }
			 return res;
		 }},
		{"stack_symbol",
		 [=, this]() {
			 auto transition = rewriting_rules["stack_symbol"];
			 auto res = parse_alternative(*transition);

			 std::string SYMBOL;

			 if (TERMINAL.empty())
				 SYMBOL = STRING;
			 else
				 SYMBOL = TERMINAL;

			 if (!symbols.empty()) {
				 *symbols.front() = SYMBOL;
				 symbols.pop();
			 } else {
				 FAtransitions.back().push.emplace_back(SYMBOL);
			 }
			 return res;
		 }},
	};

	std::map<std::string, lexy_ascii_child::children_range::iterator> rewriting_rules;
	std::set<std::string> attributes;

	std::string file;
	int cur_pos = 0;

	void read_symbols(int num);

	void parse_attribute(lexy_ascii_child ref);

	bool parse_transition(const std::string& name);

	bool parse_nonterminal(lexy_ascii_child ref);

	bool parse_reserved(const std::string& res_case);

	bool parse_terminal(lexy_ascii_child ref);

	bool parse_alternative(lexy_ascii_child ref);

  public:
	Parser() = default;

	// Поиск рекурсивный поиск вершин с названиями из names,
	// игнорируя спуск в вершины из exclude
	static std::vector<lexy_ascii_child> find_children(
		lexy_ascii_tree& tree, // NOLINT(runtime/references)
		const std::set<std::string>& names, const std::set<std::string>& exclude = {});

	// лексема первого потомка для вершины lexy
	static std::string first_child(lexy_ascii_child::children_range::iterator it);

	// лексема первого потомка для вершины lexy
	static std::string first_child(lexy_ascii_child it);

	std::variant<FiniteAutomaton, MemoryFiniteAutomaton> parse(
		lexy_ascii_tree& grammar, const std::string& filename); // NOLINT(runtime/references)

	FiniteAutomaton parse_NFA(const std::string& automaton_file,
							  const std::string& grammar_file = GrammarPath);

	FiniteAutomaton parse_DFA(const std::string& automaton_file,
							  const std::string& grammar_file = GrammarPath);

	MemoryFiniteAutomaton parse_MFA(const std::string& automaton_file,
									const std::string& grammar_file = GrammarPath);
};