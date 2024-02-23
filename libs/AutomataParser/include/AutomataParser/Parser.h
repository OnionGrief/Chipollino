#pragma once

#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>
#include <queue>
#include <functional>

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
        std::unordered_set<int> close;
        std::unordered_set<int> open;

        // PDA fields
        Symbol pop;
        std::vector<Symbol> push;
    };

    std::vector<FAtransition> FAtransitions;
    std::queue<std::string&> nodes;
    std::queue<Symbol&> symbols;

    // transition --> FAtransitions.resize(FAtransitions.size() + 1);
    // stmt --> nodes.push(FAtransitions.back().beg); nodes.push(FAtransitions.back().end);
    // symbols.push(FAtransitions.back().symbol); if(PDA) symbols.push(pop);
    // node --> if(!nodes.empty()) { nodes.front() = STRING; nodes.pop(); }
    // symbol --> if(!symbols.empty()) { symbols.front() = SYMBOL; symbols.pop(); }
    // memory_cell --> if(memory_state == 'o') { FAtransitions.back().open.insert(cell_id); }
    // else { FAtransitions.back().close.insert(cell_id); }
    // stack_symbol --> if(!symbols.empty()) { symbols.front() = SYMBOL; symbols.pop(); }
    // else { FAtransitions.back().push.push_back(SYMBOL); }

    std::string STRING;
    int NUMBER;
    std::string TERMINAL;
    char DIGIT;
    char LETTER;

    std::map<std::string, std::function<bool()>> parse_func = {
        {"transitions", [=]() {
            FAtransitions.resize(FAtransitions.size() + 1);
            auto transition = rewriting_rules["transitions"];
            return parse_alternative(transition);
        }},
        {"stmt", [=]() {
            nodes.push(FAtransitions.back().beg);
            nodes.push(FAtransitions.back().end);
            symbols.push(FAtransitions.back().symbol);
            if(attributes.count("PDA"))
                symbols.push(FAtransitions.back().pop);
            auto transition = rewriting_rules["stmt"];
            return parse_alternative(transition);
        }},
        {"node", [=]() {
            auto transition = rewriting_rules["node"];
            auto res = parse_alternative(transition);
            if(!nodes.empty()) {
                nodes.front() = STRING;
                nodes.pop();
            }
            return res;
        }},
        {"symbol", [=]() {
            DIGIT = 0;
            LETTER = 0;
            TERMINAL = "";
            NUMBER = -1;
            auto transition = rewriting_rules["symbol"];
            auto res = parse_alternative(transition);
            if(!symbols.empty()) {
                if (DIGIT != 0) {
                    std::string r;
                    r.push_back(DIGIT);
                    symbols.front() = r;
                }
                if (LETTER != 0) {
                    std::string r;
                    r.push_back(LETTER);
                    symbols.front() = r;
                }
                if (TERMINAL == "eps") {
                    symbols.front() = Symbol::Epsilon;
                }
                if (NUMBER != -1) {
                    symbols.front() = Symbol::Ref(NUMBER);
                }
                symbols.pop();
            }
            return res;
        }},
        {"memory_cell", [=]() {
            auto transition = rewriting_rules["memory_cell"];
            auto res = parse_alternative(transition);
            if(TERMINAL == "o") {
                FAtransitions.back().open.insert(NUMBER);
            } else {
                FAtransitions.back().close.insert(NUMBER);
            }
            return res;
        }},
        {"stack_symbol", [=]() {
            std::string TERMINAL = "";
            auto transition = rewriting_rules["stack_symbol"];
            auto res = parse_alternative(transition);

            std::string SYMBOL;

            if (TERMINAL == "")
                SYMBOL = STRING;
            else
                SYMBOL = TERMINAL;

            if(!symbols.empty()) {
                symbols.front() = SYMBOL;
                symbols.pop();
            } else {
                FAtransitions.back().push.push_back(SYMBOL);
            }
            return res;
        }},
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