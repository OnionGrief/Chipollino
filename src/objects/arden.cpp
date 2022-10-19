//
// Created by xtoter on 03.10.22.
//
#include "arden.h"
bool compare(expression_arden a, expression_arden b) {
	return (a.condition < b.condition);
}

vector<expression_arden> arden_minimize(vector<expression_arden> in) {
	vector<expression_arden> out;
	for (int i = 0; i < in.size(); i++) {
		int j = i + 1;
		bool cond = false;

		while ((j < in.size()) && in[i].condition == in[j].condition) {

			cond = true;
			Regex r;

			// cout << in[j].temp_regex.to_txt() << in[i].temp_regex.to_txt();
			r.regex_union(in[i].temp_regex, in[j].temp_regex);
			in[i].temp_regex = r;

			j++;
		}

		// cout << in[i].temp_regex.to_txt() << " ";
		out.push_back(in[i]);
		if (cond) {

			i = j;
		}
	} /*
	 for (int i = 0; i < out.size(); i++) {
		 cout << out[i].temp_regex.to_txt();
	 }*/
	return out;
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
		data[i] = arden_minimize(data[i]);
		vector<expression_arden> tempdata;
		for (int j = 0; j < data[i].size(); j++) {
			if (data[i][j].condition > i) {
				for (int k = 0; k < data[data[i][j].condition].size(); k++) {

					expression_arden temp;
					Regex r;
					r.regex_union(data[data[i][j].condition][k].temp_regex,
								  data[i][j].temp_regex);
					temp.temp_regex = r;
					temp.condition = data[data[i][j].condition][k].condition;
					// cout << r.to_txt() << "\n";
					tempdata.push_back(temp);
				}
			} else {
				tempdata.push_back(data[i][j]);
			}
		}
		for (int j = 0; j < tempdata.size(); j++) {

			cout << tempdata[j].condition << "-"
				 << tempdata[j].temp_regex.to_txt() << " ";
		}
		data[i] = tempdata;
		cout << "\n";
	}
	Regex f;
	return f;
}
//На нефтеперерабатывающем заводе один мужик зажигалкой нашёл утечку газа.