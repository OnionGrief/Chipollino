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
#define lexy_ascii_child lexy::_pt_node<lexy::_bra, void>

#include "Lexer.h"
#include "Objects/FiniteAutomaton.h"
#include "Objects/MemoryFiniteAutomaton.h"
#include "Objects/PushdownAutomaton.h"


class Parser {
  private:
	// Информация для сборки перехода FA
	struct FATransition_info {
		std::string beg;
		std::string end;
		Symbol symb;

		FATransition_info(std::string beg, std::string end, const Symbol& symb) : beg(std::move(beg)), end(std::move(end)), symb(symb) {}
	};

	// Информация для сборки перехода MFA
	struct MFATransition_info : FATransition_info {
		std::unordered_set<int> open;
		std::unordered_set<int> close;

		MFATransition_info(std::string beg, std::string end, const Symbol& symb,
						   std::unordered_set<int> open, std::unordered_set<int> close)
			: FATransition_info(std::move(beg), std::move(end), symb), open(std::move(open)),
			  close(std::move(close)) {}
	};

	// Информация для сборки перехода MFA
	struct PDATransition_info : FATransition_info {
		Symbol pop;
		std::vector<Symbol> push;

		PDATransition_info(std::string beg, std::string end, const Symbol& symb,
						   const Symbol& pop, const std::vector<Symbol>& push)
			: FATransition_info(std::move(beg), std::move(end), symb), pop(pop), push(push) {}
	};

	// Поиск рекурсивный поиск вершин с названиями из names, игнорируя спуск в вершины из exclude
	static std::vector<lexy_ascii_child> find_children(
		lexy_ascii_tree& tree, // NOLINT(runtime/references)
		const std::set<std::string>& names, const std::set<std::string>& exclude = {});

	// лексема первого потомка для вершины lexy
	static std::string first_child(lexy::_pt_node<lexy::_bra, void>::children_range::iterator it);

	// лексема первого потомка для вершины lexy
	static std::string first_child(lexy::_pt_node<lexy::_bra, void> it);

	// Поиск имён всех состояний
	static void parse_states(
		lexy_ascii_tree& tree, std::set<std::string>& names, // NOLINT(runtime/references)
		std::map<std::string, std::string>& labels);		 // NOLINT(runtime/references)

	// Парсинг описаний вершин
	static void parse_descriptions(
		lexy_ascii_tree& tree,						// NOLINT(runtime/references)
		std::map<std::string, std::string>& labels, // NOLINT(runtime/references)
		std::map<std::string, bool>& is_terminal,	// NOLINT(runtime/references)
		std::string& initial); // NOLINT(runtime/references)

	// Парсинг переходов для FA
	static void parse_FA_transitions(
		lexy_ascii_tree& tree,						  // NOLINT(runtime/references)
		std::vector<FATransition_info>& transitions); // NOLINT(runtime/references)

	// Парсинг переходов для MFA
	static void parse_MFA_transitions(
		lexy_ascii_tree& tree,						   // NOLINT(runtime/references)
		std::vector<MFATransition_info>& transitions); // NOLINT(runtime/references)

	static void parse_PDA_transitions(
		lexy_ascii_tree& tree,						   // NOLINT(runtime/references)
		std::vector<PDATransition_info>& transitions); // NOLINT(runtime/references)

  public:
	// Разбор MFA из файла
	static MemoryFiniteAutomaton parse_MFA(const std::string& filename);

	// Разбор FA из файла
	static FiniteAutomaton parse_FA(const std::string& filename);

	// Разбор PDA из файла
	static PushdownAutomaton parse_PDA(const std::string& filename);
};