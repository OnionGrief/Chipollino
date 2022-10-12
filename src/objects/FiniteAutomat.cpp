#include "FiniteAutomat.h"
#include <sstream>
#include <algorithm>
using namespace std;

State::State() : index(0), is_terminal(false), identifier("") {}

State::State(int index, vector<int> label, string identifier, bool is_terminal, map<char, vector<int>> transitions)
	: index(index), label(label), identifier(identifier), is_terminal(is_terminal), transitions(transitions) {}

void State::set_transition(int to, char symbol) {
	transitions[symbol].push_back(to);
}

FiniteAutomat::FiniteAutomat() { }

FiniteAutomat::FiniteAutomat(int initial_state, vector<char> alphabet, vector<State> states, bool is_deterministic)
	: initial_state(initial_state), alphabet(alphabet), states(states), is_deterministic(is_deterministic) {}

string FiniteAutomat::to_txt() {
	stringstream ss;
	ss << "digraph {\n\trankdir = LR\n\tdummy [label = \"\", shape = none]\n\t";
	for (int i = 0; i < states.size(); i++) {
		ss << i << " [label = \""  << states[i].identifier << "\", shape = ";
		ss << (states[i].is_terminal ? "doublecircle]\n\t" : "circle]\n\t");
	}
	ss << "dummy -> " << states[initial_state].index << "\n";

	for (int i = 0; i < states.size(); i++) {
		for (auto elem: states[i].transitions) {
			for (int transition: elem.second) {
				ss << "\t" << states[i].index << " -> " << transition;
				if(elem.first == '\0')
					ss  << " [label = \"" << "eps" << "\"]\n" ;
				else
					ss  << " [label = \"" << elem.first << "\"]\n" ;
			}
		}
	}

	ss << "}\n";
	return ss.str();
}

//обход автомата в глубину по eps-переходам
void dfs(vector<State> states, int index, vector<int>* c){
	if (find(c->begin(), c->end(), index) == c->end()) {
		c->push_back(index);
		for (int i = 0; i < states[index].transitions['\0'].size(); i++) {
			dfs(states, states[index].transitions['\0'][i], c);
		}
	}
}

vector<int> FiniteAutomat::closure(vector<int> x){
	vector<int> c;
	for(int i = 0; i < x.size(); i++)
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

FiniteAutomat FiniteAutomat::determinize(){
	FiniteAutomat ndm(initial_state, alphabet, states, is_deterministic), dm;
	vector<int> x = {0};
	vector<int> q0 = ndm.closure(x);

	vector<int> label = q0;
	sort(label.begin(), label.end());
	State new_initial_state = { 0, label, ndm.states[ndm.initial_state].identifier, false, map<char, vector<int>>() };
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

		vector <int> new_x;
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

			State q1 = { -1, new_label, "", false, map<char, vector<int>>() };
			bool accessory_flag = false;

			for (auto&  state : dm.states) {
				if (belong(q1, state)) {
					index = state.index;
					accessory_flag = true;
					break;
				}
			}

			if (!accessory_flag) index = -1;
			if (index != -1) q1 = dm.states[index];
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
	FiniteAutomat endm = FiniteAutomat(initial_state, alphabet, states, is_deterministic);
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

FiniteAutomat FiniteAutomat::intersection(const FiniteAutomat& dm1, const FiniteAutomat& dm2) {
	FiniteAutomat dm = FiniteAutomat();
	dm.initial_state = 0;
	dm.alphabet = dm1.alphabet;
	int counter = 0;
	for (auto& state1 : dm1.states) {
		for (auto& state2 : dm2.states) {
			dm.states.push_back({ counter, {state1.index, state2.index}, state1.identifier + state2.identifier,
								  state1.is_terminal && state2.is_terminal,
								  map<char, vector<int>>() });
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

FiniteAutomat FiniteAutomat::uunion(const FiniteAutomat& dm1, const FiniteAutomat& dm2) {
	FiniteAutomat dm = FiniteAutomat();
	dm.initial_state = 0;
	dm.alphabet = dm1.alphabet;
	int counter = 0;
	for (auto& state1 : dm1.states) {
		for (auto& state2 : dm2.states) {
			dm.states.push_back({ counter, {state1.index, state2.index}, state1.identifier + state2.identifier,
								  state1.is_terminal || state2.is_terminal,
								  map<char, vector<int>>() });
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
	FiniteAutomat dm1 = FiniteAutomat(initial_state, alphabet, states, is_deterministic);
	FiniteAutomat dm = FiniteAutomat();
	dm.initial_state = 0;
	dm.alphabet = dm1.alphabet;
	int counter = 0;
	for (auto& state1 : dm1.states) {
		for (auto& state2 : dm2.states) {
			dm.states.push_back({ counter, {state1.index, state2.index}, state1.identifier + state2.identifier,
								  state1.is_terminal && !state2.is_terminal,
								  map<char, vector<int>>() });
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
	FiniteAutomat dm = FiniteAutomat(initial_state, alphabet, states, is_deterministic);
	for (int i = 0; i < dm.states.size(); i++) {
		dm.states[i].is_terminal = !dm.states[i].is_terminal;
	}
	return dm;
}

//bisimilar

struct Item{
	enum Type {
		terminal,
		nonterminal
	};
	Type type;
	int state_number, class_number;
	string term_name;
	Item() : type(terminal), state_number(-1), class_number(-1),  term_name("") {}
	Item(Type type, int state_number, int class_number) : type(type), state_number(state_number), class_number(class_number) {}
	Item(Type type, string term_name) : type(type), term_name(term_name) {}
	bool operator!=(const Item& other) {
		return type != other.type || state_number != other.state_number ||
		class_number != other.class_number || term_name != other.term_name;
	}
	void operator=(const Item& other) {
		type = other.type;
		state_number = other.state_number;
		class_number = other.class_number;
		term_name = other.term_name;
	}
};
ostream& operator<<(ostream &os, const Item& item) {
	if(item.type == Item::terminal) return os << item.term_name;
		else return os << "S" << item.state_number;
}


void updateClasses(set<int>& checker, map<set<string>, vector<Item*>>& classesCheckMap) {
	int classNum = 0;
	checker.clear();
	for(auto elem : classesCheckMap) {
		checker.insert(elem.second[0]->state_number);
		for(Item* nont : elem.second){
			nont->class_number = classNum;
		}
		classNum++;
	}
}

void checkClasses(vector<vector<vector<Item*>>>& rules, map<set<string>, vector<Item*>>& classesCheckMap, vector<Item*>& nonterminals) {
	classesCheckMap.clear();
	for(int i = 0; i < nonterminals.size(); i++){
		set<string> tempRules;
		for(vector<Item*> rule : rules[i]){
			string newRule;

			for(Item* t : rule){
				if(t->type == Item::terminal) newRule += t->term_name;
				else newRule += to_string(t->class_number);
			}

			tempRules.insert(newRule);
		}
		classesCheckMap[tempRules].push_back(nonterminals[i]);
	}
}

vector<vector<vector<Item*>>> get_bisimilar_grammar(vector<vector<vector<Item*>>>& rules, vector<Item*>& nonterminals, vector<Item*>& bisimilarNonterminals){
	map<set<string>, vector<Item*>> classesCheckMap;
	set<int> checker;
	//checker
	while(true) {
		set<int> temp = checker;
		checkClasses(rules, classesCheckMap, nonterminals);
		updateClasses(checker, classesCheckMap);
		if(checker == temp) break;
	}
	//output
	/*for(auto & elem : classesCheckMap) {
		cout << "{";
		for(int i = 0; i < elem.second.size() - 1; i++)  cout << *elem.second[i] << ",";
		cout << *elem.second[elem.second.size() - 1] << "}";
	}
	cout << endl;*/
	map<int, Item*> classToNonterm;
	for(auto elem : classesCheckMap)
		classToNonterm[elem.second[0]->class_number] = elem.second[0];

	vector<vector<vector<Item*>>> bisimilarRules;
	for(auto elem : classesCheckMap) {
		Item* curNonterm = elem.second[0];
		vector<vector<Item*>> tempRules;
		for (vector<Item*> rule: rules[curNonterm->state_number]) {
			vector<Item*> tempRule;
			for (Item* item: rule) {
				if (item->type == Item::nonterminal) {
					tempRule.push_back(classToNonterm[item->class_number]);
				}else tempRule.push_back(item);
			}
			tempRules.push_back(tempRule);
		}
		bisimilarNonterminals.push_back(curNonterm);
		bisimilarRules.push_back(tempRules);
		/*for (vector<Item*> rule: tempRules) {
			cout << *curNonterm << " -> ";
			for (Item* item: rule) cout << *item << " ";
			cout << endl;
		}*/
	}
	
	return bisimilarRules;
}

vector<vector<vector<Item*>>> fa_to_grammar(const vector<State>& states, const vector<char>& alphabet, int initial_state, vector<Item>& fa_items, vector<Item*>& nonterminals, vector<Item*>& terminals) {
	vector<vector<vector<Item*>>> rules (states.size());
	fa_items.resize(states.size() + alphabet.size() + 2);
	int ind = 0;
	while(ind < states.size()){
		fa_items[ind] = Item(Item::nonterminal, ind, 0);
		nonterminals.push_back(&fa_items[ind]);
		ind++;
	}
	fa_items[ind]=(Item(Item::terminal, "eps"));
	terminals.push_back(&fa_items[ind]);
	ind++;
	for(int i = 0; i < alphabet.size(); i++) {
		fa_items[ind]=(Item(Item::terminal, string(1, alphabet[i])));
		terminals.push_back(&fa_items[ind]);
		ind++;
	}
	fa_items[ind]=(Item(Item::terminal, "init"));
	terminals.push_back(&fa_items[ind]);

	for(int i = 0; i < states.size(); i++){
		for (int j = 0; j <= alphabet.size(); j++) {
			for (int k = 0; k < states[i].transitions[j].size(); k++)
				rules[i].push_back({terminals[j], nonterminals[states[i].transitions[j][k]]});
		}
		if(states[i].is_terminal)
			rules[i].push_back({terminals[0]});
	}
	rules[initial_state].push_back({terminals[alphabet.size() + 1]});

	return rules;
}

FiniteAutomat FiniteAutomat::merge_bisimilar() {
	vector<Item> fa_items;
	vector<Item*> nonterminals;
	vector<Item*> terminals;

	vector<vector<vector<Item*>>> rules = fa_to_grammar(this->states, this->alphabet, this->initial_state, fa_items, nonterminals, terminals);


	vector<Item*> bisimilarNonterminals;
	vector<vector<vector<Item*>>> bisimilarRules = get_bisimilar_grammar(rules, nonterminals, bisimilarNonterminals);


	return FiniteAutomat();
}

bool FiniteAutomat::bisimilar(const FiniteAutomat& fa1, const FiniteAutomat& fa2){
	vector<Item> fa1_items;
	vector<Item*> fa1_nonterminals;
	vector<Item*> fa1_terminals;
	vector<vector<vector<Item*>>> fa1_rules = fa_to_grammar(fa1.states, fa1.alphabet, fa1.initial_state, fa1_items, fa1_nonterminals, fa1_terminals);

	
	vector<Item> fa2_items;
	vector<Item*> fa2_nonterminals;
	vector<Item*> fa2_terminals;
	vector<vector<vector<Item*>>> fa2_rules = fa_to_grammar(fa2.states, fa2.alphabet, fa2.initial_state, fa2_items, fa2_nonterminals, fa2_terminals);

	if(fa1_terminals.size() != fa2_terminals.size()) return false;
	for(int i = 0; i < fa1_terminals.size(); i++) if(*fa1_terminals[i] != *fa2_terminals[i]) return false;

	vector<Item*> fa1_bisimilarNonterminals;
	vector<vector<vector<Item*>>> fa1_bisimilarRules = get_bisimilar_grammar(fa1_rules, fa1_nonterminals, fa1_bisimilarNonterminals);

	vector<Item*> fa2_bisimilarNonterminals;
	vector<vector<vector<Item*>>> fa2_bisimilarRules = get_bisimilar_grammar(fa2_rules, fa2_nonterminals, fa2_bisimilarNonterminals);

	if(fa1_bisimilarNonterminals.size() != fa2_bisimilarNonterminals.size()) return false;

	vector<Item*> fa12_nonterminals(fa1_bisimilarNonterminals);
	fa12_nonterminals.insert(fa12_nonterminals.end(), fa2_bisimilarNonterminals.begin(), fa2_bisimilarNonterminals.end());
	vector<vector<vector<Item*>>> fa12_rules(fa1_bisimilarRules);
	fa12_rules.insert(fa12_rules.end(), fa2_bisimilarRules.begin(), fa2_bisimilarRules.end());

	/*for(int i = 0; i < fa12_nonterminals.size(); i++){
		cout << *fa12_nonterminals[i] << ": ";
		for(int j = 0; j < fa12_rules[i].size(); j++){
			for(int k = 0; k < fa12_rules[i][j].size(); k++){
				cout << *fa12_rules[i][j][k];
			}
			cout << " ";
		}
		cout << endl;
	}*/

	vector<Item*> fa12_bisimilarNonterminals;
	vector<vector<vector<Item*>>> fa12_bisimilarRules = get_bisimilar_grammar(fa12_rules, fa12_nonterminals, fa12_bisimilarNonterminals);

	if(fa1_bisimilarNonterminals.size() != fa12_bisimilarNonterminals.size()) return false;

	return true;
}

bool FiniteAutomat::equiv(const FiniteAutomat& r1, const FiniteAutomat& r2){
	return false;
}