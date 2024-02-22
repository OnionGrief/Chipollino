#pragma once

#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <lexy/callback/string.hpp>
#include <lexy/lexeme.hpp>
#include <variant>
#define lexy_ascii_child lexy::_pt_node<lexy::_bra, void>

#include "Lexer.h"
#include "Objects/FiniteAutomaton.h"
#include "Objects/MemoryFiniteAutomaton.h"


class Parser {
  private:
    struct FAtransition {
        std::string beg;
        std::string end;

        Symbol symbol;

        // MFA fields
        unordered_set<int> close;
        unordered_set<int> open;

        // PDA fields
        Symbol pop;
        std::vector<Symbol> push;
    };

	// Поиск рекурсивный поиск вершин с названиями из names, игнорируя спуск в вершины из exclude
	static std::vector<lexy_ascii_child> find_children(
		lexy_ascii_tree& tree, // NOLINT(runtime/references)
		const std::set<std::string>& names, const std::set<std::string>& exclude = {});

	// лексема первого потомка для вершины lexy
	static std::string first_child(lexy::_pt_node<lexy::_bra, void>::children_range::iterator it);

	// лексема первого потомка для вершины lexy
	static std::string first_child(lexy::_pt_node<lexy::_bra, void> it);

    std::map<std::string, lexy::_pt_node<lexy::_bra, void>> rewriting_rules;
    std::set<std::string> attributes;

    std::string file;
    int cur_pos;
    void read_symbols(int num);

    void parse_attribute(lexy::_pt_node<lexy::_bra, void> ref);

    bool parse_transition(std::string name);

    bool parse_nonterminal(lexy::_pt_node<lexy::_bra, void> ref);
    
    bool parse_reserved(std::string res_case);

    bool parse_terminal(lexy::_pt_node<lexy::_bra, void> ref);

    bool parse_alternative(lexy::_pt_node<lexy::_bra, void> ref);

  public:
	bool parse(lexy_ascii_tree& grammar, std::string filename);
};