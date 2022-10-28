#pragma once
#include "AlphabetSymbol.h"
#include "FiniteAutomaton.h"
#include <iostream>
#include <map>
#include <set>
#include <vector>
using namespace std;

struct GrammarItem {
	enum Type {
		terminal,
		nonterminal
	};
	Type type;
	int state_index, class_number;
	string term_name;
	GrammarItem()
		: type(terminal), state_index(-1), class_number(-1), term_name("") {}
	GrammarItem(Type type, int state_index, int class_number)
		: type(type), state_index(state_index), class_number(class_number) {}
	GrammarItem(Type type, string term_name)
		: type(type), term_name(term_name) {}
	bool operator!=(const GrammarItem& other) {
		return type != other.type || state_index != other.state_index ||
			   class_number != other.class_number ||
			   term_name != other.term_name;
	}
	void operator=(const GrammarItem& other) {
		type = other.type;
		state_index = other.state_index;
		class_number = other.class_number;
		term_name = other.term_name;
	}
};
// для отладки
ostream& operator<<(ostream& os, const GrammarItem& item);
class Grammar {
  public:
	// обновляет значение class_number для каждого нетерминала
	static void update_classes(
		set<int>& checker,
		map<set<string>, vector<GrammarItem*>>& classes_check_map);
	// строит новые классы эквивалентности по терминальным формам
	static void check_classes(
		vector<vector<vector<GrammarItem*>>>& rules,
		map<set<string>, vector<GrammarItem*>>& classes_check_map,
		vector<GrammarItem*>& nonterminals);
	// преобразует данную грамматику в бисимилярную
	static vector<vector<vector<GrammarItem*>>> get_bisimilar_grammar(
		vector<vector<vector<GrammarItem*>>>& rules,
		vector<GrammarItem*>& nonterminals,
		vector<GrammarItem*>& bisimilar_nonterminals);
	// преобразование конечного автомата в грамматику
	// в векторе терминалов по 0му индексу лежит epsilon,
	// по terminals.size()-1 лежит initial
	static vector<vector<vector<GrammarItem*>>> fa_to_grammar(
		const vector<State>& states, const set<alphabet_symbol>& alphabet,
		vector<GrammarItem>& fa_items, vector<GrammarItem*>& nonterminals,
		vector<GrammarItem*>& terminals);
	// преобразование переходов автомата в грамматику (переход -> состояние
	// переход)
	static vector<vector<vector<GrammarItem*>>> tansitions_to_grammar(
		const vector<State>& states,
		const vector<GrammarItem*>& fa_nonterminals,
		vector<pair<GrammarItem, map<alphabet_symbol, vector<GrammarItem>>>>&
			fa_items,
		vector<GrammarItem*>& nonterminals, vector<GrammarItem*>& terminals);
	// построение обратной грамматики
	static vector<vector<vector<GrammarItem*>>> get_reverse_grammar(
		vector<vector<vector<GrammarItem*>>>& rules,
		vector<GrammarItem*>& nonterminals, vector<GrammarItem*>& terminals,
		int initial_state);
};