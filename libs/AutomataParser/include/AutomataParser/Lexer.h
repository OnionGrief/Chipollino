#pragma once

#include <fstream>
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

class Lexer {
  private:
	//=== grammar ===//
	struct nonterminal {
		static constexpr auto rule
				// One or more alphanumeric characters, underscores or hyphens.
				= dsl::identifier(dsl::unicode::alnum / dsl::lit_c<'_'> / dsl::lit_c<'-'>);
	};

	struct reserved {
		static constexpr auto rule = LEXY_LIT("LETTER") | LEXY_LIT("DIGIT") | LEXY_LIT("STRING") |
									 LEXY_LIT("NUMBER") | LEXY_LIT("EPS");
	};

	struct terminal {
		static constexpr auto c = -dsl::apostrophe;

		static constexpr auto rule = dsl::single_quoted(c);
	};

	struct attribute {
		static constexpr auto rule = dsl::lit_c<'!'> >> dsl::p<nonterminal>;
	};

	struct alternative {
		static constexpr auto sep = dsl::sep(dsl::lit_c<'|'>);

		static constexpr auto ternary_operator =
				dsl::parenthesized(dsl::p<nonterminal> + LEXY_LIT("?") + dsl::recurse<alternative> +
								   LEXY_LIT(":") + dsl::recurse<alternative>);

		static constexpr auto expr =
				dsl::while_(dsl::p<reserved> | dsl::p<nonterminal> | dsl::p<terminal> |
							dsl::p<attribute> | ternary_operator);

		static constexpr auto rule = dsl::list(expr, sep);
	};

	struct transition {
		static constexpr auto rule = dsl::p<nonterminal> + LEXY_LIT("-->") + dsl::p<alternative>;
	};

	struct production {
		static constexpr auto whitespace = dsl::ascii::space;

		static constexpr auto sep = dsl::sep(dsl::semicolon);

		static constexpr auto rule = dsl::list(dsl::p<transition>, sep);
	};

  public:
	// Строит дерево разбора по грамматике описанной в private лексера
	static void parse_buffer(
		lexy_ascii_tree& tree,						 // NOLINT(runtime/references)
		lexy::buffer<lexy::ascii_encoding>& buffer); // NOLINT(runtime/references)
};