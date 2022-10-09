#include "FiniteAutomat.h"
#include <sstream>
using namespace std;

State::State() : index(0), is_terminal(false), identifier("") {}

State::State(int index, vector<int> label, string identifier, bool is_terminal, map<char, vector<int> > transitions)
    : index(index), label(label), identifier(identifier), is_terminal(is_terminal), transitions(transitions) {}

void State::set_transition(int to, char symbol) {
    transitions[symbol].push_back(to);
}

FiniteAutomat::FiniteAutomat() { }

FiniteAutomat::FiniteAutomat(int initial_state, vector<char> alphabet, vector<State> states, bool is_deterministic)
	: initial_state(initial_state), alphabet(alphabet), states(states), is_deterministic(is_deterministic) {
	number_of_states = (int)states.size();
}

string FiniteAutomat::to_txt() {
	stringstream ss;
	ss << "digraph {\n\trankdir = LR\n\tdummy [label = \"\", shape = none]\n\t";
    for (int i = 0; i < states.size(); i++) {
        ss << i << " [label = \""  << states[i].identifier << "\", shape = ";
        ss << (states[i].is_terminal ? "doublecircle]\n\t" : "circle]\n\t");
    }
    ss << "dummy -> " << states[initial_state].index << "\n";

	for (int i = 0; i < number_of_states; i++) {
		for (int j = 0; j < alphabet.size(); j++) {
			for (int k = 0; k < states[i].transitions[alphabet[j]].size(); k++) {
				ss << "\t" << states[i].index << " -> "
					<< states[i].transitions[alphabet[j]][k]
					<< " [label = \"" << string(1, alphabet[j]) << "\"]\n";
			}
		}
	}

	ss << "}";
	return ss.str();
}

//обход автомата в глубину по eps-переходам
void dfs(vector<State> states, int index, vector<int>* C){
    if (find(C->begin(), C->end(), index) == C->end()) {
        C->push_back(index);
        for (int i = 0; i < states[index].transitions['\0'].size(); i++) {
            dfs(states, states[index].transitions['\0'][i], C);
        }
    }
}

//поиск множества состояний НКА, достижимых из множества состояний x по eps-переходам
vector<int> FiniteAutomat::closure(vector<int> x){
    vector<int> C;
    for(int i = 0; i < x.size(); i++) dfs(states, x[i], &C);
    return C;
}

//проверка меток на равенство
bool belong(State q, State u) {
    if (q.label.size() != u.label.size()) return false;
    for (int i = 0; i < q.label.size(); i++) {
        if (q.label[i] != u.label[i]) return false;
    }
    return true;
}

//НКА -> ДКА
FiniteAutomat FiniteAutomat::determinize(){
    FiniteAutomat NDM(this->initial_state, this->alphabet,
                      this->states, this->is_deterministic), DM;
    vector<int> x = {0};
    vector<int> q0 = NDM.closure(x);

    //инициализация начального состояния
    vector<int> label;
    for (int i = 0; i < q0.size(); i++) { label.push_back(q0[i]); }
    sort(label.begin(), label.end());
    State new_initial_state = { 0, label, NDM.states[NDM.initial_state].identifier,
                               false, map<char, vector<int> >() };
    DM.states.push_back(new_initial_state);
    DM.initial_state = 0;

    stack<vector <int> > s1;
    stack<int> s2;
    s1.push(q0);
    s2.push(0);

    while (not s1.empty()) {
        vector<int> z = s1.top();
        int index = s2.top();
        s1.pop();
        s2.pop();
        State q = DM.states[index];

        for (int i = 0; i < z.size(); i++) {
            if (NDM.states[z[i]].is_terminal) {
                DM.states[index].is_terminal = true;
                break;
            }
        }

        vector <int> new_x;
        for (int i = 0; i < NDM.alphabet.size(); i++) {
            new_x.clear();
            for (int j = 0; j < z.size(); j++) {
                for (int k = 0; k < NDM.states[z[j]].transitions[NDM.alphabet[i]].size(); k++) {
                    new_x.push_back(NDM.states[z[j]].transitions[NDM.alphabet[i]][k]);
                }
            }

            vector<int> z1 = NDM.closure(new_x);
            vector<int> new_label;
            for (int j = 0; j < z1.size(); j++) { new_label.push_back(z1[j]); }
            sort(new_label.begin(), new_label.end());

            //доработать с идентификаторами
            State q1 = { -1, new_label, "", false, map<char, vector<int> >() };
            bool accessory_flag = false;

            for (int j = 0; j < DM.states.size(); j++) {
                if (belong(q1, DM.states[j])) {
                    index = j;
                    accessory_flag = true;
                    break;
                }
            }

            if (!accessory_flag) index = -1;
            if (index != -1) q1 = DM.states[index];
            else {
                index = DM.states.size();
                q1.index = index;
                DM.states.push_back(q1);
                s1.push(z1);
                s2.push(index);
            }
            DM.states[q.index].transitions[NDM.alphabet[i]].push_back(q1.index);
        }
    }
    DM.number_of_states = DM.states.size();
    DM.alphabet = NDM.alphabet;
    DM.is_deterministic = true;
    return DM;
}