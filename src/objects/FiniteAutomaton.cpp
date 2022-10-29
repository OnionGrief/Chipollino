#include "FiniteAutomaton.h"
#include "Grammar.h"
#include "Language.h"
#include <algorithm>
#include <iostream>
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
	FiniteAutomaton dfa = FiniteAutomaton(0, {}, language);
	set<int> q0 = closure({0}, true);

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
	return dfa;
}

FiniteAutomaton FiniteAutomaton::minimize() const {
	const optional<FiniteAutomaton>& language_min_dfa = language->get_min_dfa();
	if (language->get_min_dfa())
		return *language_min_dfa; // Нужно решить, что делаем с идентификаторами
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
	return minimized_dfa;
}

FiniteAutomaton FiniteAutomaton::remove_eps() const {
	FiniteAutomaton new_nfa(initial_state, states, language);

	for (auto& state : new_nfa.states)
		state.transitions = map<alphabet_symbol, set<int>>();

	for (int i = 0; i < states.size(); i++) {
		set<int> q = closure({states[i].index}, true);
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
				if (new_nfa.states[elem].is_terminal) {
					new_nfa.states[i].is_terminal = true;
				}
				new_nfa.states[i].transitions[symb].insert(elem);
			}
		}
	}
	return new_nfa;
}

FiniteAutomaton FiniteAutomaton::intersection(const FiniteAutomaton& fa1,
											  const FiniteAutomaton& fa2) {
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
	return new_dfa;
}

FiniteAutomaton FiniteAutomaton::uunion(const FiniteAutomaton& fa1,
										const FiniteAutomaton& fa2) {
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

	return new_dfa;
}

FiniteAutomaton FiniteAutomaton::difference(const FiniteAutomaton& fa1,
											const FiniteAutomaton& fa2) {
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

	return new_dfa;
}

FiniteAutomaton FiniteAutomaton::complement() const {
	FiniteAutomaton new_dfa =
		FiniteAutomaton(initial_state, states, language->get_alphabet());
	for (int i = 0; i < new_dfa.states.size(); i++) {
		new_dfa.states[i].is_terminal = !new_dfa.states[i].is_terminal;
	}
	return new_dfa;
}
FiniteAutomaton FiniteAutomaton::reverse() const {
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
	return enfa;
}

FiniteAutomaton FiniteAutomaton::add_trap_state() const {
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
						 to_string(size),
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
	return new_dfa;
}

FiniteAutomaton FiniteAutomaton::remove_trap_states() const {
	vector<map<alphabet_symbol, set<int>>> new_transitions;
	FiniteAutomaton new_dfa(initial_state, states, language->get_alphabet());
	int count = new_dfa.states.size();
	for (int i = 0; i >= 0 && i < count; i++) {
		bool is_trap_state = false;
		set<int> reachable_states = new_dfa.closure({i}, false);
		for (int j = 0; j < new_dfa.states.size(); j++) {
			if (states[j].is_terminal) {
				bool is_state_found = false;
				for (auto elem : reachable_states) {
					if (j == elem) {
						is_state_found = true;
					}
				}
				if (is_state_found) {
					break;
				} else {
					is_trap_state = true;
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
			new_dfa.states = new_states;
			for (int j = 0; j < new_dfa.states.size(); j++) {
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
					transition.second = new_transition;
				}
			}
			i--;
			count--;
		}
	}
	return new_dfa;
}

FiniteAutomaton FiniteAutomaton::annote() const {
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
				new_alphabet.insert(elem.first);
			}
		}
	}
	new_fa.language = shared_ptr<Language>(new Language(new_alphabet));
	for (int i = 0; i < new_transitions.size(); i++) {
		new_fa.states[i].transitions = new_transitions[i];
	}
	return new_fa;
}

FiniteAutomaton FiniteAutomaton::deannote() const {
	set<alphabet_symbol> new_alphabet;
	FiniteAutomaton new_fa =
		FiniteAutomaton(initial_state, states, shared_ptr<Language>());
	vector<map<alphabet_symbol, set<int>>> new_transitions(
		new_fa.states.size());
	for (int i = 0; i < new_fa.states.size(); i++) {
		for (const auto& elem : new_fa.states[i].transitions) {
			if (elem.first.size() > 1 && elem.first != epsilon()) {
				string str = to_string(elem.first);
				str.pop_back();
				new_alphabet.insert(str);
				for (int transition_to : elem.second) {
					new_transitions[i][str].insert(transition_to);
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
	vector<GrammarItem> fa_items;
	vector<GrammarItem*> nonterminals;
	vector<GrammarItem*> terminals;

	vector<vector<vector<GrammarItem*>>> rules = Grammar::fa_to_grammar(
		states, language->get_alphabet(), fa_items, nonterminals, terminals);

	vector<GrammarItem*> bisimilar_nonterminals;
	vector<vector<vector<GrammarItem*>>> bisimilar_rules =
		Grammar::get_bisimilar_grammar(rules, nonterminals,
									   bisimilar_nonterminals);
	vector<int> classes;
	for (const auto& nont : nonterminals)
		classes.push_back(nont->class_number);
	return merge_equivalent_classes(classes);
}

bool FiniteAutomaton::bisimilar(const FiniteAutomaton& fa1,
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
	vector<vector<vector<GrammarItem*>>> fa1_bisimilar_rules =
		Grammar::get_bisimilar_grammar(fa1_rules, fa1_nonterminals,
									   fa1_bisimilar_nonterminals);

	vector<GrammarItem*> fa2_bisimilar_nonterminals;
	vector<vector<vector<GrammarItem*>>> fa2_bisimilar_rules =
		Grammar::get_bisimilar_grammar(fa2_rules, fa2_nonterminals,
									   fa2_bisimilar_nonterminals);
	if (fa1_bisimilar_nonterminals.size() != fa2_bisimilar_nonterminals.size())
		return false;
	// из объединения полученных ранее получаем итоговую
	vector<GrammarItem*> nonterminals(fa1_bisimilar_nonterminals);
	nonterminals.insert(nonterminals.end(), fa2_bisimilar_nonterminals.begin(),
						fa2_bisimilar_nonterminals.end());
	vector<vector<vector<GrammarItem*>>> rules(fa1_bisimilar_rules);
	rules.insert(rules.end(), fa2_bisimilar_rules.begin(),
				 fa2_bisimilar_rules.end());

	for (GrammarItem* nont : nonterminals)
		nont->class_number = 0; // сбрасываю номера классов

	vector<GrammarItem*> bisimilar_nonterminals;
	vector<vector<vector<GrammarItem*>>> bisimilar_rules =
		Grammar::get_bisimilar_grammar(rules, nonterminals,
									   bisimilar_nonterminals);

	// проверяю равенство классов начальных состояний
	if (fa1_nonterminals[fa1.initial_state]->class_number !=
		fa2_nonterminals[fa2.initial_state]->class_number)
		return false;
	if (fa1_bisimilar_nonterminals.size() != bisimilar_nonterminals.size())
		return false;

	return true;
}

bool FiniteAutomaton::equal(const FiniteAutomaton& fa1,
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
	vector<vector<vector<GrammarItem*>>> bisimilar_rules =
		Grammar::get_bisimilar_grammar(rules, nonterminals,
									   bisimilar_nonterminals);
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
		nont->class_number = 0; // сбрасываю номера классов

	vector<GrammarItem*> reverse_bisimilar_nonterminals;
	vector<vector<vector<GrammarItem*>>> reverse_bisimilar_rules =
		Grammar::get_bisimilar_grammar(reverse_rules, nonterminals,
									   reverse_bisimilar_nonterminals);
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
		Grammar::get_bisimilar_grammar(transitions_rules,
									   transitions_nonterminals,
									   transitions_bisimilar_nonterminals);
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

bool FiniteAutomaton::equivalent(const FiniteAutomaton& fa1,
								 const FiniteAutomaton& fa2) {
	return equal(fa1.minimize(), fa2.minimize());
}

std::optional<std::string> FiniteAutomaton::get_prefix(int state_beg, int state_end,
									  map<int, bool>& was) {
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

bool FiniteAutomaton::semdet() {
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
		cout << "Try " << i << "\n";
		if (!prefix.has_value()) continue;
		Regex reg;
		// Получение языка из производной регулярки автомата по префиксу:
		//		this -> reg (arden?)
		reg = nfa_to_regex(*this);
		cout << "State: " << i << "\n";
		cout << "Prefix: " << prefix.value() << "\n";
		cout << "Regex: " << reg.to_txt() << "\n";
		auto derevative = reg.prefix_derevative(prefix.value());
		if (!derevative.has_value()) continue;
		state_languages[i] = derevative.value();
		cout << "Derevative: " << state_languages[i].to_txt() << "\n";
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
				if (!verified_ambiguity) return false;
			}
		}
	}
	return true;
}

bool FiniteAutomaton::parsing_nfa(string s, State state) const {
	// cout << s << endl;
	if (s.size() == 0 && state.is_terminal) {
		return true;
	}

	if (s.size() == 0 && !state.is_terminal) {
		return false;
	}
	alphabet_symbol elem = char_to_alphabet_symbol(s[0]);
	set<int> tr = state.transitions[elem];
	set<int> tr_eps = state.transitions[char_to_alphabet_symbol('\0')];
	vector<int> trans{tr.begin(), tr.end()};
	vector<int> trans_eps{tr_eps.begin(), tr_eps.end()};
	for (size_t i = 0; i < trans.size(); i++) {
		if (parsing_nfa(s.substr(1), states[trans[i]])) {
			return true;
		}
	}
	for (size_t i = 0; i < trans_eps.size(); i++) {
		if (parsing_nfa(s, states[trans_eps[i]])) {
			return true;
		}
	}
	return false;
}

bool FiniteAutomaton::parsing_by_nfa(const string& s) const {
	State state = states[0];
	return parsing_nfa(s, state);
}
bool FiniteAutomaton::subset(const FiniteAutomaton& fa) const {
	FiniteAutomaton fa_instersection(intersection(*this, fa));
	// автомат с перечесеченным алфавитом
	FiniteAutomaton check_fa(fa.initial_state, fa.states,
							 fa_instersection.language->get_alphabet());
	return equivalent(fa_instersection, check_fa);
}

int FiniteAutomaton::get_states_size() {
	return states.size();
}
State FiniteAutomaton::get_state(int i) {
	return states[i];
}

int FiniteAutomaton::get_initial() {
	return initial_state;
}

const set<alphabet_symbol>& FiniteAutomaton::get_alphabet() {
	return language->get_alphabet();
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
