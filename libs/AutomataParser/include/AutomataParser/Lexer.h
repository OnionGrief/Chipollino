#pragma once

#include <iostream>
#include <stdexcept>
#include <string>
#include <map>
#include <lexy/dsl.hpp>
#include <lexy/action/parse_as_tree.hpp>
#include <lexy/visualize.hpp>
#include <lexy/input/file.hpp>
#include <lexy_ext/report_error.hpp>
#define lexy_ascii_tree lexy::parse_tree_for<lexy::buffer<lexy::ascii_encoding>, void, void>

namespace dsl = lexy::dsl;

//=== string constants ===//
namespace AutomataParser {
    const std::string node_id = "node_id";
    const std::string statement = "stmt";
    const std::string terminal_mark = "terminal_mark";
    const std::string initial_state = "initial_state";
    const std::string state_label = "state_label";
    const std::string state_description = "state_description";
    const std::string MFA_edge = "MFA_edge";
    const std::string cell_id = "cell_id";
    const std::string memory_cell = "memory_cell";

    const std::string epsilon = "eps";
} // namespace AutomataParser

class Lexer{
  private:
    //=== grammar ===//
    struct node_id {
        static constexpr auto rule
            // One or more alpha numeric characters, underscores or hyphens.
            = dsl::identifier(dsl::unicode::alnum / dsl::lit_c<'_'> / dsl::lit_c<'-'>);  
    };

    struct cell_id {
        static constexpr auto rule
            = dsl::digits<>.no_leading_zero();
    };

    struct transition {
        static constexpr auto rule = LEXY_LIT("eps") | dsl::unicode::alnum | (LEXY_LIT("&") >> dsl::p<cell_id>);
    };

    struct stmt {
        static constexpr auto rule = dsl::p<node_id> + dsl::p<node_id> + dsl::p<transition>;
    };

    struct memory_state {
        static constexpr auto rule = (LEXY_LIT("o") | LEXY_LIT("c"));
    };

    struct memory_cell {
        static constexpr auto rule = dsl::p<cell_id> >> dsl::p<memory_state>;
    };

    struct memory_lists {
        static constexpr auto rule = dsl::while_(dsl::p<memory_cell>);
    };

    struct MFA_edge {
        static constexpr auto rule = (dsl::p<stmt> + dsl::p<memory_lists>);
    };

    struct FA_edge {
        static constexpr auto rule = dsl::p<stmt>;
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
        static constexpr auto variants = dsl::partial_combination(dsl::p<state_label>, dsl::p<terminal_mark>, dsl::p<initial_mark>);
        
        static constexpr auto rule = dsl::p<node_id> + variants;
    };

    struct states {
        static constexpr auto sep  = dsl::sep(dsl::semicolon);
        
        static constexpr auto rule = dsl::list(dsl::p<state_description>, sep) + LEXY_LIT("...");
    };

    struct MFA {
        // Allow arbitrary spaces between individual tokens.

        static constexpr auto whitespace = dsl::ascii::space;
        
        static constexpr auto sep  = dsl::sep(dsl::semicolon);
        
        static constexpr auto rule = LEXY_LIT("{") + dsl::p<states> + dsl::list(dsl::p<MFA_edge>, sep) + LEXY_LIT("}");
    };

    struct FA {
        // Allow arbitrary spaces between individual tokens.

        static constexpr auto whitespace = dsl::ascii::space;
        
        static constexpr auto sep  = dsl::sep(dsl::semicolon);
        
        static constexpr auto rule = LEXY_LIT("{") + dsl::p<states> + dsl::list(dsl::p<FA_edge>, sep) + LEXY_LIT("}");
    };

    struct production {
        static constexpr auto rule = LEXY_LIT("MFA") >> dsl::p<MFA> | LEXY_LIT("FA") >> dsl::p<FA>;
    };

  public:
    
    // Строит дерево разбора по грамматике описанной в private лексера
    static void parse_buffer(lexy_ascii_tree& tree, // NOLINT(runtime/references)
     lexy::buffer<lexy::ascii_encoding>& buffer); // NOLINT(runtime/references)
}; 