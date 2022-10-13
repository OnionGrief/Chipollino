#include <iostream>
#include <vector>

#include "FiniteAutomat.h"
#include "TransformationMonoid.h"
using namespace std;

int main() {
    vector<State> states;
    for (int i = 0; i < 4; i++) {
        State s = {i, {i}, "", false, map<char, vector<int>>()};
        states.push_back(s);
    }

    states[0].set_transition(1, 'a');
    states[0].set_transition(0, 'b');
    states[0].set_transition(3, 'c');

    states[1].set_transition(1, 'a');
    states[1].set_transition(2, 'b');
    states[1].set_transition(1, 'c');

    states[2].set_transition(1, 'a');
    states[2].set_transition(2, 'b');
    states[2].set_transition(2, 'c');

    states[3].set_transition(3, 'a');

    states[1].is_terminal = true;
    states[2].is_terminal = true;

    FiniteAutomat NDM(0, {'a', 'b', 'c'}, states, false);
    TransformationMonoid a(&NDM);
    vector<Term> cur = a.getEqualenseClasses();

    Term test = cur[0];

    cout << test.name << "\n";
    a.getEqualenseClassesVWV(test);
    //   cout << NDM.to_txt();
}