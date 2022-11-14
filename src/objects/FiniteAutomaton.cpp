#include "FiniteAutomaton.h"
#include "Fraction.h"
#include "Grammar.h"
#include "InfInt/InfInt.h"
#include "Language.h"
#include "Logger.h"
#include <algorithm>
#include <iostream>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
using namespace std;

State::State() : index(0), is_terminal(false), identifier("") {}

State::State(int index, set<int> label, string identifier, bool is_terminal,
			 map<alphabet_symbol, set<int>> transitions)
	: index(index), label(label), identifier(identifier),
	  is_terminal(is_terminal), transitions(transitions) {}

void State::set_transition(int to, alphabet_symbol symbol) {
	transitions[symbol].insert(to);
}

FiniteAutomaton::FiniteAutomaton() {}

FiniteAutomaton::FiniteAutomaton(int initial_state, vector<State> states,
								 shared_ptr<Language> language)
	: BaseObject(language), initial_state(initial_state), states(states) {}

FiniteAutomaton::FiniteAutomaton(int initial_state, vector<State> states,
								 set<alphabet_symbol> alphabet)
	: BaseObject(alphabet), initial_state(initial_state), states(states) {}

FiniteAutomaton::FiniteAutomaton(const FiniteAutomaton& other)
	: BaseObject(other.language), initial_state(other.initial_state),
	  states(other.states) {}

string FiniteAutomaton::to_txt() const {
	stringstream ss;
	ss << "digraph {\n\trankdir = LR\n\tdummy [label = \"\", shape = none]\n\t";
	for (int i = 0; i < states.size(); i++) {
		ss << i << " [label = \"" << states[i].identifier << "\", shape = ";
		ss << (states[i].is_terminal ? "doublecircle]\n\t" : "circle]\n\t");
	}
	if (states.size() > initial_state)
		ss << "dummy -> " << states[initial_state].index << "\n";

	for (const auto& state : states) {
		for (const auto& elem : state.transitions) {
			for (int transition_to : elem.second) {
				ss << "\t" << state.index << " -> " << transition_to
				   << " [label = \"" << to_string(elem.first) << "\"]\n";
			}
		}
	}

	ss << "}\n";
	return ss.str();
}

//обход автомата в глубину
void dfs(vector<State> states, const set<alphabet_symbol>& alphabet, int index,
		 set<int>& reachable, bool use_epsilons_only) {
	if (reachable.find(index) == reachable.end()) {
		reachable.insert(index);
		for (int transition_to : states[index].transitions[epsilon()]) {
			dfs(states, alphabet, transition_to, reachable, use_epsilons_only);
		}
		if (!use_epsilons_only) {
			for (alphabet_symbol symb : alphabet) {
				for (int transition_to : states[index].transitions[symb]) {
					dfs(states, alphabet, transition_to, reachable,
						use_epsilons_only);
				}
			}
		}
	}
}

set<int> FiniteAutomaton::closure(const set<int>& indices,
								  bool use_epsilons_only) const {
	set<int> reachable;
	for (int index : indices)
		dfs(states, language->get_alphabet(), index, reachable,
			use_epsilons_only);
	return reachable;
}

FiniteAutomaton FiniteAutomaton::determinize() const {
	Logger::init_step("Determinize");
	FiniteAutomaton dfa = FiniteAutomaton(0, {}, language);
	set<int> q0 = closure({initial_state}, true);

	set<int> label = q0;
	string new_identifier;
	for (auto elem : label) {
		new_identifier +=
			(new_identifier.empty() ? "" : ", ") + states[elem].identifier;
	}
	State new_initial_state = {0, label, new_identifier, false,
							   map<alphabet_symbol, set<int>>()};
	dfa.states.push_back(new_initial_state);

	stack<set<int>> s1;
	stack<int> s2;
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
		for (alphabet_symbol symb : language->get_alphabet()) {
			new_x.clear();
			for (int j : z) {
				auto transitions_by_symbol = states[j].transitions.find(symb);
				if (transitions_by_symbol != states[j].transitions.end())
					for (int k : transitions_by_symbol->second) {
						new_x.insert(k);
					}
			}

			set<int> z1 = closure(new_x, true);
			string new_identifier;
			for (auto elem : z1) {
				new_identifier += (new_identifier.empty() ? "" : ", ") +
								  states[elem].identifier;
			}

			State q1 = {-1, z1, new_identifier, false,
						map<alphabet_symbol, set<int>>()};
			bool accessory_flag = false;

			for (const auto& state : dfa.states) {
				if (q1.label == state.label) {
					index = state.index;
					accessory_flag = true;
					break;
				}
			}

			if (!accessory_flag) index = -1;
			if (index != -1)
				q1 = dfa.states[index];
			else {
				index = dfa.states.size();
				q1.index = index;
				dfa.states.push_back(q1);
				s1.push(z1);
				s2.push(index);
			}
			dfa.states[q.index].transitions[symb].insert(q1.index);
		}
	}
	Logger::log("Автомат до детерминизации", "Автомат после детерминизации",
				*this, dfa);
	Logger::finish_step();
	return dfa;
}

FiniteAutomaton FiniteAutomaton::minimize() const {
	Logger::init_step("Minimize");
	const optional<FiniteAutomaton>& language_min_dfa = language->get_min_dfa();
	if (language->get_min_dfa()) {
		Logger::finish_step();
		return *language_min_dfa; // Нужно решить, что делаем с идентификаторами
	}
	// минимизация
	FiniteAutomaton dfa = determinize();
	vector<bool> table(dfa.states.size() * dfa.states.size());
	int counter = 1;
	for (int i = 1; i < dfa.states.size(); i++) {
		for (int j = 0; j < counter; j++) {
			if (dfa.states[i].is_terminal ^ dfa.states[j].is_terminal) {
				table[i * dfa.states.size() + j] = true;
			}
		}
		counter++;
	}

	bool flag = true;
	while (flag) {
		counter = 1;
		flag = false;
		for (int i = 1; i < dfa.states.size(); i++) {
			for (int j = 0; j < counter; j++) {
				if (!table[i * dfa.states.size() + j]) {
					for (alphabet_symbol symb : language->get_alphabet()) {
						vector<int> to = {
							*dfa.states[i].transitions[symb].begin(),
							*dfa.states[j].transitions[symb].begin()};
						if (*dfa.states[i].transitions[symb].begin() <
							*dfa.states[j].transitions[symb].begin()) {
							to = {*dfa.states[j].transitions[symb].begin(),
								  *dfa.states[i].transitions[symb].begin()};
						}
						if (table[to[0] * dfa.states.size() + to[1]]) {
							table[i * dfa.states.size() + j] = true;
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
	for (int i = 1; i < dfa.states.size(); i++) {
		for (int j = 0; j < counter; j++) {
			if (!table[i * dfa.states.size() + j]) {
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
			if (find(groups[i].begin(), groups[i].end(), groups[j][0]) !=
				groups[i].end()) {
				in_first = true;
			}
			if (find(groups[i].begin(), groups[i].end(), groups[j][1]) !=
				groups[i].end()) {
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

	for (int i = 0; i < dfa.states.size(); i++) {
		if (find(visited.begin(), visited.end(), dfa.states[i].index) ==
			visited.end()) {
			groups.push_back({dfa.states[i].index});
		}
	}

	vector<int> classes(dfa.states.size());
	for (int i = 0; i < groups.size(); i++) {
		for (int j = 0; j < groups[i].size(); j++) {
			classes[groups[i][j]] = i;
		}
	}
	FiniteAutomaton minimized_dfa = dfa.merge_equivalent_classes(classes);
	// кэширование
	language->set_min_dfa(minimized_dfa);
	Logger::log("Автомат до минимизации", "Автомат после минимизации", *this,
				minimized_dfa);
	stringstream ss;
	for (const auto& state : minimized_dfa.states) {
		ss << "\\{" << state.identifier << "\\} ";
	}
	Logger::log("Эквивалентные классы", ss.str());
	Logger::finish_step();
	return minimized_dfa;
}

FiniteAutomaton FiniteAutomaton::remove_eps() const {
	Logger::init_step("RemEps");
	FiniteAutomaton new_nfa(initial_state, states, language);

	for (auto& state : new_nfa.states)
		state.transitions = map<alphabet_symbol, set<int>>();

	for (int i = 0; i < states.size(); i++) {
		set<int> q = closure({states[i].index}, true);
		for (int elem : q) {
			if (states[elem].is_terminal) {
				new_nfa.states[i].is_terminal = true;
			}
		}
		vector<set<int>> x;
		for (alphabet_symbol symb : language->get_alphabet()) {
			x.clear();
			for (int k : q) {
				auto transitions_by_symbol = states[k].transitions.find(symb);
				if (transitions_by_symbol != states[k].transitions.end())
					x.push_back(transitions_by_symbol->second);
			}
			set<int> q1;
			set<int> x1;
			for (auto k : x) {
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
	Logger::log("Автомат до удаления eps-переходов",
				"Автомат после удаления eps-переходов", *this, new_nfa);
	Logger::finish_step();
	return new_nfa;
}

FiniteAutomaton FiniteAutomaton::intersection(const FiniteAutomaton& fa1,
											  const FiniteAutomaton& fa2) {
	Logger::init_step("Intersection");
	set<alphabet_symbol> merged_alphabets = fa1.language->get_alphabet();
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
			new_identifier +=
				(state2.identifier.empty() ? "" : ", " + state2.identifier);
			new_dfa.states.push_back({counter,
									  {state1.index, state2.index},
									  new_identifier,
									  state1.is_terminal && state2.is_terminal,
									  map<alphabet_symbol, set<int>>()});
			state_pair.push_back({state1.index, state2.index});
			counter++;
		}
	}

	for (int i = 0; i < new_dfa.states.size(); i++) {
		for (alphabet_symbol symb : new_dfa.language->get_alphabet()) {
			new_dfa.states[i].transitions[symb].insert(
				*new_dfa1.states[state_pair[i].first]
						.transitions.at(symb)
						.begin() *
					new_dfa2.states.size() +
				*new_dfa2.states[state_pair[i].second]
					 .transitions.at(symb)
					 .begin());
		}
	}
	set<alphabet_symbol> new_alphabet;
	set_intersection(fa1.language->get_alphabet().begin(),
					 fa1.language->get_alphabet().end(),
					 fa2.language->get_alphabet().begin(),
					 fa2.language->get_alphabet().end(),
					 inserter(new_alphabet, new_alphabet.begin()));
	new_dfa.language->set_alphabet(new_alphabet);
	for (int i = 0; i < new_dfa.states.size(); i++) {
		map<alphabet_symbol, set<int>> new_transitions;
		for (alphabet_symbol symb : merged_alphabets) {
			if (new_dfa.states[i].transitions.find(symb) !=
				new_dfa.states[i].transitions.end()) {
				new_transitions[symb] = new_dfa.states[i].transitions[symb];
			}
		}
		new_dfa.states[i].transitions = new_transitions;
	}
	new_dfa = new_dfa.remove_unreachable_states();
	Logger::log("Первый автомат", "Второй автомат", "Результат пересечения",
				fa1, fa2, new_dfa);
	Logger::finish_step();
	return new_dfa;
}

FiniteAutomaton FiniteAutomaton::uunion(const FiniteAutomaton& fa1,
										const FiniteAutomaton& fa2) {
	Logger::init_step("Union");
	set<alphabet_symbol> merged_alphabets = fa1.language->get_alphabet();
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
			new_identifier +=
				(state2.identifier.empty() ? "" : ", " + state2.identifier);
			new_dfa.states.push_back({counter,
									  {state1.index, state2.index},
									  new_identifier,
									  state1.is_terminal || state2.is_terminal,
									  map<alphabet_symbol, set<int>>()});
			state_pair.push_back({state1.index, state2.index});
			counter++;
		}
	}

	for (int i = 0; i < new_dfa.states.size(); i++) {
		for (alphabet_symbol symb : new_dfa.language->get_alphabet()) {
			new_dfa.states[i].transitions[symb].insert(
				*new_dfa1.states[state_pair[i].first]
						.transitions.at(symb)
						.begin() *
					new_dfa2.states.size() +
				*new_dfa2.states[state_pair[i].second]
					 .transitions.at(symb)
					 .begin());
		}
	}
	new_dfa = new_dfa.remove_unreachable_states();
	Logger::log("Первый автомат", "Второй автомат", "Результат объединения",
				fa1, fa2, new_dfa);
	Logger::finish_step();
	return new_dfa;
}

FiniteAutomaton FiniteAutomaton::difference(const FiniteAutomaton& fa1,
											const FiniteAutomaton& fa2) {
	Logger::init_step("Difference");
	set<alphabet_symbol> merged_alphabets = fa1.language->get_alphabet();
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
			new_identifier +=
				(state2.identifier.empty() ? "" : ", " + state2.identifier);
			new_dfa.states.push_back({counter,
									  {state1.index, state2.index},
									  new_identifier,
									  state1.is_terminal && !state2.is_terminal,
									  map<alphabet_symbol, set<int>>()});
			state_pair.push_back({state1.index, state2.index});
			counter++;
		}
	}

	for (int i = 0; i < new_dfa.states.size(); i++) {
		for (alphabet_symbol symb : new_dfa.language->get_alphabet()) {
			new_dfa.states[i].transitions[symb].insert(
				*new_dfa1.states[state_pair[i].first]
						.transitions.at(symb)
						.begin() *
					new_dfa2.states.size() +
				*new_dfa2.states[state_pair[i].second]
					 .transitions.at(symb)
					 .begin());
		}
	}
	new_dfa.language->set_alphabet(fa1.language->get_alphabet());
	for (int i = 0; i < new_dfa.states.size(); i++) {
		map<alphabet_symbol, set<int>> new_transitions;
		for (alphabet_symbol symb : merged_alphabets) {
			if (new_dfa.states[i].transitions.find(symb) !=
				new_dfa.states[i].transitions.end()) {
				new_transitions[symb] = new_dfa.states[i].transitions[symb];
			}
		}
		new_dfa.states[i].transitions = new_transitions;
	}
	new_dfa = new_dfa.remove_unreachable_states();
	Logger::log("Первый автомат", "Второй автомат", "Результат разности", fa1,
				fa2, new_dfa);
	Logger::finish_step();
	return new_dfa;
}

FiniteAutomaton FiniteAutomaton::complement() const {
	Logger::init_step("Complement");
	FiniteAutomaton new_dfa =
		FiniteAutomaton(initial_state, states, language->get_alphabet());
	new_dfa = new_dfa.determinize();
	for (int i = 0; i < new_dfa.states.size(); i++) {
		new_dfa.states[i].is_terminal = !new_dfa.states[i].is_terminal;
	}
	Logger::log("Автомат до дополнения", "Автомат после дополнения", *this,
				new_dfa);
	Logger::finish_step();
	return new_dfa;
}
FiniteAutomaton FiniteAutomaton::reverse() const {
	Logger::init_step("Reverse");
	FiniteAutomaton enfa =
		FiniteAutomaton(states.size(), states, language->get_alphabet());
	int final_states_counter = 0;
	for (int i = 0; i < enfa.states.size(); i++) {
		if (enfa.states[i].is_terminal) {
			final_states_counter++;
		}
	}
	int final_states_flag = 0;
	if (final_states_counter > 1) {
		final_states_flag = 1;
		enfa.states.push_back({enfa.initial_state,
							   {enfa.initial_state},
							   "RevS",
							   false,
							   map<alphabet_symbol, set<int>>()});
	}
	for (int i = 0; i < enfa.states.size() - final_states_flag; i++) {
		if (enfa.states[i].is_terminal) {
			enfa.states[i].is_terminal = false;
			if (final_states_counter > 1) {
				enfa.states[enfa.initial_state].transitions[epsilon()].insert(
					i);
			} else {
				enfa.initial_state = i;
			}
		}
	}
	enfa.states[initial_state].is_terminal = true;
	vector<map<alphabet_symbol, set<int>>> new_transition_matrix(
		enfa.states.size() - final_states_flag);
	for (int i = 0; i < enfa.states.size() - final_states_flag; i++) {
		for (const auto& transition : enfa.states[i].transitions) {
			for (int elem : transition.second) {
				new_transition_matrix[elem][transition.first].insert(
					enfa.states[i].index);
			}
		}
	}
	for (int i = 0; i < enfa.states.size() - final_states_flag; i++) {
		enfa.states[i].transitions = new_transition_matrix[i];
	}
	enfa = enfa.remove_unreachable_states();
	Logger::log("Автомат до обращения", "Автомат после обращения", *this, enfa);
	Logger::finish_step();
	return enfa;
}

FiniteAutomaton FiniteAutomaton::add_trap_state() const {
	Logger::init_step("AddTrapState");
	FiniteAutomaton new_dfa(initial_state, states, language->get_alphabet());
	bool flag = true;
	int count = new_dfa.states.size();
	for (int i = 0; i < count; i++) {
		for (alphabet_symbol symb : language->get_alphabet()) {
			if (!new_dfa.states[i].transitions[symb].size()) {
				if (flag) {
					new_dfa.states[i].set_transition(new_dfa.states.size(),
													 symb);
					int size = new_dfa.states.size();
					new_dfa.states.push_back(
						{size,
						 {size},
						 "",
						 false,
						 map<alphabet_symbol, set<int>>()});
				} else {
					new_dfa.states[i].set_transition(new_dfa.states.size() - 1,
													 symb);
				}
				flag = false;
			}
		}
	}
	if (!flag) {
		for (alphabet_symbol symb : language->get_alphabet()) {
			new_dfa.states[new_dfa.states.size() - 1].transitions[symb].insert(
				new_dfa.states.size() - 1);
		}
	}
	Logger::log("Автомат до добавления ловушки",
				"Автомат после добавления ловушки", *this, new_dfa);
	Logger::finish_step();
	return new_dfa;
}

FiniteAutomaton FiniteAutomaton::remove_trap_states() const {
	Logger::init_step("RemoveTrapState");
	FiniteAutomaton new_dfa(initial_state, states, language->get_alphabet());
	int count = new_dfa.states.size();
	for (int i = 0; i >= 0 && i < count; i++) {
		bool is_trap_state = true;
		set<int> reachable_states = new_dfa.closure({i}, false);
		for (int j = 0; j < new_dfa.states.size(); j++) {
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
			vector<State> new_states;
			for (int j = 0; j < new_dfa.states.size(); j++) {
				if (j < i) {
					new_states.push_back(new_dfa.states[j]);
				}
				if (j > i && i != count - 1) {
					new_states.push_back({new_dfa.states[j].index - 1,
										  new_dfa.states[j].label,
										  new_dfa.states[j].identifier,
										  new_dfa.states[j].is_terminal,
										  new_dfa.states[j].transitions});
				}
			}
			if (new_dfa.initial_state > i) new_dfa.initial_state -= 1;
			new_dfa.states = new_states;
			for (int j = 0; j < new_dfa.states.size(); j++) {
				map<alphabet_symbol, set<int>> new_transitions;
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
					if (new_transition.size())
						new_transitions[transition.first] = new_transition;
				}
				new_dfa.states[j].transitions = new_transitions;
			}
			i--;
			count--;
		}
	}
	Logger::log("Автомат до удаления ловушек", "Автомат после удаления ловушек",
				*this, new_dfa);
	Logger::finish_step();
	return new_dfa;
}

FiniteAutomaton FiniteAutomaton::remove_unreachable_states() const {
	FiniteAutomaton new_dfa(initial_state, states, language);
	int count = new_dfa.states.size();
	for (int i = 0; i >= 0 && i < count; i++) {
		bool is_unreachable_state = false;
		set<int> reachable_states =
			new_dfa.closure({new_dfa.initial_state}, false);
		if (find(reachable_states.begin(), reachable_states.end(), i) ==
			reachable_states.end()) {
			is_unreachable_state = true;
		}
		if (is_unreachable_state) {
			vector<State> new_states;
			for (int j = 0; j < new_dfa.states.size(); j++) {
				if (j < i) {
					new_states.push_back(new_dfa.states[j]);
				}
				if (j > i && i != count - 1) {
					new_states.push_back({new_dfa.states[j].index - 1,
										  new_dfa.states[j].label,
										  new_dfa.states[j].identifier,
										  new_dfa.states[j].is_terminal,
										  new_dfa.states[j].transitions});
				}
			}
			if (new_dfa.initial_state > i) new_dfa.initial_state -= 1;
			new_dfa.states = new_states;
			for (int j = 0; j < new_dfa.states.size(); j++) {
				map<alphabet_symbol, set<int>> new_transitions;
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
					if (new_transition.size())
						new_transitions[transition.first] = new_transition;
				}
				new_dfa.states[j].transitions = new_transitions;
			}
			i--;
			count--;
		}
	}
	return new_dfa;
}

FiniteAutomaton FiniteAutomaton::annote() const {
	Logger::init_step("Annote");
	set<alphabet_symbol> new_alphabet;
	FiniteAutomaton new_fa =
		FiniteAutomaton(initial_state, states, shared_ptr<Language>());
	vector<map<alphabet_symbol, set<int>>> new_transitions(
		new_fa.states.size());
	for (int i = 0; i < new_fa.states.size(); i++) {
		for (const auto& elem : new_fa.states[i].transitions) {
			if (elem.second.size() > 1) {
				int counter = 1;
				for (int transition_to : elem.second) {
					alphabet_symbol new_symb =
						to_string(elem.first) + to_string(counter);
					new_transitions[i][new_symb].insert(transition_to);
					new_alphabet.insert(new_symb);
					counter++;
				}
			} else {
				new_transitions[i][elem.first] =
					new_fa.states[i].transitions[elem.first];
				if (!is_epsilon(elem.first)) {
					new_alphabet.insert(elem.first);
				}
			}
		}
	}
	new_fa.language = shared_ptr<Language>(new Language(new_alphabet));
	for (int i = 0; i < new_transitions.size(); i++) {
		new_fa.states[i].transitions = new_transitions[i];
	}
	Logger::log("Автомат до навешивания разметки",
				"Автомат после навешивания разметки", *this, new_fa);
	Logger::finish_step();
	return new_fa;
}

FiniteAutomaton FiniteAutomaton::deannote() const {
	Logger::init_step("DeAnnote");
	set<alphabet_symbol> new_alphabet;
	FiniteAutomaton new_fa =
		FiniteAutomaton(initial_state, states, shared_ptr<Language>());
	vector<map<alphabet_symbol, set<int>>> new_transitions(
		new_fa.states.size());
	for (int i = 0; i < new_fa.states.size(); i++) {
		for (const auto& elem : new_fa.states[i].transitions) {
			if (elem.first.size() > 1) {
				alphabet_symbol symb = remove_numbers(elem.first);
				if (!is_epsilon(symb)) {
					new_alphabet.insert(symb);
				}
				for (int transition_to : elem.second) {
					new_transitions[i][symb].insert(transition_to);
				}
			} else {
				new_transitions[i][elem.first] =
					new_fa.states[i].transitions[elem.first];
				new_alphabet.insert(elem.first);
			}
		}
	}
	new_fa.language = shared_ptr<Language>(new Language(new_alphabet));
	for (int i = 0; i < new_transitions.size(); i++) {
		new_fa.states[i].transitions = new_transitions[i];
	}
	Logger::log("Автомат до удаления разметки",
				"Автомат после удаления разметки", *this, new_fa);
	Logger::finish_step();
	return new_fa;
}

FiniteAutomaton FiniteAutomaton::merge_equivalent_classes(
	vector<int> classes) const {
	map<int, vector<int>>
		class_to_index; // нужен для подсчета количества классов
	for (int i = 0; i < classes.size(); i++)
		class_to_index[classes[i]].push_back(i);
	// индексы состояний в новом автомате соответсвуют номеру класса
	// эквивалентности
	vector<State> new_states;
	for (int i = 0; i < class_to_index.size(); i++) {
		string new_identifier;
		for (int index : class_to_index[i])
			new_identifier +=
				(new_identifier.empty() ? "" : ", ") + states[index].identifier;
		State s = {
			i, {i}, new_identifier, false, map<alphabet_symbol, set<int>>()};
		new_states.push_back(s);
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

	return FiniteAutomaton(classes[initial_state], new_states, language);
}

FiniteAutomaton FiniteAutomaton::merge_bisimilar() const {
	Logger::init_step("MergeBisim");

	vector<GrammarItem> fa_items;
	vector<GrammarItem*> nonterminals;
	vector<GrammarItem*> terminals;
	vector<vector<vector<GrammarItem*>>> rules = Grammar::fa_to_grammar(
		states, language->get_alphabet(), fa_items, nonterminals, terminals);
	vector<GrammarItem*> bisimilar_nonterminals;
	map<int, vector<GrammarItem*>> class_to_nonterminals;
	vector<vector<vector<GrammarItem*>>> bisimilar_rules =
		Grammar::get_bisimilar_grammar(
			rules, nonterminals, bisimilar_nonterminals, class_to_nonterminals);

	// log
	vector<int> classes;
	for (const auto& nont : nonterminals)
		classes.push_back(nont->class_number);
	FiniteAutomaton result_fa = merge_equivalent_classes(classes);

	Logger::log("Автомат до преобразования", "Автомат после преобразования",
				*this, result_fa);
	stringstream ss;
	for (auto& elem : class_to_nonterminals) {
		ss << "\\{";
		for (int i = 0; i < elem.second.size() - 1; i++)
			ss << elem.second[i]->name << ",";
		ss << elem.second[elem.second.size() - 1]->name << "\\}";
	}
	Logger::log("Эквивалентные классы", ss.str());
	Logger::finish_step();
	return result_fa;
}

bool FiniteAutomaton::bisimilarity_checker(const FiniteAutomaton& fa1,
										   const FiniteAutomaton& fa2) {
	// грамматики из автоматов
	vector<GrammarItem> fa1_items;
	vector<GrammarItem*> fa1_nonterminals;
	vector<GrammarItem*> fa1_terminals;
	vector<vector<vector<GrammarItem*>>> fa1_rules =
		Grammar::fa_to_grammar(fa1.states, fa1.language->get_alphabet(),
							   fa1_items, fa1_nonterminals, fa1_terminals);

	vector<GrammarItem> fa2_items;
	vector<GrammarItem*> fa2_nonterminals;
	vector<GrammarItem*> fa2_terminals;
	vector<vector<vector<GrammarItem*>>> fa2_rules =
		Grammar::fa_to_grammar(fa2.states, fa2.language->get_alphabet(),
							   fa2_items, fa2_nonterminals, fa2_terminals);

	if (fa1_terminals.size() != fa2_terminals.size()) return false;
	for (int i = 0; i < fa1_terminals.size(); i++)
		if (*fa1_terminals[i] != *fa2_terminals[i]) return false;
	// сначала получаем бисимилярные грамматики из данных автоматов
	vector<GrammarItem*> fa1_bisimilar_nonterminals;
	map<int, vector<GrammarItem*>> fa1_class_to_nonterminals;
	vector<vector<vector<GrammarItem*>>> fa1_bisimilar_rules =
		Grammar::get_bisimilar_grammar(fa1_rules, fa1_nonterminals,
									   fa1_bisimilar_nonterminals,
									   fa1_class_to_nonterminals);

	vector<GrammarItem*> fa2_bisimilar_nonterminals;
	map<int, vector<GrammarItem*>> fa2_class_to_nonterminals;
	vector<vector<vector<GrammarItem*>>> fa2_bisimilar_rules =
		Grammar::get_bisimilar_grammar(fa2_rules, fa2_nonterminals,
									   fa2_bisimilar_nonterminals,
									   fa2_class_to_nonterminals);
	if (fa1_bisimilar_nonterminals.size() != fa2_bisimilar_nonterminals.size())
		return false;
	// из объединения полученных ранее получаем итоговую
	vector<GrammarItem*> nonterminals(fa1_bisimilar_nonterminals);
	nonterminals.insert(nonterminals.end(), fa2_bisimilar_nonterminals.begin(),
						fa2_bisimilar_nonterminals.end());
	vector<vector<vector<GrammarItem*>>> rules(fa1_bisimilar_rules);
	rules.insert(rules.end(), fa2_bisimilar_rules.begin(),
				 fa2_bisimilar_rules.end());

	vector<int> fa1_classes; // сохраяняю классы
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
	vector<vector<vector<GrammarItem*>>> bisimilar_rules =
		Grammar::get_bisimilar_grammar(
			rules, nonterminals, bisimilar_nonterminals, class_to_nonterminals);

	map<int, vector<string>> class_to_nonterminals_names;

	for (int i = 0; i < fa1_nonterminals.size(); i++) {
		int nont_class =
			fa1_class_to_nonterminals[fa1_classes[i]][0]
				->class_number; // класс нетерминала в общей грамматике, 0й
								// элемент попал в бисимилярную грамматику
		class_to_nonterminals_names[nont_class].push_back(
			"FA1:" + fa1_nonterminals[i]->name);
	}

	for (int i = 0; i < fa2_nonterminals.size(); i++) {
		int nont_class =
			fa2_class_to_nonterminals[fa2_classes[i]][0]
				->class_number; // класс нетерминала в общей грамматике, 0й
								// элемент попал в бисимилярную грамматику
		class_to_nonterminals_names[nont_class].push_back(
			"FA2:" + fa2_nonterminals[i]->name);
	}
	// log
	stringstream ss;
	for (auto& elem : class_to_nonterminals_names) {
		ss << "\\{";
		for (int i = 0; i < elem.second.size() - 1; i++)
			ss << elem.second[i] << ",";
		ss << elem.second[elem.second.size() - 1] << "\\}";
	}
	Logger::log("Эквивалентные классы", ss.str());

	// проверяю равенство классов начальных состояний
	if (fa1_nonterminals[fa1.initial_state]->class_number !=
		fa2_nonterminals[fa2.initial_state]->class_number)
		return false;
	if (fa1_bisimilar_nonterminals.size() != bisimilar_nonterminals.size())
		return false;

	return true;
}

bool FiniteAutomaton::bisimilar(const FiniteAutomaton& fa1,
								const FiniteAutomaton& fa2) {
	Logger::init_step("Bisimilar");
	Logger::log("Автоматы:");
	Logger::log("Первый автомат", "Второй автомат", fa1, fa2);
	bool result = bisimilarity_checker(fa1, fa2);
	if (result)
		Logger::log("Результат Bisimilar", "true");
	else
		Logger::log("Результат Bisimilar", "false");
	Logger::finish_step();
	return result;
}

bool FiniteAutomaton::equality_checker(const FiniteAutomaton& fa1,
									   const FiniteAutomaton& fa2) {
	// проверка равенства количества состояний
	if (fa1.states.size() != fa2.states.size()) return false;
	// грамматики из состояний автоматов
	vector<GrammarItem> fa1_items;
	vector<GrammarItem*> fa1_nonterminals;
	vector<GrammarItem*> fa1_terminals;
	vector<vector<vector<GrammarItem*>>> fa1_rules =
		Grammar::fa_to_grammar(fa1.states, fa1.language->get_alphabet(),
							   fa1_items, fa1_nonterminals, fa1_terminals);

	vector<GrammarItem> fa2_items;
	vector<GrammarItem*> fa2_nonterminals;
	vector<GrammarItem*> fa2_terminals;
	vector<vector<vector<GrammarItem*>>> fa2_rules =
		Grammar::fa_to_grammar(fa2.states, fa2.language->get_alphabet(),
							   fa2_items, fa2_nonterminals, fa2_terminals);
	// проверка на равенство алфавитов
	if (fa1_terminals.size() != fa2_terminals.size()) return false;
	for (int i = 0; i < fa1_terminals.size(); i++)
		if (*fa1_terminals[i] != *fa2_terminals[i]) return false;
	// биективная бисимуляция состояний
	vector<GrammarItem*> nonterminals(fa1_nonterminals);
	nonterminals.insert(nonterminals.end(), fa2_nonterminals.begin(),
						fa2_nonterminals.end());
	vector<vector<vector<GrammarItem*>>> rules(fa1_rules);
	rules.insert(rules.end(), fa2_rules.begin(), fa2_rules.end());
	for (GrammarItem* nont : nonterminals)
		nont->class_number = 0; // сбрасываю номера классов
	vector<GrammarItem*> bisimilar_nonterminals;
	map<int, vector<GrammarItem*>> class_to_nonterminals;
	vector<vector<vector<GrammarItem*>>> bisimilar_rules =
		Grammar::get_bisimilar_grammar(
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
		if (t != 0) return false;

	vector<int> bisimilar_classes;
	for (GrammarItem* nont : nonterminals)
		bisimilar_classes.push_back(nont->class_number);
	// биективная бисимуляция обратных грамматик
	vector<vector<vector<GrammarItem*>>> fa1_reverse_rules =
		Grammar::get_reverse_grammar(fa1_rules, fa1_nonterminals, fa1_terminals,
									 fa1.initial_state);
	vector<vector<vector<GrammarItem*>>> fa2_reverse_rules =
		Grammar::get_reverse_grammar(fa2_rules, fa2_nonterminals, fa2_terminals,
									 fa2.initial_state);

	vector<vector<vector<GrammarItem*>>> reverse_rules(fa1_reverse_rules);
	reverse_rules.insert(reverse_rules.end(), fa2_reverse_rules.begin(),
						 fa2_reverse_rules.end());
	for (GrammarItem* nont : nonterminals)
		nont->class_number = -1; // сбрасываю номера классов

	vector<GrammarItem*> reverse_bisimilar_nonterminals;
	vector<vector<vector<GrammarItem*>>> reverse_bisimilar_rules =
		Grammar::get_bisimilar_grammar(reverse_rules, nonterminals,
									   reverse_bisimilar_nonterminals,
									   class_to_nonterminals);
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
		if (nonterminals[i]->class_number != -1) continue;
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
	vector<pair<GrammarItem, map<alphabet_symbol, vector<GrammarItem>>>>
		transitions1_items;
	vector<GrammarItem*> transitions1_nonterminals;
	vector<GrammarItem*> transitions1_terminals;
	vector<vector<vector<GrammarItem*>>> transitions1_rules =
		Grammar::tansitions_to_grammar(
			fa1.states, fa1_nonterminals, transitions1_items,
			transitions1_nonterminals, transitions1_terminals);

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

	vector<pair<GrammarItem, map<alphabet_symbol, vector<GrammarItem>>>>
		transitions2_items;
	vector<GrammarItem*> transitions2_nonterminals;
	vector<GrammarItem*> transitions2_terminals;
	vector<vector<vector<GrammarItem*>>> transitions2_rules =
		Grammar::tansitions_to_grammar(
			fa2.states, fa2_nonterminals, transitions2_items,
			transitions2_nonterminals, transitions2_terminals);

	// проверка равенства количества переходов
	if (transitions1_nonterminals.size() != transitions2_nonterminals.size())
		return false;
	// биективная бисимуляция переходов
	vector<GrammarItem*> transitions_nonterminals(transitions1_nonterminals);
	transitions_nonterminals.insert(transitions_nonterminals.end(),
									transitions2_nonterminals.begin(),
									transitions2_nonterminals.end());
	vector<vector<vector<GrammarItem*>>> transitions_rules(transitions1_rules);
	transitions_rules.insert(transitions_rules.end(),
							 transitions2_rules.begin(),
							 transitions2_rules.end());
	for (GrammarItem* nont : transitions_nonterminals)
		nont->class_number = 0; // сбрасываю номера классов
	vector<GrammarItem*> transitions_bisimilar_nonterminals;
	vector<vector<vector<GrammarItem*>>> transitions_bisimilar_rules =
		Grammar::get_bisimilar_grammar(
			transitions_rules, transitions_nonterminals,
			transitions_bisimilar_nonterminals, class_to_nonterminals);
	// проверяю бисимилярность переходов
	classes.clear();
	classes.resize(transitions_bisimilar_nonterminals.size(), 0);
	for (auto t : transitions1_nonterminals)
		classes[t->class_number]++;
	for (auto t : transitions2_nonterminals)
		classes[t->class_number]--;
	for (auto t : classes)
		if (t != 0) return false;

	return true;
}

bool FiniteAutomaton::equal(const FiniteAutomaton& fa1,
							const FiniteAutomaton& fa2) {
	Logger::init_step("Equal");
	Logger::log("Автоматы:");
	Logger::log("Первый автомат", "Второй автомат", fa1, fa2);
	bool result = equality_checker(fa1, fa2);
	if (result)
		Logger::log("Результат Equal", "true");
	else
		Logger::log("Результат Equal", "false");
	Logger::finish_step();
	return result;
}

bool FiniteAutomaton::equivalent(const FiniteAutomaton& fa1,
								 const FiniteAutomaton& fa2) {
	Logger::init_step("Equiv");
	Logger::log("Автоматы:");
	Logger::log("Первый автомат", "Второй автомат", fa1, fa2);
	bool result = equal(fa1.minimize(), fa2.minimize());
	if (result)
		Logger::log("Результат Equiv", "true");
	else
		Logger::log("Результат Equiv", "false");
	Logger::finish_step();
	return result;
}

bool FiniteAutomaton::subset(const FiniteAutomaton& fa) const {
	Logger::init_step("Subset");
	Logger::log("Автоматы:");
	Logger::log("Первый автомат", "Второй автомат", *this, fa);
	FiniteAutomaton dfa1(determinize());
	FiniteAutomaton dfa2(fa.determinize());
	FiniteAutomaton dfa_instersection(intersection(dfa1, dfa2));
	bool result = equivalent(dfa_instersection, dfa2);
	if (result)
		Logger::log("Результат Subset", "true");
	else
		Logger::log("Результат Subset", "false");
	Logger::finish_step();
	return result;
}

Fraction calc_ambiguity(int i, int n, const vector<Fraction>& f1,
						vector<vector<Fraction>>& calculated,
						vector<vector<char>>& is_calculated) {
	if (i == 0) return f1[n];
	Fraction d1, d2;
	if (!is_calculated[i][n + 1]) {
		calculated[i][n + 1] =
			calc_ambiguity(i - 1, n + 1, f1, calculated, is_calculated);
		is_calculated[i][n + 1] = 1;
	}
	d1 = calculated[i][n + 1];
	if (!is_calculated[i][n]) {
		calculated[i][n] =
			calc_ambiguity(i - 1, n, f1, calculated, is_calculated);
		is_calculated[i][n] = 1;
	}
	d2 = calculated[i][n];
	return d1 - d2;
}

FiniteAutomaton::AmbiguityValue FiniteAutomaton::get_ambiguity_value() const {
	FiniteAutomaton fa = remove_eps();
	FiniteAutomaton min_fa = fa.minimize().remove_trap_states();
	fa = fa.remove_trap_states();

	int i = 2;
	int s = fa.states.size();
	int min_s = min_fa.states.size();
	int N = s * s + s + i + 1;
	vector<InfInt> paths_number(N);
	vector<vector<int>> adjacency_matrix(s, vector<int>(s));
	vector<InfInt> min_paths_number(N);
	vector<vector<int>> min_adjacency_matrix(min_s, vector<int>(min_s));
	vector<vector<InfInt>> d(N + 1, vector<InfInt>(s));
	vector<vector<InfInt>> min_d(N + 1, vector<InfInt>(min_s));
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
	optional<Fraction> prev_val;
	bool return_flag = true;
	for (int k = 1; k < N + 1; k++) {
		for (int v = 0; v < s; v++) {
			for (int i = 0; i < s; i++) {
				d[k][v] += InfInt(adjacency_matrix[i][v]) * d[k - 1][i];
			}
			if (fa.states[v].is_terminal)
				paths_number[k - 1] = paths_number[k - 1] + d[k][v];
		}
		for (int v = 0; v < min_s; v++) {
			for (int i = 0; i < min_s; i++) {
				min_d[k][v] +=
					InfInt(min_adjacency_matrix[i][v]) * min_d[k - 1][i];
			}
			if (min_fa.states[v].is_terminal)
				min_paths_number[k - 1] = min_paths_number[k - 1] + min_d[k][v];
		}
		if (min_paths_number[k - 1] == 0) continue;
		f1.push_back(Fraction(paths_number[k - 1], min_paths_number[k - 1]));

		if (!prev_val) {
			prev_val = f1[f1.size() - 1];
			continue;
		}
		if (!(f1[f1.size() - 1] == *prev_val)) {
			return_flag = false;
		}
		prev_val = f1[f1.size() - 1];
	}

	if (return_flag) {
		if (*prev_val == Fraction(1, 1))
			return FiniteAutomaton::unambigious;
		else
			return FiniteAutomaton::almost_unambigious;
	}

	if (f1.size() < 3) return FiniteAutomaton::polynomially_ambigious;
	int new_s = floor(double(-1 + sqrt(-11 + 4 * f1.size())) / 2);
	i += f1.size() - (new_s * new_s + new_s + i + 1);

	vector<vector<Fraction>> calculated(new_s + i + 1,
										vector<Fraction>(f1.size()));
	vector<vector<char>> is_calculated(new_s + i + 1,
									   vector<char>(f1.size(), 0));
	Fraction val =
		calc_ambiguity(new_s + i, new_s * new_s, f1, calculated, is_calculated);
	prev_val = val;
	while (val > Fraction()) {
		i++;
		int return_counter = 0;
		do {
			if (return_counter == s)
				return FiniteAutomaton::polynomially_ambigious;
			N++;
			d.push_back(vector<InfInt>(s));
			paths_number.push_back(0);
			for (int v = 0; v < s; v++) {
				for (int i = 0; i < s; i++) {
					d[N][v] += InfInt(adjacency_matrix[i][v]) * d[N - 1][i];
				}
				if (fa.states[v].is_terminal) paths_number[N - 1] += d[N][v];
			}
			min_d.push_back(vector<InfInt>(s));
			min_paths_number.push_back(0);
			for (int v = 0; v < min_s; v++) {
				for (int i = 0; i < min_s; i++) {
					min_d[N][v] +=
						InfInt(min_adjacency_matrix[i][v]) * min_d[N - 1][i];
				}
				if (min_fa.states[v].is_terminal)
					min_paths_number[N - 1] += min_d[N][v];
			}
			return_counter++;
		} while (min_paths_number[N - 1] == 0);
		f1.push_back(Fraction(paths_number[N - 1], min_paths_number[N - 1]));
		for (int j = 0; j < calculated.size(); j++) {
			calculated[j].push_back(Fraction());
			is_calculated[j].push_back(0);
		}
		calculated.push_back(vector<Fraction>(f1.size()));
		is_calculated.push_back(vector<char>(f1.size(), 0));
		val = calc_ambiguity(new_s + i, new_s * new_s, f1, calculated,
							 is_calculated);
		if (val > *prev_val || val == *prev_val)
			return FiniteAutomaton::exponentially_ambiguous;
		prev_val = val;
	}
	return FiniteAutomaton::polynomially_ambigious;
}

FiniteAutomaton::AmbiguityValue FiniteAutomaton::ambiguity() const {
	Logger::init_step("Ambiguity");
	FiniteAutomaton::AmbiguityValue result = get_ambiguity_value();
	Logger::log("Автомат:", *this);
	switch (result) {
	case FiniteAutomaton::exponentially_ambiguous:
		Logger::log("Результат Ambiguity", "Exponentially ambiguous");
		break;
	case FiniteAutomaton::almost_unambigious:
		Logger::log("Результат Ambiguity", "Almost unambigious");
		break;
	case FiniteAutomaton::unambigious:
		Logger::log("Результат Ambiguity", "Unambigious");
		break;
	case FiniteAutomaton::polynomially_ambigious:
		Logger::log("Результат Ambiguity", "Polynomially ambiguous");
		break;
	default:
		break;
	}
	Logger::finish_step();
	return result;
}

std::optional<std::string> FiniteAutomaton::get_prefix(
	int state_beg, int state_end, map<int, bool>& was) const {
	std::optional<std::string> ans = nullopt;
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
					ans = it->first + res.value();
				}
				return ans;
			}
		}
	}
	return ans;
}

bool FiniteAutomaton::semdet() const {
	Logger::init_step("SemDet");
	Logger::log(
		"Получение языка из производной регулярки автомата по префиксу");
	std::map<int, bool> was;
	std::vector<int> final_states;
	for (int i = 0; i < states.size(); i++) {
		if (states[i].is_terminal) final_states.push_back(i);
	}
	std::vector<Regex> state_languages;
	state_languages.resize(states.size());
	for (int i = 0; i < states.size(); i++) {
		auto prefix = get_prefix(initial_state, i, was);
		was.clear();
		// cout << "Try " << i << "\n";
		if (!prefix.has_value()) continue;
		Regex reg;
		// Получение языка из производной регулярки автомата по префиксу:
		//		this -> reg (arden?)
		reg = nfa_to_regex();
		// cout << "State: " << i << "\n";
		// cout << "Prefix: " << prefix.value() << "\n";
		// cout << "Regex: " << reg.to_txt() << "\n";
		Logger::log("State", to_string(i));
		Logger::log("Prefix", prefix.value());
		Logger::log("Regex", reg.to_txt());
		auto derevative = reg.prefix_derevative(prefix.value());
		if (!derevative.has_value()) continue;
		state_languages[i] = derevative.value();
		// cout << "Derevative: " << state_languages[i].to_txt() << "\n";
		Logger::log("Derevative", state_languages[i].to_txt());
		state_languages[i].make_language();
	}
	for (int i = 0; i < states.size(); i++) {
		for (int j = 0; j < states.size(); j++) {
			for (auto transition = states[j].transitions.begin();
				 transition != states[j].transitions.end(); transition++) {
				bool verified_ambiguity = false;
				for (auto it = transition->second.begin();
					 it != transition->second.end(); it++) {
					bool reliability = true;
					for (auto it2 = transition->second.begin();
						 it2 != transition->second.end(); it2++) {
						if (!state_languages[*it].subset(
								state_languages[*it2])) {
							reliability = false;
							break;
						}
					}
					verified_ambiguity |= reliability;
				}
				if (!verified_ambiguity) {
					Logger::log("Результат SemDet", "false");
					Logger::finish_step();
					return false;
				}
			}
		}
	}
	Logger::log("Результат SemDet", "true");
	Logger::finish_step();
	return true;
}

bool FiniteAutomaton::parsing_nfa(const string& s, int index_state) const {
	// cout << s.size() << endl;
	State state = states[index_state];

	if (s.size() == 0 && state.is_terminal) {
		return true;
	}
	set<int> tr_eps =
		state.transitions[epsilon()]; // char_to_alphabet_symbol('\0')];
	vector<int> trans_eps{tr_eps.begin(), tr_eps.end()};
	int n;
	// tr_eps = {};
	if (s.size() == 0 && !state.is_terminal) {
		n = trans_eps.size();
		for (size_t i = 0; i < trans_eps.size(); i++) {
			// cout << trans_eps[i] << endl;
			if (parsing_nfa(s, trans_eps[i])) {
				return true;
			}
		}
		return false;
	}

	alphabet_symbol elem = char_to_alphabet_symbol(s[0]);
	set<int> tr = state.transitions[elem];
	vector<int> trans{tr.begin(), tr.end()};
	// tr = {};
	n = trans.size();
	for (size_t i = 0; i < n; i++) {
		if (parsing_nfa(s.substr(1), trans[i])) {
			return true;
		}
	}
	n = trans_eps.size();
	for (size_t i = 0; i < n; i++) {
		// cout << trans_eps[i] << endl;
		if (parsing_nfa(s, trans_eps[i])) {
			return true;
		}
	}
	return false;
}

bool FiniteAutomaton::parsing_nfa_for(const string& s) const {
	// cout << s.size() << endl;
	stack<State> stac_state;
	stack<int> stack_s;
	stack<int> stack_index;
	stac_state.push(states[0]);
	stack_s.push(0);
	stack_index.push(0);
	int n, n_eps;
	int index = 0;
	int state_s = 0;
	State state = stac_state.top();
	// cout << !stac_state.empty() << endl;
	while (!stac_state.empty()) {
		state = stac_state.top();
		state_s = stack_s.top();
		index = stack_index.top();
		if (state.is_terminal && index == s.size()) {
			break;
		}
		stac_state.pop();
		stack_s.pop();
		stack_index.pop();
		alphabet_symbol elem = char_to_alphabet_symbol(s[index]);
		set<int> tr = state.transitions[elem];
		vector<int> trans{tr.begin(), tr.end()};
		set<int> tr_eps =
			state.transitions[epsilon()]; // char_to_alphabet_symbol('\0')];
		vector<int> trans_eps{tr_eps.begin(), tr_eps.end()};
		// tr = {};
		// cout << elem << " " << state.identifier << " " << index << " "
		//	 << stac_state.size() << endl;
		n = trans.size();
		n_eps = trans_eps.size();
		if (n + n_eps == 0) {
			// stac_state.pop();
			//  stack_s.pop();
			if (state_s != 0) {
				// index--;
			}
		}
		for (size_t i = 0; i < n; i++) {
			if (index + 1 <= s.size()) {
				stac_state.push(states[trans[i]]);
				stack_s.push(1);
				stack_index.push(index + 1);
			}
		}
		for (size_t i = 0; i < n_eps; i++) {
			stac_state.push(states[trans_eps[i]]);
			stack_s.push(0);
			stack_index.push(index);
		}
		// if (state_s != 0) {
		//	index++;
		// }
	}
	if (/*(stac_state.empty() && s.size() <= index) ||*/
		(s.size() == index && state.is_terminal)) {
		return true;
	}
	return false;
}

bool FiniteAutomaton::is_deterministic() {
	for (int i = 0; i < states.size(); i++) {
		for (auto elem : states[i].transitions) {
			if (elem.first == "/0") {
				return false;
			}
			if (elem.second.size() > 1) {
				return false;
			}
		}
	}
	return true;
}

bool FiniteAutomaton::parsing_by_nfa(const string& s) const {
	State state = states[0];
	return parsing_nfa_for(s);
}

int FiniteAutomaton::get_initial() {
	return initial_state;
}

int FiniteAutomaton::states_number() const {
	Logger::init_step("States");
	Logger::log(to_string(states.size()));
	Logger::finish_step();
	return states.size();
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

bool compare(expression_arden a, expression_arden b) {
	return (a.condition < b.condition);
}

vector<expression_arden> arden_minimize(vector<expression_arden> in) {
	vector<expression_arden> out;
	for (int i = 0; i < in.size(); i++) {
		int j = i + 1;
		bool cond = false;
		expression_arden temp;
		temp.temp_regex = in[i].temp_regex->copy();
		temp.condition = in[i].condition;
		out.push_back(temp);
		while ((j < in.size()) && in[i].condition == in[j].condition) {
			cond = true;
			Regex* r = new Regex();
			Regex* s1 = new Regex();
			Regex* s2 = new Regex();
			s1->regex_star(out[i].temp_regex);
			s2->regex_star(in[j].temp_regex);
			r->regex_union(s1, s2);
			delete out[out.size() - 1].temp_regex;
			out[out.size() - 1].temp_regex = r;
			delete s1;
			delete s2;
			j++;
		}
		if (cond) {
			i = j - 1;
		}
	}
	return out;
}

vector<expression_arden> arden(vector<expression_arden> in, int index) {
	vector<expression_arden> out;
	int indexcur = -1;
	for (int i = 0; (i < in.size() && indexcur == -1); i++) {
		if (in[i].condition == index) {
			indexcur = i;
		}
	}
	if (indexcur == -1) {
		for (int i = 0; i < in.size(); i++) {
			Regex* r = in[i].temp_regex->copy();
			expression_arden temp;
			temp.temp_regex = r;
			temp.condition = in[i].condition;
			out.push_back(temp);
		}
		return out;
	}
	if (in.size() < 2) {
		Regex* r = new Regex();
		r->regex_star(in[0].temp_regex);
		expression_arden temp;
		temp.temp_regex = r;
		temp.condition = -1;
		out.push_back(temp);
		return out;
	}
	for (int i = 0; i < in.size(); i++) {
		if (i != indexcur) {
			Regex* r = new Regex();
			r->regex_star(in[indexcur].temp_regex);
			Regex* k = new Regex();
			k->regex_union(in[i].temp_regex, r);
			expression_arden temp;

			delete r;
			temp.temp_regex = k;
			temp.condition = in[i].condition;
			out.push_back(temp);
		}
	}
	return out;
}
Regex FiniteAutomaton::nfa_to_regex() const {
	Logger::init_step("Arden");
	vector<int> endstate;
	vector<vector<expression_arden>> data;
	set<alphabet_symbol> alphabet = language->get_alphabet();

	for (int i = 0; i < states.size(); i++) {
		vector<expression_arden> temp;
		data.push_back(temp);
	}
	Regex* r = new Regex;
	expression_arden temp;
	temp.condition = -1;
	string str = "";
	r->regex_eps();
	temp.temp_regex = r;
	data[initial_state].push_back(temp);
	for (int i = 0; i < states.size(); i++) {
		State a = states[i];
		if (a.is_terminal) {
			endstate.push_back(i);
		}
		if (a.transitions["eps"].size()) {
			set<int> trans = a.transitions.at("eps");
			for (set<int>::iterator itt = trans.begin(); itt != trans.end();
				 itt++) {
				Regex* r = new Regex;
				expression_arden temp;
				temp.condition = i;

				r->regex_eps();
				temp.temp_regex = r;
				data[*itt].push_back(temp);
			}
		}
		for (set<alphabet_symbol>::iterator it = alphabet.begin();
			 it != alphabet.end(); it++) {
			if (a.transitions[*it].size()) {
				set<int> trans = a.transitions.at(*it);
				for (set<int>::iterator itt = trans.begin(); itt != trans.end();
					 itt++) {
					expression_arden temp;
					temp.condition = i;
					string str = "";
					str += *it;
					Regex* r = new Regex(str);
					temp.temp_regex = r;
					data[*itt].push_back(temp);
				}
			}
		}
	}
	// //сортируем

	for (int i = data.size() - 1; i >= 0; i--) {

		vector<expression_arden> tempdata;
		for (int j = 0; j < data[i].size(); j++) {
			if (data[i][j].condition > i) {
				for (int k = 0; k < data[data[i][j].condition].size(); k++) {
					expression_arden temp;
					Regex* r = new Regex;
					r->regex_union(data[data[i][j].condition][k].temp_regex,
								   data[i][j].temp_regex);
					temp.temp_regex = r;
					temp.condition = data[data[i][j].condition][k].condition;
					tempdata.push_back(temp);
				}
			} else {
				expression_arden temp;
				Regex* r = new Regex(*data[i][j].temp_regex);
				temp.temp_regex = r;
				temp.condition = data[i][j].condition;
				tempdata.push_back(temp);
			}
		}

		sort(data[i].begin(), data[i].end(), compare);
		for (int o = 0; o < data[i].size(); o++) {
			delete data[i][o].temp_regex;
		}
		data[i].clear();
		vector<expression_arden> tempdata1 = arden_minimize(tempdata);
		vector<expression_arden> tempdata2 = arden(tempdata1, i);
		for (int o = 0; o < tempdata.size(); o++) {
			delete tempdata[o].temp_regex;
		}
		for (int o = 0; o < tempdata1.size(); o++) {
			delete tempdata1[o].temp_regex;
		}

		for (int o = 0; o < tempdata2.size(); o++) {
		}
		data[i] = tempdata2;
	}
	if (data[0].size() > 1) {
		// cout << "error";
		Regex* f = new Regex("");
		Regex temp1 = *f;
		delete f;
		return temp1;
	}
	for (int i = 0; i < data.size(); i++) {
		for (int j = 0; j < data[i].size(); j++) {
			if (data[i][j].condition != -1) {
				Regex* ra = new Regex;
				ra->regex_union(data[data[i][j].condition][0].temp_regex,
								data[i][j].temp_regex);
				data[i][j].condition = -1;
				delete data[i][j].temp_regex;
				data[i][j].temp_regex = ra;
			}
		}

		vector<expression_arden> tempdata3 = arden_minimize(data[i]);
		for (int o = 0; o < data[i].size(); o++) {
			delete data[i][o].temp_regex;
		}
		data[i].clear();
		data[i] = tempdata3;
		Logger::log("State ", std::to_string(i));
		for (int j = 0; j < data[i].size(); j++) {

			Logger::log("from state ", std::to_string(data[i][j].condition));
			Logger::log("with regex ", data[i][j].temp_regex->to_txt());
		}
	}
	if (endstate.size() == 0) {
		Regex* f = new Regex("");
		Regex temp1 = *f;
		delete f;
		return temp1;
	}
	if (endstate.size() < 2) {
		Regex* r1;
		r1 = data[endstate[0]][0].temp_regex->copy();
		for (int i = 0; i < data.size(); i++) {
			for (int j = 0; j < data[i].size(); j++) {
				delete data[i][j].temp_regex;
			}
		}
		Regex temp = *r;
		delete r;
		return temp;
	}
	Regex* r1;
	r1 = data[endstate[0]][0].temp_regex->copy();
	for (int i = 1; i < endstate.size(); i++) {
		Regex* r2 = new Regex;
		r2->regex_alt(r1, data[endstate[i]][0].temp_regex);
		delete r1;
		r1 = r2;
	}
	for (int i = 0; i < data.size(); i++) {
		for (int j = 0; j < data[i].size(); j++) {
			delete data[i][j].temp_regex;
		}
	}
	Regex temp1 = *r1;
	delete r1;
	Logger::finish_step();
	return temp1;
	// return r1;
}