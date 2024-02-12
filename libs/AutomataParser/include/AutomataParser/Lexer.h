#pragma once

#include <time.h>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>

#include <lexy/action/parse_as_tree.hpp>
#include <lexy/dsl.hpp>
#include <lexy/input/file.hpp>
#include <lexy/visualize.hpp>
#include <lexy_ext/report_error.hpp>
#define lexy_ascii_tree lexy::parse_tree_for<lexy::buffer<lexy::ascii_encoding>, void, void>

namespace dsl = lexy::dsl;

//=== string constants ===//
namespace AutomataParser {
const char node_id[] = "node_id";
const char statement[] = "stmt";
const char terminal_mark[] = "terminal_mark";
const char initial_mark[] = "initial_mark";
const char state_label[] = "state_label";
const char state_description[] = "state_description";
const char MFA_edge[] = "MFA_edge";
const char cell_id[] = "cell_id";
const char memory_cell[] = "memory_cell";

const char epsilon[] = "eps";
} // namespace AutomataParser

class Lexer {
  private:
    static inline int seed_it = 0;
    
    static void change_seed();

    static bool dice_throwing(int percentage);

	//=== grammar ===//
	struct node_id {
		static constexpr auto rule
			// One or more alphanumeric characters, underscores or hyphens.
			= dsl::identifier(dsl::unicode::alnum / dsl::lit_c<'_'> / dsl::lit_c<'-'>);
	};

	struct cell_id {
		static constexpr auto rule = dsl::digits<>.no_leading_zero();
	};

	struct transition {
		static constexpr auto rule =
			LEXY_LIT("eps") | dsl::unicode::alnum | (LEXY_LIT("&") >> dsl::p<cell_id>);

        static std::string generate(int cells_number) {
            if (dice_throwing(5)) {
                return "eps";
            }
            std::string res = "";
            if (cells_number == 0 || dice_throwing(70)) {
                if (dice_throwing(50)) {
                    res += ('a' + (rand() % 26));
                    return res;
                } else {
                    res += ('A' + (rand() % 26));
                    return res;
                }
            } else {
                res += "&";
                auto x = rand() % cells_number;
                res += std::to_string(x);
            }
        }
	};

	struct stmt {
		static constexpr auto rule = dsl::p<node_id> + dsl::p<node_id> + dsl::p<transition>;

        static std::string generate(int states_number, int cells_number) {
            change_seed();
            auto beg = rand() % states_number;
            change_seed();
            auto end = rand() % states_number;
            return std::to_string(beg) + " " + std::to_string(end) + " " + transition::generate(cells_number);   
        }
	};

	struct memory_state {
		static constexpr auto rule = (LEXY_LIT("o") | LEXY_LIT("c"));
	};

	struct memory_cell {
		static constexpr auto rule = dsl::p<cell_id> >> dsl::p<memory_state>;
	};

	struct memory_lists {
		static constexpr auto rule = dsl::while_(dsl::p<memory_cell>);

        static std::string generate(int cells_number) {
            std::string res = "";
            for (int i = 0; i < cells_number; i++) {
                if (dice_throwing(50)) {
                    if (dice_throwing(50)) {
                        res += " " + std::to_string(i) + " o";
                    } else {
                        res += " " + std::to_string(i) + " c";
                    }
                }
            }

            return res;
        }
	};

	struct MFA_edge {
		static constexpr auto rule = (dsl::p<stmt> + dsl::p<memory_lists>);

        static std::string generate(int states_number, int cells_number) {
            return stmt::generate(states_number, cells_number) + memory_lists::generate(cells_number);
        }
	};

	struct FA_edge {
		static constexpr auto rule = dsl::p<stmt>;

        static std::string generate(int states_number) {
            return stmt::generate(states_number, 0);
        }
	};

	struct state_label {
		static constexpr auto rule = LEXY_LIT("label") >> (LEXY_LIT("=") + dsl::p<node_id>);
	};

	struct terminal_mark {
		static constexpr auto rule = LEXY_LIT("terminal");
	};

	struct initial_mark {
		static constexpr auto rule = LEXY_LIT("initial_state");
	};

	struct state_description {
		static constexpr auto variants = dsl::partial_combination(
			dsl::p<state_label>, dsl::p<terminal_mark>, dsl::p<initial_mark>);

		static constexpr auto rule = dsl::p<node_id> + variants;
	};

	struct states {
		static constexpr auto sep = dsl::sep(dsl::semicolon);

		static constexpr auto rule = dsl::list(dsl::p<state_description>, sep) + LEXY_LIT("...");

        static std::string generate(int states_number) {
            std::string res = "0 initial_state;\n";
            
            for (int i = 1; i < states_number; i++) {
                if (dice_throwing(50)) {
                    res += std::to_string(i) + " terminal;\n";
                }
            }

            return res;
        }
	};

	struct MFA {
		// Allow arbitrary spaces between individual tokens.

		static constexpr auto whitespace = dsl::ascii::space;

		static constexpr auto sep = dsl::sep(dsl::semicolon);

		static constexpr auto rule =
			LEXY_LIT("{") + dsl::p<states> + dsl::list(dsl::p<MFA_edge>, sep) + LEXY_LIT("}");

        static std::string generate() {
            std::string res = "{";

            auto states_number = rand() % 10 + 1;
            auto cells_number = rand() % 10 + 1;


            res += states::generate(states_number);

            auto edges_number = rand() % 30 + 1;

            for (int i = 0; i < edges_number - 1; i++) {
                res += MFA_edge::generate(states_number, cells_number) + ";\n";
            }

            res += MFA_edge::generate(states_number, cells_number) + "\n";

            res += "}";
            return res;
        }
	};

	struct FA {
		// Allow arbitrary spaces between individual tokens.

		static constexpr auto whitespace = dsl::ascii::space;

		static constexpr auto sep = dsl::sep(dsl::semicolon);

		static constexpr auto rule =
			LEXY_LIT("{") + dsl::p<states> + dsl::list(dsl::p<FA_edge>, sep) + LEXY_LIT("}");

        static std::string generate() {
            std::string res = "{";

            auto states_number = rand() % 10 + 1;
            res += states::generate(states_number);

            auto edges_number = rand() % 10 + 1;

            for (int i = 0; i < edges_number - 1; i++) {
                res += FA_edge::generate(states_number) + ";\n";
            }

            res += FA_edge::generate(states_number) + "\n";

            res += "}";
            return res;
        }
	};

	struct production {
		static constexpr auto rule = LEXY_LIT("MFA") >> dsl::p<MFA> | LEXY_LIT("FA") >> dsl::p<FA>;

        static std::string generate(std::string type) {
            if (type == "MFA") {
                return "MFA " + MFA::generate();   
            } else {
                return "FA " + FA::generate();   
            }
        }
	};

  public:
	// Строит дерево разбора по грамматике описанной в private лексера
	static void parse_buffer(
		lexy_ascii_tree& tree,						 // NOLINT(runtime/references)
		lexy::buffer<lexy::ascii_encoding>& buffer); // NOLINT(runtime/references)
};