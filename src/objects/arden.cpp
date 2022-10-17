//
// Created by xtoter on 03.10.22.
//
#include "arden.h"
bool compare(expression_arden a, expression_arden b) {
	return (a.condition < b.condition);
}
Regex nfa_to_regex(FiniteAutomat in) {
	vector<vector<expression_arden>> data;
	vector<char> alphabet = in.get_alphabet();
	for (int i = 0; i < in.get_states_size(); i++) {
		vector<expression_arden> temp;
		data.push_back(temp);
	}
	for (int i = 0; i < in.get_states_size(); i++) {
		State a = in.get_state(i);
		for (int j = 0; j < alphabet.size(); j++) {
			if (a.transitions[alphabet[j]].size()) {
				vector<int> trans = a.transitions.at(alphabet[j]);
				for (int m = 0; m < trans.size(); m++) {
					Regex r;
					expression_arden temp;
					temp.condition = i;
					string str = "";
					str += alphabet[j];
					r.from_string(str);
					temp.temp_regex = r;
					data[trans[m]].push_back(temp);
				}
			}
		}
	}
	//сортируем
	for (int i = data.size() - 1; i >= 0; i--) {
		sort(data[i].begin(), data[i].end(), compare);
	}
}