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

struct GrammarItem{
	enum Type {
		terminal,
		nonterminal
	};
	Type type;
	int state_number, class_number;
	int automaton_characteristic; // у первого автомата +1, у второго -1. при проверке классов сумма = 0
	string term_name;
	GrammarItem() : type(terminal), state_number(-1), class_number(-1),  term_name("") {}
	GrammarItem(Type type, int state_number, int class_number) : type(type), state_number(state_number), class_number(class_number) {}
	GrammarItem(Type type, string term_name) : type(type), term_name(term_name) {}
	bool operator!=(const GrammarItem& other) {
		return type != other.type || state_number != other.state_number ||
		class_number != other.class_number || term_name != other.term_name;
	}
	void operator=(const GrammarItem& other) {
		type = other.type;
		state_number = other.state_number;
		class_number = other.class_number;
		term_name = other.term_name;
	}
};
ostream& operator<<(ostream &os, const GrammarItem& item) {
	if(item.type == GrammarItem::terminal) return os << "" << item.term_name;
		else return os << "S" << item.state_number;
}


void update_classes(set<int>& checker, map<set<string>, vector<GrammarItem*>>& classes_check_map) {
	int classNum = 0;
	checker.clear();
	for(auto elem : classes_check_map) {
		checker.insert(elem.second[0]->state_number);
		for(GrammarItem* nont : elem.second){
			nont->class_number = classNum;
		}
		classNum++;
	}
}

void check_classes(vector<vector<vector<GrammarItem*>>>& rules, map<set<string>, vector<GrammarItem*>>& classes_check_map, vector<GrammarItem*>& nonterminals) {
	classes_check_map.clear();
	for(int i = 0; i < nonterminals.size(); i++){
		set<string> tempRules;
		for(vector<GrammarItem*> rule : rules[i]){
			string newRule;

			for(GrammarItem* t : rule){
				if(t->type == GrammarItem::terminal) newRule += t->term_name;
				else newRule += to_string(t->class_number);
			}

			tempRules.insert(newRule);
		}
		classes_check_map[tempRules].push_back(nonterminals[i]);
	}
}

vector<vector<vector<GrammarItem*>>> get_bisimilar_grammar(vector<vector<vector<GrammarItem*>>>& rules,
	vector<GrammarItem*>& nonterminals, vector<GrammarItem*>& bisimilar_nonterminals)
{
	map<set<string>, vector<GrammarItem*>> classes_check_map;
	set<int> checker;
	//checker
	while(true) {
		set<int> temp = checker;
		check_classes(rules, classes_check_map, nonterminals);
		update_classes(checker, classes_check_map);
		if(checker == temp) break;
	}
	//output
	/*for(auto & elem : classes_check_map) {
		cout << "{";
		for(int i = 0; i < elem.second.size() - 1; i++)  cout << *elem.second[i] << ",";
		cout << *elem.second[elem.second.size() - 1] << "}";
	}
	cout << endl;*/
	map<int, GrammarItem*> classToNonterm;
	for(auto elem : classes_check_map)
		classToNonterm[elem.second[0]->class_number] = elem.second[0];

	vector<vector<vector<GrammarItem*>>> bisimilar_rules;
	for(auto elem : classes_check_map) {
		GrammarItem* curNonterm = elem.second[0];
		vector<vector<GrammarItem*>> tempRules;
		for (vector<GrammarItem*> rule: rules[curNonterm->state_number]) {
			vector<GrammarItem*> tempRule;
			for (GrammarItem* item: rule) {
				if (item->type == GrammarItem::nonterminal) {
					tempRule.push_back(classToNonterm[item->class_number]);
				}else tempRule.push_back(item);
			}
			tempRules.push_back(tempRule);
		}
		bisimilar_nonterminals.push_back(curNonterm);
		bisimilar_rules.push_back(tempRules);
		/*for (vector<GrammarItem*> rule: tempRules) {
			cout << *curNonterm << " -> ";
			for (GrammarItem* item: rule) cout << *item << " ";
			cout << endl;
		}*/
	}
	
	return bisimilar_rules;
}

vector<vector<vector<GrammarItem*>>> fa_to_grammar(const vector<State>& states, const vector<char>& alphabet, int initial_state,
	vector<GrammarItem>& fa_items, vector<GrammarItem*>& nonterminals, vector<GrammarItem*>& terminals)
{
	vector<vector<vector<GrammarItem*>>> rules (states.size());
	fa_items.resize(states.size() + alphabet.size() + 2);
	int ind = 0;
	while(ind < states.size()){
		fa_items[ind] = GrammarItem(GrammarItem::nonterminal, ind, 0);
		nonterminals.push_back(&fa_items[ind]);
		ind++;
	}
	map<char, int> terminal_index;
	fa_items[ind]=(GrammarItem(GrammarItem::terminal, "eps"));
	terminals.push_back(&fa_items[ind]);
	terminal_index['\0'] = 0;
	ind++;
	for(int i = 0; i < alphabet.size(); i++) {
		fa_items[ind]=(GrammarItem(GrammarItem::terminal, string(1, alphabet[i])));
		terminals.push_back(&fa_items[ind]);
		terminal_index[alphabet[i]] = i+1;
		ind++;
	}
	fa_items[ind]=(GrammarItem(GrammarItem::terminal, "init"));
	terminals.push_back(&fa_items[ind]);

	for(int i = 0; i < states.size(); i++){
		for(auto elem : states[i].transitions){
			for(int j = 0; j < elem.second.size(); j++)
				rules[i].push_back({terminals[terminal_index[elem.first]], nonterminals[elem.second[j]]});
		}
		if(states[i].is_terminal)
			rules[i].push_back({terminals[0]});
	}
	rules[initial_state].push_back({terminals[alphabet.size() + 1]});

	return rules;
}

FiniteAutomat FiniteAutomat::merge_bisimilar() {
	vector<GrammarItem> fa_items;
	vector<GrammarItem*> nonterminals;
	vector<GrammarItem*> terminals;

	vector<vector<vector<GrammarItem*>>> rules = fa_to_grammar(this->states, this->alphabet, this->initial_state, fa_items, nonterminals, terminals);

	vector<GrammarItem*> bisimilar_nonterminals;
	vector<vector<vector<GrammarItem*>>> bisimilar_rules = get_bisimilar_grammar(rules, nonterminals, bisimilar_nonterminals);

	// порождение автомата

	return FiniteAutomat();
}

bool FiniteAutomat::bisimilar(const FiniteAutomat& fa1, const FiniteAutomat& fa2){
	vector<GrammarItem> fa1_items;
	vector<GrammarItem*> fa1_nonterminals;
	vector<GrammarItem*> fa1_terminals;
	vector<vector<vector<GrammarItem*>>> fa1_rules = fa_to_grammar(fa1.states, fa1.alphabet, fa1.initial_state, fa1_items, fa1_nonterminals, fa1_terminals);

	vector<GrammarItem> fa2_items;
	vector<GrammarItem*> fa2_nonterminals;
	vector<GrammarItem*> fa2_terminals;
	vector<vector<vector<GrammarItem*>>> fa2_rules = fa_to_grammar(fa2.states, fa2.alphabet, fa2.initial_state, fa2_items, fa2_nonterminals, fa2_terminals);

	if(fa1_terminals.size() != fa2_terminals.size()) return false;
	for(int i = 0; i < fa1_terminals.size(); i++) if(*fa1_terminals[i] != *fa2_terminals[i]) return false;

	vector<GrammarItem*> fa1_bisimilar_nonterminals;
	vector<vector<vector<GrammarItem*>>> fa1_bisimilar_rules = get_bisimilar_grammar(fa1_rules, fa1_nonterminals, fa1_bisimilar_nonterminals);

	vector<GrammarItem*> fa2_bisimilar_nonterminals;
	vector<vector<vector<GrammarItem*>>> fa2_bisimilar_rules = get_bisimilar_grammar(fa2_rules, fa2_nonterminals, fa2_bisimilar_nonterminals);

	if(fa1_bisimilar_nonterminals.size() != fa2_bisimilar_nonterminals.size()) return false;

	vector<GrammarItem*> nonterminals(fa1_bisimilar_nonterminals);
	nonterminals.insert(nonterminals.end(), fa2_bisimilar_nonterminals.begin(), fa2_bisimilar_nonterminals.end());
	vector<vector<vector<GrammarItem*>>> rules(fa1_bisimilar_rules);
	rules.insert(rules.end(), fa2_bisimilar_rules.begin(), fa2_bisimilar_rules.end());

	vector<GrammarItem*> bisimilar_nonterminals;
	vector<vector<vector<GrammarItem*>>> bisimilar_rules = get_bisimilar_grammar(rules, nonterminals, bisimilar_nonterminals);

	if(fa1_bisimilar_nonterminals.size() != bisimilar_nonterminals.size()) return false;

	return true;
}

void update_bijective_Classes(set<int>& checker, map<multiset<string>, vector<GrammarItem*>>& classes_check_map) {
	int classNum = 0;
	checker.clear();
	for(auto elem : classes_check_map) {
		checker.insert(elem.second[0]->state_number);
		for(GrammarItem* nont : elem.second){
			nont->class_number = classNum;
		}
		classNum++;
	}
}

void check_bijective_classes(vector<vector<vector<GrammarItem*>>>& rules, map<multiset<string>, vector<GrammarItem*>>& classes_check_map, vector<GrammarItem*>& nonterminals) {
	classes_check_map.clear();
	for(int i = 0; i < nonterminals.size(); i++){
		multiset<string> tempRules;
		for(vector<GrammarItem*> rule : rules[i]){
			string newRule;

			for(GrammarItem* t : rule){
				if(t->type == GrammarItem::terminal) newRule += t->term_name;
				else newRule += to_string(t->class_number);
			}

			tempRules.insert(newRule);
		}
		classes_check_map[tempRules].push_back(nonterminals[i]);
	}
}

vector<vector<vector<GrammarItem*>>> get_bijective_bibisimilar_grammar(vector<vector<vector<GrammarItem*>>>& rules, vector<GrammarItem*>& nonterminals, vector<GrammarItem*>& bisimilar_nonterminals, int& classes_num){
	map<multiset<string>, vector<GrammarItem*>> classes_check_map;
	set<int> checker;
	//checker
	while(true) {
		set<int> temp = checker;
		check_bijective_classes(rules, classes_check_map, nonterminals);
		update_bijective_Classes(checker, classes_check_map);
		if(checker == temp) break;
	}
	//output
	/*for(auto & elem : classes_check_map) {
		cout << "{";
		for(int i = 0; i < elem.second.size() - 1; i++)  cout << *elem.second[i] << ",";
		cout << *elem.second[elem.second.size() - 1] << "}";
	}
	cout << endl;*/
	map<int, GrammarItem*> classToNonterm;
	for(auto elem : classes_check_map)
		classToNonterm[elem.second[0]->class_number] = elem.second[0];

	vector<vector<vector<GrammarItem*>>> bisimilar_rules;
	for(auto elem : classes_check_map) {
		GrammarItem* curNonterm = elem.second[0];
		vector<vector<GrammarItem*>> tempRules;
		for (vector<GrammarItem*> rule: rules[curNonterm->state_number]) {
			vector<GrammarItem*> tempRule;
			for (GrammarItem* item: rule) {
				if (item->type == GrammarItem::nonterminal) {
					tempRule.push_back(classToNonterm[item->class_number]);
				}else tempRule.push_back(item);
			}
			tempRules.push_back(tempRule);
		}
		bisimilar_nonterminals.push_back(curNonterm);
		bisimilar_rules.push_back(tempRules);
	}

	classes_num = classes_check_map.size();
	
	return bisimilar_rules;
}

vector<vector<vector<GrammarItem*>>> tansitions_to_grammar(const vector<State>& states, int initial_state,
	vector<pair<GrammarItem, map<char, vector<GrammarItem>>>>& fa_items, vector<GrammarItem*>& nonterminals, vector<GrammarItem*>& terminals)
{
	fa_items.resize(states.size());
	int ind = 0;
	for(int i = 0; i < states.size(); i++){
		fa_items[i].first = GrammarItem(GrammarItem::terminal, to_string(i));
		terminals.push_back(&fa_items[i].first);
		for(auto elem : states[i].transitions){
			vector<GrammarItem>& item_vec = fa_items[i].second[elem.first];
			item_vec.resize(elem.second.size());
			for(int j = 0; j < elem.second.size(); j++){
				item_vec[j] = (GrammarItem(GrammarItem::nonterminal, ind, 0));
				nonterminals.push_back(&item_vec[j]);
				ind++;
			}
		}
	}
	
	vector<vector<vector<GrammarItem*>>> rules (nonterminals.size());

	ind = 0;
	for(int i = 0; i < states.size(); i++){
		for(auto elem : states[i].transitions){
			for(int j = 0; j < elem.second.size(); j++){
				int transInd = elem.second[j]; // индекс состояния, в которое идет переход
				// смотрим все переходы из этого состояния
				for(auto transition_elem : states[transInd].transitions){
					for(int k = 0; k < transition_elem.second.size(); k++){
						int nonterm_ind = fa_items[transInd].second[transition_elem.first][k].state_number;
						rules[ind].push_back({terminals[transInd], nonterminals[nonterm_ind]});
						//cout << *nonterminals[ind] << " -> " << *terminals[transInd] << " " << *nonterminals[nonterm_ind] << endl;
					}
				}
				ind++;
			}
		}
	}

	return rules;
}


bool FiniteAutomat::equal(const FiniteAutomat& fa1, const FiniteAutomat& fa2){
	if(fa1.states.size() != fa2.states.size()) return false;

	vector<GrammarItem> fa1_items;
	vector<GrammarItem*> fa1_nonterminals;
	vector<GrammarItem*> fa1_terminals;
	vector<vector<vector<GrammarItem*>>> fa1_rules = fa_to_grammar(fa1.states, fa1.alphabet, fa1.initial_state, fa1_items, fa1_nonterminals, fa1_terminals);

	vector<GrammarItem> fa2_items;
	vector<GrammarItem*> fa2_nonterminals;
	vector<GrammarItem*> fa2_terminals;
	vector<vector<vector<GrammarItem*>>> fa2_rules = fa_to_grammar(fa2.states, fa2.alphabet, fa2.initial_state, fa2_items, fa2_nonterminals, fa2_terminals);

	if(fa1_terminals.size() != fa2_terminals.size()) return false;
	for(int i = 0; i < fa1_terminals.size(); i++) if(*fa1_terminals[i] != *fa2_terminals[i]) return false;

	vector<GrammarItem*> nonterminals(fa1_nonterminals);
	nonterminals.insert(nonterminals.end(), fa2_nonterminals.begin(), fa2_nonterminals.end());
	vector<vector<vector<GrammarItem*>>> rules(fa1_rules);
	rules.insert(rules.end(), fa2_rules.begin(), fa2_rules.end());

	vector<GrammarItem*> bisimilar_nonterminals;
	int classes_num;
	vector<vector<vector<GrammarItem*>>> bisimilar_rules = get_bijective_bibisimilar_grammar(rules, nonterminals, bisimilar_nonterminals, classes_num);

	vector<int> classes (classes_num, 0);

	for(auto t : fa1_nonterminals) classes[t->class_number]++;
	for(auto t : fa2_nonterminals) classes[t->class_number]--;

	for(auto t : classes)
		if(t != 0) return false;
		
	vector<pair<GrammarItem, map<char, vector<GrammarItem>>>> transitions1_items;
	vector<GrammarItem*> transitions1_nonterminals;
	vector<GrammarItem*> transitions1_terminals;
	vector<vector<vector<GrammarItem*>>> transitions1_rules = tansitions_to_grammar(fa1.states, fa1.initial_state, transitions1_items, transitions1_nonterminals, transitions1_terminals);

	vector<pair<GrammarItem, map<char, vector<GrammarItem>>>> transitions2_items;
	vector<GrammarItem*> transitions2_nonterminals;
	vector<GrammarItem*> transitions2_terminals;
	vector<vector<vector<GrammarItem*>>> transitions2_rules = tansitions_to_grammar(fa2.states, fa2.initial_state, transitions2_items, transitions2_nonterminals, transitions2_terminals);

	if(transitions1_terminals.size() != transitions2_terminals.size()) return false;
	if(transitions1_nonterminals.size() != transitions2_nonterminals.size()) return false;
	//for(int i = 0; i < fa1_terminals.size(); i++) if(*fa1_terminals[i] != *fa2_terminals[i]) return false;

	vector<GrammarItem*> transitions_nonterminals(transitions1_nonterminals);
	transitions_nonterminals.insert(transitions_nonterminals.end(), transitions2_nonterminals.begin(), transitions2_nonterminals.end());
	vector<vector<vector<GrammarItem*>>> transitions_rules(transitions1_rules);
	transitions_rules.insert(transitions_rules.end(), transitions2_rules.begin(), transitions2_rules.end());

	vector<GrammarItem*> transitions_bisimilar_nonterminals;
	vector<vector<vector<GrammarItem*>>> transitions_bisimilar_rules = get_bijective_bibisimilar_grammar(transitions_rules, transitions_nonterminals, transitions_bisimilar_nonterminals, classes_num);

	classes.clear();
	classes.resize(classes_num, 0);

	for(auto t : transitions1_nonterminals) classes[t->class_number]++;
	for(auto t : transitions2_nonterminals) classes[t->class_number]--;

	for(auto t : classes)
		if(t != 0) return false;

	return true;
}

bool FiniteAutomat::equivalent(const FiniteAutomat& fa1, const FiniteAutomat& fa2){
	return false;
}

