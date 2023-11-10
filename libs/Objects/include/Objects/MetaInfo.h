#pragma once
#include "AlphabetSymbol.h"
#include <set>
#include <variant>
using namespace std;

class FiniteAutomaton;

struct EdgeMeta {
	int from;
	int to;
	alphabet_symbol label;
	int group;
};

struct NodeMeta {
	int id;
	int group;
};

using Meta = variant<EdgeMeta, NodeMeta>;

class MetaInfo {
  private:
	vector<Meta> metadata;

  public:
	// Числовой идентификатор разметки ловушки
	static const int trap_color = 100;
	// Добавление единственного элемента разметки
	void upd(Meta item);
	// Преобразование в формат, входной для модулей рефала (и печати во вспомогательный файл)
	string to_output() const;
	// Разметка всех переходов автомата из множества выделенных состояний from в множество to по символу by
	void mark_transitions(const FiniteAutomaton&, set<int> from, set<int> to, alphabet_symbol by,
						 int group_id);
	MetaInfo() = default;
};