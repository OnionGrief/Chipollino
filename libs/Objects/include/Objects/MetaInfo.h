#pragma once
#include <set>
#include <string>
#include <variant>
#include <vector>

#include "Symbol.h"

class FiniteAutomaton;

struct EdgeMeta {
	int from;
	int to;
	Symbol label;
	int group;
};

struct NodeMeta {
	int id;
	int group;
};

using Meta = std::variant<EdgeMeta, NodeMeta>;

class MetaInfo {
  private:
	std::vector<Meta> metadata;

  public:
	// Числовой идентификатор разметки ловушки
	static const int trap_color = 100;
	// Добавление единственного элемента разметки
	void upd(const Meta& item);
	// Преобразование в формат, входной для модулей рефала (и печати во вспомогательный файл)
	std::string to_output() const;
	// Разметка всех переходов автомата из множества выделенных состояний from в множество to по
	// символу by
	void mark_transitions(const FiniteAutomaton&, const std::set<int>& from, std::set<int> to,
						  const Symbol& by, int group_id);
	MetaInfo() = default;
};