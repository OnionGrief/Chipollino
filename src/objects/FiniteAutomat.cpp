#include "FiniteAutomat.h"
#include <algorithm>
#include <sstream>
using namespace std;

State::State() : index(0), is_terminal(false), identifier("") {}

State::State(int index, vector<int> label, string identifier, bool is_terminal,
			 map<char, vector<int>> transitions)
	: index(index), label(label), identifier(identifier),
	  is_terminal(is_terminal), transitions(transitions) {}

void State::set_transition(int to, char symbol) {
	transitions[symbol].push_back(to);
}

FiniteAutomat::FiniteAutomat() {}

FiniteAutomat::FiniteAutomat(int initial_state, vector<char> alphabet,
							 vector<State> states, bool is_deterministic)
	: initial_state(initial_state), alphabet(alphabet), states(states),
	  is_deterministic(is_deterministic) {}

string FiniteAutomat::to_txt() {
	stringstream ss;
	ss << "digraph {\n\trankdir = LR\n\tdummy [label = \"\", shape = none]\n\t";
	for (int i = 0; i < states.size(); i++) {
		ss << i << " [label = \"" << states[i].identifier << "\", shape = ";
		ss << (states[i].is_terminal ? "doublecircle]\n\t" : "circle]\n\t");
	}
	ss << "dummy -> " << states[initial_state].index << "\n";

	for (auto & state : states) {
		for (auto elem : state.transitions) {
			for (int transition : elem.second) {
				ss << "\t" << state.index << " -> " << transition;
				if (elem.first == '\0')
					ss << " [label = \""
					   << "eps"
					   << "\"]\n";
				else
					ss << " [label = \"" << elem.first << "\"]\n";
			}
		}
	}

	ss << "}\n";
	return ss.str();
}

//обход автомата в глубину по eps-переходам
void dfs(vector<State> states, int index, vector<int>* c) {
	if (find(c->begin(), c->end(), index) == c->end()) {
		c->push_back(index);
		for (int i = 0; i < states[index].transitions['\0'].size(); i++) {
			dfs(states, states[index].transitions['\0'][i], c);
		}
	}
}

vector<int> FiniteAutomat::closure(vector<int> x) {
	vector<int> c;
	for (int i = 0; i < x.size(); i++)
		dfs(states, x[i], &c);
	return c;
}

//проверка меток на равенство
bool belong(State q, State u) {
	if (q.label.size() != u.label.size()) return false;
	for (int i = 0; i < q.label.size(); i++) {
		if (q.label[i] != u.label[i]) return false;
	}
	return true;
}

FiniteAutomat FiniteAutomat::determinize() {
	FiniteAutomat ndm(initial_state, alphabet, states, is_deterministic);
	FiniteAutomat dm = FiniteAutomat();
	vector<int> x = {0};
	vector<int> q0 = ndm.closure(x);

	vector<int> label = q0;
	sort(label.begin(), label.end());
	State new_initial_state = {0, label,
							   ndm.states[ndm.initial_state].identifier, false,
							   map<char, vector<int>>()};
	dm.states.push_back(new_initial_state);
	dm.initial_state = 0;

	stack<vector<int>> s1;
	stack<int> s2;
	s1.push(q0);
	s2.push(0);

	while (!s1.empty()) {
		vector<int> z = s1.top();
		int index = s2.top();
		s1.pop();
		s2.pop();
		State q = dm.states[index];

		for (int i : z) {
			if (ndm.states[i].is_terminal) {
				dm.states[index].is_terminal = true;
				break;
			}
		}

		vector<int> new_x;
		for (char ch : ndm.alphabet) {
			new_x.clear();
			for (int j : z) {
				for (int k : ndm.states[j].transitions[ch]) {
					new_x.push_back(k);
				}
			}

			vector<int> z1 = ndm.closure(new_x);
			vector<int> new_label = z1;
			sort(new_label.begin(), new_label.end());

			State q1 = {-1, new_label, "", false, map<char, vector<int>>()};
			bool accessory_flag = false;

			for (auto& state : dm.states) {
				if (belong(q1, state)) {
					index = state.index;
					accessory_flag = true;
					break;
				}
			}

			if (!accessory_flag) index = -1;
			if (index != -1)
				q1 = dm.states[index];
			else {
				index = dm.states.size();
				q1.index = index;
				dm.states.push_back(q1);
				s1.push(z1);
				s2.push(index);
			}
			dm.states[q.index].transitions[ch].push_back(q1.index);
		}
	}
	dm.alphabet = ndm.alphabet;
	dm.is_deterministic = true;
	return dm;
}

FiniteAutomat FiniteAutomat::remove_eps() {
	FiniteAutomat endm =
		FiniteAutomat(initial_state, alphabet, states, is_deterministic);
	FiniteAutomat ndm = FiniteAutomat();
	ndm.states = endm.states;

	for (auto& state : ndm.states) {
		state.transitions = map<char, vector<int>>();
	}

	for (int i = 0; i < endm.states.size(); i++) {
		vector<int> state = {endm.states[i].index};
		vector<int> q = endm.closure(state);
		vector<vector<int>> x;
		for (char ch : endm.alphabet) {
			x.clear();
			for (int k : q) {
				x.push_back(endm.states[k].transitions[ch]);
			}
			vector<int> q1;
			set<int> x1;
			for (auto& k : x) {
				q1 = endm.closure(k);
				for (int& m : q1) {
					x1.insert(m);
				}
			}
			for (auto elem : x1) {
				if (ndm.states[elem].is_terminal) {
					ndm.states[i].is_terminal = true;
				}
				ndm.states[i].transitions[ch].push_back(elem);
			}
		}
	}
	ndm.initial_state = endm.initial_state;
	ndm.alphabet = endm.alphabet;
	ndm.is_deterministic = false;
	return ndm;
}

FiniteAutomat FiniteAutomat::intersection(const FiniteAutomat& dm1,
										  const FiniteAutomat& dm2) {
	FiniteAutomat dm = FiniteAutomat();
	dm.initial_state = 0;
	dm.alphabet = dm1.alphabet;
	int counter = 0;
	for (auto& state1 : dm1.states) {
		for (auto& state2 : dm2.states) {
			dm.states.push_back({counter,
								 {state1.index, state2.index},
								 state1.identifier + state2.identifier,
								 state1.is_terminal && state2.is_terminal,
								 map<char, vector<int>>()});
			counter++;
		}
	}

	for (auto& state : dm.states) {
		for (char ch : dm.alphabet) {
			state.transitions[ch].push_back(
				dm1.states[state.label[0]].transitions.at(ch)[0] * 3 +
				dm2.states[state.label[1]].transitions.at(ch)[0]);
		}
	}
	dm.is_deterministic = true;
	return dm;
}

FiniteAutomat FiniteAutomat::uunion(const FiniteAutomat& dm1,
									const FiniteAutomat& dm2) {
	FiniteAutomat dm = FiniteAutomat();
	dm.initial_state = 0;
	dm.alphabet = dm1.alphabet;
	int counter = 0;
	for (auto& state1 : dm1.states) {
		for (auto& state2 : dm2.states) {
			dm.states.push_back({counter,
								 {state1.index, state2.index},
								 state1.identifier + state2.identifier,
								 state1.is_terminal || state2.is_terminal,
								 map<char, vector<int>>()});
			counter++;
		}
	}

	for (auto& state : dm.states) {
		for (char ch : dm.alphabet) {
			state.transitions[ch].push_back(
				dm1.states[state.label[0]].transitions.at(ch)[0] * 3 +
				dm2.states[state.label[1]].transitions.at(ch)[0]);
		}
	}
	dm.is_deterministic = true;
	return dm;
}

FiniteAutomat FiniteAutomat::difference(const FiniteAutomat& dm2) {
	FiniteAutomat dm1 =
		FiniteAutomat(initial_state, alphabet, states, is_deterministic);
	FiniteAutomat dm = FiniteAutomat();
	dm.initial_state = 0;
	dm.alphabet = dm1.alphabet;
	int counter = 0;
	for (auto& state1 : dm1.states) {
		for (auto& state2 : dm2.states) {
			dm.states.push_back({counter,
								 {state1.index, state2.index},
								 state1.identifier + state2.identifier,
								 state1.is_terminal && !state2.is_terminal,
								 map<char, vector<int>>()});
			counter++;
		}
	}

	for (auto& state : dm.states) {
		for (char ch : dm.alphabet) {
			state.transitions[ch].push_back(
				dm1.states[state.label[0]].transitions.at(ch)[0] * 3 +
				dm2.states[state.label[1]].transitions.at(ch)[0]);
		}
	}
	dm.is_deterministic = true;
	return dm;
}

FiniteAutomat FiniteAutomat::complement() {
	FiniteAutomat dm =
		FiniteAutomat(initial_state, alphabet, states, is_deterministic);
	for (int i = 0; i < dm.states.size(); i++) {
		dm.states[i].is_terminal = !dm.states[i].is_terminal;
	}
	return dm;
}

FiniteAutomat FiniteAutomat::reverse() {
	FiniteAutomat endm = FiniteAutomat(initial_state, alphabet, states, is_deterministic);
	for (auto& state : endm.states) {
		state.index += 1;
	}
	endm.states.insert(endm.states.begin(), { 0, {0}, "", false, map<char, vector<int>>() });
	endm.initial_state = 0;
	for (int i = 1; i < endm.states.size(); i++) {
		if (endm.states[i].is_terminal) {
			endm.states[initial_state].transitions['\0'].push_back(endm.states[i].index);
		}
		endm.states[i].is_terminal = !endm.states[i].is_terminal;
	}
	vector<map<char, vector<int>>> new_transition_matrix(endm.states.size() - 1);
	for (int i = 1; i < endm.states.size(); i++) {
		for (auto& transition : endm.states[i].transitions) {
			for (int elem : transition.second) {
				new_transition_matrix[elem][transition.first].push_back(endm.states[i].index);
			}
		}
	}
	for (int i = 1; i < endm.states.size(); i++) {
		endm.states[i].transitions = new_transition_matrix[i - 1];
	}
	return endm;
}

FiniteAutomat FiniteAutomat::add_trap_state() {
	FiniteAutomat dm = FiniteAutomat(initial_state, alphabet, states, is_deterministic);
	bool flag = true;
	int count = dm.states.size();
	for (int i = 0; i < count; i++) {
		for (char ch : alphabet) {
			if (!dm.states[i].transitions[ch].size()) {
				if (flag) {
					dm.states[i].set_transition(dm.states.size(), ch);
					int size = dm.states.size();
					dm.states.push_back({ size, {size}, "", false, map<char, vector<int>>() });
				} else {
					dm.states[i].set_transition(dm.states.size() - 1, ch);
				}
				flag = false;
			}
		}
	}
	if (!flag) {
		for (char ch: alphabet) {
			dm.states[dm.states.size() - 1].transitions[ch].push_back(dm.states.size() - 1);
		}
	}
	return dm;
}

FiniteAutomat FiniteAutomat::remove_trap_state() {
	FiniteAutomat dm = FiniteAutomat(initial_state, alphabet, states, is_deterministic);
	int count = dm.states.size();
	for (int i = 0; i < count; i++) {
		bool flag = false;
		for (auto& transitions: dm.states[i].transitions) {
			for (int transition: transitions.second) {
				if (i == transition && !dm.states[i].is_terminal) {
					flag = true;
				}
			}
		}
		if (flag) {
			dm.states.erase(dm.states.begin() + i);
			if (i != count - 1) {
				for (int j = dm.states[i].index - 1; j < dm.states.size(); j++) {
					dm.states[j].index -= 1;
				}
			}
			for (int j = 0; j < dm.states.size(); j++) {
				for (auto& transitions : dm.states[j].transitions) {
					for (int k = 0; k < transitions.second.size(); k++) {
						if (transitions.second[k] == i) {
							transitions.second.erase(transitions.second.begin() + k);
						}
						if (transitions.second[k] > i) {
							transitions.second[k] -= 1;
						}
					}
				}
			}
			i--;
			count--;
		}
	}
	return dm;
}