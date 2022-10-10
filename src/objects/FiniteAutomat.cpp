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
	int state_number = -1, class_number = -1;
	string term_name;
	Item(Type type, int state_number, int class_number) : type(type), state_number(state_number), class_number(class_number) {}
	Item(Type type, string term_name) : type(type), term_name(term_name) {}
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

void checkClasses(vector<vector<vector<Item*>>>& rules, map<set<string>, vector<Item*>>& classesCheckMap, vector<Item>& nonterminals) {
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
        classesCheckMap[tempRules].push_back(&nonterminals[i]);
    }
}

vector<vector<vector<Item*>>> fa_to_grammar(const vector<State>& states, const vector<char>& alphabet, int initial_state, vector<Item>& nonterminals, vector<Item>& terminals) {
	vector<vector<vector<Item*>>> rules (states.size());
	for(int i = 0; i < states.size(); i++){
		nonterminals.push_back(Item(Item::nonterminal, i, 0));
	}
	terminals.push_back(Item(Item::terminal, "eps"));
	for(int i = 0; i < alphabet.size(); i++) {
		terminals.push_back(Item(Item::terminal, string(1, alphabet[i])));
	}
	terminals.push_back(Item(Item::terminal, "init"));
	for(int i = 0; i < states.size(); i++){
		for (int j = 0; j <= alphabet.size(); j++) {
			for (int k = 0; k < states[i].transitions[j].size(); k++)
				rules[i].push_back({&terminals[j], &nonterminals[states[i].transitions[j][k]]});
		}
		if(states[i].is_terminal)
			rules[i].push_back({&terminals[0]});
	}
	rules[initial_state].push_back({&terminals[alphabet.size() + 1]});
	return rules;
}

vector<vector<vector<Item*>>> get_bisimilar_grammar(vector<vector<vector<Item*>>>& rules, vector<Item>& nonterminals, vector<Item*>& newNonterminals){
	map<set<string>, vector<Item*>> classesCheckMap;
	set<int> checker;	
	/*for(int i = 0; i < nonterminals.size(); i++){
		cout << i << ": ";
		for(int j = 0; j < rules[i].size(); j++){
			for(int k = 0; k < rules[i][j].size(); k++){
				cout << *rules[i][j][k];
			}
			cout << " ";
		}
		cout << endl;
	}*/

    //checker
    while(true) {
        set<int> temp = checker;
		checkClasses(rules, classesCheckMap, nonterminals);
        updateClasses(checker, classesCheckMap);
        if(checker == temp) break;
    }
    //output
    for(auto & elem : classesCheckMap) {
        cout << "{";
        for(int i = 0; i < elem.second.size() - 1; i++)  cout << *elem.second[i] << ",";
        cout << *elem.second[elem.second.size() - 1] << "}";
    }
    cout << endl;
    map<int, Item*> classToNonterm;
    /*for(int i = 0; i < states.size(); i++){
		auto t = classToNonterm.find(nonterminals[i].class_number);
        if(t == classToNonterm.end()) classToNonterm[nonterminals[i].class_number] = &nonterminals[i];
    }*/
	for(auto elem : classesCheckMap)
		classToNonterm[elem.second[0]->class_number] = elem.second[0];
	//for(auto elem : classToNonterm) cout << elem.first << ": " << *elem.second << endl;

	vector<vector<vector<Item*>>> newRules;
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
		newNonterminals.push_back(curNonterm);
		newRules.push_back(tempRules);
		for (vector<Item*> rule: tempRules) {
			cout << *curNonterm << " -> ";
			for (Item* item: rule) cout << *item << " ";
			cout << endl;
		}
	}

	return newRules;
}

FiniteAutomat FiniteAutomat::merge_bisimilar() {
	vector<Item> nonterminals;
	vector<Item> terminals;
	set<int> checker;	

	vector<vector<vector<Item*>>> rules = fa_to_grammar(this->states, this->alphabet, this->initial_state, nonterminals, terminals);

	vector<Item*> newNonterminals;
	vector<vector<vector<Item*>>> newRules = get_bisimilar_grammar(rules, nonterminals, newNonterminals);

	return FiniteAutomat();
}

bool bisimilar(FiniteAutomat& r1, FiniteAutomat& r2){
	return false;
}

bool equiv(FiniteAutomat& r1, FiniteAutomat& r2){
	return false;
}