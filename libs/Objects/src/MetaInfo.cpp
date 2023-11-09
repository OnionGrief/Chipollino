#include "Objects/MetaInfo.h"
#include "Objects/FiniteAutomaton.h"
#include <set>
using namespace std;

// MetaInfo::MetaInfo() {} констуктор уже объявлен в заголовке как default

string to_color_id(int group_id) {
	switch (group_id) {
	case MetaInfo::trap_color:
		return ("Trap");
	default:
		return ("group" + to_string(group_id));
	}
}

string colorize_node(int id, int group_id) {
	return "@" + to_string(id) + "@::=" + to_color_id(group_id) + "\n";
}

string colorize_edge(int from_ind, int to_id, alphabet_symbol by, int group_id) {
	return "@" + to_string(from_ind) + "-" + to_string(to_id) + "@" + string(by) +
		   "@::=" + to_color_id(group_id) + "\n";
}

void MetaInfo::MarkTransitions(const FiniteAutomaton& fa, set<int> from, set<int> to,
							   alphabet_symbol by, int group_id) {
	for (auto elem : from)
		for (auto trans : fa.states[elem].transitions) {
			for (auto reach : trans.second)
				if (to.find(reach) != to.end() && (trans.first == by))
					this->Upd(EdgeMeta{fa.states[elem].index, reach, trans.first, group_id});
		}
}

void MetaInfo::Upd(Meta item) {
	this->metadata.push_back(item);
}

string MetaInfo::Colorize() const {
	string colors = "";
	for (auto elem : metadata) {
		if (holds_alternative<NodeMeta>(elem)) {
			auto node_m = get<NodeMeta>(elem);
			colors += colorize_node(node_m.id, node_m.group);
		} else {
			auto edge_m = get<EdgeMeta>(elem);
			colors += colorize_edge(edge_m.from, edge_m.to, edge_m.label, edge_m.group);
		}
	}
	return colors;
}
