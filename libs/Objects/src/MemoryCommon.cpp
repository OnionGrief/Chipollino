#include "Objects/MemoryCommon.h"

Cell::Cell(int number, int lin_number) : number(number), lin_number(lin_number) {}

bool Cell::operator==(const Cell& other) const {
	return number == other.number && lin_number == other.lin_number;
}

size_t Cell::Hasher::operator()(const Cell& c) const {
	IntPairHasher hasher;
	return hasher({c.number, c.lin_number});
}

CellSet get_union(const CellSet& set1, const CellSet& set2) {
	CellSet result = set1;
	result.insert(set2.begin(), set2.end());
	return result;
}

CellSet get_intersection(const CellSet& set1, const CellSet& set2) {
	CellSet result;
	for (const auto& element : set1) {
		if (set2.find(element) != set2.end()) {
			result.insert(element);
		}
	}
	return result;
}

std::size_t CaptureGroup::State::Hasher::operator()(const State& s) const {
	IntPairHasher hasher;
	return hasher({s.index, s.class_num});
}

bool CaptureGroup::State::operator==(const State& other) const {
	return index == other.index && class_num == other.class_num;
}

CaptureGroup::CaptureGroup(int cell, const std::vector<std::vector<int>>& _paths,
						   const std::vector<int>& _state_classes, bool reset)
	: cell(cell) {
	for (const auto& path : _paths) {
		paths.insert(path);
		for (auto st : path) {
			int class_num = (reset) ? State::reset_class : _state_classes[st];
			states.insert({st, class_num});
			state_classes.insert(class_num);
		}
	}
}

bool CaptureGroup::operator==(const CaptureGroup& other) const {
	return cell == other.cell && states == other.states;
}

std::unordered_set<int> CaptureGroup::get_states_diff(
	const std::unordered_set<int>& other_state_classes) const {
	std::unordered_set<int> res;
	for (auto st : states)
		if (st.class_num != State::reset_class && !other_state_classes.count(st.class_num))
			res.insert(st.index);

	for (const auto& path : paths)
		for (int i = path.size() - 1; i > 0; i--)
			if (res.count(path[i - 1]))
				res.insert(path[i]);
	return res;
}

std::ostream& operator<<(std::ostream& os, const CaptureGroup& cg) {
	os << "{\n";
	for (const auto& i : cg.paths)
		os << i;
	os << "}\n";
	for (const auto& i : cg.states)
		os << "{" << i.index << ": " << i.class_num << "} ";
	return os << "\n";
}
