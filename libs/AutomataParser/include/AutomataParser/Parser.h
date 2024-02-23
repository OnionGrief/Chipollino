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

    std::string cur_state, cur_label;
    bool init, term, was_labeled;
    std::set<std::string> states;
    std::set<std::string> terminal;
    std::map<std::string, std::string> labels;
    std::string initial;

    std::vector<FAtransition> FAtransitions;
    std::queue<std::string*> nodes;
    std::queue<Symbol*> symbols;

    // state_description --> nodes.push(cur_state); init = false; term = false; was_labeled = false;
    // ||| if (was_labeled) labels[cur_state] = STRING; if (init) initial = cur_state; if (term) terminal.insert(cur_state);
    // label --> was_labeled = true;
    // initial_state --> init = true;
    // terminal --> term = true;


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
        {"state_description", [=]() {
            nodes.push(&cur_state);
            init = false;
            term = false;
            was_labeled = false;

            auto transition = rewriting_rules["state_description"];
            auto res = parse_alternative(*transition);

            if (was_labeled)
                labels[cur_state] = STRING;
            if (init)
                initial = cur_state;
            if (term)
                terminal.insert(cur_state);

            return res;     
        }},
        {"label", [=]() {
            this->was_labeled = true;

            auto transition = rewriting_rules["label"];
            return parse_alternative(*transition);
        }},
        {"initial_state", [=]() {
            init = true;

            auto transition = rewriting_rules["initial_state"];
            return parse_alternative(*transition);
        }},
        {"terminal", [=]() {
            term = true;

            auto transition = rewriting_rules["terminal"];
            return parse_alternative(*transition);
        }},
        {"transitions", [=]() {
            FAtransitions.resize(FAtransitions.size() + 1);
            auto transition = rewriting_rules["transitions"];
            return parse_alternative(*transition);
        }},
        {"stmt", [=]() {
            nodes.push(&FAtransitions.back().beg);
            nodes.push(&FAtransitions.back().end);
            symbols.push(&FAtransitions.back().symbol);
            if(attributes.count("PDA"))
                symbols.push(&FAtransitions.back().pop);
            auto transition = rewriting_rules["stmt"];
            return parse_alternative(*transition);
        }},
        {"node", [=]() {
            auto transition = rewriting_rules["node"];
            auto res = parse_alternative(*transition);
            if(!nodes.empty()) {
                *nodes.front() = STRING;
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
            auto res = parse_alternative(*transition);
            if(!symbols.empty()) {
                if (DIGIT != 0) {
                    std::string r;
                    r.push_back(DIGIT);
                    *symbols.front() = r;
                }
                if (LETTER != 0) {
                    std::string r;
                    r.push_back(LETTER);
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
        {"memory_cell", [=]() {
            auto transition = rewriting_rules["memory_cell"];
            auto res = parse_alternative(*transition);
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
            auto res = parse_alternative(*transition);

            std::string SYMBOL;

            if (TERMINAL == "")
                SYMBOL = STRING;
            else
                SYMBOL = TERMINAL;

            if(!symbols.empty()) {
                *symbols.front() = SYMBOL;
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
	static std::string first_child(lexy_ascii_child::children_range::iterator it);

	// лексема первого потомка для вершины lexy
	static std::string first_child(lexy_ascii_child it);

    std::map<std::string, lexy_ascii_child::children_range::iterator> rewriting_rules;
    std::set<std::string> attributes;

    std::string file;
    int cur_pos;
    void read_symbols(int num);

    void parse_attribute(lexy_ascii_child ref);

    bool parse_transition(std::string name);

    bool parse_nonterminal(lexy_ascii_child ref);
    
    bool parse_reserved(std::string res_case);

    bool parse_terminal(lexy_ascii_child ref);

    bool parse_alternative(lexy_ascii_child ref);

    void grammar_parser(std::string grammar_file, lexy_ascii_tree& tree);

  public:
    Parser() {};

	std::variant<FiniteAutomaton, MemoryFiniteAutomaton> parse(lexy_ascii_tree& grammar, std::string filename);

    FiniteAutomaton parse_NFA(std::string grammar_file, std::string automaton_file);

    FiniteAutomaton parse_DFA(std::string grammar_file, std::string automaton_file);

    MemoryFiniteAutomaton parse_MFA(std::string grammar_file, std::string automaton_file);
};