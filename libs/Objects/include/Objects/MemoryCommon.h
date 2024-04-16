#pragma once
#include <iostream>
#include <optional>
#include <unordered_set>
#include <utility>
#include <vector>

#include "Tools.h"

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

CellSet get_union(const CellSet& set1, const CellSet& set2);

CellSet get_intersection(const CellSet& set1, const CellSet& set2);

struct CaptureGroup {
	struct State {
		int index;
		int class_num;

		static const int reset_class = -1;
		bool operator==(const State& other) const;

		struct Hasher {
			std::size_t operator()(const State&) const;
		};
	};
	int cell;
	std::unordered_set<std::vector<int>, VectorHasher<int>> traces;
	std::unordered_set<State, State::Hasher> states;
	std::unordered_set<int> state_classes;

	CaptureGroup() = default;
	CaptureGroup(int, const std::vector<std::vector<int>>&, const std::vector<int>&);
	bool operator==(const CaptureGroup& other) const;

	std::unordered_set<int> get_states_diff(
		const std::unordered_set<int>& other_state_classes) const;
};

std::ostream& operator<<(std::ostream& os, const CaptureGroup& cg);
