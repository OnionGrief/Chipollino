#pragma once
#include <iostream>
#include <unordered_set>
#include <utility>

struct PairHasher {
	size_t operator()(const std::pair<int, int>& p) const;
};

using IntPairSet = std::unordered_set<std::pair<int, int>, PairHasher>;

struct Cell {
	int number;
	int lin_number;

	Cell(int, int);
	bool operator==(const Cell& other) const;

	struct Hasher {
		std::size_t operator()(const Cell&) const;
	};
};

using CellSet = std::unordered_set<Cell, Cell::Hasher>;

CellSet merge_sets(const CellSet& set1, const CellSet& set2);