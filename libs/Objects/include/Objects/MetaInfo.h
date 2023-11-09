#pragma once
#include "AlphabetSymbol.h"
#include <set>
#include <variant>
using namespace std;

class FiniteAutomaton;

struct EdgeMeta {
	int from;
	int to;
	alphabet_symbol label;
	int group;
};

struct NodeMeta {
	int id;
	int group;
};

using Meta = variant<EdgeMeta, NodeMeta>;

class MetaInfo {
  private:
	vector<Meta> metadata;

  public:
	void Upd(Meta item);
	static const int trap_color = 100;
	string Colorize() const;
	void MarkTransitions(const FiniteAutomaton&, set<int> from, set<int> to, alphabet_symbol by,
						 int group_id);
	MetaInfo() = default;
};