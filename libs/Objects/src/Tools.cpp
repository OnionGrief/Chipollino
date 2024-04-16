#include "Objects/Tools.h"

using std::ostream;
using std::set;
using std::unordered_set;
using std::vector;

size_t TupleHasher::operator()(const std::tuple<int, int, int>& t) const {
	size_t seed = 0;
	hash_combine(seed, std::get<0>(t));
	hash_combine(seed, std::get<1>(t));
	hash_combine(seed, std::get<2>(t));
	return seed;
}

ostream& operator<<(ostream& os, const vector<int>& vec) {
	os << "[";
	for (size_t i = 0; i < vec.size(); ++i) {
		os << vec[i];
		if (i != vec.size() - 1) {
			os << ", ";
		}
	}
	return os << "]\n";
}

ostream& operator<<(ostream& os, const unordered_set<int>& uset) {
	os << "{";
	bool first = true;
	for (const auto& element : uset) {
		if (!first) {
			os << ", ";
		}
		os << element;
		first = false;
	}
	return os << "}\n";
}

ostream& operator<<(ostream& os, const set<int>& set) {
	os << "{";
	bool first = true;
	for (const auto& element : set) {
		if (!first) {
			os << ", ";
		}
		os << element;
		first = false;
	}
	return os << "}\n";
}

ostream& operator<<(ostream& os, const std::pair<int, int>& pair) {
	return os << "{" << pair.first << ", " << pair.second << "}\n";
}

ostream& operator<<(ostream& os, const std::tuple<int, int, int>& tuple) {
	return os << "{" << std::get<0>(tuple) << ", " << std::get<1>(tuple) << ", "
			  << std::get<2>(tuple) << "}\n";
}
