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

class ParseTreeDescend {
    protected:
        int cur_pos = 0;

        std::map<std::string, lexy_ascii_child::children_range::iterator> rewriting_rules;
        std::set<std::string> attributes;

        std::map<std::string, std::function<bool()>> parse_func;

        // Поиск рекурсивный поиск вершин с названиями из names, игнорируя спуск в вершины из exclude
        std::vector<lexy_ascii_child> find_children(
            lexy_ascii_tree& tree, // NOLINT(runtime/references)
            const std::set<std::string>& names, const std::set<std::string>& exclude = {});

        // лексема первого потомка для вершины lexy
        std::string first_child(lexy_ascii_child::children_range::iterator it);

        // лексема первого потомка для вершины lexy
        std::string first_child(lexy_ascii_child it);

        std::function<bool(std::string)> parse_reserved;

        std::function<bool(lexy_ascii_child ref)> parse_terminal;

        void parse_attribute(lexy_ascii_child ref);

        bool parse_transition(std::string name);

        bool parse_nonterminal(lexy_ascii_child ref);

        bool parse_alternative(lexy_ascii_child ref);
    public:
        ParseTreeDescend() {}
};