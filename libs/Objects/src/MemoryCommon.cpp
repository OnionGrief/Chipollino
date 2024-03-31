#include "Objects/MemoryCommon.h"

size_t PairHasher::operator()(const std::pair<int, int>& p) const {
	size_t hash1 = std::hash<int>()(p.first);
	size_t hash2 = std::hash<int>()(p.second);
	return hash1 ^ (hash2 + 0x9e3779b9 + (hash1 << 6) + (hash1 >> 2));
}

Cell::Cell(int number, int lin_number) : number(number), lin_number(lin_number) {}

bool Cell::operator==(const Cell& other) const {
	return number == other.number && lin_number == other.lin_number;
}

size_t Cell::Hasher::operator()(const Cell& c) const {
	PairHasher hasher;
	return hasher({c.number, c.lin_number});
}

CellSet merge_sets(const CellSet& set1, const CellSet& set2) {
	CellSet result = set1;
	result.insert(set2.begin(), set2.end());
	return result;
}