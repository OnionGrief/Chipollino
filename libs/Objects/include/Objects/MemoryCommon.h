#pragma once
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
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

struct MFATransition {
	enum MemoryAction {
		// idle, ◇
		open,  // o
		close, // c
		reset, // r
	};

	using MemoryActions = std::unordered_map<int, MemoryAction>;

	int to;
	MemoryActions memory_actions;

	explicit MFATransition(int to);
	MFATransition(int, MemoryActions);
	MFATransition(int, const std::unordered_set<int>&, const std::unordered_set<int>&);
	MFATransition(int, const std::unordered_set<int>&, const std::unordered_set<int>&,
				  const std::unordered_set<int>&);

	struct TransitionConfig {
		// пары {номер ячейки, линеаризованный номер оператора}
		const CellSet* destination_first;
		const std::unordered_set<int>* source_in_lin_cells;
		const std::unordered_set<int>* iteration_over_cells;
		// пары {номер ячейки, линеаризованный номер оператора}
		const CellSet* source_last;
		const std::unordered_set<int>* destination_in_lin_cells;
		const CellSet* to_reset;
	};
	MFATransition(int, const TransitionConfig& config);

	std::string get_actions_str() const;
	bool operator==(const MFATransition& other) const;

	struct Hasher {
		std::size_t operator()(const MFATransition&) const;
	};
};

class CaptureGroup {
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
	std::unordered_set<std::vector<int>, VectorHasher<int>> paths;
	std::unordered_set<State, State::Hasher> states;
	std::unordered_set<int> state_classes;
	bool is_reset;

  public:
	CaptureGroup() = default;
	CaptureGroup(int, const std::vector<std::vector<int>>&, const std::vector<int>&,
				 bool is_reset = false);
	bool operator==(const CaptureGroup& other) const;

	bool get_is_reset() const;
	int get_opening_state_index() const;

	const std::unordered_set<std::vector<int>, VectorHasher<int>>& get_paths() const;
	const std::unordered_set<State, State::Hasher>& get_states() const;

	std::unordered_set<int> get_states_diff(const CaptureGroup& other) const;

	friend std::ostream& operator<<(std::ostream& os, const CaptureGroup& cg);
};

std::ostream& operator<<(std::ostream& os, const CaptureGroup& cg);
