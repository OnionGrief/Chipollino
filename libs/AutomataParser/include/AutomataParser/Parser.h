#pragma once

#include <Objects/FiniteAutomaton.h>
#include <Objects/MemoryFiniteAutomaton.h>
#include <vector>
#include <map>
#include <unordered_set>
#include <string>
#include <sstream>
#include <lexy/callback/string.hpp>
#include <lexy/lexeme.hpp>
#include <lexy/callback/string.hpp>
#include "Lexer.h"
#define lexy_ascii_child lexy::_pt_node<lexy::_bra, void>

class Parser {
  private:
    // Информация для сборки перехода MFA
    struct MFATransition_info{
        std::string beg;
        std::string end;
        Symbol symb;

        std::unordered_set<int> open;
        std::unordered_set<int> close;
    };

    // Поиск рекурсивный поиск вершин с названиями из names, игнорируя спуск в вершины из exclude
    static std::vector<lexy_ascii_child> find_children(lexy_ascii_tree& tree,
     std::set<std::string> names, std::set<std::string> exclude = {});

    // Поиск имён всех состояний 
    static void parse_states(lexy_ascii_tree& tree, std::set<std::string>& names,
     std::map<std::string, std::string>& labels);

    // Парсинг описаний вершин
    static void parse_descriptions(lexy_ascii_tree& tree, std::map<std::string, std::string>& labels,
     std::map<std::string, bool>& is_terminal, std::string& initial);
    
    // Парсинг переходов для FA
    static void parse_transitions(lexy_ascii_tree& tree, std::vector<std::string>& beg,
     std::vector<std::string>& end, std::vector<std::string>& symb);

    // Парсинг переходов для MFA
    static std::vector<MFATransition_info> parse_MFA_transitions(lexy_ascii_tree& tree);


  public:
    // Разбор MFA из файла
    static MemoryFiniteAutomaton parse_MFA(std::string filename);

    // Разбор FA из файла
    static FiniteAutomaton parse_FA(std::string filename);
};