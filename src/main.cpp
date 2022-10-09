#include <iostream>
#include <Regex.h>
#include <FiniteAutomat.h>
using namespace std;

int main() {
	cout << "Chipollino :-)\n";
	string reg = "((((a*c)))|(bd|q))";
	Regex r(reg);
	r.pre_order_travers();
	r.clear();

    vector<State> states;
    for (int i = 0; i < 5; i++) {
        State s = {i, {i}, "", false, map<char, vector <int> >()};
        states.push_back(s);
    }

    states[0].set_transition(1, 'a');
    states[0].set_transition(2, 'a');
    states[1].set_transition(3, 'b');
    states[3].set_transition(1, '\0');
    states[3].set_transition(3, 'a');
    states[2].set_transition(4, 'c');
    states[4].set_transition(4, 'a');
    states[4].set_transition(4, 'b');

    states[3].is_terminal = true;
    states[4].is_terminal = true;

    FiniteAutomat NDM(0, {'a', 'b', 'c'}, states, false);
    FiniteAutomat DM = NDM.determinize();
    cout << DM.to_txt();
}