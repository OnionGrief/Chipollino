#include <set>

#include "Objects/FiniteAutomaton.h"
#include "Objects/MetaInfo.h"

using std::set;
using std::string;
using std::to_string;

// Преобразование числового идентификатора цвета в строковый
string to_color_id(int group_id) {
	switch (group_id) {
	case MetaInfo::trap_color:
		return ("Trap");
	default:
		return ("group" + to_string(group_id));
	}
}

// Преобразование метаданных для вершины графа в строку
string colorize_node(int id, int group_id) {
	return "@" + to_string(id) + "@::=" + to_color_id(group_id) + "\n";
}

// Преобразование метаданных для ребра графа в строку
string colorize_edge(int from_ind, int to_id, const Symbol& by, int group_id) {
	return "@" + to_string(from_ind) + "-" + to_string(to_id) + "@" + string(by) +
		   "@::=" + to_color_id(group_id) + "\n";
}

void MetaInfo::mark_transitions(const FiniteAutomaton& fa, const set<int>& from, set<int> to,
								const Symbol& by, int group_id) {
	for (auto elem : from)
		for (auto [rune, reach_set] : fa.states[elem].transitions) {
			for (auto reach : reach_set)
				if (to.find(reach) != to.end() && (rune == by))
					this->upd(EdgeMeta{fa.states[elem].index, reach, rune, group_id});
		}
}

void MetaInfo::upd(const Meta& item) {
	this->metadata.push_back(item);
}

string MetaInfo::to_output() const {
	string colors;
	for (auto elem : metadata) {
		if (std::holds_alternative<NodeMeta>(elem)) {
			auto node_m = std::get<NodeMeta>(elem);
			colors += colorize_node(node_m.id, node_m.group);
		} else {
			auto edge_m = std::get<EdgeMeta>(elem);
			colors += colorize_edge(edge_m.from, edge_m.to, edge_m.label, edge_m.group);
		}
	}
	return colors;
}
