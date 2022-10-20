#include "FiniteAutomat.h"
#include <algorithm>
#include <iostream>
#include <set>
#include <sstream>
#include <stack>
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

	for (auto& state : states) {
		for (const auto& elem : state.transitions) {
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

//обход автомата в глубину
void dfs(vector<State> states, vector<char> alphabet, int index,
		 vector<int>& reachable, bool use_epsilons_only) {
	if (find(reachable.begin(), reachable.end(), index) == reachable.end()) {
		reachable.push_back(index);
		if (use_epsilons_only) {
			for (int i = 0; i < states[index].transitions['\0'].size(); i++) {
				dfs(states, alphabet, states[index].transitions['\0'][i],
					reachable, use_epsilons_only);
			}
		} else {
			for (char ch : alphabet) {
				for (int i = 0; i < states[index].transitions[ch].size(); i++) {
					dfs(states, alphabet, states[index].transitions[ch][i],
						reachable, use_epsilons_only);
				}
			}
		}
	}
}

vector<int> FiniteAutomat::closure(const vector<int>& indices,
								   bool use_epsilons_only) {
	vector<int> reachable;
	for (int i = 0; i < indices.size(); i++)
		dfs(states, alphabet, indices[i], reachable, use_epsilons_only);
	return reachable;
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
	FiniteAutomat nfa(initial_state, alphabet, states, is_deterministic);
	FiniteAutomat dfa = FiniteAutomat();
	vector<int> x = {0};
	vector<int> q0 = nfa.closure(x, true);

	vector<int> label = q0;
	sort(label.begin(), label.end());
	State new_initial_state = {0, label,
							   nfa.states[nfa.initial_state].identifier, false,
							   map<char, vector<int>>()};
	dfa.states.push_back(new_initial_state);
	dfa.initial_state = 0;

	stack<vector<int>> s1;
	stack<int> s2;
	s1.push(q0);
	s2.push(0);

	while (!s1.empty()) {
		vector<int> z = s1.top();
		int index = s2.top();
		s1.pop();
		s2.pop();
		State q = dfa.states[index];

		for (int i : z) {
			if (nfa.states[i].is_terminal) {
				dfa.states[index].is_terminal = true;
				break;
			}
		}

		vector<int> new_x;
		for (char ch : nfa.alphabet) {
			new_x.clear();
			for (int j : z) {
				for (int k : nfa.states[j].transitions[ch]) {
					new_x.push_back(k);
				}
			}

			vector<int> z1 = nfa.closure(new_x, true);
			vector<int> new_label = z1;
			sort(new_label.begin(), new_label.end());
			string new_identifier;
			for (auto elem : new_label) {
				new_identifier += (new_identifier.empty() ? "" : ", ") +
								  nfa.states[elem].identifier;
			}

			State q1 = {-1, new_label, new_identifier, false,
						map<char, vector<int>>()};
			bool accessory_flag = false;

			for (auto& state : dfa.states) {
				if (belong(q1, state)) {
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
			dfa.states[q.index].transitions[ch].push_back(q1.index);
		}
	}
	dfa.alphabet = nfa.alphabet;
	dfa.is_deterministic = true;
	return dfa;
}

FiniteAutomat FiniteAutomat::minimize() {
	FiniteAutomat nfa =
		FiniteAutomat(initial_state, alphabet, states, is_deterministic);
	FiniteAutomat dfa = nfa.determinize();
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
					for (char ch : alphabet) {
						vector<int> to = {dfa.states[i].transitions[ch][0],
										  dfa.states[j].transitions[ch][0]};
						if (dfa.states[i].transitions[ch][0] <
							dfa.states[j].transitions[ch][0]) {
							to = {dfa.states[j].transitions[ch][0],
								  dfa.states[i].transitions[ch][0]};
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
	vector<bool> state_flags(dfa.states.size());
	for (int i = 0; i < groups.size(); i++) {
		for (int j = 0; j < groups[i].size(); j++) {
			state_flags[groups[i][j]] = true;
		}
		for (int j = counter; j < groups.size(); j++) {
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

	for (int i = 0; i < classes.size(); i++)
		cout << classes[i] << " ";
	cout << endl;

	FiniteAutomat minimized_dfa = dfa.merge_equivalent_classes(classes);
	return minimized_dfa;
}

FiniteAutomat FiniteAutomat::remove_eps() {
	FiniteAutomat enfa =
		FiniteAutomat(initial_state, alphabet, states, is_deterministic);
	FiniteAutomat nfa = FiniteAutomat();
	nfa.states = enfa.states;

	for (auto& state : nfa.states) {
		state.transitions = map<char, vector<int>>();
	}

	for (int i = 0; i < enfa.states.size(); i++) {
		vector<int> state = {enfa.states[i].index};
		vector<int> q = enfa.closure(state, true);
		vector<vector<int>> x;
		for (char ch : enfa.alphabet) {
			x.clear();
			for (int k : q) {
				x.push_back(enfa.states[k].transitions[ch]);
			}
			vector<int> q1;
			set<int> x1;
			for (auto& k : x) {
				q1 = enfa.closure(k, true);
				for (int& m : q1) {
					x1.insert(m);
				}
			}
			for (auto elem : x1) {
				if (nfa.states[elem].is_terminal) {
					nfa.states[i].is_terminal = true;
				}
				nfa.states[i].transitions[ch].push_back(elem);
			}
		}
	}
	nfa.initial_state = enfa.initial_state;
	nfa.alphabet = enfa.alphabet;
	nfa.is_deterministic = false;
	return nfa;
}

FiniteAutomat FiniteAutomat::intersection(const FiniteAutomat& dfa1,
										  const FiniteAutomat& dfa2) {
	FiniteAutomat new_dfa = FiniteAutomat();
	new_dfa.initial_state = 0;
	new_dfa.alphabet = dfa1.alphabet;
	int counter = 0;
	for (auto& state1 : dfa1.states) {
		for (auto& state2 : dfa2.states) {
			new_dfa.states.push_back(
				{counter,
				 {state1.index, state2.index},
				 state1.identifier + "&" + state2.identifier,
				 state1.is_terminal && state2.is_terminal,
				 map<char, vector<int>>()});
			counter++;
		}
	}

	for (auto& state : new_dfa.states) {
		for (char ch : new_dfa.alphabet) {
			state.transitions[ch].push_back(
				dfa1.states[state.label[0]].transitions.at(ch)[0] *
					dfa1.states.size() +
				dfa2.states[state.label[1]].transitions.at(ch)[0]);
		}
	}

	return new_dfa.determinize();
}

FiniteAutomat FiniteAutomat::uunion(const FiniteAutomat& dfa1,
									const FiniteAutomat& dfa2) {
	FiniteAutomat new_dfa = FiniteAutomat();
	new_dfa.initial_state = 0;
	new_dfa.alphabet = dfa1.alphabet;
	int counter = 0;
	for (auto& state1 : dfa1.states) {
		for (auto& state2 : dfa2.states) {
			new_dfa.states.push_back({counter,
									  {state1.index, state2.index},
									  state1.identifier + state2.identifier,
									  state1.is_terminal || state2.is_terminal,
									  map<char, vector<int>>()});
			counter++;
		}
	}

	for (auto& state : new_dfa.states) {
		for (char ch : new_dfa.alphabet) {
			state.transitions[ch].push_back(
				dfa1.states[state.label[0]].transitions.at(ch)[0] *
					dfa1.states.size() +
				dfa2.states[state.label[1]].transitions.at(ch)[0]);
		}
	}

	return new_dfa.determinize();
}

FiniteAutomat FiniteAutomat::difference(const FiniteAutomat& dfa2) {
	FiniteAutomat dfa1 =
		FiniteAutomat(initial_state, alphabet, states, is_deterministic);
	FiniteAutomat new_dfa = FiniteAutomat();
	new_dfa.initial_state = 0;
	new_dfa.alphabet = dfa1.alphabet;
	int counter = 0;
	for (auto& state1 : dfa1.states) {
		for (auto& state2 : dfa2.states) {
			new_dfa.states.push_back({counter,
									  {state1.index, state2.index},
									  state1.identifier + state2.identifier,
									  state1.is_terminal && !state2.is_terminal,
									  map<char, vector<int>>()});
			counter++;
		}
	}

	for (auto& state : new_dfa.states) {
		for (char ch : new_dfa.alphabet) {
			state.transitions[ch].push_back(
				dfa1.states[state.label[0]].transitions.at(ch)[0] *
					dfa1.states.size() +
				dfa2.states[state.label[1]].transitions.at(ch)[0]);
		}
	}

	return new_dfa.determinize();
}

FiniteAutomat FiniteAutomat::complement() {
	FiniteAutomat dfa =
		FiniteAutomat(initial_state, alphabet, states, is_deterministic);
	for (int i = 0; i < dfa.states.size(); i++) {
		dfa.states[i].is_terminal = !dfa.states[i].is_terminal;
	}
	return dfa;
}
FiniteAutomat FiniteAutomat::reverse() {
	FiniteAutomat enfa =
		FiniteAutomat(initial_state, alphabet, states, is_deterministic);
	for (auto& state : enfa.states) {
		state.index += 1;
	}
	enfa.states.insert(enfa.states.begin(),
					   {0, {0}, "", false, map<char, vector<int>>()});
	enfa.initial_state = 0;
	for (int i = 1; i < enfa.states.size(); i++) {
		if (enfa.states[i].is_terminal) {
			enfa.states[initial_state].transitions['\0'].push_back(
				enfa.states[i].index);
		}
		enfa.states[i].is_terminal = !enfa.states[i].is_terminal;
	}
	vector<map<char, vector<int>>> new_transition_matrix(enfa.states.size() -
														 1);
	for (int i = 1; i < enfa.states.size(); i++) {
		for (auto& transition : enfa.states[i].transitions) {
			for (int elem : transition.second) {
				new_transition_matrix[elem][transition.first].push_back(
					enfa.states[i].index);
			}
		}
	}
	for (int i = 1; i < enfa.states.size(); i++) {
		enfa.states[i].transitions = new_transition_matrix[i - 1];
	}
	return enfa;
}

FiniteAutomat FiniteAutomat::add_trap_state() {
	FiniteAutomat dfa =
		FiniteAutomat(initial_state, alphabet, states, is_deterministic);
	bool flag = true;
	int count = dfa.states.size();
	for (int i = 0; i < count; i++) {
		for (char ch : alphabet) {
			if (!dfa.states[i].transitions[ch].size()) {
				if (flag) {
					dfa.states[i].set_transition(dfa.states.size(), ch);
					int size = dfa.states.size();
					dfa.states.push_back(
						{size, {size}, "", false, map<char, vector<int>>()});
				} else {
					dfa.states[i].set_transition(dfa.states.size() - 1, ch);
				}
				flag = false;
			}
		}
	}
	if (!flag) {
		for (char ch : alphabet) {
			dfa.states[dfa.states.size() - 1].transitions[ch].push_back(
				dfa.states.size() - 1);
		}
	}
	return dfa;
}

FiniteAutomat FiniteAutomat::remove_trap_state() {
	FiniteAutomat dfa =
		FiniteAutomat(initial_state, alphabet, states, is_deterministic);
	int count = dfa.states.size();
	for (int i = 0; i < count; i++) {
		bool flag = false;
		for (auto& transitions : dfa.states[i].transitions) {
			for (int transition : transitions.second) {
				if (i == transition && !dfa.states[i].is_terminal) {
					flag = true;
				}
			}
		}
		if (flag) {
			dfa.states.erase(dfa.states.begin() + i);
			if (i != count - 1) {
				for (int j = dfa.states[i].index - 1; j < dfa.states.size();
					 j++) {
					dfa.states[j].index -= 1;
				}
			}
			for (int j = 0; j < dfa.states.size(); j++) {
				for (auto& transitions : dfa.states[j].transitions) {
					for (int k = 0; k < transitions.second.size(); k++) {
						if (transitions.second[k] == i) {
							transitions.second.erase(
								transitions.second.begin() + k);
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
	return dfa;
}

FiniteAutomat FiniteAutomat::merge_equivalent_classes(vector<int> classes) {
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
		State s = {i, {i}, new_identifier, false, map<char, vector<int>>()};
		new_states.push_back(s);
	}

	for (int i = 0; i < states.size(); i++) {
		int from = classes[i];
		for (const auto& elem : states[i].transitions) {
			for (int transition : elem.second) {
				int to = classes[transition];
				auto it = new_states[from].transitions.find(elem.first);
				if (it == new_states[from].transitions.end()) {
					new_states[from].transitions[elem.first].push_back(to);
				} else {
					if (find(it->second.begin(), it->second.end(), to) ==
						it->second.end())
						it->second.push_back(to);
				}
			}
		}
	}

	for (const auto& elem : class_to_index)
		if (states[elem.second[0]].is_terminal)
			new_states[elem.first].is_terminal = true;

	return FiniteAutomat(classes[initial_state], alphabet, new_states,
						 is_deterministic);
}
// структуры и функции для работы с грамматикой
struct GrammarItem {
	enum Type {
		terminal,
		nonterminal
	};
	Type type;
	int state_index, class_number;
	string term_name;
	GrammarItem()
		: type(terminal), state_index(-1), class_number(-1), term_name("") {}
	GrammarItem(Type type, int state_index, int class_number)
		: type(type), state_index(state_index), class_number(class_number) {}
	GrammarItem(Type type, string term_name)
		: type(type), term_name(term_name) {}
	bool operator!=(const GrammarItem& other) {
		return type != other.type || state_index != other.state_index ||
			   class_number != other.class_number ||
			   term_name != other.term_name;
	}
	void operator=(const GrammarItem& other) {
		type = other.type;
		state_index = other.state_index;
		class_number = other.class_number;
		term_name = other.term_name;
	}
};
// для отладки
ostream& operator<<(ostream& os, const GrammarItem& item) {
	if (item.type == GrammarItem::terminal)
		return os << item.term_name;
	else
		return os << "S" << item.state_index;
}
// обновляю значение class_number для каждого нетерминала
void update_classes(set<int>& checker,
					map<set<string>, vector<GrammarItem*>>& classes_check_map) {
	int classNum = 0;
	checker.clear();
	for (const auto& elem : classes_check_map) {
		checker.insert(elem.second[0]->state_index);
		for (GrammarItem* nont : elem.second) {
			nont->class_number = classNum;
		}
		classNum++;
	}
}
// строю новые классы эквивалентности по терминальным формам
void check_classes(vector<vector<vector<GrammarItem*>>>& rules,
				   map<set<string>, vector<GrammarItem*>>& classes_check_map,
				   vector<GrammarItem*>& nonterminals) {
	classes_check_map.clear();
	for (int i = 0; i < nonterminals.size(); i++) {
		set<string> temp_rules;
		for (vector<GrammarItem*> rule : rules[i]) {
			string newRule;

			for (GrammarItem* t : rule) {
				if (t->type == GrammarItem::terminal)
					newRule += t->term_name;
				else
					newRule += to_string(t->class_number);
			}

			temp_rules.insert(newRule);
		}
		classes_check_map[temp_rules].push_back(nonterminals[i]);
	}
}

vector<vector<vector<GrammarItem*>>> get_bisimilar_grammar(
	vector<vector<vector<GrammarItem*>>>& rules,
	vector<GrammarItem*>& nonterminals,
	vector<GrammarItem*>& bisimilar_nonterminals) {
	map<set<string>, vector<GrammarItem*>> classes_check_map;
	set<int> checker;
	// checker
	while (true) {
		set<int> temp = checker;
		check_classes(rules, classes_check_map, nonterminals);
		update_classes(checker, classes_check_map);
		if (checker == temp) break;
	}
	// формирование бисимилярной грамматики
	map<int, GrammarItem*> class_to_nonterm;
	for (const auto& elem : classes_check_map)
		class_to_nonterm[elem.second[0]->class_number] = elem.second[0];
	vector<vector<vector<GrammarItem*>>> bisimilar_rules;
	for (const auto& elem : classes_check_map) {
		GrammarItem* curNonterm = elem.second[0];
		vector<vector<GrammarItem*>> temp_rules;
		for (vector<GrammarItem*> rule : rules[curNonterm->state_index]) {
			vector<GrammarItem*> tempRule;
			for (GrammarItem* item : rule) {
				if (item->type == GrammarItem::nonterminal) {
					tempRule.push_back(class_to_nonterm[item->class_number]);
				} else
					tempRule.push_back(item);
			}
			temp_rules.push_back(tempRule);
		}
		bisimilar_nonterminals.push_back(curNonterm);
		bisimilar_rules.push_back(temp_rules);
	}

	return bisimilar_rules;
}
// преобразование конечного автомата в грамматику
vector<vector<vector<GrammarItem*>>> fa_to_grammar(
	const vector<State>& states, const vector<char>& alphabet,
	int initial_state, vector<GrammarItem>& fa_items,
	vector<GrammarItem*>& nonterminals, vector<GrammarItem*>& terminals) {
	vector<vector<vector<GrammarItem*>>> rules(states.size());
	fa_items.resize(states.size() + alphabet.size() + 2);
	int ind = 0;
	while (ind < states.size()) {
		fa_items[ind] = GrammarItem(GrammarItem::nonterminal, ind, 0);
		nonterminals.push_back(&fa_items[ind]);
		ind++;
	}
	map<char, int> terminal_index;
	fa_items[ind] = (GrammarItem(GrammarItem::terminal, "\0"));
	terminals.push_back(&fa_items[ind]);
	terminal_index['\0'] = 0;
	ind++;
	for (int i = 0; i < alphabet.size(); i++) {
		fa_items[ind] =
			(GrammarItem(GrammarItem::terminal, string(1, alphabet[i])));
		terminals.push_back(&fa_items[ind]);
		terminal_index[alphabet[i]] = i + 1;
		ind++;
	}
	fa_items[ind] = (GrammarItem(GrammarItem::terminal, "init"));
	terminals.push_back(&fa_items[ind]);

	for (int i = 0; i < states.size(); i++) {
		for (const auto& elem : states[i].transitions) {
			for (int j = 0; j < elem.second.size(); j++)
				rules[i].push_back({terminals[terminal_index[elem.first]],
									nonterminals[elem.second[j]]});
		}
		if (states[i].is_terminal) rules[i].push_back({terminals[0]});
	}
	rules[initial_state].push_back({terminals[alphabet.size() + 1]});

	return rules;
}

FiniteAutomat FiniteAutomat::merge_bisimilar() {
	vector<GrammarItem> fa_items;
	vector<GrammarItem*> nonterminals;
	vector<GrammarItem*> terminals;

	vector<vector<vector<GrammarItem*>>> rules = fa_to_grammar(
		states, alphabet, initial_state, fa_items, nonterminals, terminals);

	vector<GrammarItem*> bisimilar_nonterminals;
	vector<vector<vector<GrammarItem*>>> bisimilar_rules =
		get_bisimilar_grammar(rules, nonterminals, bisimilar_nonterminals);
	/*// порождение автомата
	vector<int> classes;
	for (const auto& nont : nonterminals)
		classes.push_back(nont->class_number);
	return merge_equivalent_classes(classes);*/
	// порождение автомата
	vector<State> states;
	map<int, int> new_state_number;
	for (int i = 0; i < bisimilar_nonterminals.size(); i++) {
		states.push_back(
			{i, {i}, to_string(i), false, map<char, vector<int>>()});
		new_state_number[bisimilar_nonterminals[i]->state_index] = i;
	}
	int initial_state = 0;
	for (int i = 0; i < bisimilar_nonterminals.size(); i++) {
		for (vector<GrammarItem*> rule : bisimilar_rules[i]) {
			if (rule.size() ==
				1) { // особый случай терминального и начального состояния
				if (rule[0]->term_name == "\0") states[i].is_terminal = true;
				if (rule[0]->term_name == "init") initial_state = i;
				continue;
			} else if (rule.size() ==
					   2) { // случай Н -> Т Н (устанавливаем переход)
				states[i].set_transition(new_state_number[rule[1]->state_index],
										 rule[0]->term_name[0]);
				continue;
			}
		}
	}

	return FiniteAutomat(initial_state, alphabet, states, is_deterministic);
}

bool FiniteAutomat::bisimilar(const FiniteAutomat& fa1,
							  const FiniteAutomat& fa2) {
	// грамматики из автоматов
	vector<GrammarItem> fa1_items;
	vector<GrammarItem*> fa1_nonterminals;
	vector<GrammarItem*> fa1_terminals;
	vector<vector<vector<GrammarItem*>>> fa1_rules =
		fa_to_grammar(fa1.states, fa1.alphabet, fa1.initial_state, fa1_items,
					  fa1_nonterminals, fa1_terminals);

	vector<GrammarItem> fa2_items;
	vector<GrammarItem*> fa2_nonterminals;
	vector<GrammarItem*> fa2_terminals;
	vector<vector<vector<GrammarItem*>>> fa2_rules =
		fa_to_grammar(fa2.states, fa2.alphabet, fa2.initial_state, fa2_items,
					  fa2_nonterminals, fa2_terminals);

	if (fa1_terminals.size() != fa2_terminals.size()) return false;
	for (int i = 0; i < fa1_terminals.size(); i++)
		if (*fa1_terminals[i] != *fa2_terminals[i]) return false;
	// сначала получаем бисимилярные грамматики из данных автоматов
	vector<GrammarItem*> fa1_bisimilar_nonterminals;
	vector<vector<vector<GrammarItem*>>> fa1_bisimilar_rules =
		get_bisimilar_grammar(fa1_rules, fa1_nonterminals,
							  fa1_bisimilar_nonterminals);

	vector<GrammarItem*> fa2_bisimilar_nonterminals;
	vector<vector<vector<GrammarItem*>>> fa2_bisimilar_rules =
		get_bisimilar_grammar(fa2_rules, fa2_nonterminals,
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

	vector<GrammarItem*> bisimilar_nonterminals;
	vector<vector<vector<GrammarItem*>>> bisimilar_rules =
		get_bisimilar_grammar(rules, nonterminals, bisimilar_nonterminals);

	if (fa1_bisimilar_nonterminals.size() != bisimilar_nonterminals.size())
		return false;

	return true;
}
// обновляю значение class_number для каждого нетерминала
void update_bijective_Classes(
	set<int>& checker,
	map<multiset<string>, vector<GrammarItem*>>& classes_check_map) {
	int classNum = 0;
	checker.clear();
	for (const auto& elem : classes_check_map) {
		checker.insert(elem.second[0]->state_index);
		for (GrammarItem* nont : elem.second) {
			nont->class_number = classNum;
		}
		classNum++;
	}
}
// работает аналогично check_classes, только в случае (A->a1 B->a1 B->a1)
// A и B будут иметь разные классы
void check_bijective_classes(
	vector<vector<vector<GrammarItem*>>>& rules,
	map<multiset<string>, vector<GrammarItem*>>& classes_check_map,
	vector<GrammarItem*>& nonterminals) {
	classes_check_map.clear();
	for (int i = 0; i < nonterminals.size(); i++) {
		multiset<string> temp_rules;
		for (vector<GrammarItem*> rule : rules[i]) {
			string newRule;

			for (GrammarItem* t : rule) {
				if (t->type == GrammarItem::terminal)
					newRule += t->term_name;
				else
					newRule += to_string(t->class_number);
			}

			temp_rules.insert(newRule);
		}
		classes_check_map[temp_rules].push_back(nonterminals[i]);
	}
}

vector<vector<vector<GrammarItem*>>> get_bijective_bibisimilar_grammar(
	vector<vector<vector<GrammarItem*>>>& rules,
	vector<GrammarItem*>& nonterminals,
	vector<GrammarItem*>& bisimilar_nonterminals) {
	map<multiset<string>, vector<GrammarItem*>> classes_check_map;
	set<int> checker;
	// checker
	while (true) {
		set<int> temp = checker;
		check_bijective_classes(rules, classes_check_map, nonterminals);
		update_bijective_Classes(checker, classes_check_map);
		if (checker == temp) break;
	}
	// формирование бисимилярной грамматики
	map<int, GrammarItem*> class_to_nonterm;
	for (const auto& elem : classes_check_map)
		class_to_nonterm[elem.second[0]->class_number] = elem.second[0];

	vector<vector<vector<GrammarItem*>>> bisimilar_rules;
	for (const auto& elem : classes_check_map) {
		GrammarItem* curNonterm = elem.second[0];
		vector<vector<GrammarItem*>> temp_rules;
		for (vector<GrammarItem*> rule : rules[curNonterm->state_index]) {
			vector<GrammarItem*> tempRule;
			for (GrammarItem* item : rule) {
				if (item->type == GrammarItem::nonterminal) {
					tempRule.push_back(class_to_nonterm[item->class_number]);
				} else
					tempRule.push_back(item);
			}
			temp_rules.push_back(tempRule);
		}
		bisimilar_nonterminals.push_back(curNonterm);
		bisimilar_rules.push_back(temp_rules);
	}

	return bisimilar_rules;
}
// преобразование переходов автомата в грамматику (переход -> состояние переход)
vector<vector<vector<GrammarItem*>>> tansitions_to_grammar(
	const vector<State>& states, int initial_state,
	const vector<GrammarItem*>& fa_nonterminals,
	vector<pair<GrammarItem, map<char, vector<GrammarItem>>>>& fa_items,
	vector<GrammarItem*>& nonterminals, vector<GrammarItem*>& terminals) {
	// fa_items вектор пар <терминал (состояние), map нетерминалов (переходов)>
	fa_items.resize(states.size());
	int ind = 0;
	for (int i = 0; i < states.size(); i++) {
		fa_items[i].first = GrammarItem(
			GrammarItem::terminal, to_string(fa_nonterminals[i]->class_number));
		terminals.push_back(&fa_items[i].first);
		for (const auto& elem : states[i].transitions) {
			vector<GrammarItem>& item_vec = fa_items[i].second[elem.first];
			item_vec.resize(elem.second.size());
			for (int j = 0; j < elem.second.size(); j++) {
				item_vec[j] = (GrammarItem(GrammarItem::nonterminal, ind, 0));
				nonterminals.push_back(&item_vec[j]);
				ind++;
			}
		}
	}

	vector<vector<vector<GrammarItem*>>> rules(nonterminals.size());
	ind = 0;
	for (int i = 0; i < states.size(); i++) {
		for (const auto& elem : states[i].transitions) {
			for (int j = 0; j < elem.second.size(); j++) {
				// индекс состояния, в которое идет переход
				int transInd = elem.second[j];
				// смотрим все переходы из этого состояния
				for (auto transition_elem : states[transInd].transitions) {
					for (int k = 0; k < transition_elem.second.size(); k++) {
						int nonterm_ind = fa_items[transInd]
											  .second[transition_elem.first][k]
											  .state_index;
						rules[ind].push_back(
							{terminals[transInd], nonterminals[nonterm_ind]});
					}
				}
				ind++;
			}
		}
	}

	return rules;
}

bool FiniteAutomat::equal(const FiniteAutomat& fa1, const FiniteAutomat& fa2) {
	// проверка равенства количества состояний
	if (fa1.states.size() != fa2.states.size()) return false;
	// грамматики из состояний автоматов
	vector<GrammarItem> fa1_items;
	vector<GrammarItem*> fa1_nonterminals;
	vector<GrammarItem*> fa1_terminals;
	vector<vector<vector<GrammarItem*>>> fa1_rules =
		fa_to_grammar(fa1.states, fa1.alphabet, fa1.initial_state, fa1_items,
					  fa1_nonterminals, fa1_terminals);

	vector<GrammarItem> fa2_items;
	vector<GrammarItem*> fa2_nonterminals;
	vector<GrammarItem*> fa2_terminals;
	vector<vector<vector<GrammarItem*>>> fa2_rules =
		fa_to_grammar(fa2.states, fa2.alphabet, fa2.initial_state, fa2_items,
					  fa2_nonterminals, fa2_terminals);
	// проверка на равенство букв переходов
	if (fa1_terminals.size() != fa2_terminals.size()) return false;
	for (int i = 0; i < fa1_terminals.size(); i++)
		if (*fa1_terminals[i] != *fa2_terminals[i]) return false;
	// грамматики из переходов
	vector<pair<GrammarItem, map<char, vector<GrammarItem>>>>
		transitions1_items;
	vector<GrammarItem*> transitions1_nonterminals;
	vector<GrammarItem*> transitions1_terminals;
	vector<vector<vector<GrammarItem*>>> transitions1_rules =
		tansitions_to_grammar(fa1.states, fa1.initial_state, fa1_nonterminals,
							  transitions1_items, transitions1_nonterminals,
							  transitions1_terminals);

	vector<pair<GrammarItem, map<char, vector<GrammarItem>>>>
		transitions2_items;
	vector<GrammarItem*> transitions2_nonterminals;
	vector<GrammarItem*> transitions2_terminals;
	vector<vector<vector<GrammarItem*>>> transitions2_rules =
		tansitions_to_grammar(fa2.states, fa2.initial_state, fa2_nonterminals,
							  transitions2_items, transitions2_nonterminals,
							  transitions2_terminals);
	// проверка равенства количества переходов
	if (transitions1_nonterminals.size() != transitions2_nonterminals.size())
		return false;
	// for(int i = 0; i < fa1_terminals.size(); i++) if(*fa1_terminals[i] !=
	// *fa2_terminals[i]) return false; биективная бисимуляция состояний
	vector<GrammarItem*> nonterminals(fa1_nonterminals);
	nonterminals.insert(nonterminals.end(), fa2_nonterminals.begin(),
						fa2_nonterminals.end());
	vector<vector<vector<GrammarItem*>>> rules(fa1_rules);
	rules.insert(rules.end(), fa2_rules.begin(), fa2_rules.end());

	vector<GrammarItem*> bisimilar_nonterminals;
	vector<vector<vector<GrammarItem*>>> bisimilar_rules =
		get_bijective_bibisimilar_grammar(rules, nonterminals,
										  bisimilar_nonterminals);

	vector<int> classes(bisimilar_nonterminals.size(), 0);
	for (auto t : fa1_nonterminals)
		classes[t->class_number]++;
	for (auto t : fa2_nonterminals)
		classes[t->class_number]--;
	for (auto t : classes)
		if (t != 0) return false;
	// биективная бисимуляция переходов
	vector<GrammarItem*> transitions_nonterminals(transitions1_nonterminals);
	transitions_nonterminals.insert(transitions_nonterminals.end(),
									transitions2_nonterminals.begin(),
									transitions2_nonterminals.end());
	vector<vector<vector<GrammarItem*>>> transitions_rules(transitions1_rules);
	transitions_rules.insert(transitions_rules.end(),
							 transitions2_rules.begin(),
							 transitions2_rules.end());

	vector<GrammarItem*> transitions_bisimilar_nonterminals;
	vector<vector<vector<GrammarItem*>>> transitions_bisimilar_rules =
		get_bijective_bibisimilar_grammar(transitions_rules,
										  transitions_nonterminals,
										  transitions_bisimilar_nonterminals);

	classes.clear();
	classes.resize(bisimilar_nonterminals.size(), 0);
	for (auto t : transitions1_nonterminals)
		classes[t->class_number]++;
	for (auto t : transitions2_nonterminals)
		classes[t->class_number]--;
	for (auto t : classes)
		if (t != 0) return false;

	return true;
}

bool FiniteAutomat::equivalent(const FiniteAutomat& fa1,
							   const FiniteAutomat& fa2) {
	return false;
}

bool FiniteAutomat::subset(const FiniteAutomat& fa) {
	/*FiniteAutomat fa_instersection(FiniteAutomat::intersection(*this, fa));
	cout << fa_instersection.to_txt() << endl;*/
	return false;
}