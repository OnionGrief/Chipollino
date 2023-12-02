#pragma once
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "FiniteAutomaton.h"
#include "Symbol.h"
#include "TransformationMonoid.h"

struct PrefixGrammarItem {
	// конечное состояние автомата
	bool is_terminal = false;
	// начальное состояние автомата
	bool is_started = false;
	// Type type = terminal;
	int state_index = -1;
	bool is_visit = false;
	// классы эквивалентности у состояния в автомате
	std::set<std::string> equivalence_class;
	// правила переписывания для данного состояния
	std::map<Symbol, std::set<int>> rules;
	PrefixGrammarItem();
	// PrefixGrammarItem(Type type, int state_index);
};

struct GrammarItem {
	enum Type {
		terminal,
		nonterminal
	};
	Type type = terminal;
	int state_index = -1, class_number = -1;
	std::string name;
	GrammarItem();
	GrammarItem(Type type, std::string name, int state_index, int class_number);
	GrammarItem(Type type, std::string name, int state_index);
	GrammarItem(Type type, std::string name);
	bool operator!=(const GrammarItem& other) const;
};
// для отладки
std::ostream& operator<<(std::ostream& os, const GrammarItem& item);
class Grammar {
  private:
	std::vector<PrefixGrammarItem> prefix_grammar;

	int fa_to_g(const FiniteAutomaton&, Symbol, int, int,
				const std::vector<PrefixGrammarItem*>&, // вспомогательная функции для
														// получения префиксной грамматики
				const std::set<std::string>&, std::string);
	int fa_to_g_TM(const FiniteAutomaton&, std::string, int, int,
				   const std::vector<PrefixGrammarItem*>&, const std::set<std::string>&,
				   std::string); // вспомогательная функции для
								 //  получения префиксной грамматики через ТМ

  public:
	// обновляет значение class_number для каждого нетерминала
	static void update_classes(std::set<int>& checker, // NOLINT(runtime/references)
							   std::map<std::set<std::string>, std::vector<GrammarItem*>>&
								   classes_check_map); // NOLINT(runtime/references)
	// строит новые классы эквивалентности по терминальным формам
	static void check_classes(
		std::vector<std::vector<std::vector<GrammarItem*>>>& rules, // NOLINT(runtime/references)
		std::map<std::set<std::string>, std::vector<GrammarItem*>>&
			classes_check_map,					  // NOLINT(runtime/references)
		std::vector<GrammarItem*>& nonterminals); // NOLINT(runtime/references)
	// преобразует данную грамматику в бисимилярную
	static std::vector<std::vector<std::vector<GrammarItem*>>> get_bisimilar_grammar(
		std::vector<std::vector<std::vector<GrammarItem*>>>& rules, // NOLINT(runtime/references)
		std::vector<GrammarItem*>& nonterminals,					// NOLINT(runtime/references)
		std::vector<GrammarItem*>& bisimilar_nonterminals,			// NOLINT(runtime/references)
		std::map<int, std::vector<GrammarItem*>>&
			class_to_nonterminals); // NOLINT(runtime/references)
	// преобразование конечного автомата в грамматику
	// в векторе терминалов по 0му индексу лежит epsilon
	static std::vector<std::vector<std::vector<GrammarItem*>>> fa_to_grammar(
		const std::vector<FiniteAutomaton::State>& states, const std::set<Symbol>& alphabet,
		std::vector<GrammarItem>& fa_items,		 // NOLINT(runtime/references)
		std::vector<GrammarItem*>& nonterminals, // NOLINT(runtime/references)
		std::vector<GrammarItem*>& terminals);	 // NOLINT(runtime/references)
	// преобразование переходов автомата в грамматику (переход -> состояние
	// переход)
	static std::vector<std::vector<std::vector<GrammarItem*>>> tansitions_to_grammar(
		const std::vector<FiniteAutomaton::State>& states,
		const std::vector<GrammarItem*>& fa_nonterminals,
		std::vector<std::pair<GrammarItem, std::map<Symbol, std::vector<GrammarItem>>>>&
			fa_items,							 // NOLINT(runtime/references)
		std::vector<GrammarItem*>& nonterminals, // NOLINT(runtime/references)
		std::vector<GrammarItem*>& terminals);	 // NOLINT(runtime/references)
	// построение обратной грамматики
	static std::vector<std::vector<std::vector<GrammarItem*>>> get_reverse_grammar(
		std::vector<std::vector<std::vector<GrammarItem*>>>& rules, // NOLINT(runtime/references)
		std::vector<GrammarItem*>& nonterminals,					// NOLINT(runtime/references)
		std::vector<GrammarItem*>& terminals, int initial_state);	// NOLINT(runtime/references)
	// создание пр грамматики по НКА
	void fa_to_prefix_grammar(const FiniteAutomaton&, iLogTemplate* log = nullptr);
	// создание пр грамматики по НКА с помощью ТМ
	void fa_to_prefix_grammar_TM(const FiniteAutomaton&, iLogTemplate* log = nullptr);
	// создает автомат по пр грамматике
	FiniteAutomaton prefix_grammar_to_automaton(iLogTemplate* log = nullptr) const;
	// вывод пр грамматики в формате std::string
	std::string pg_to_txt() const;
};