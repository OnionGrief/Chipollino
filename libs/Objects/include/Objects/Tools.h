#pragma once
#include <iostream>
#include <set>
#include <tuple>
#include <unordered_set>
#include <utility>
#include <vector>

template <typename T>
void hash_combine(std::size_t& seed, const T& value) { // NOLINT(runtime/references)
	seed ^= std::hash<T>{}(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template <typename T1, typename T2> struct PairHasher {
	size_t operator()(const std::pair<T1, T2>& p) const {
		return std::hash<T1>()(p.first) ^ std::hash<T2>()(p.second);
	}
};

template <typename T> struct VectorHasher {
	size_t operator()(const std::vector<T>& vec) const {
		size_t seed = 0;
		for (const auto& elem : vec) {
			hash_combine(seed, elem);
		}
		return seed;
	}
};

using IntPairHasher = PairHasher<int, int>;

struct TupleHasher {
	size_t operator()(const std::tuple<int, int, int>& p) const;
};

using IntPairsSet = std::unordered_set<std::pair<int, int>, IntPairHasher>;

std::ostream& operator<<(std::ostream& os, const std::vector<int>& vec);

std::ostream& operator<<(std::ostream& os, const std::unordered_set<int>& uset);

std::ostream& operator<<(std::ostream& os, const std::set<int>& set);

std::ostream& operator<<(std::ostream& os, const std::pair<int, int>& pair);

std::ostream& operator<<(std::ostream& os, const std::tuple<int, int, int>& tuple);