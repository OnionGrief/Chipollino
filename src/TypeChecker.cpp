#include <iostream>
#include "FiniteAutomat.h"
using namespace std;

//проверка на детерминированность
bool isdeterm(vector<State> states) {
	for (int i = 0; i < states.size(); i++) {
		for (auto elem : states[i].transitions) {
			if (elem.first == '\0') {
				return false;
			}
			if (elem.second.size() > 1) {
				return false;
			}
		}
	}
	return true;
}



