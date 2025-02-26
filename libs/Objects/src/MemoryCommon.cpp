#include "Objects/MemoryCommon.h"

using std::tuple;
using std::unordered_set;
using std::vector;

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

CaptureGroup::CaptureGroup(int cell, const vector<vector<int>>& _paths,
						   const vector<int>& _state_classes, bool is_reset)
	: cell(cell), is_reset(is_reset) {
	for (const auto& path : _paths) {
		paths.insert(path);
		for (auto st : path) {
			int class_num = (is_reset) ? State::reset_class : _state_classes[st];
			states.insert({st, class_num});
			state_classes.insert(class_num);
		}
	}
}

bool CaptureGroup::operator==(const CaptureGroup& other) const {
	return cell == other.cell && states == other.states;
}

bool CaptureGroup::get_is_reset() const {
	return is_reset;
}

bool CaptureGroup::get_cell_number() const {
	return cell;
}

int CaptureGroup::get_opening_state_index() const {
	return (*paths.begin())[0];
}

const std::unordered_set<std::vector<int>, VectorHasher<int>>& CaptureGroup::get_paths() const {
	return paths;
}

const unordered_set<CaptureGroup::State, CaptureGroup::State::Hasher>& CaptureGroup::get_states()
	const {
	return states;
}

tuple<unordered_set<int>, unordered_set<int>> CaptureGroup::get_states_diff(
	const CaptureGroup& other) const {
	unordered_set<int> diff;
	for (auto st : states)
		if (st.class_num != State::reset_class && !other.state_classes.count(st.class_num))
			diff.insert(st.index);

	unordered_set<int> following(diff);
	for (const auto& path : paths)
		for (size_t i = path.size() - 1; i > 0; i--)
			if (diff.count(path[i - 1]))
				following.insert(path[i]);
	return {diff, following};
}

std::ostream& operator<<(std::ostream& os, const CaptureGroup& cg) {
	os << "{\n";
	for (const auto& i : cg.paths)
		os << i;
	os << "}\n[ ";
	for (const auto& i : cg.states)
		os << "{" << i.index << ": " << i.class_num << "} ";
	return os << "]\n";
}