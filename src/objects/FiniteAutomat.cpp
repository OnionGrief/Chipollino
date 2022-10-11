#include "FiniteAutomat.h"
#include <sstream>
#include <map>
#include <set>
#include <iostream>
using namespace std;

State::State() : index(0), is_terminal(0), identifier("") {}

State::State(int index, bool is_terminal, string identifier, vector<vector<int>> transitions)
	: index(index), is_terminal(is_terminal), identifier(identifier), transitions(transitions) {}

FiniteAutomat::FiniteAutomat() { }

FiniteAutomat::FiniteAutomat(int initial_state, vector<char> alphabet, vector<State> states, bool is_deterministic)
	: initial_state(initial_state), alphabet(alphabet), states(states), is_deterministic(is_deterministic) {
	number_of_states = (int)states.size();
}

State& FiniteAutomat::get_transition(int i, int j, int k) {
	return states[states[i].transitions[j][k]];
}

string FiniteAutomat::to_txt() {
	stringstream ss;
	ss << "digraph {\n\trankdir = LR\n\tdummy [label = \"\", shape = none]\n\tdummy -> "
		<< states[initial_state].identifier << "\n";

	for (int i = 0; i < number_of_states; i++) {
		for (int j = 0; j <= alphabet.size(); j++) {
			string letter = (j == 0) ? "eps" : string(1, alphabet[j - 1]);
			for (int k = 0; k < states[i].transitions[j].size(); k++) {
				ss << "\t" << states[i].identifier << " -> "
					<< this->get_transition(i, j, k).identifier
					<< " [label = \"" << letter << "\"]\n";
			}
		}
	}

	ss << "}";
	return ss.str();
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