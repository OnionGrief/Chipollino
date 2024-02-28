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
#include <variant>

#include <lexy/callback/string.hpp>
#include <lexy/lexeme.hpp>
#define lexy_ascii_child lexy::_pt_node<lexy::_bra, void>

#include "Lexer.h"
#include "ParseTreeDescend.h"
#include "Objects/FiniteAutomaton.h"
#include "Objects/MemoryFiniteAutomaton.h"

class Parser : protected ParseTreeDescend  {
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

            if (res) {
                states.insert(cur_state);
                if (was_labeled)
                    labels[cur_state] = STRING;
                if (init)
                    initial = cur_state;
                if (term)
                    terminal.insert(cur_state);
            }

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
        {"transition", [=]() {
            FAtransitions.resize(FAtransitions.size() + 1);
            auto transition = rewriting_rules["transition"];
            auto res = parse_alternative(*transition);
            if (!res) {
                FAtransitions.pop_back();
            }
            // std::cout << "TRANSSSSITION: " << res;
            return res;
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
        {"node_id", [=]() {
            auto transition = rewriting_rules["node_id"];
            auto res = parse_alternative(*transition);
            if(!nodes.empty()) {
                // std::cout << "SSSSSSSTRING " << STRING << "\n";
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
                // std::cout << "SSYYYYYMBOLS: " << DIGIT << " " << LETTER << " " << TERMINAL << " " << NUMBER << "\n";
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
            TERMINAL = "";
            auto transition = rewriting_rules["memory_cell"];
            auto res = parse_alternative(*transition);
            if(TERMINAL == "o") {
                FAtransitions.back().open.insert(NUMBER);
            }
            if(TERMINAL == "c"){
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

    std::map<std::string, lexy_ascii_child::children_range::iterator> rewriting_rules;
    std::set<std::string> attributes;

    std::string file;
    int cur_pos = 0;
    void read_symbols(int num);
    
    bool _parse_reserved(std::string res_case);

    bool _parse_terminal(lexy_ascii_child ref);


  public:
    Parser() {
        parse_reserved = [=](std::string x) {
            return _parse_reserved(x);
        };
        parse_terminal = [=](lexy_ascii_child ref) {
            return _parse_terminal(ref);
        };
    }

	std::variant<FiniteAutomaton, MemoryFiniteAutomaton> parse(
        lexy_ascii_tree& grammar, std::string filename); // NOLINT(runtime/references)

    FiniteAutomaton parse_NFA(std::string grammar_file, std::string automaton_file);

    FiniteAutomaton parse_DFA(std::string grammar_file, std::string automaton_file);

    MemoryFiniteAutomaton parse_MFA(std::string grammar_file, std::string automaton_file);
};