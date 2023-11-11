#pragma once
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "AlphabetSymbol.h"
#include "FiniteAutomaton.h"
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
	set<string> equivalence_class;
	// правила переписывания для данного состояния
	map<alphabet_symbol, set<int>> rules;
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
	string name = "";
	GrammarItem();
	GrammarItem(Type type, string name, int state_index, int class_number);
	GrammarItem(Type type, string name, int state_index);
	GrammarItem(Type type, string name);
	bool operator!=(const GrammarItem& other);
	void operator=(const GrammarItem& other);
};
// для отладки
std::ostream& operator<<(std::ostream& os, const GrammarItem& item);
class Grammar {
  private:
	vector<PrefixGrammarItem> prefix_grammar;

	const int fa_to_g(const FiniteAutomaton&, alphabet_symbol, int, int,
					  const vector<PrefixGrammarItem*>&, // вспомогательная функции для
														 // получения префиксной грамматики
					  const set<string>&, string);
	const int fa_to_g_TM(const FiniteAutomaton&, string, int, int,
						 const vector<PrefixGrammarItem*>&, const set<string>&,
						 string); // вспомогательная функции для
								  //  получения префиксной грамматики через ТМ

  public:
	// обновляет значение class_number для каждого нетерминала
	static void update_classes(
		set<int>& checker,											// NOLINT(runtime/references)
		map<set<string>, vector<GrammarItem*>>& classes_check_map); // NOLINT(runtime/references)
	// строит новые классы эквивалентности по терминальным формам
	static void check_classes(
		vector<vector<vector<GrammarItem*>>>& rules,			   // NOLINT(runtime/references)
		map<set<string>, vector<GrammarItem*>>& classes_check_map, // NOLINT(runtime/references)
		vector<GrammarItem*>& nonterminals);					   // NOLINT(runtime/references)
	// преобразует данную грамматику в бисимилярную
	static vector<vector<vector<GrammarItem*>>> get_bisimilar_grammar(
		vector<vector<vector<GrammarItem*>>>& rules,			// NOLINT(runtime/references)
		vector<GrammarItem*>& nonterminals,						// NOLINT(runtime/references)
		vector<GrammarItem*>& bisimilar_nonterminals,			// NOLINT(runtime/references)
		map<int, vector<GrammarItem*>>& class_to_nonterminals); // NOLINT(runtime/references)
	// преобразование конечного автомата в грамматику
	// в векторе терминалов по 0му индексу лежит epsilon
	static vector<vector<vector<GrammarItem*>>> fa_to_grammar(
		const vector<State>& states, const set<alphabet_symbol>& alphabet,
		vector<GrammarItem>& fa_items,		// NOLINT(runtime/references)
		vector<GrammarItem*>& nonterminals, // NOLINT(runtime/references)
		vector<GrammarItem*>& terminals);	// NOLINT(runtime/references)
	// преобразование переходов автомата в грамматику (переход -> состояние
	// переход)
	static vector<vector<vector<GrammarItem*>>> tansitions_to_grammar(
		const vector<State>& states, const vector<GrammarItem*>& fa_nonterminals,
		vector<std::pair<GrammarItem, map<alphabet_symbol, vector<GrammarItem>>>>&
			fa_items,						// NOLINT(runtime/references)
		vector<GrammarItem*>& nonterminals, // NOLINT(runtime/references)
		vector<GrammarItem*>& terminals); // NOLINT(runtime/references)
	// построение обратной грамматики
	static vector<vector<vector<GrammarItem*>>> get_reverse_grammar(
		vector<vector<vector<GrammarItem*>>>& rules,		 // NOLINT(runtime/references)
		vector<GrammarItem*>& nonterminals,					 // NOLINT(runtime/references)
		vector<GrammarItem*>& terminals, int initial_state); // NOLINT(runtime/references)
	// создание пр грамматики по НКА
	void fa_to_prefix_grammar(const FiniteAutomaton&, iLogTemplate* log = nullptr);
	// создание пр грамматики по НКА с помощью ТМ
	void fa_to_prefix_grammar_TM(const FiniteAutomaton&, iLogTemplate* log = nullptr);
	// создает автомат по пр грамматике
	FiniteAutomaton prefix_grammar_to_automaton(iLogTemplate* log = nullptr) const;
	// вывод пр грамматики в формате string
	string pg_to_txt() const;
};