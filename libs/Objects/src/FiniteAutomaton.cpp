#include <algorithm>
#include <cmath>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <tuple>
#include <unordered_map>
#include <utility>

#include "Fraction/Fraction.h"
#include "Fraction/InfInt.h"
#include "Objects/FiniteAutomaton.h"
#include "Objects/Grammar.h"
#include "Objects/Language.h"
#include "Objects/MetaInfo.h"
#include "Objects/TransformationMonoid.h"
#include "Objects/iLogTemplate.h"

using std::cout;
using std::make_shared;
using std::map;
using std::pair;
using std::set;
using std::string;
using std::stringstream;
using std::to_string;
using std::vector;

FiniteAutomaton::State::State(int index, string identifier, bool is_terminal)
	: AbstractMachine::State::State(index, std::move(identifier), is_terminal) {}

FiniteAutomaton::State::State(int index, string identifier, bool is_terminal,
							  map<Symbol, set<int>> transitions)
	: AbstractMachine::State::State(index, std::move(identifier), is_terminal),
	  transitions(std::move(transitions)) {}

FiniteAutomaton::State::State(int index, set<int> label, string identifier, bool is_terminal,
							  map<Symbol, set<int>> transitions)
	: AbstractMachine::State::State(index, std::move(identifier), is_terminal),
	  label(std::move(label)), transitions(std::move(transitions)) {}

void FiniteAutomaton::State::set_transition(int to, const Symbol& symbol) {
	transitions[symbol].insert(to);
}

FiniteAutomaton::FiniteAutomaton() : AbstractMachine() {}

FiniteAutomaton::FiniteAutomaton(int initial_state, vector<State> states,
								 std::shared_ptr<Language> language)
	: AbstractMachine(initial_state, std::move(language)), states(std::move(states)) {}

FiniteAutomaton::FiniteAutomaton(int initial_state, vector<State> states, set<Symbol> alphabet)
	: AbstractMachine(initial_state, std::move(alphabet)), states(std::move(states)) {}

FiniteAutomaton::FiniteAutomaton(const FiniteAutomaton& other)
	: AbstractMachine(other.initial_state, other.language), states(other.states) {}

template <typename T> FiniteAutomaton* FiniteAutomaton::cast(std::unique_ptr<T>&& uptr) {
	auto* fa = dynamic_cast<FiniteAutomaton*>(uptr.get());
	if (!fa) {
		throw std::runtime_error("Failed to cast to FiniteAutomaton");
	}

	return fa;
}

string FiniteAutomaton::to_txt() const {
	stringstream ss;
	ss << "digraph {\n\trankdir = LR\n\tdummy [label = \"\", shape = none]\n\t";
	for (int i = 0; i < states.size(); i++) {
		ss << states[i].index << " [label = \"" << states[i].identifier << "\", shape = ";
		ss << (states[i].is_terminal ? "doublecircle]\n\t" : "circle]\n\t");
	}
	if (states.size() > initial_state)
		ss << "dummy -> " << states[initial_state].index << "\n";

	for (const auto& state : states) {
		for (const auto& elem : state.transitions) {
			for (int transition_to : elem.second) {
				ss << "\t" << state.index << " -> " << transition_to << " [label = \""
				   << string(elem.first) << "\"]\n";
			}
		}
	}

	ss << "}\n";
	return ss.str();
}

// обход автомата в глубину
void FiniteAutomaton::dfs(int index,
						  set<int>& reachable, // NOLINT(runtime/references)
						  bool use_epsilons_only) const {
	if (reachable.find(index) == reachable.end()) {
		reachable.insert(index);
		const auto& by_eps = states[index].transitions.find(Symbol::epsilon());
		if (by_eps != states[index].transitions.end()) {
			for (int transition_to : by_eps->second)
				dfs(transition_to, reachable, use_epsilons_only);
		}
		if (!use_epsilons_only) {
			for (const auto& [symb, to_states] : states[index].transitions) {
				for (int transition_to : to_states) {
					dfs(transition_to, reachable, use_epsilons_only);
				}
			}
		}
	}
}

set<int> FiniteAutomaton::closure(const set<int>& indices, bool use_epsilons_only) const {
	set<int> reachable;
	for (int index : indices)
		dfs(index, reachable, use_epsilons_only);
	return reachable;
}

FiniteAutomaton FiniteAutomaton::determinize(bool is_trim, iLogTemplate* log) const {
	if (!is_trim)
		if (log)
			log->set_parameter("trap", " (с добавлением ловушки)");
	FiniteAutomaton dfa = FiniteAutomaton(0, {}, language);
	set<int> q0 = closure({initial_state}, true);
	MetaInfo old_meta, new_meta;
	set<int> label = q0;
	string new_identifier;
	int group_counter = 0;
	for (auto elem : label) {
		new_identifier += (new_identifier.empty() || states[elem].identifier.empty() ? "" : ", ") +
						  states[elem].identifier;
	}
	State new_initial_state = {0, label, new_identifier, false, map<Symbol, set<int>>()};
	dfa.states.push_back(new_initial_state);

	if (q0.size() > 1) {
		for (auto elem : q0) {
			old_meta.upd(NodeMeta{states[elem].index, group_counter});
		}
		old_meta.mark_transitions(*this, q0, q0, Symbol::epsilon(), group_counter);
		new_meta.upd(NodeMeta{new_initial_state.index, group_counter});
		group_counter++;
	}

	std::stack<set<int>> s1;
	std::stack<int> s2;
	s1.push(q0);
	s2.push(0);

	while (!s1.empty()) {
		set<int> z = s1.top();
		int index = s2.top();
		s1.pop();
		s2.pop();
		State q = dfa.states[index];

		for (int i : z) {
			if (states[i].is_terminal) {
				dfa.states[index].is_terminal = true;
				break;
			}
		}

		set<int> new_x;
		for (const Symbol& symb : language->get_alphabet()) {
			new_x.clear();
			for (int j : z) {
				auto transitions_by_symbol = states[j].transitions.find(symb);
				if (transitions_by_symbol != states[j].transitions.end())
					for (int k : transitions_by_symbol->second) {
						new_x.insert(k);
					}
			}

			set<int> z1 = closure(new_x, true);
			new_identifier = "";
			for (auto elem : z1) {
				new_identifier +=
					(new_identifier.empty() || states[elem].identifier.empty() ? "" : ", ") +
					states[elem].identifier;
			}

			State q1 = {-1, z1, new_identifier, false, map<Symbol, set<int>>()};
			bool accessory_flag = false;

			for (const auto& state : dfa.states) {
				if (q1.label == state.label) {
					index = state.index;
					accessory_flag = true;
					break;
				}
			}

			if (!accessory_flag)
				index = -1;
			if (index != -1) {
				q1 = dfa.states[index];
			} else {
				index = dfa.size();
				q1.index = index;
				dfa.states.push_back(q1);
				s1.push(z1);
				s2.push(index);
				if (z1.size() > 1) {
					for (auto elem : z1) {
						old_meta.upd(NodeMeta{states[elem].index, group_counter});
					}
					old_meta.mark_transitions(*this, z, z1, symb, group_counter);
					new_meta.upd(NodeMeta{q1.index, group_counter});
					group_counter++;
				}
			}
			dfa.states[q.index].transitions[symb].insert(q1.index);
		}
	}
	if (log) {
		log->set_parameter("oldautomaton", *this, old_meta);
		if (is_trim) {
			// удаление ловушки по желанию пользователя
			log->set_parameter("trapdfa", dfa, new_meta);
			log->set_parameter("to_removetrap", "Автомат перед удалением ловушки: ");
			dfa = dfa.remove_trap_states();
			log->set_parameter("result", dfa);
		} else {
			log->set_parameter("result", dfa, new_meta);
		}
	}
	return dfa;
}

FiniteAutomaton FiniteAutomaton::minimize(bool is_trim, iLogTemplate* log) const {
	if (!is_trim && log)
		log->set_parameter("trap", " (с добавлением ловушки)");
	if (language->is_min_dfa_cached()) {
		FiniteAutomaton language_min_dfa = language->get_min_dfa();
		// удаление ловушки по желанию пользователя
		if (is_trim)
			language_min_dfa = language_min_dfa.remove_trap_states();
		if (log) {
			log->set_parameter("oldautomaton", *this);
			log->set_parameter("cach", "(!) минимальный автомат получен из кэша");
			log->set_parameter("result", language_min_dfa);
		}
		return language_min_dfa;
	}
	// минимизация
	FiniteAutomaton dfa = determinize();
	vector<bool> table(dfa.size() * dfa.size());
	int counter = 1;
	for (int i = 1; i < dfa.size(); i++) {
		for (int j = 0; j < counter; j++) {
			if (dfa.states[i].is_terminal ^ dfa.states[j].is_terminal) {
				table[i * dfa.size() + j] = true;
			}
		}
		counter++;
	}

	bool flag = true;
	while (flag) {
		counter = 1;
		flag = false;
		for (int i = 1; i < dfa.size(); i++) {
			for (int j = 0; j < counter; j++) {
				if (!table[i * dfa.size() + j]) {
					for (const Symbol& symb : language->get_alphabet()) {
						vector<int> to = {*dfa.states[i].transitions[symb].begin(),
										  *dfa.states[j].transitions[symb].begin()};
						if (*dfa.states[i].transitions[symb].begin() <
							*dfa.states[j].transitions[symb].begin()) {
							to = {*dfa.states[j].transitions[symb].begin(),
								  *dfa.states[i].transitions[symb].begin()};
						}
						if (table[to[0] * dfa.size() + to[1]]) {
							table[i * dfa.size() + j] = true;
							flag = true;
						}
					}
				}
			}
			counter++;
		}
	}

	set<int> visited;
	vector<vector<int>> groups;
	counter = 1;
	for (int i = 1; i < dfa.size(); i++) {
		for (int j = 0; j < counter; j++) {
			if (!table[i * dfa.size() + j]) {
				groups.push_back({i, j});
				visited.insert(i);
				visited.insert(j);
			}
		}
		counter++;
	}

	counter = 1;
	for (int i = 0; i >= 0 && i < groups.size(); i++) {
		for (int j = counter; i >= 0 && j < groups.size(); j++) {
			bool in_first = false;
			bool in_second = false;
			if (find(groups[i].begin(), groups[i].end(), groups[j][0]) != groups[i].end()) {
				in_first = true;
			}
			if (find(groups[i].begin(), groups[i].end(), groups[j][1]) != groups[i].end()) {
				in_second = true;
			}
			if (in_first && in_second) {
				groups.erase(groups.begin() + j);
				i--;
				counter--;
				continue;
			}
			if (in_first) {
				groups[i].push_back(groups[j][1]);
				groups.erase(groups.begin() + j);
				i--;
				counter--;
			}
			if (in_second) {
				groups[i].push_back(groups[j][0]);
				groups.erase(groups.begin() + j);
				i--;
				counter--;
			}
		}
		counter++;
	}

	for (int i = 0; i < dfa.size(); i++) {
		if (visited.find(dfa.states[i].index) == visited.end()) {
			groups.push_back({dfa.states[i].index});
		}
	}

	vector<int> classes(dfa.size());
	for (int i = 0; i < groups.size(); i++) {
		for (int j = 0; j < groups[i].size(); j++) {
			classes[groups[i][j]] = i;
		}
	}
	FiniteAutomaton minimized_dfa = dfa.merge_equivalent_classes(classes);

	// кэширование
	language->set_min_dfa(
		minimized_dfa.initial_state, minimized_dfa.states, minimized_dfa.language);

	// удаление ловушки по желанию пользователя
	if (is_trim)
		minimized_dfa = minimized_dfa.remove_trap_states();

	stringstream ss;
	for (const auto& state : minimized_dfa.states) {
		ss << "\\{" << state.identifier << "\\};";
	}
	MetaInfo old_meta, new_meta;
	for (int i = 0; i < dfa.states.size(); i++) {
		for (int j = 0; j < dfa.states.size(); j++)
			if (classes[i] == classes[j] && (i != j)) {
				old_meta.upd(NodeMeta{dfa.states[i].index, classes[i]});
				new_meta.upd(NodeMeta{classes[i], classes[i]});
				break;
			}
	}

	if (log) {
		if (!is_deterministic()) {
			log->set_parameter("oldautomaton", *this);
			log->set_parameter("to_determ", "Автомат после предварительной детерминизации: ");
			log->set_parameter("detautomaton", dfa, old_meta);
		} else {
			log->set_parameter("oldautomaton", dfa, old_meta);
		}
		log->set_parameter("equivclasses", ss.str());
		log->set_parameter("result", minimized_dfa, new_meta);
	}
	return minimized_dfa;
}

FiniteAutomaton FiniteAutomaton::remove_eps(iLogTemplate* log) const {
	FiniteAutomaton new_nfa(initial_state, states, language);

	vector<State> new_states;
	map<set<int>, int> visited_states;
	MetaInfo old_meta, new_meta;
	int group_counter = 0;
	set<int> q = closure({initial_state}, true);
	string initial_state_identifier;
	for (auto elem : q) {
		initial_state_identifier +=
			(initial_state_identifier.empty() || states[elem].identifier.empty() ? "" : ", ") +
			states[elem].identifier;
	}
	State new_initial_state = {0, q, initial_state_identifier, false, map<Symbol, set<int>>()};
	if (q.size() > 1) {
		for (auto elem : q) {
			old_meta.upd(NodeMeta{states[elem].index, group_counter});
		}
		old_meta.mark_transitions(*this, q, q, Symbol::epsilon(), group_counter);
		new_meta.upd(NodeMeta{new_initial_state.index, group_counter});
		group_counter++;
	}
	visited_states[q] = 0;
	new_states.push_back(new_initial_state);

	std::stack<set<int>> s;
	s.push(q);
	set<int> x;
	int states_counter = 1;
	while (!s.empty()) {
		q = s.top();
		s.pop();
		for (const Symbol& symb : language->get_alphabet()) {
			x.clear();
			for (int k : q) {
				auto transitions_by_symbol = states[k].transitions.find(symb);
				if (transitions_by_symbol != states[k].transitions.end()) {
					for (int transition_by_symbol : transitions_by_symbol->second)
						x.insert(transition_by_symbol);
				}
			}
			set<int> q1;
			set<int> x1;
			for (int k : x) {
				x1.clear();
				q1 = closure({k}, true);
				for (int m : q1) {
					x1.insert(m);
				}
				if (!x1.empty()) {
					if (visited_states.find(x1) == visited_states.end()) {
						string new_state_identifier;
						for (auto elem : x1) {
							new_state_identifier +=
								(new_state_identifier.empty() || states[elem].identifier.empty()
									 ? ""
									 : ", ") +
								states[elem].identifier;
						}
						State new_state = {states_counter,
										   x1,
										   new_state_identifier,
										   false,
										   map<Symbol, set<int>>()};
						if (q1.size() > 1) {
							for (auto elem : q1) {
								old_meta.upd(NodeMeta{states[elem].index, group_counter});
							}

							old_meta.mark_transitions(
								*this, q1, q1, Symbol::epsilon(), group_counter);
							new_meta.upd(NodeMeta{new_state.index, group_counter});
							group_counter++;
						}
						new_states.push_back(new_state);
						visited_states[x1] = states_counter;
						s.push(x1);
						states_counter++;
					}
					new_states[visited_states[q]].transitions[symb].insert(visited_states[x1]);
				}
			}
		}
	}
	for (auto& state : new_states) {
		for (auto elem : state.label) {
			if (states[elem].is_terminal)
				state.is_terminal = true;
		}
	}
	new_nfa.initial_state = 0;
	new_nfa.states = new_states;
	new_nfa = new_nfa.remove_unreachable_states();
	if (log) {
		log->set_parameter("oldautomaton", *this, old_meta);
		log->set_parameter("result", new_nfa, new_meta);
	}
	return new_nfa;
}

FiniteAutomaton FiniteAutomaton::remove_eps_additional(iLogTemplate* log) const {
	FiniteAutomaton new_nfa(initial_state, states, language);

	for (auto& state : new_nfa.states)
		state.transitions = map<Symbol, set<int>>();

	for (int i = 0; i < states.size(); i++) {
		set<int> q = closure({states[i].index}, true);
		for (int elem : q) {
			if (states[elem].is_terminal) {
				new_nfa.states[i].is_terminal = true;
			}
		}
		vector<set<int>> x;
		for (const Symbol& symb : language->get_alphabet()) {
			x.clear();
			for (int k : q) {
				auto transitions_by_symbol = states[k].transitions.find(symb);
				if (transitions_by_symbol != states[k].transitions.end())
					x.push_back(transitions_by_symbol->second);
			}
			set<int> q1;
			set<int> x1;
			for (const auto& k : x) {
				q1 = closure(k, true);
				for (int m : q1) {
					x1.insert(m);
				}
			}
			for (auto elem : x1) {
				new_nfa.states[i].transitions[symb].insert(elem);
			}
		}
	}
	new_nfa = new_nfa.remove_unreachable_states();
	if (log) {
		log->set_parameter("oldautomaton", *this);
		log->set_parameter("result", new_nfa);
	}
	return new_nfa;
}

FiniteAutomaton FiniteAutomaton::intersection(const FiniteAutomaton& fa1,
											  const FiniteAutomaton& fa2, iLogTemplate* log) {
	set<Symbol> merged_alphabets = fa1.language->get_alphabet();
	for (const auto& symb : fa2.language->get_alphabet()) {
		merged_alphabets.insert(symb);
	}
	FiniteAutomaton new_dfa1(fa1.initial_state, fa1.states, merged_alphabets);
	FiniteAutomaton new_dfa2(fa2.initial_state, fa2.states, merged_alphabets);
	new_dfa1 = new_dfa1.determinize();
	new_dfa2 = new_dfa2.determinize();
	FiniteAutomaton new_dfa(0, {}, merged_alphabets);
	int counter = 0;
	vector<pair<int, int>> state_pair; // пары индексов состояний
	for (const auto& state1 : new_dfa1.states) {
		for (const auto& state2 : new_dfa2.states) {
			string new_identifier;
			new_identifier = state1.identifier.empty() ? "" : state1.identifier;
			new_identifier += (state2.identifier.empty() ? "" : ", " + state2.identifier);
			new_dfa.states.push_back({counter,
									  {state1.index, state2.index},
									  new_identifier,
									  state1.is_terminal && state2.is_terminal,
									  map<Symbol, set<int>>()});
			state_pair.emplace_back(state1.index, state2.index);
			counter++;
		}
	}

	for (int i = 0; i < new_dfa.size(); i++) {
		for (const Symbol& symb : new_dfa.language->get_alphabet()) {
			new_dfa.states[i].transitions[symb].insert(
				*new_dfa1.states[state_pair[i].first].transitions.at(symb).begin() *
					new_dfa2.size() +
				*new_dfa2.states[state_pair[i].second].transitions.at(symb).begin());
		}
	}
	set<Symbol> new_alphabet;
	set_intersection(fa1.language->get_alphabet().begin(),
					 fa1.language->get_alphabet().end(),
					 fa2.language->get_alphabet().begin(),
					 fa2.language->get_alphabet().end(),
					 inserter(new_alphabet, new_alphabet.begin()));
	new_dfa.language->set_alphabet(new_alphabet);
	for (int i = 0; i < new_dfa.size(); i++) {
		map<Symbol, set<int>> new_transitions;
		for (const Symbol& symb : merged_alphabets) {
			if (new_dfa.states[i].transitions.find(symb) != new_dfa.states[i].transitions.end()) {
				new_transitions[symb] = new_dfa.states[i].transitions[symb];
			}
		}
		new_dfa.states[i].transitions = new_transitions;
	}
	new_dfa = new_dfa.remove_unreachable_states();
	if (log) {
		log->set_parameter("automaton1", fa1);
		log->set_parameter("automaton2", fa2);
		log->set_parameter("result", new_dfa);
	}
	return new_dfa;
}

FiniteAutomaton FiniteAutomaton::uunion(const FiniteAutomaton& fa1, const FiniteAutomaton& fa2,
										iLogTemplate* log) {
	set<Symbol> merged_alphabets = fa1.language->get_alphabet();
	for (const auto& symb : fa2.language->get_alphabet()) {
		merged_alphabets.insert(symb);
	}
	FiniteAutomaton new_dfa1(fa1.initial_state, fa1.states, merged_alphabets);
	FiniteAutomaton new_dfa2(fa2.initial_state, fa2.states, merged_alphabets);
	new_dfa1 = new_dfa1.determinize();
	new_dfa2 = new_dfa2.determinize();
	FiniteAutomaton new_dfa(0, {}, merged_alphabets);
	int counter = 0;
	vector<pair<int, int>> state_pair; // пары индексов состояний
	for (const auto& state1 : new_dfa1.states) {
		for (const auto& state2 : new_dfa2.states) {
			string new_identifier;
			new_identifier = state1.identifier.empty() ? "" : state1.identifier;
			new_identifier += (state2.identifier.empty() ? "" : ", " + state2.identifier);
			new_dfa.states.push_back({counter,
									  {state1.index, state2.index},
									  new_identifier,
									  state1.is_terminal || state2.is_terminal,
									  map<Symbol, set<int>>()});
			state_pair.emplace_back(state1.index, state2.index);
			counter++;
		}
	}

	for (int i = 0; i < new_dfa.size(); i++) {
		for (const Symbol& symb : new_dfa.language->get_alphabet()) {
			new_dfa.states[i].transitions[symb].insert(
				*new_dfa1.states[state_pair[i].first].transitions.at(symb).begin() *
					new_dfa2.size() +
				*new_dfa2.states[state_pair[i].second].transitions.at(symb).begin());
		}
	}
	new_dfa = new_dfa.remove_unreachable_states();
	if (log) {
		log->set_parameter("automaton1", fa1);
		log->set_parameter("automaton2", fa2);
		log->set_parameter("result", new_dfa);
	}
	return new_dfa;
}

FiniteAutomaton FiniteAutomaton::difference(const FiniteAutomaton& fa1, const FiniteAutomaton& fa2,
											iLogTemplate* log) {
	set<Symbol> merged_alphabets = fa1.language->get_alphabet();
	for (const auto& symb : fa2.language->get_alphabet()) {
		merged_alphabets.insert(symb);
	}
	FiniteAutomaton new_dfa1(fa1.initial_state, fa1.states, merged_alphabets);
	FiniteAutomaton new_dfa2(fa2.initial_state, fa2.states, merged_alphabets);
	new_dfa1 = new_dfa1.determinize();
	new_dfa2 = new_dfa2.determinize();
	FiniteAutomaton new_dfa(0, {}, merged_alphabets);
	int counter = 0;
	vector<pair<int, int>> state_pair; // пары индексов состояний
	for (const auto& state1 : new_dfa1.states) {
		for (const auto& state2 : new_dfa2.states) {
			string new_identifier;
			new_identifier = state1.identifier.empty() ? "" : state1.identifier;
			new_identifier += (state2.identifier.empty() ? "" : ", " + state2.identifier);
			new_dfa.states.push_back({counter,
									  {state1.index, state2.index},
									  new_identifier,
									  state1.is_terminal && !state2.is_terminal,
									  map<Symbol, set<int>>()});
			state_pair.emplace_back(state1.index, state2.index);
			counter++;
		}
	}

	for (int i = 0; i < new_dfa.size(); i++) {
		for (const Symbol& symb : new_dfa.language->get_alphabet()) {
			new_dfa.states[i].transitions[symb].insert(
				*new_dfa1.states[state_pair[i].first].transitions.at(symb).begin() *
					new_dfa2.size() +
				*new_dfa2.states[state_pair[i].second].transitions.at(symb).begin());
		}
	}
	new_dfa.language->set_alphabet(fa1.language->get_alphabet());
	for (int i = 0; i < new_dfa.size(); i++) {
		map<Symbol, set<int>> new_transitions;
		for (const Symbol& symb : merged_alphabets) {
			if (new_dfa.states[i].transitions.find(symb) != new_dfa.states[i].transitions.end()) {
				new_transitions[symb] = new_dfa.states[i].transitions[symb];
			}
		}
		new_dfa.states[i].transitions = new_transitions;
	}
	new_dfa = new_dfa.remove_unreachable_states();
	if (log) {
		log->set_parameter("automaton1", fa1);
		log->set_parameter("automaton2", fa2);
		log->set_parameter("result", new_dfa);
	}
	return new_dfa;
}

FiniteAutomaton FiniteAutomaton::complement(iLogTemplate* log) const {
	FiniteAutomaton new_dfa = FiniteAutomaton(initial_state, states, language->get_alphabet());
	new_dfa = new_dfa.add_trap_state();
	for (int i = 0; i < new_dfa.size(); i++) {
		new_dfa.states[i].is_terminal = !new_dfa.states[i].is_terminal;
	}
	int final_states_counter = 0;
	for (int i = 0; i < new_dfa.size(); i++)
		if (new_dfa.states[i].is_terminal)
			final_states_counter++;
	if (!final_states_counter)
		new_dfa = new_dfa.minimize();
	if (log) {
		log->set_parameter("oldautomaton", *this);
		log->set_parameter("result", new_dfa);
	}
	return new_dfa;
}
FiniteAutomaton FiniteAutomaton::reverse(iLogTemplate* log) const {
	FiniteAutomaton enfa = FiniteAutomaton(states.size(), states, language->get_alphabet());
	int final_states_counter = 0;
	for (int i = 0; i < enfa.size(); i++) {
		if (enfa.states[i].is_terminal) {
			final_states_counter++;
		}
	}
	int final_states_flag = 0;
	if (final_states_counter > 1) {
		final_states_flag = 1;
		enfa.states.push_back(
			{enfa.initial_state, {enfa.initial_state}, "RevS", false, map<Symbol, set<int>>()});
	}
	if (final_states_counter) {
		for (int i = 0; i < enfa.size() - final_states_flag; i++) {
			if (enfa.states[i].is_terminal) {
				enfa.states[i].is_terminal = false;
				if (final_states_counter > 1) {
					enfa.states[enfa.initial_state].transitions[Symbol::epsilon()].insert(i);
				} else {
					enfa.initial_state = i;
				}
			}
		}
		enfa.states[initial_state].is_terminal = true;
		vector<map<Symbol, set<int>>> new_transition_matrix(enfa.size() - final_states_flag);
		for (int i = 0; i < enfa.size() - final_states_flag; i++) {
			for (const auto& transition : enfa.states[i].transitions) {
				for (int elem : transition.second) {
					new_transition_matrix[elem][transition.first].insert(enfa.states[i].index);
				}
			}
		}
		for (int i = 0; i < enfa.size() - final_states_flag; i++) {
			enfa.states[i].transitions = new_transition_matrix[i];
		}
	} else {
		enfa.initial_state = initial_state;
	}
	enfa = enfa.remove_unreachable_states();
	if (log) {
		log->set_parameter("oldautomaton", *this);
		log->set_parameter("result", enfa);
	}
	return enfa;
}

FiniteAutomaton FiniteAutomaton::add_trap_state(iLogTemplate* log) const {
	FiniteAutomaton new_dfa(initial_state, states, language->get_alphabet());
	bool flag = true;
	MetaInfo new_meta;
	int count = static_cast<int>(new_dfa.size());
	for (int i = 0; i < count; i++) {
		for (const Symbol& symb : language->get_alphabet()) {
			if (new_dfa.states[i].transitions[symb].empty()) {
				if (flag) {
					new_dfa.states[i].set_transition(new_dfa.size(), symb);
					new_dfa.states.push_back({count, {count}, "", false, map<Symbol, set<int>>()});
					new_meta.upd(
						EdgeMeta{new_dfa.states[i].index, count, symb, MetaInfo::trap_color});
				} else {
					new_dfa.states[i].set_transition(count, symb);
					new_meta.upd(
						EdgeMeta{new_dfa.states[i].index, count, symb, MetaInfo::trap_color});
				}
				flag = false;
			}
		}
	}
	if (!flag) {
		for (const Symbol& symb : language->get_alphabet()) {
			new_dfa.states[count].transitions[symb].insert(count);
			new_meta.upd(EdgeMeta{count, count, symb, MetaInfo::trap_color});
		}
	}
	if (log) {
		log->set_parameter("oldautomaton", *this);
		log->set_parameter("result", new_dfa, new_meta);
	}
	return new_dfa;
}

FiniteAutomaton FiniteAutomaton::remove_trap_states(iLogTemplate* log) const {
	// тест Regex("(a|b)*a").get_one_unambiguous_regex() ломается, если оставить этот вариант:
	// FiniteAutomaton new_dfa(initial_state, states, language->get_alphabet());
	FiniteAutomaton new_dfa(*this);
	int count = static_cast<int>(new_dfa.size());
	// Поправка, чтобы можно было вычислить реальное число состояний прежнего автомата.
	int traps = 0;
	MetaInfo old_meta;
	for (int i = 0; i >= 0 && i < count; i++) {
		bool is_trap_state = true;
		set<int> reachable_states = new_dfa.closure({i}, false);
		for (int j = 0; j < new_dfa.size(); j++) {
			if (new_dfa.states[j].is_terminal) {
				for (auto elem : reachable_states) {
					if (j == elem) {
						is_trap_state = false;
					}
				}
				if (!is_trap_state) {
					break;
				}
			}
		}
		if (is_trap_state) {
			old_meta.upd(NodeMeta{states[i + traps].index, MetaInfo::trap_color});
			vector<State> new_states;
			for (int j = 0; j < new_dfa.size(); j++) {
				if (j < i) {
					new_states.push_back(new_dfa.states[j]);
				}
				if (j > i && i != count - 1) {
					new_states.emplace_back(new_dfa.states[j].index - 1,
											new_dfa.states[j].label,
											new_dfa.states[j].identifier,
											new_dfa.states[j].is_terminal,
											new_dfa.states[j].transitions);
				}
			}
			if (new_dfa.initial_state > i)
				new_dfa.initial_state -= 1;
			new_dfa.states = new_states;
			for (int j = 0; j < new_dfa.size(); j++) {
				map<Symbol, set<int>> new_transitions;
				for (auto& transition : new_dfa.states[j].transitions) {
					set<int> new_transition;
					for (int transition_to : transition.second) {
						if (transition_to < i) {
							new_transition.insert(transition_to);
						}
						if (transition_to > i) {
							new_transition.insert(transition_to - 1);
						}
					}
					if (!new_transition.empty())
						new_transitions[transition.first] = new_transition;
				}
				new_dfa.states[j].transitions = new_transitions;
			}
			i--;
			traps++;
			count--;
		}
	}
	/* Если весь автомат состоит из ловушек, то останется лишь одна из них. */
	if (new_dfa.is_empty()) {
		new_dfa = *this;
		new_dfa = new_dfa.minimize();
	}
	if (log) {
		log->set_parameter("oldautomaton", *this, old_meta);
		log->set_parameter("result", new_dfa);
	}
	return new_dfa;
}

FiniteAutomaton FiniteAutomaton::remove_unreachable_states() const {
	if (states.size() == 1)
		return *this;
	FiniteAutomaton new_dfa(initial_state, states, language);
	int count = new_dfa.size();
	for (int i = 0; i >= 0 && i < count; i++) {
		bool is_unreachable_state = false;
		set<int> reachable_states = new_dfa.closure({new_dfa.initial_state}, false);
		if (reachable_states.find(i) == reachable_states.end()) {
			is_unreachable_state = true;
		}
		if (is_unreachable_state) {
			vector<State> new_states;
			for (int j = 0; j < new_dfa.size(); j++) {
				if (j < i) {
					new_states.push_back(new_dfa.states[j]);
				}
				if (j > i && i != count - 1) {
					new_states.emplace_back(new_dfa.states[j].index - 1,
											new_dfa.states[j].label,
											new_dfa.states[j].identifier,
											new_dfa.states[j].is_terminal,
											new_dfa.states[j].transitions);
				}
			}
			if (new_dfa.initial_state > i)
				new_dfa.initial_state -= 1;
			new_dfa.states = new_states;
			for (int j = 0; j < new_dfa.size(); j++) {
				map<Symbol, set<int>> new_transitions;
				for (auto& transition : new_dfa.states[j].transitions) {
					set<int> new_transition;
					for (int transition_to : transition.second) {
						if (transition_to < i) {
							new_transition.insert(transition_to);
						}
						if (transition_to > i) {
							new_transition.insert(transition_to - 1);
						}
					}
					if (!new_transition.empty())
						new_transitions[transition.first] = new_transition;
				}
				new_dfa.states[j].transitions = new_transitions;
			}
			i--;
			count--;
		}
	}
	if (new_dfa.is_empty())
		new_dfa = *this;
	return new_dfa;
}

FiniteAutomaton FiniteAutomaton::annote(iLogTemplate* log) const {
	set<Symbol> new_alphabet;
	MetaInfo meta;
	int group_id = 1;
	FiniteAutomaton new_fa = FiniteAutomaton(initial_state, states, make_shared<Language>());
	vector<map<Symbol, set<int>>> new_transitions(new_fa.size());
	for (int i = 0; i < new_fa.size(); i++) {
		for (const auto& elem : new_fa.states[i].transitions) {
			if (elem.second.size() > 1) {
				meta.mark_transitions(*this, {i}, elem.second, elem.first, group_id);
				group_id++;
				int counter = 1;
				for (int transition_to : elem.second) {
					Symbol new_symb = elem.first;
					new_symb.annote(counter);
					new_transitions[i][new_symb].insert(transition_to);
					new_alphabet.insert(new_symb);
					counter++;
				}
			} else {
				new_transitions[i][elem.first] = elem.second;
				if (!elem.first.is_epsilon()) {
					new_alphabet.insert(elem.first);
				}
			}
		}
	}
	new_fa.language = make_shared<Language>(new_alphabet);
	for (int i = 0; i < new_transitions.size(); i++) {
		new_fa.states[i].transitions = new_transitions[i];
	}
	if (log) {
		log->set_parameter("oldautomaton", *this, meta);
		log->set_parameter("result", new_fa);
	}
	return new_fa;
}

FiniteAutomaton FiniteAutomaton::deannote(iLogTemplate* log) const {
	set<Symbol> new_alphabet;
	FiniteAutomaton new_fa = FiniteAutomaton(initial_state, states, make_shared<Language>());
	vector<map<Symbol, set<int>>> new_transitions(new_fa.size());
	for (int i = 0; i < new_fa.size(); i++) {
		for (const auto& elem : new_fa.states[i].transitions) {
			Symbol new_symb = elem.first;
			if (elem.first.is_annotated()) {
				new_symb.deannote();
				for (int transition_to : elem.second) {
					new_transitions[i][new_symb].insert(transition_to);
				}
			} else {
				new_transitions[i][new_symb] = elem.second;
			}
			if (!new_symb.is_epsilon())
				new_alphabet.insert(new_symb);
		}
	}
	new_fa.language = make_shared<Language>(new_alphabet);
	for (int i = 0; i < new_transitions.size(); i++) {
		new_fa.states[i].transitions = new_transitions[i];
	}
	new_fa = new_fa.remove_trap_states();
	if (log) {
		log->set_parameter("oldautomaton", *this);
		log->set_parameter("result", new_fa);
	}
	return new_fa;
}

FiniteAutomaton FiniteAutomaton::delinearize(iLogTemplate* log) const {
	set<Symbol> new_alphabet;
	FiniteAutomaton new_fa = FiniteAutomaton(initial_state, states, make_shared<Language>());
	vector<map<Symbol, set<int>>> new_transitions(new_fa.size());
	for (int i = 0; i < new_fa.size(); i++) {
		for (const auto& elem : new_fa.states[i].transitions) {
			Symbol new_symb = elem.first;
			if (elem.first.is_linearized()) {
				new_symb.delinearize();
				for (int transition_to : elem.second) {
					new_transitions[i][new_symb].insert(transition_to);
				}
			} else {
				new_transitions[i][new_symb] = elem.second;
			}
			if (!new_symb.is_epsilon())
				new_alphabet.insert(new_symb);
		}
	}
	new_fa.language = make_shared<Language>(new_alphabet);
	for (int i = 0; i < new_transitions.size(); i++) {
		new_fa.states[i].transitions = new_transitions[i];
	}
	new_fa = new_fa.remove_trap_states();
	if (log) {
		log->set_parameter("oldautomaton", *this);
		log->set_parameter("result", new_fa);
	}
	return new_fa;
}

bool FiniteAutomaton::is_one_unambiguous(iLogTemplate* log) const {
	if (log)
		log->set_parameter("oldautomaton", *this);
	MetaInfo meta;
	if (language->is_one_unambiguous_flag_cached()) {
		if (log) {
			log->set_parameter("result", language->get_one_unambiguous_flag() ? "True" : "False");

			log->set_parameter("cach", "(!) результат OneUnambiguous получен из кэша");
		}
		return language->get_one_unambiguous_flag();
	}
	if (language->is_min_dfa_cached() && log) {
		log->set_parameter("cachedMINDFA", "Минимальный автомат сохранен в кэше");
	}

	FiniteAutomaton min_fa = minimize(true);

	set<map<Symbol, set<int>>> final_states_transitions;
	for (int i = 0; i < min_fa.size(); i++) {
		if (min_fa.states[i].is_terminal) {
			final_states_transitions.insert(min_fa.states[i].transitions);
		}
	}

	set<Symbol> min_fa_consistent;
	// calculate a set of min_fa_consistent symbols
	for (const Symbol& symb : min_fa.language->get_alphabet()) {
		set<int> reachable_by_symb;
		bool is_symb_min_fa_consistent = true;
		for (int i = 0; i < min_fa.size(); i++) {
			for (const auto& transition : min_fa.states[i].transitions) {
				if (transition.first == symb) {
					for (int elem : transition.second) {
						reachable_by_symb.insert(elem);
					}
				}
			}
		}
		for (int elem : reachable_by_symb) {
			is_symb_min_fa_consistent = true;
			for (auto final_state_transitions : final_states_transitions) {
				if (find(final_state_transitions[symb].begin(),
						 final_state_transitions[symb].end(),
						 elem) == final_state_transitions[symb].end()) {
					is_symb_min_fa_consistent = false;
					break;
				}
			}
			if (is_symb_min_fa_consistent)
				min_fa_consistent.insert(symb);
		}
	}

	// calculate an orbit of each state
	// search for strongly connected component of each state
	set<int> states_with_trivial_orbit;
	set<set<int>> min_fa_orbits;
	for (int i = 0; i < min_fa.size(); i++) {
		set<int> orbit_of_state;
		orbit_of_state.insert(i);
		set<int> reachable_states = min_fa.closure({i}, false);
		for (int reachable_state : reachable_states) {
			set<int> reachable_states_for_reachable = min_fa.closure({reachable_state}, false);
			if (reachable_states_for_reachable.find(i) != reachable_states_for_reachable.end()) {
				orbit_of_state.insert(reachable_state);
			}
		}
		bool is_state_has_transitions_to_itself = false;
		for (const auto& transition : min_fa.states[i].transitions) {
			for (int elem : transition.second) {
				if (elem == i)
					is_state_has_transitions_to_itself = true;
			}
		}
		// check if orbit of this state is trivial
		// if so, insert into states_with_trivial_orbit
		if (orbit_of_state.size() == 1 && !is_state_has_transitions_to_itself) {
			states_with_trivial_orbit.insert(i);
		}
		min_fa_orbits.insert(orbit_of_state);
	}

	int curr_orbit = 0;
	for (const auto& iter_orbit : min_fa_orbits)
		if (iter_orbit.size() > 1) {
			for (auto iter_state : iter_orbit)
				meta.upd(NodeMeta{iter_state, curr_orbit});
			for (const auto& symbol : language->get_alphabet())
				meta.mark_transitions(min_fa, iter_orbit, iter_orbit, symbol, curr_orbit);
			curr_orbit++;
		}

	// check if min_fa has a single, trivial orbit
	// return true if it exists
	if (min_fa_orbits.size() == 1 && states_with_trivial_orbit.size() == 1) {
		language->set_one_unambiguous_flag(true);
		if (log) {
			log->set_parameter("mindfa", min_fa, meta);
			log->set_parameter("result", "True");
		}
		return true;
	}

	// check if min_fa has a single, nontrivial orbit and
	// min_fa_consistent.size() is 0
	// return true if it exists
	if (min_fa_orbits.size() == 1 && states_with_trivial_orbit.empty() &&
		min_fa_consistent.empty()) {
		language->set_one_unambiguous_flag(false);
		if (log) {
			log->set_parameter("mindfa", min_fa, meta);
			log->set_parameter("result", "False");
		}
		return false;
	}

	// construct a min_fa_consistent cut of min_fa
	// to construct it, we will remove for each symb in min_fa_consistent
	// all symb-transitions that leave a final state of min_fa
	FiniteAutomaton min_fa_cut =
		FiniteAutomaton(min_fa.initial_state, min_fa.states, min_fa.language);

	for (int i = 0; i < min_fa.size(); i++) {
		if (min_fa.states[i].is_terminal) {
			map<Symbol, set<int>> new_transitions;
			for (const auto& transition : min_fa.states[i].transitions) {
				if (min_fa_consistent.find(transition.first) == min_fa_consistent.end()) {
					new_transitions[transition.first] = transition.second;
				}
			}
			min_fa_cut.states[i].transitions = new_transitions;
		}
	}

	// calculate the orbits of min_fa_cut
	set<set<int>> min_fa_cut_orbits;
	vector<set<int>> min_fa_cut_orbits_of_states;
	for (int i = 0; i < min_fa_cut.size(); i++) {
		set<int> orbit_of_state;
		set<int> reachable_states = min_fa_cut.closure({i}, false);
		for (int reachable_state : reachable_states) {
			set<int> reachable_states_for_reachable = min_fa_cut.closure({reachable_state}, false);
			if (reachable_states_for_reachable.find(i) != reachable_states_for_reachable.end()) {
				orbit_of_state.insert(reachable_state);
			}
		}
		min_fa_cut_orbits.insert(orbit_of_state);
		min_fa_cut_orbits_of_states.push_back(orbit_of_state);
	}

	// calculate gates of each orbit of min_fa_cut
	vector<set<int>> min_fa_cut_gates;
	for (auto min_fa_cut_orbit : min_fa_cut_orbits) {
		set<int> gates_of_orbit;
		for (auto elem : min_fa_cut_orbit) {
			if (min_fa_cut.states[elem].is_terminal) {
				gates_of_orbit.insert(elem);
				continue;
			}
			bool is_exists_transition_outside_orbit = false;
			for (const auto& transition : min_fa_cut.states[elem].transitions) {
				for (int elem1 : transition.second) {
					if (find(min_fa_cut_orbit.begin(), min_fa_cut_orbit.end(), elem1) ==
						min_fa_cut_orbit.end()) {
						is_exists_transition_outside_orbit = true;
					}
				}
			}
			if (is_exists_transition_outside_orbit)
				gates_of_orbit.insert(elem);
		}
		min_fa_cut_gates.push_back(gates_of_orbit);
	}

	// check if min_fa_cut has an orbit property
	// we need to show that all the gates of each orbit have identical
	// connections to the outside world
	bool is_min_fa_cut_has_an_orbit_property = true;
	for (auto min_fa_cut_orbit_gates : min_fa_cut_gates) {
		auto it1 = min_fa_cut_orbit_gates.begin();
		for (int i = 0; i < min_fa_cut_orbit_gates.size(); i++) {
			map<Symbol, set<int>> q1_transitions_outside_orbit;
			for (const Symbol& symb : min_fa_cut.language->get_alphabet()) {
				set<int> q1_symb_transitions_outside_orbit;
				for (int transition : min_fa_cut.states[*it1].transitions[symb]) {
					bool is_transition_outside_orbit = true;
					for (int elem : min_fa_cut_orbits_of_states[*it1]) {
						if (transition == elem) {
							is_transition_outside_orbit = false;
						}
					}
					if (is_transition_outside_orbit) {
						q1_symb_transitions_outside_orbit.insert(transition);
					}
				}
				q1_transitions_outside_orbit[symb] = q1_symb_transitions_outside_orbit;
			}
			auto it2 = it1;
			for (int j = i; j < min_fa_cut_orbit_gates.size(); j++) {
				// check if for any pair q1 and q2 of gates in the same orbit
				// q1 is final if and only if q2 is final
				if (min_fa_cut.states[i].is_terminal != min_fa_cut.states[j].is_terminal) {
					is_min_fa_cut_has_an_orbit_property = false;
				}
				map<Symbol, set<int>> q2_transitions_outside_orbit;
				for (const Symbol& symb : min_fa_cut.language->get_alphabet()) {
					set<int> q2_symb_transitions_outside_orbit;
					for (int transition : min_fa_cut.states[*it2].transitions[symb]) {
						bool is_transition_outside_orbit = true;
						for (int elem : min_fa_cut_orbits_of_states[*it2]) {
							if (transition == elem) {
								is_transition_outside_orbit = false;
							}
						}
						if (is_transition_outside_orbit) {
							q2_symb_transitions_outside_orbit.insert(transition);
						}
					}
					q2_transitions_outside_orbit[symb] = q2_symb_transitions_outside_orbit;
					// check if for all states q outside the orbit of q1 and q2
					// there is a transition (q1, a, q) in min_fa_cut
					// if and only if there is a transition (q2, a, q) in
					// min_fa_cut
					if (q1_transitions_outside_orbit[symb] != q2_symb_transitions_outside_orbit) {
						is_min_fa_cut_has_an_orbit_property = false;
					}
				}
				++it2;
			}
			++it1;
		}
	}
	if (!is_min_fa_cut_has_an_orbit_property) {
		language->set_one_unambiguous_flag(false);
		if (log) {
			log->set_parameter("mindfa", min_fa, meta);
			log->set_parameter("result", "False");
		}
		return false;
	}

	// check if all orbit languages of min_fa_cut are 1-unambiguous
	int i = 0;
	for (auto min_fa_cut_orbit : min_fa_cut_orbits) {
		int orbit_automaton_initial_state = 0;
		for (int state_of_orbit : min_fa_cut_orbit) {
			// construction of an orbit automaton for a state_of_orbit
			FiniteAutomaton orbit_automaton =
				FiniteAutomaton(orbit_automaton_initial_state, {}, make_shared<Language>());
			for (int elem : min_fa_cut_orbit) {
				orbit_automaton.states.push_back(min_fa_cut.states[elem]);
				orbit_automaton.states[orbit_automaton.size() - 1].index =
					orbit_automaton.size() - 1;
				orbit_automaton.states[orbit_automaton.size() - 1].is_terminal = false;
				if (find(min_fa_cut_gates[i].begin(), min_fa_cut_gates[i].end(), elem) !=
					min_fa_cut_gates[i].end()) {
					orbit_automaton.states[orbit_automaton.size() - 1].is_terminal = true;
				}
			}
			set<Symbol> orbit_automaton_alphabet;
			for (int j = 0; j < orbit_automaton.size(); j++) {
				map<Symbol, set<int>> orbit_automaton_state_transitions;
				for (const auto& symb_transitions : orbit_automaton.states[j].transitions) {
					set<int> orbit_automaton_symb_transitions;
					int k = 0;
					for (int transition : symb_transitions.second) {
						if (find(min_fa_cut_orbit.begin(), min_fa_cut_orbit.end(), transition) !=
							min_fa_cut_orbit.end()) {
							orbit_automaton_symb_transitions.insert(k);
							k++;
						}
					}
					if (!orbit_automaton_symb_transitions.empty()) {
						orbit_automaton_state_transitions[symb_transitions.first] =
							orbit_automaton_symb_transitions;
						orbit_automaton_alphabet.insert(symb_transitions.first);
					}
				}
				orbit_automaton.states[j].transitions = orbit_automaton_state_transitions;
			}
			orbit_automaton.language = make_shared<Language>(orbit_automaton_alphabet);
			if (!orbit_automaton.is_one_unambiguous()) {
				language->set_one_unambiguous_flag(false);
				if (log) {
					log->set_parameter("mindfa", min_fa, meta);
					log->set_parameter("result", "False");
				}
				return false;
			}
			orbit_automaton_initial_state++;
		}
		i++;
	}
	language->set_one_unambiguous_flag(true);
	if (log) {
		log->set_parameter("mindfa", min_fa, meta);
		log->set_parameter("result", "True");
	}
	return true;
}

FiniteAutomaton FiniteAutomaton::merge_equivalent_classes(vector<int> classes) const {
	map<int, vector<int>> class_to_index; // нужен для подсчета количества классов
	for (int i = 0; i < classes.size(); i++)
		class_to_index[classes[i]].push_back(i);
	// индексы состояний в новом автомате соответствуют номеру класса эквивалентности
	vector<State> new_states;
	for (int i = 0; i < class_to_index.size(); i++) {
		string new_identifier;
		for (int index : class_to_index[i]) {
			new_identifier +=
				(new_identifier.empty() || states[index].identifier.empty() ? "" : ", ") +
				states[index].identifier;
		}
		new_states.emplace_back(i, set<int>({i}), new_identifier, false, map<Symbol, set<int>>());
	}

	for (int i = 0; i < states.size(); i++) {
		int from = classes[i];
		for (const auto& elem : states[i].transitions) {
			for (int transition_to : elem.second) {
				int to = classes[transition_to];
				new_states[from].transitions[elem.first].insert(to);
			}
		}
	}

	for (const auto& elem : class_to_index)
		if (states[elem.second[0]].is_terminal)
			new_states[elem.first].is_terminal = true;

	return {classes[initial_state], new_states, language};
}

FiniteAutomaton FiniteAutomaton::merge_bisimilar(iLogTemplate* log) const {
	vector<GrammarItem> fa_items;
	vector<GrammarItem*> nonterminals;
	vector<GrammarItem*> terminals;
	MetaInfo old_meta, new_meta;
	vector<vector<vector<GrammarItem*>>> rules =
		Grammar::fa_to_grammar(states, language->get_alphabet(), fa_items, nonterminals, terminals);
	vector<GrammarItem*> bisimilar_nonterminals;
	map<int, vector<GrammarItem*>> class_to_nonterminals;
	vector<vector<vector<GrammarItem*>>> bisimilar_rules = Grammar::get_bisimilar_grammar(
		rules, nonterminals, bisimilar_nonterminals, class_to_nonterminals);

	// log
	vector<int> classes;
	for (const auto& nont : nonterminals)
		classes.push_back(nont->class_number);
	FiniteAutomaton result_fa = merge_equivalent_classes(classes);

	for (int i = 0; i < classes.size(); i++) {
		for (int j = 0; j < classes.size(); j++)
			if (classes[i] == classes[j] && (i != j)) {
				old_meta.upd(NodeMeta{i, classes[i]});
				new_meta.upd(NodeMeta{classes[i], classes[i]});
			}
	}

	stringstream ss;
	for (auto& elem : class_to_nonterminals) {
		ss << "\\{";
		for (int i = 0; i < elem.second.size() - 1; i++)
			ss << elem.second[i]->name << ",\\ ";
		ss << elem.second[elem.second.size() - 1]->name << "\\};";
	}
	if (log) {
		log->set_parameter("oldautomaton", *this, old_meta);
		log->set_parameter("equivclasses", ss.str()); // TODO: logs
		log->set_parameter("result", result_fa, new_meta);
	}
	return result_fa;
}

bool FiniteAutomaton::bisimilarity_checker(const FiniteAutomaton& fa1, const FiniteAutomaton& fa2) {
	// грамматики из автоматов
	vector<GrammarItem> fa1_items;
	vector<GrammarItem*> fa1_nonterminals;
	vector<GrammarItem*> fa1_terminals;
	vector<vector<vector<GrammarItem*>>> fa1_rules = Grammar::fa_to_grammar(
		fa1.states, fa1.language->get_alphabet(), fa1_items, fa1_nonterminals, fa1_terminals);

	vector<GrammarItem> fa2_items;
	vector<GrammarItem*> fa2_nonterminals;
	vector<GrammarItem*> fa2_terminals;
	vector<vector<vector<GrammarItem*>>> fa2_rules = Grammar::fa_to_grammar(
		fa2.states, fa2.language->get_alphabet(), fa2_items, fa2_nonterminals, fa2_terminals);

	if (fa1_terminals.size() != fa2_terminals.size())
		return false;
	for (int i = 0; i < fa1_terminals.size(); i++)
		if (*fa1_terminals[i] != *fa2_terminals[i])
			return false;
	// сначала получаем бисимилярные грамматики из данных автоматов
	vector<GrammarItem*> fa1_bisimilar_nonterminals;
	map<int, vector<GrammarItem*>> fa1_class_to_nonterminals;
	vector<vector<vector<GrammarItem*>>> fa1_bisimilar_rules = Grammar::get_bisimilar_grammar(
		fa1_rules, fa1_nonterminals, fa1_bisimilar_nonterminals, fa1_class_to_nonterminals);

	vector<GrammarItem*> fa2_bisimilar_nonterminals;
	map<int, vector<GrammarItem*>> fa2_class_to_nonterminals;
	vector<vector<vector<GrammarItem*>>> fa2_bisimilar_rules = Grammar::get_bisimilar_grammar(
		fa2_rules, fa2_nonterminals, fa2_bisimilar_nonterminals, fa2_class_to_nonterminals);
	if (fa1_bisimilar_nonterminals.size() != fa2_bisimilar_nonterminals.size())
		return false;
	// из объединения полученных ранее получаем итоговую
	vector<GrammarItem*> nonterminals(fa1_bisimilar_nonterminals);
	nonterminals.insert(
		nonterminals.end(), fa2_bisimilar_nonterminals.begin(), fa2_bisimilar_nonterminals.end());
	vector<vector<vector<GrammarItem*>>> rules(fa1_bisimilar_rules);
	rules.insert(rules.end(), fa2_bisimilar_rules.begin(), fa2_bisimilar_rules.end());

	vector<int> fa1_classes; // сохраняю классы
	for (GrammarItem* nont : fa1_nonterminals) {
		fa1_classes.push_back(nont->class_number);
		nont->class_number = -1; // сбрасываю номера классов
	}
	vector<int> fa2_classes; // сохраняю классы
	for (GrammarItem* nont : fa2_nonterminals) {
		fa2_classes.push_back(nont->class_number);
		nont->class_number = -1; // сбрасываю номера классов
	}

	vector<GrammarItem*> bisimilar_nonterminals;
	map<int, vector<GrammarItem*>> class_to_nonterminals;
	vector<vector<vector<GrammarItem*>>> bisimilar_rules = Grammar::get_bisimilar_grammar(
		rules, nonterminals, bisimilar_nonterminals, class_to_nonterminals);

	map<int, vector<string>> class_to_nonterminals_names;

	for (int i = 0; i < fa1_nonterminals.size(); i++) {
		int nont_class = fa1_class_to_nonterminals[fa1_classes[i]][0]
							 ->class_number; // класс нетерминала в общей грамматике, 0й
											 // элемент попал в бисимилярную грамматику
		class_to_nonterminals_names[nont_class].push_back("FA1:" + fa1_nonterminals[i]->name);
	}

	for (int i = 0; i < fa2_nonterminals.size(); i++) {
		int nont_class = fa2_class_to_nonterminals[fa2_classes[i]][0]
							 ->class_number; // класс нетерминала в общей грамматике, 0й
											 // элемент попал в бисимилярную грамматику
		class_to_nonterminals_names[nont_class].push_back("FA2:" + fa2_nonterminals[i]->name);
	}
	// log
	stringstream ss;
	for (auto& elem : class_to_nonterminals_names) {
		ss << "\\{";
		for (int i = 0; i < elem.second.size() - 1; i++)
			ss << elem.second[i] << ",";
		ss << elem.second[elem.second.size() - 1] << "\\}";
	}

	// проверяю равенство классов начальных состояний
	if (fa1_nonterminals[fa1.initial_state]->class_number !=
		fa2_nonterminals[fa2.initial_state]->class_number)
		return false;
	if (fa1_bisimilar_nonterminals.size() != bisimilar_nonterminals.size())
		return false;

	return true;
}

bool FiniteAutomaton::bisimilar(const FiniteAutomaton& fa1, const FiniteAutomaton& fa2,
								iLogTemplate* log) {
	bool result = bisimilarity_checker(fa1, fa2);
	if (log) {
		log->set_parameter("automaton1", fa1);
		log->set_parameter("automaton2", fa2);
		log->set_parameter("result", result);
	}
	return result;
}

bool FiniteAutomaton::equality_checker(const FiniteAutomaton& fa1, const FiniteAutomaton& fa2) {
	// проверка равенства количества состояний
	if (fa1.size() != fa2.size())
		return false;
	// грамматики из состояний автоматов
	vector<GrammarItem> fa1_items;
	vector<GrammarItem*> fa1_nonterminals;
	vector<GrammarItem*> fa1_terminals;
	vector<vector<vector<GrammarItem*>>> fa1_rules = Grammar::fa_to_grammar(
		fa1.states, fa1.language->get_alphabet(), fa1_items, fa1_nonterminals, fa1_terminals);

	vector<GrammarItem> fa2_items;
	vector<GrammarItem*> fa2_nonterminals;
	vector<GrammarItem*> fa2_terminals;
	vector<vector<vector<GrammarItem*>>> fa2_rules = Grammar::fa_to_grammar(
		fa2.states, fa2.language->get_alphabet(), fa2_items, fa2_nonterminals, fa2_terminals);
	// проверка на равенство алфавитов
	if (fa1_terminals.size() != fa2_terminals.size())
		return false;
	for (int i = 0; i < fa1_terminals.size(); i++)
		if (*fa1_terminals[i] != *fa2_terminals[i])
			return false;
	// биективная бисимуляция состояний
	vector<GrammarItem*> nonterminals(fa1_nonterminals);
	nonterminals.insert(nonterminals.end(), fa2_nonterminals.begin(), fa2_nonterminals.end());
	vector<vector<vector<GrammarItem*>>> rules(fa1_rules);
	rules.insert(rules.end(), fa2_rules.begin(), fa2_rules.end());
	for (GrammarItem* nont : nonterminals)
		nont->class_number = 0; // сбрасываю номера классов
	vector<GrammarItem*> bisimilar_nonterminals;
	map<int, vector<GrammarItem*>> class_to_nonterminals;
	vector<vector<vector<GrammarItem*>>> bisimilar_rules = Grammar::get_bisimilar_grammar(
		rules, nonterminals, bisimilar_nonterminals, class_to_nonterminals);
	// проверяю равенство классов начальных состояний
	if (fa1_nonterminals[fa1.initial_state]->class_number !=
		fa2_nonterminals[fa2.initial_state]->class_number)
		return false;
	// проверяю бисимилярность состояний
	vector<int> classes(bisimilar_nonterminals.size(), 0);
	for (GrammarItem* nont : fa1_nonterminals)
		classes[nont->class_number]++;
	for (GrammarItem* nont : fa2_nonterminals)
		classes[nont->class_number]--;
	for (auto t : classes)
		if (t != 0)
			return false;

	vector<int> bisimilar_classes;
	for (GrammarItem* nont : nonterminals)
		bisimilar_classes.push_back(nont->class_number);
	// биективная бисимуляция обратных грамматик
	vector<vector<vector<GrammarItem*>>> fa1_reverse_rules =
		Grammar::get_reverse_grammar(fa1_rules, fa1_nonterminals, fa1_terminals, fa1.initial_state);
	vector<vector<vector<GrammarItem*>>> fa2_reverse_rules =
		Grammar::get_reverse_grammar(fa2_rules, fa2_nonterminals, fa2_terminals, fa2.initial_state);

	vector<vector<vector<GrammarItem*>>> reverse_rules(fa1_reverse_rules);
	reverse_rules.insert(reverse_rules.end(), fa2_reverse_rules.begin(), fa2_reverse_rules.end());
	for (GrammarItem* nont : nonterminals)
		nont->class_number = -1; // сбрасываю номера классов

	vector<GrammarItem*> reverse_bisimilar_nonterminals;
	vector<vector<vector<GrammarItem*>>> reverse_bisimilar_rules = Grammar::get_bisimilar_grammar(
		reverse_rules, nonterminals, reverse_bisimilar_nonterminals, class_to_nonterminals);
	// сопоставление состояний 1 к 1
	vector<int> reverse_bisimilar_classes;
	for (GrammarItem* nont : nonterminals) {
		reverse_bisimilar_classes.push_back(nont->class_number);
		nont->class_number = -1;
	}

	/*for (auto t : bisimilar_classes)
		cout << t << " ";
	cout << endl;
	for (auto t : reverse_bisimilar_classes)
		cout << t << " ";
	cout << endl;*/

	int new_class = 0;
	for (int i = 0; i < bisimilar_classes.size(); i++) {
		if (nonterminals[i]->class_number != -1)
			continue;
		nonterminals[i]->class_number = new_class;
		vector<int> equiv_nont; // нетерминалы с таким же классом, что и i-й
		for (int j = i + 1; j < bisimilar_classes.size(); j++) {
			if (bisimilar_classes[j] == bisimilar_classes[i])
				equiv_nont.push_back(j);
		}
		for (int ind : equiv_nont)
			if (reverse_bisimilar_classes[ind] == reverse_bisimilar_classes[i])
				nonterminals[ind]->class_number = new_class;
		new_class++;
	}

	/*for (auto t : nonterminals)
		cout << t->class_number << " ";
	cout << endl;*/

	// грамматики из переходов
	vector<pair<GrammarItem, map<Symbol, vector<GrammarItem>>>> transitions1_items;
	vector<GrammarItem*> transitions1_nonterminals;
	vector<GrammarItem*> transitions1_terminals;
	vector<vector<vector<GrammarItem*>>> transitions1_rules =
		Grammar::tansitions_to_grammar(fa1.states,
									   fa1_nonterminals,
									   transitions1_items,
									   transitions1_nonterminals,
									   transitions1_terminals);

	/*for (int i = 0; i < transitions1_rules.size(); i++) {
		cout << *transitions1_nonterminals[i] << ": ";
		for (int j = 0; j < transitions1_rules[i].size(); j++) {
			for (int k = 0; k < transitions1_rules[i][j].size(); k++) {
				cout << *transitions1_rules[i][j][k];
			}
			cout << " ";
		}
		cout << endl;
	}*/

	vector<pair<GrammarItem, map<Symbol, vector<GrammarItem>>>> transitions2_items;
	vector<GrammarItem*> transitions2_nonterminals;
	vector<GrammarItem*> transitions2_terminals;
	vector<vector<vector<GrammarItem*>>> transitions2_rules =
		Grammar::tansitions_to_grammar(fa2.states,
									   fa2_nonterminals,
									   transitions2_items,
									   transitions2_nonterminals,
									   transitions2_terminals);

	// проверка равенства количества переходов
	if (transitions1_nonterminals.size() != transitions2_nonterminals.size())
		return false;
	// биективная бисимуляция переходов
	vector<GrammarItem*> transitions_nonterminals(transitions1_nonterminals);
	transitions_nonterminals.insert(transitions_nonterminals.end(),
									transitions2_nonterminals.begin(),
									transitions2_nonterminals.end());
	vector<vector<vector<GrammarItem*>>> transitions_rules(transitions1_rules);
	transitions_rules.insert(
		transitions_rules.end(), transitions2_rules.begin(), transitions2_rules.end());
	for (GrammarItem* nont : transitions_nonterminals)
		nont->class_number = 0; // сбрасываю номера классов
	vector<GrammarItem*> transitions_bisimilar_nonterminals;
	vector<vector<vector<GrammarItem*>>> transitions_bisimilar_rules =
		Grammar::get_bisimilar_grammar(transitions_rules,
									   transitions_nonterminals,
									   transitions_bisimilar_nonterminals,
									   class_to_nonterminals);
	// проверяю бисимилярность переходов
	classes.clear();
	classes.resize(transitions_bisimilar_nonterminals.size(), 0);
	for (auto t : transitions1_nonterminals)
		classes[t->class_number]++;
	for (auto t : transitions2_nonterminals)
		classes[t->class_number]--;
	for (auto t : classes)
		if (t != 0)
			return false;

	return true;
}

bool FiniteAutomaton::equal(const FiniteAutomaton& fa1, const FiniteAutomaton& fa2,
							iLogTemplate* log) {
	bool result = equality_checker(fa1, fa2);
	if (log) {
		log->set_parameter("automaton1", fa1);
		log->set_parameter("automaton2", fa2);
		log->set_parameter("result", result);
	}
	return result;
}

bool FiniteAutomaton::equivalent(const FiniteAutomaton& fa1, const FiniteAutomaton& fa2,
								 iLogTemplate* log) {
	bool result = true;
	if (fa1.language == fa2.language) {
		if (log)
			log->set_parameter("samelanguage",
							   "(!) автоматы изначально принадлежат одному языку"); // TODO:
																					// logs
	} else {
		if ((!fa1.language->is_min_dfa_cached() || !fa2.language->is_min_dfa_cached()) && log) {
			log->set_parameter("cachedMINDFA", "Минимальные автоматы сохранены в кэше");
		}
		result = equal(fa1.minimize(), fa2.minimize());
	}
	if (log) {
		log->set_parameter("automaton1", fa1);
		log->set_parameter("automaton2", fa2);
		log->set_parameter("result", result);
	}
	return result;
}

bool FiniteAutomaton::subset(const FiniteAutomaton& fa, iLogTemplate* log) const {
	FiniteAutomaton fa_instersection(intersection(*this, fa));
	bool result = equivalent(fa_instersection, fa);
	if (log) {
		log->set_parameter("automaton1", *this);
		log->set_parameter("automaton2", fa);
		log->set_parameter("result", result);
	}
	return result;
}

Fraction calc_ambiguity(int i, int n, const vector<Fraction>& f1,
						vector<vector<Fraction>>& calculated,  // NOLINT(runtime/references)
						vector<vector<char>>& is_calculated) { // NOLINT(runtime/references)
	if (i == 0)
		return f1[n];
	Fraction d1, d2;
	if (!is_calculated[i][n + 1]) {
		calculated[i][n + 1] = calc_ambiguity(i - 1, n + 1, f1, calculated, is_calculated);
		is_calculated[i][n + 1] = 1;
	}
	d1 = calculated[i][n + 1];
	if (!is_calculated[i][n]) {
		calculated[i][n] = calc_ambiguity(i - 1, n, f1, calculated, is_calculated);
		is_calculated[i][n] = 1;
	}
	d2 = calculated[i][n];
	return d1 - d2;
}

FiniteAutomaton::AmbiguityValue FiniteAutomaton::get_ambiguity_value(
	int digits_number_limit, std::optional<int>& word_length) const {
	FiniteAutomaton fa = remove_eps();
	FiniteAutomaton min_fa = fa.minimize(true);
	fa = fa.remove_trap_states();

	int i = 2;
	int s = fa.size();
	int min_s = min_fa.size();
	int N = s * s + s + i + 1;
	// количество путей до финальных из начального
	InfInt paths_number;
	InfInt min_paths_number;
	// матрица смежности
	vector<vector<int>> adjacency_matrix(s, vector<int>(s));
	vector<vector<int>> min_adjacency_matrix(min_s, vector<int>(min_s));
	// количество путей длины n до всех вершин из начальной
	vector<vector<InfInt>> d(2, vector<InfInt>(s));
	vector<vector<InfInt>> min_d(2, vector<InfInt>(min_s));
	d[0][fa.initial_state] = 1;
	min_d[0][min_fa.initial_state] = 1;
	for (int i = 0; i < s; i++)
		for (const auto& elem : fa.states[i].transitions)
			for (int transition : elem.second)
				adjacency_matrix[i][transition]++;
	for (int i = 0; i < min_s; i++)
		for (const auto& elem : min_fa.states[i].transitions)
			for (int transition : elem.second)
				min_adjacency_matrix[i][transition]++;
	vector<Fraction> f1;
	Fraction max_checker; // максимальное значение для проверки
						  // на однозначность
	bool max_return_flag = true;
	std::optional<Fraction> prev_f1_val;
	Fraction max_delta_checker; // максимальный прирост для
								// проверки на однозначность
	Fraction prev_val;			// значение calc_ambiguity
	bool max_delta_return_flag = true;
	bool unambigious_return_flag = true;
	bool is_exponentially_ambiguous = false;
	// для сохранения результатов calc_ambiguity
	vector<vector<Fraction>> calculated(i);
	vector<vector<char>> is_calculated(i);
	int return_counter = 0;
	for (int k = 0; true; k++) {
		paths_number = 0;
		min_paths_number = 0;
		d[(k + 1) % 2] = vector<InfInt>(s);
		min_d[(k + 1) % 2] = vector<InfInt>(min_s);
		for (int v = 0; v < s; v++) {
			for (int i = 0; i < s; i++) {
				d[(k + 1) % 2][v] += InfInt(adjacency_matrix[i][v]) * d[k % 2][i];
			}
			if (fa.states[v].is_terminal)
				paths_number += d[(k + 1) % 2][v];
		}
		for (int v = 0; v < min_s; v++) {
			for (int i = 0; i < min_s; i++) {
				min_d[(k + 1) % 2][v] += InfInt(min_adjacency_matrix[i][v]) * min_d[k % 2][i];
			}
			if (min_fa.states[v].is_terminal)
				min_paths_number += min_d[(k + 1) % 2][v];
		}
		Fraction new_f1_value;
		if (min_paths_number == 0) {
			new_f1_value = Fraction();
		} else {
			new_f1_value = Fraction(paths_number, min_paths_number);
			// обработка проверок на однозначность
			if (!(new_f1_value == Fraction(1, 1)))
				unambigious_return_flag = false;
			if (k < (s + 1) * 3) {
				if (new_f1_value > max_checker)
					max_checker = new_f1_value;
				if (!prev_f1_val) {
					prev_f1_val = new_f1_value;
				} else {
					Fraction delta = new_f1_value - *prev_f1_val;
					if (delta > max_delta_checker)
						max_delta_checker = delta;
					prev_f1_val = new_f1_value;
				}
			} else if (max_return_flag || max_delta_return_flag) {
				if (new_f1_value > max_checker)
					max_return_flag = false;
				Fraction delta = new_f1_value - *prev_f1_val;
				if (delta >= max_delta_checker)
					max_delta_return_flag = false;
				prev_f1_val = new_f1_value;
			}
		}
		if (k >= s * s && unambigious_return_flag)
			return FiniteAutomaton::unambigious;
		if (k >= s * s && k >= (s + 1) * 3 && (max_return_flag || max_delta_return_flag))
			return FiniteAutomaton::almost_unambigious;

		vector<Fraction> f1_check = f1;
		f1_check.push_back(new_f1_value);
		if (f1_check.size() >= 3) {
			int new_s = floor(double(-1 + sqrt((1 - 4 * (i + 1)) + 4 * f1_check.size())) / 2);
			int delta = f1_check.size() - (new_s * new_s + new_s + i + 1);

			vector<vector<Fraction>> calculated_check = calculated;
			vector<vector<char>> is_calculated_check = is_calculated;
			for (int j = 0; j < calculated_check.size(); j++) {
				calculated_check[j].resize(f1_check.size(), Fraction());
				is_calculated_check[j].resize(f1_check.size(), 0);
			}
			calculated_check.emplace_back(f1_check.size());
			is_calculated_check.emplace_back(f1_check.size(), 0);
			Fraction val = calc_ambiguity(
				new_s + i, new_s * new_s + delta, f1_check, calculated_check, is_calculated_check);
			// limit check
			if (Fraction::last_number_of_digits >= digits_number_limit ||
				double(paths_number.numberOfDigits() + min_paths_number.numberOfDigits()) >=
					double(digits_number_limit) / 2) {
				word_length = k;
				if (unambigious_return_flag)
					return FiniteAutomaton::unambigious;
				if (k >= (s + 1) * 3 && (max_return_flag || max_delta_return_flag))
					return FiniteAutomaton::almost_unambigious;
				if (val > Fraction() && val >= prev_val)
					return FiniteAutomaton::exponentially_ambiguous;
				if (is_exponentially_ambiguous)
					return FiniteAutomaton::exponentially_ambiguous;
				return FiniteAutomaton::polynomially_ambigious;
			}

			if (Fraction() >= val) {
				return_counter++;
				if (k >= N && return_counter >= s)
					break;
				continue;
			}

			if (val >= prev_val) {
				is_exponentially_ambiguous = true;
				if (k >= N)
					return FiniteAutomaton::exponentially_ambiguous;
			} else {
				is_exponentially_ambiguous = false;
			}
			return_counter = 0;
			calculated = calculated_check;
			is_calculated = is_calculated_check;
			f1.push_back(new_f1_value);
			prev_val = val;
		} else {
			f1.push_back(new_f1_value);
		}
	}

	return FiniteAutomaton::polynomially_ambigious;
}

FiniteAutomaton::AmbiguityValue FiniteAutomaton::ambiguity(iLogTemplate* log) const {
	std::optional<int> word_length;
	FiniteAutomaton::AmbiguityValue result = get_ambiguity_value(300, word_length);
	if (log) {
		log->set_parameter("oldautomaton", *this);
		if (word_length.has_value()) {
			log->set_parameter("Для максимальной длины слова", std::to_string(*word_length));
		}
		switch (result) {
		case FiniteAutomaton::exponentially_ambiguous:
			log->set_parameter("result", "Exponentially ambiguous");
			break;
		case FiniteAutomaton::almost_unambigious:
			log->set_parameter("result", "Almost unambigious");
			break;
		case FiniteAutomaton::unambigious:
			log->set_parameter("result", "Unambigious");
			break;
		case FiniteAutomaton::polynomially_ambigious:
			log->set_parameter("result", "Polynomially ambiguous");
			break;
		default:
			break;
		}
	}
	return result;
}

TransformationMonoid FiniteAutomaton::get_syntactic_monoid() const {
	if (language->is_syntactic_monoid_cached()) {
		return language->get_syntactic_monoid();
	}
	FiniteAutomaton min_dfa = minimize();
	TransformationMonoid syntactic_monoid(min_dfa);
	// syntactic_monoid.is_minimal(); ТМ делает это автоматически
	//  кэширование
	language->set_syntactic_monoid(syntactic_monoid);
	return syntactic_monoid;
}

void set_result(int& res, int size,						  // NOLINT(runtime/references)
				vector<pair<int, int>>& result_yx,		  // NOLINT(runtime/references)
				vector<pair<int, int>>& temp_result_yx) { // NOLINT(runtime/references)
	if (size > res) {
		res = size;
		result_yx = temp_result_yx;
	}
}

void find_maximum_identity_matrix(vector<int>& rows,		   // NOLINT(runtime/references)
								  vector<vector<bool>>& table, // NOLINT(runtime/references)
								  int& res,					   // NOLINT(runtime/references)
								  int size,
								  vector<pair<int, int>>& result_yx, // NOLINT(runtime/references)
								  vector<pair<int, int>> temp_result_yx, vector<bool> used_x,
								  vector<bool> used_y, int unused_x, int unused_y) {
	if (rows.empty()) {
		set_result(res, size, result_yx, temp_result_yx);
		return;
	}
	int n = table.size(), m = table[0].size();
	vector<int> y_ind(m, -1);
	for (int j = 0; j < m; j++) {
		if (used_x[j])
			continue;
		for (int i = 0; i < n; i++) {
			if (used_y[i])
				continue;
			if (!table[i][j])
				continue;
			if (y_ind[j] == -1)
				y_ind[j] = i;
			else
				y_ind[j] = -2;
		}
	}
	// отмечаю столбцы с единственной единицей
	for (int j = 0; j < m; j++) {
		if (y_ind[j] >= 0) {
			if (used_y[y_ind[j]])
				continue;
			if (used_x[j])
				continue;
			used_x[j] = true;
			used_y[y_ind[j]] = true;
			unused_y--;
			unused_x--;
			size++;
			temp_result_yx.emplace_back(y_ind[j], j);
		}
	}
	if (unused_y == 0) {
		set_result(res, size, result_yx, temp_result_yx);
		return;
	}
	unused_y--;
	for (auto row : rows) {
		if (used_y[row])
			continue;
		used_y[row] = true;
		// ищу новую 1-цу на каждой строке
		if (unused_x <= (res - size))
			return;
		if (unused_y < (res - size))
			return;
		vector<bool> new_used_x = used_x;
		int new_unused_x = unused_x;
		vector<int> true_x;
		for (int i = 0; i < table[row].size(); i++) {
			if (new_used_x[i])
				continue;
			if (!table[row][i])
				continue;
			true_x.push_back(i);
			new_used_x[i] = true;
			new_unused_x--;
		}
		// если нет 1-цы завершаюсь
		if (true_x.empty()) {
			set_result(res, size, result_yx, temp_result_yx);
			used_y[row] = false;
			continue;
		}
		for (int x : true_x) {
			vector<bool> new_used_y = used_y;
			int new_unused_y = unused_y;
			vector<int> false_y;
			for (int i = 0; i < table.size(); i++) {
				if (new_used_y[i])
					continue;
				false_y.push_back(i);
			}
			vector<pair<int, int>> new_result_yx = temp_result_yx;
			new_result_yx.emplace_back(row, x);
			vector<bool> temp_used_x = new_used_x;
			// перехожу на все свободные строки
			find_maximum_identity_matrix(false_y,
										 table,
										 res,
										 size + 1,
										 result_yx,
										 new_result_yx,
										 temp_used_x,
										 new_used_y,
										 new_unused_x,
										 new_unused_y);
		}
		//
		used_y[row] = false;
	}
}

int FiniteAutomaton::get_classes_number_GlaisterShallit(iLogTemplate* log) const {
	if (log)
		log->set_parameter("oldautomaton", *this);
	if (language->is_nfa_minimum_size_cached()) {
		if (log) {
			log->set_parameter("result", language->get_nfa_minimum_size());
			log->set_parameter("cach", "(!) результат получен из кэша");
			// TODO: таблицу из кэша
		}
		return language->get_nfa_minimum_size();
	}

	TransformationMonoid sm = get_syntactic_monoid();
	sm.get_classes_number_MyhillNerode();

	vector<string> table_rows;
	vector<string> table_columns;
	vector<vector<bool>> equivalence_classes_table =
		sm.get_equivalence_classes_table(table_rows, table_columns);

	int result = -1;
	vector<pair<int, int>> result_yx;
	int m = equivalence_classes_table[0].size(), n = equivalence_classes_table.size();
	vector<bool> used_x(m);
	vector<bool> used_y(n);
	vector<int> rows;
	for (int i = 0; i < n; i++)
		rows.push_back(i);
	find_maximum_identity_matrix(
		rows, equivalence_classes_table, result, 0, result_yx, {}, used_x, used_y, m, n);

	// DEBUG
	// cout << sm.to_txt_MyhillNerode() << endl;
	// int maxlen = table_columns[table_columns.size() - 1].size();
	// cout << string(maxlen + 2, ' ');
	// for (int i = 0; i < result_yx.size(); i++) {
	// 	cout << table_columns[result_yx[i].second]
	// 		 << string(maxlen + 2 - table_columns[result_yx[i].second].size(),
	// 				   ' ');
	// }
	// cout << endl;
	// for (int i = 0; i < result_yx.size(); i++) {
	// 	cout << table_rows[result_yx[i].first]
	// 		 << string(maxlen + 2 - table_rows[result_yx[i].first].size(), ' ');
	// 	for (int j = 0; j < result_yx.size(); j++) {
	// 		cout << equivalence_classes_table[result_yx[i].first]
	// 										 [result_yx[j].second]
	// 			 << string(maxlen + 1, ' ');
	// 	}
	// 	cout << endl;
	// }

	if (log) {
		iLogTemplate::Table t;
		for (auto& i : result_yx) {
			t.columns.push_back(table_columns[i.second]);
		}
		for (int i = 0; i < result_yx.size(); i++) {
			t.rows.push_back(table_rows[result_yx[i].first]);
			for (int j = 0; j < result_yx.size(); j++) {
				t.data.push_back(
					to_string(equivalence_classes_table[result_yx[i].first][result_yx[j].second]));
			}
		}
		log->set_parameter("result", result);
		log->set_parameter("table", t);
	}

	// кэширование
	language->set_nfa_minimum_size(result);
	return result;
}

std::optional<bool> FiniteAutomaton::get_nfa_minimality_value() const {
	if (!language->is_pump_length_cached())
		return std::nullopt;
	int language_pump_length = language->get_pump_length();

	int transition_states_counter = 0;
	for (const State& state : states)
		if (!state.transitions.empty())
			transition_states_counter++;
	if (language_pump_length == transition_states_counter + 1)
		return true;
	if (states.size() > language_pump_length)
		return states.size() == get_classes_number_GlaisterShallit();

	return std::nullopt;
}

std::optional<bool> FiniteAutomaton::is_nfa_minimal(iLogTemplate* log) const {
	if (log) {
		log->set_parameter("oldautomaton", *this);
	}
	std::optional<bool> result = get_nfa_minimality_value();
	if (result.has_value()) {
		if (log) {
			log->set_parameter("result", result.value() ? "True" : "False");
		}
	} else if (log) {
		log->set_parameter("result", "Unknown");
	}
	return result;
}

bool FiniteAutomaton::is_dfa_minimal(iLogTemplate* log) const {
	if (log) {
		log->set_parameter("oldautomaton", *this);
		if ((language->is_min_dfa_cached())) {
			log->set_parameter("cachedMINDFA", "Минимальный автомат сохранен в кэше");
		}
	}
	bool result = states.size() == minimize().size();

	if (log) {
		log->set_parameter("result", result ? "True" : "False");
	}
	return result;
}

std::optional<string> FiniteAutomaton::get_prefix(int state_beg, int state_end,
												  map<int, bool>& was) const {
	std::optional<string> ans = std::nullopt;
	if (state_beg == state_end) {
		ans = "";
		return ans;
	}
	auto trans = &states[state_beg].transitions;
	for (auto it = trans->begin(); it != trans->end(); it++) {
		for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
			if (!was[*it2]) {
				was[*it2] = true;
				auto res = get_prefix(*it2, state_end, was);
				if (res.has_value()) {
					ans = (string)it->first + (string)res.value();
				}
				return ans;
			}
		}
	}
	return ans;
}

bool FiniteAutomaton::semdet_entry(bool annoted, iLogTemplate* log) const {
	if (!annoted) {
		return annote().semdet_entry(true);
	}
	map<int, bool> was;
	vector<int> final_states;
	for (int i = 0; i < states.size(); i++) {
		if (states[i].is_terminal)
			final_states.push_back(i);
	}
	vector<Regex> state_languages;
	state_languages.resize(states.size());
	for (int i = 0; i < states.size(); i++) {
		auto prefix = get_prefix(initial_state, i, was);
		was.clear();
		// cout << "Try " << i << "\n";
		if (!prefix.has_value())
			continue;
		Regex reg;
		// Получение языка из производной регулярки автомата по префиксу:
		//		this -> reg (arden?)
		reg = to_regex();
		//  cout << "State: " << i << "\n";
		//  cout << "Prefix: " << prefix.value() << "\n";
		//  cout << "Regex: " << reg.to_txt() << "\n";
		auto derivative = reg.prefix_derivative(prefix.value());
		if (!derivative.has_value())
			continue;
		state_languages[i] = derivative.value();
		// cout << "Derevative: " << state_languages[i].to_txt() << "\n";

		// TODO: logs
		if (log) {
			log->set_parameter("state", i);
			log->set_parameter("prefix", prefix.value());
			log->set_parameter("regex", reg);
			log->set_parameter("derivative", state_languages[i]);
		}
		state_languages[i].make_language();
	}
	for (int i = 0; i < states.size(); i++) {
		for (const auto& state : states) {
			for (auto transition = state.transitions.begin(); transition != state.transitions.end();
				 transition++) {
				bool verified_ambiguity = false;
				for (auto it = transition->second.begin(); it != transition->second.end(); it++) {
					bool reliability = true;
					for (auto it2 = transition->second.begin(); it2 != transition->second.end();
						 it2++) {
						if (!state_languages[*it].subset(state_languages[*it2])) {
							reliability = false;
							break;
						}
					}
					verified_ambiguity |= reliability;
				}
				if (!verified_ambiguity) {
					return false;
				}
			}
		}
	}
	return true;
}

bool FiniteAutomaton::semdet(iLogTemplate* log) const {
	if (log) {
		log->set_parameter("oldautomaton", *this);
	}
	bool result = semdet_entry(log);
	if (log) {
		log->set_parameter("result", result);
	}
	return result;
}

bool FiniteAutomaton::parsing_nfa(const string& s, int index_state) const {
	State state = states[index_state];

	if (s.empty() && state.is_terminal) {
		return true;
	}
	set<int> tr_eps = state.transitions[Symbol::epsilon()];
	vector<int> trans_eps{tr_eps.begin(), tr_eps.end()};

	if (s.empty() && !state.is_terminal) {
		for (size_t i = 0; i < trans_eps.size(); i++)
			if (parsing_nfa(s, trans_eps[i])) {
				return true;
			}
		return false;
	}

	Symbol elem(s[0]);
	set<int> tr = state.transitions[elem];
	vector<int> trans{tr.begin(), tr.end()};

	for (size_t i = 0; i < trans.size(); i++) {
		if (parsing_nfa(s.substr(1), trans[i])) {
			return true;
		}
	}

	for (size_t i = 0; i < trans_eps.size(); i++) {
		if (parsing_nfa(s, trans_eps[i])) {
			return true;
		}
	}
	return false;
}

pair<int, bool> FiniteAutomaton::parsing_by_nfa(const string& s) const {
	// Пара (актуальный индекс элемента в строке, состояние)
	std::stack<pair<int, State>> stack_state;
	// Тройка (актуальный индекс элемента в строке, начало эпсилон-перехода, конец эпсилон-перехода)
	set<std::tuple<int, int, int>> visited_eps, aux_eps;
	int counter = 0;
	int parsed_len = 0;
	State state = states[initial_state];
	stack_state.push({parsed_len, state});
	while (!stack_state.empty()) {
		if (state.is_terminal && parsed_len == s.size()) {
			break;
		}
		state = stack_state.top().second;
		parsed_len = stack_state.top().first;
		stack_state.pop();
		counter++;
		Symbol elem(s[parsed_len]);
		set<int> trans = state.transitions[elem];
		// Переходы в новые состояния по очередному символу строки
		if (parsed_len + 1 <= s.size()) {
			for (auto new_state : trans) {
				stack_state.push({parsed_len + 1, states[new_state]});
			}
		}

		// Если произошёл откат по строке, то эпсилон-переходы из рассмотренных состояний больше не
		// считаются повторными
		if (!visited_eps.empty()) {
			for (auto pos : visited_eps) {
				if (std::get<0>(pos) <= parsed_len)
					aux_eps.insert(pos);
			}
			visited_eps = aux_eps;
			aux_eps.clear();
		}
		// Добавление тех эпсилон-переходов, по которым ещё не было разбора от этой позиции и этого
		// состояния
		auto reach_eps = state.transitions[Symbol::epsilon()];
		for (int eps_tr : reach_eps) {
			if (visited_eps.find({parsed_len, state.index, eps_tr}) == visited_eps.end()) {
				stack_state.push({parsed_len, states[eps_tr]});
				visited_eps.insert({parsed_len, state.index, eps_tr});
			}
		}
	}
	if (s.size() == parsed_len && state.is_terminal) {
		return {counter, true};
	}
	return {counter, false};
}

bool FiniteAutomaton::is_deterministic(iLogTemplate* log) const {
	if (log) {
		log->set_parameter("oldautomaton", *this);
	}
	bool result = true;
	for (const auto& state : states) {
		for (const auto& elem : state.transitions) {
			if (elem.first == Symbol::epsilon()) {
				result = false;
				break;
			}
			if (elem.second.size() > 1) {
				result = false;
				break;
			}
		}
	}
	if (log) {
		log->set_parameter("result", result ? "True" : "False");
	}
	return result;
}

int FiniteAutomaton::get_initial() {
	return initial_state;
}

size_t FiniteAutomaton::size(iLogTemplate* log) const {
	if (log) {
		log->set_parameter("oldautomaton", *this);
		log->set_parameter("result", static_cast<int>(states.size()));
	}
	return states.size();
}

bool FiniteAutomaton::is_empty() const {
	return states.empty();
}

/*
Джун программист прибегает с проекта к своему ментору и кричит:
— Меня отпустили с проекта пораньше!
— Как это здорово! Мы будем делать с тобой рефакторинг
— рефакторинг? А как это?
— Как! Ты не знаешь, что такое рефакторинг?! — и отказывается от менторства.
Изумленный джун приходит к тимлиду и говорит:
— Я вернулся пораньше с проекта, чтобы делать с тобой рефакторинг.
— Как?! Рефакторинг со мной?! — дает негативный отзыв, поднимает HR и прогоняет
из компании.
Обалдевший джуниор выходит на фриланс, пишет первому попавшемуся
девелоперу и говорит:
— Вот вам 20 фунтов, покажите мне, что такое рефакторинг.
— Что?! Рефакторинг за эти деньги??? — дает запрет на входящие сообщения и
уходит.
Тут программист вспоминает, что в ядре монолитного legacy проекта живет
90летний Сишник, который много повидал в жизни и наверняка знает, что такое
рефакторинг. Он вбегает к Сишнику, который сидит по уши в отладке и проблемах с
выделением памяти.
— Помогите! — кричит он. — Уж вы-то знаете, что такое
рефакторинг?!
Ядровик подпрыгивает от неожиданности и с паникой в голосе
вскликает:
— Ах! Рефакторинг, рефакторинг! - и умирает.
*/

Regex FiniteAutomaton::to_regex(iLogTemplate* log) const {
	if (log) {
		log->set_parameter("oldautomaton", *this);
	}

	// a system of linear algebraic equations
	std::unordered_map<int, std::unordered_map<int, Regex>> SLAE{};
	// индекс стартового состояния (должен быть среди состояний)
	const int start_state_index = initial_state;
	// индекс глобального конечного состояния (должен не быть среди состояний)
	const int end_state_index = -1;
	/*
		@algorithm_sample
		from/to	| 0 | 1 | -1(end)		from/to	| 0 | 1	  | -1(end)
		0 		| a | b |		--->	0 		|	| a*b |	   --->
		1		|	| a | e				1		|	| a	  | e

		from/to	| 0 | 1   | -1(end)		from/to	| 0 | 1 | -1(end)
		0 		|   | a*b | 	--->	0 		|	|	| a*b(a*|e))
		1		|	| 	  | a*|e		1		|	|	|
	*/

	// заполнение уравнений системы актуальными значениями
	for (const auto& state : states) {
		SLAE.insert({state.index, std::unordered_map<int, Regex>{}});

		// если завершающее состояние, добавляем eps-переход
		if (state.is_terminal) {
			SLAE[state.index].insert({end_state_index, Regex{}});
		}

		// итерируемся по всем путям из state
		for (const auto& [symbol, states_to] : state.transitions) {

			// распознание eps-перехода
			Regex symbol_regex{};
			if (!symbol.is_epsilon()) {
				symbol_regex = {symbol};
			}

			for (int state_index_to : states_to) {
				if (SLAE[state.index].count(state_index_to)) {
					SLAE[state.index][state_index_to] =
						Regex(Regex::Type::alt, &SLAE[state.index][state_index_to], &symbol_regex);
				} else {
					SLAE[state.index].insert({state_index_to, symbol_regex});
				}
			}
		}
	}

	// теорема Ардена о переходах в себя
	auto arden_theorem = [&SLAE](int state_index) {
		// передан индекс несуществующего состояния
		if (!SLAE.count(state_index)) {
			return;
		}

		// случай отсутствия в уравнении переходов в себя
		if (!SLAE[state_index].count(state_index)) {
			return;
		}

		// подготавливаем звёздную регулярку
		Regex state_self_regex(Regex::Type::star, &SLAE[state_index][state_index]);

		// добавление звёздного перехода к остальным переходам уравнения
		for (auto& [state_index_to, to_regex] : SLAE[state_index]) {
			to_regex = Regex(Regex::Type::conc, &state_self_regex, &to_regex);
		}

		// удаление рассмотренного перехода состояния в себя же
		SLAE[state_index].erase(state_index);
	};

	// решение СЛАУ методом Гаусса с применением леммы Ардена
	// итерация по строкам системы уравнений
	for (auto& [state_index_row, equation_row] : SLAE) {
		// ссылка не константная, тк уравнения
		// будут подвержены изменениям

		// начальное состояние должно остаться последним не рассмотренным
		if (state_index_row == start_state_index) {
			continue;
		}

		// устранение переходов из рассматриваемого состояния в себя же
		arden_theorem(state_index_row);

		// подстановка рассматриваемого уравнения в его упоминания в прочих
		// итерациях по всем уравнениям с переходами в исходное для подстановки регулярки
		for (auto& [state_index_from, equation_from] : SLAE) {
			// пропуск итерации, если в рассматриваемом уравнения нет исходного
			if (!equation_from.count(state_index_row)) {
				continue;
			}

			// итерация по всем переходам из исходного уравнения
			for (auto& [state_index_col, regex_col] : equation_row) {
				// регулярка перехода из рассматриваемого уравнения в исходное
				Regex regex_from = Regex(Regex::Type::conc, &equation_from[state_index_row], &regex_col);

				// объединяем полученную регулярку с имеющейся в рассматриваемом уравнении
				if (equation_from.count(state_index_col)) {
					equation_from[state_index_col] =
						Regex(Regex::Type::alt, &equation_from[state_index_col], &regex_from);
				} else {
					equation_from.insert({state_index_col, regex_from});
				}
			}

			equation_from.erase(state_index_row);
		}

		// обнуляем рассмотренное выражение за избыточностью подстановки последующих в данное
		//// удаление элемента из словаря во время итерации привело бы к ошибке
		SLAE[state_index_row].clear();
	}

	// применяем теорему Ардена к начальному состоянию
	arden_theorem(start_state_index);

	// возвращаем путь из начало в конец
	if (SLAE.count(start_state_index) && SLAE[start_state_index].count(end_state_index)) {
		auto& result_regex = SLAE[start_state_index][end_state_index];

		// подстановка нужного языка в финальную регулярку
		result_regex.set_language(language);

		if (log) {
			log->set_parameter("result", result_regex.to_txt());
		}
		return result_regex;
	}

	// случай недостижимости ни одного из конечных состояний или их отсутствия
	if (log) {
		log->set_parameter("result", "Unknown");
	}
	return Regex{};
}
