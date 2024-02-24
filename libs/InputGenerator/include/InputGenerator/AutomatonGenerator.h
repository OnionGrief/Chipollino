#pragma once
#include <string>
#include <sstream>
#include <ctime>
#include <optional>
#include <vector>
#include <fstream>
#include <map>
#include <set>
#include <utility>
#include <iostream>
#include <set>
#include "AutomataParser/Lexer.h"
#include "AutomataParser/Parser.h"

enum class FA_type { MFA, NFA, DFA };

class AutomatonGenerator {
private:
    struct AutomatonGeneratorConstants {
        static inline int terminal_probability = 20;
        static inline int max_memory_cells_number = 10;
        static inline int max_states_number = 20;
        static inline int alphabet_size = 10;
        // макс кол-во переходов = кол-во рёбер в полном графе + additional_max_transitions_number
        static inline int additional_max_transitions_number = 10;
        // вероятность генерации открытия/закрытия для ячейки
        static inline int action_probability = 50;
        // вероятность генерации закрытия для ячейки, при генерации взаимодействия
        static inline int action_closing_probability = 50;
        static inline bool initial_state_not_terminal = false;
    };

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
    };

    int seed_it, memory_cells_number;
    std::vector<char> alphabet;

    void change_seed();

	// возвращает true с вероятностью percentage%
	bool dice_throwing(int percentage);

    Symbol generate_symbol() {}

    std::vector<StateDescription> stateDescriptions;
    std::vector<std::vector<FAtransition>> graph;

    void generate_graph();

    std::stringstream output;

    std::queue<std::string> TERMINAL, STRING, NUMBER, LETTER, DIGIT, STACK_SYMBOLS;
    int cur_state = 0, cur_transition = 0;

    std::map<std::string, lexy_ascii_child::children_range::iterator> rewriting_rules;
    std::set<std::string> attributes;

    std::map<std::string, std::function<bool()>> parse_func = {
        // transition --> stmt (MFA? memory_lists : EPS) (PDA? stack_actions : EPS);
        {"states", [=](){
            for (int i = 0; i < stateDescriptions.size(); i++) {
                if (stateDescriptions[i].initial || stateDescriptions[i].terminal) {
                    STRING.push(std::to_string(stateDescriptions[i].index));
                    if (stateDescriptions[i].initial)
                        TERMINAL.push("initial_state");
                    if (stateDescriptions[i].terminal)
                        TERMINAL.push("terminal");
                    TERMINAL.push(";");
                }
            }
            TERMINAL.push("...");
            auto transition = rewriting_rules["states"];
            auto res = parse_alternative(*transition);
            if (res) {
                std::cout << "\n";
            }
            return res;
        }},
        {"state", [=](){
            auto transition = rewriting_rules["state"];
            auto res = parse_alternative(*transition);
            if (res) {
                std::cout << "\n";
            }
            return res;
        }},
        {"transition", [=](){
            while(cur_state < graph.size() && cur_transition >= graph[cur_state].size()) {
                cur_state++;
                cur_transition = 0;
            }
            if (cur_state < graph.size()) {
                // stmt
                STRING.push(std::to_string(cur_state));
                STRING.push(std::to_string(graph[cur_state][cur_transition].end));
                if (graph[cur_state][cur_transition].symbol.is_epsilon()) {
                    TERMINAL.push("eps");
                } else {
                    if (graph[cur_state][cur_transition].symbol.is_ref()) {
                        TERMINAL.push("&");
                        NUMBER.push(std::to_string(graph[cur_state][cur_transition].symbol.get_ref()));
                    } else {
                        LETTER.push(graph[cur_state][cur_transition].symbol); 
                    }
                }

                // memory_lists
                for (auto cell : graph[cur_state][cur_transition].open) {
                    NUMBER.push(std::to_string(cell));
                    TERMINAL.push("o");
                }
                for (auto cell : graph[cur_state][cur_transition].close) {
                    NUMBER.push(std::to_string(cell));
                    TERMINAL.push("c");
                }

                // stack_actions
                STACK_SYMBOLS.push(graph[cur_state][cur_transition].pop);
                for (auto sym : graph[cur_state][cur_transition].push)
                    STACK_SYMBOLS.push(sym);

                TERMINAL.push(";");

                cur_transition++;
            }

            auto transition = rewriting_rules["transition"];
            auto res = parse_alternative(*transition);
            if (res)
                output << "\n";
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

    void generate_alphabet(int max_alphabet_size);

    void read_symbols(int num);

    void parse_attribute(lexy_ascii_child ref);

    bool parse_transition(std::string name);

    bool parse_nonterminal(lexy_ascii_child ref);
    
    bool parse_reserved(std::string res_case);

    bool parse_terminal(lexy_ascii_child ref);

    bool parse_alternative(lexy_ascii_child ref);

    void grammar_parser(std::string grammar_file, lexy_ascii_tree& tree); // NOLINT(runtime/references)

public:
    explicit AutomatonGenerator(FA_type type = FA_type::NFA);

    void write_to_file(std::string filename);

   static  void set_terminal_probability(int elem);
   static  void set_initial_state_not_terminal(bool f);
};