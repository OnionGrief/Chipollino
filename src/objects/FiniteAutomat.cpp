#include "FiniteAutomat.h"
#include <sstream>
#include <algorithm>
using namespace std;

State::State() : index(0), is_terminal(false), identifier("") {}

State::State(int index, vector<int> label, string identifier, bool is_terminal, map<char, vector<int> > transitions)
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
		for (int j = 0; j < alphabet.size(); j++) {
			for (auto elem : states[i].transitions[alphabet[j]]) {
				ss << "\t" << states[i].index << " -> "
				<< elem
				<< " [label = \"" << string(1, alphabet[j]) << "\"]\n";
			}
		}
		for (auto elem : states[i].transitions['\0']) {
			ss << "\t" << states[i].index << " -> "
			<< elem
			<< " [label = \"" << "eps" << "\"]\n";
		}
	}

	ss << "}\n";
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
	FiniteAutomat NDM(this->initial_state, this->alphabet, this->states, this->is_deterministic), DM;
	vector<int> x = {0};
	vector<int> q0 = NDM.closure(x);

	//инициализация начального состояния
	vector<int> label;
	for (int i : q0) { label.push_back(i); }
	sort(label.begin(), label.end());
	State new_initial_state = { 0, label, NDM.states[NDM.initial_state].identifier, false, map<char, vector<int> >() };
	DM.states.push_back(new_initial_state);
	DM.initial_state = 0;

	stack<vector <int> > s1;
	stack<int> s2;
	s1.push(q0);
	s2.push(0);

	while (!s1.empty()) {
		vector<int> z = s1.top();
		int index = s2.top();
		s1.pop();
		s2.pop();
		State q = DM.states[index];

		for (int i : z) {
			if (NDM.states[i].is_terminal) {
				DM.states[index].is_terminal = true;
				break;
			}
		}

		vector <int> new_x;
		for (char ch : NDM.alphabet) {
			new_x.clear();
			for (int j : z) {
				for (int k : NDM.states[j].transitions[ch]) {
					new_x.push_back(k);
				}
			}

			vector<int> z1 = NDM.closure(new_x);
			vector<int> new_label;
			for (int j : z1) { new_label.push_back(j); }
			sort(new_label.begin(), new_label.end());

			//доработать с идентификаторами
			State q1 = { -1, new_label, "", false, map<char, vector<int> >() };
			bool accessory_flag = false;

			for (auto&  state : DM.states) {
				if (belong(q1, state)) {
					index = state.index;
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
			DM.states[q.index].transitions[ch].push_back(q1.index);
		}
	}
	DM.alphabet = NDM.alphabet;
	DM.is_deterministic = true;
	return DM;
}

//eps-замыкание
FiniteAutomat FiniteAutomat::rem_eps() {
	FiniteAutomat ENDM = FiniteAutomat(this->initial_state, this->alphabet, this->states, this->is_deterministic), NDM;

	NDM.states = ENDM.states;
	for (auto& state : NDM.states) {
		state.transitions = map<char,vector<int> >();
	}

	for (int i = 0; i < ENDM.states.size(); i++) {
		//для каждого состояния находим множество состояний, достижимых из данного по eps-переходам
		vector<int> state = {ENDM.states[i].index};
		vector<int> q = ENDM.closure(state);
		//смотрим, куда каждое переходит по символам алфавита
		vector<vector<int> > x;
		for (char ch : ENDM.alphabet) {
			x.clear();
			for (int k : q) {
				x.push_back(ENDM.states[k].transitions[ch]);
			}
			vector<int> q1;
			set<int> x1;
			for (auto& k : x) {
				q1 = ENDM.closure(k);
				for (int& m : q1) {
					x1.insert(m);
				}
			}
			for (auto elem : x1) {
				if (NDM.states[elem].is_terminal) {
					NDM.states[i].is_terminal = true;
				}
				NDM.states[i].transitions[ch].push_back(elem);
			}
		}
	}
	NDM.initial_state = ENDM.initial_state;
	NDM.alphabet = ENDM.alphabet;
	NDM.is_deterministic = false;
	return NDM;
}

//пересечение детерминированных автоматов (получается автомат, распознающий слова пересечения языков L1 и L2)
FiniteAutomat FiniteAutomat::intersection(FiniteAutomat DM2) {
	FiniteAutomat DM1 = FiniteAutomat(this->initial_state, this->alphabet, this->states, this->is_deterministic);
	FiniteAutomat DM = FiniteAutomat();
	DM.initial_state = 0;
	DM.alphabet = DM1.alphabet;
	int counter = 0;
	for (auto& state1 : DM1.states) {
		for (auto& state2 : DM2.states) {
			DM.states.push_back({ counter, {state1.index, state2.index}, state1.identifier + state2.identifier,
								  state1.is_terminal && state2.is_terminal,
								  map<char, vector<int> >() });
			counter++;
		}
	}

	for (auto& state : DM.states) {
		for (char ch : DM.alphabet) {
			state.transitions[ch].push_back(
					DM1.states[state.label[0]].transitions[ch][0] * 3 +
					DM2.states[state.label[1]].transitions[ch][0]);
		}
	}
	DM.is_deterministic = true;
	return DM;
}

//объединение детерминированных автоматов (получается автомат, распознающий слова объединенеия языков L1 и L2)
FiniteAutomat FiniteAutomat::uunion(FiniteAutomat DM2) {
	FiniteAutomat DM1 = FiniteAutomat(this->initial_state, this->alphabet, this->states, this->is_deterministic);
	FiniteAutomat DM = FiniteAutomat();
	DM.initial_state = 0;
	DM.alphabet = DM1.alphabet;
	int counter = 0;
	for (auto& state1 : DM1.states) {
		for (auto& state2 : DM2.states) {
			DM.states.push_back({ counter, {state1.index, state2.index}, state1.identifier + state2.identifier,
								  state1.is_terminal || state2.is_terminal,
								  map<char, vector<int> >() });
			counter++;
		}
	}

	for (auto& state : DM.states) {
		for (char ch : DM.alphabet) {
			state.transitions[ch].push_back(
					DM1.states[state.label[0]].transitions[ch][0] * 3 +
					DM2.states[state.label[1]].transitions[ch][0]);
		}
	}
	DM.is_deterministic = true;
	return DM;
}

//разность детерминированных автоматов (получается автомат, распознающий слова разности языков L1 и L2)
FiniteAutomat FiniteAutomat::difference(FiniteAutomat DM2) {
	FiniteAutomat DM1 = FiniteAutomat(this->initial_state, this->alphabet, this->states, this->is_deterministic);
	FiniteAutomat DM = FiniteAutomat();
	DM.initial_state = 0;
	DM.alphabet = DM1.alphabet;
	int counter = 0;
	for (auto& state1 : DM1.states) {
		for (auto& state2 : DM2.states) {
			DM.states.push_back({ counter, {state1.index, state2.index}, state1.identifier + state2.identifier,
								  state1.is_terminal && !state2.is_terminal,
								  map<char, vector<int> >() });
			counter++;
		}
	}

	for (auto& state : DM.states) {
		for (char ch : DM.alphabet) {
			state.transitions[ch].push_back(
					DM1.states[state.label[0]].transitions[ch][0] * 3 +
					DM2.states[state.label[1]].transitions[ch][0]);
		}
	}
	DM.is_deterministic = true;
	return DM;
}

//дополнение ДКА (получается автомат, распознающий язык L' = Σ* - L)
FiniteAutomat FiniteAutomat::complement() {
	FiniteAutomat DM = FiniteAutomat(this->initial_state, this->alphabet, this->states, this->is_deterministic);
	for (int i = 0; i < DM.states.size(); i++) {
		DM.states[i].is_terminal = !DM.states[i].is_terminal;
	}
	return DM;
}
