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

			i = j - 1;
		}
	}

	return out;
}

vector<expression_arden> arden(vector<expression_arden> in, int index) {
	vector<expression_arden> out;
	int indexcur = -1;
	for (int i = 0; (i < in.size() && indexcur == -1); i++) {
		if (in[i].condition == index) {
			indexcur = i;
		}
	}
	if (indexcur == -1) {
		return in;
	}
	if (in.size() < 2) {
		Regex r;
		r.regex_star(in[0].temp_regex);
		expression_arden temp;
		temp.temp_regex = r;
		temp.condition = -1;
		out.push_back(temp);
		return out;
	}
	for (int i = 0; i < in.size(); i++) {
		if (i != indexcur) {
			Regex r;
			r.regex_star(in[indexcur].temp_regex);
			Regex k;
			k.regex_union(in[i].temp_regex, r);
			expression_arden temp;
			temp.temp_regex = k;
			temp.condition = in[i].condition;
			out.push_back(temp);
		}
	}
	// cout << "arden";
	return out;
}
Regex nfa_to_regex(FiniteAutomat in) {
	vector<int> endstate;
	vector<vector<expression_arden>> data;
	vector<char> alphabet = in.get_alphabet();

	for (int i = 0; i < in.get_states_size(); i++) {
		vector<expression_arden> temp;
		data.push_back(temp);
	}
	Regex r;
	expression_arden temp;
	temp.condition = -1;
	string str = "";
	r.regex_eps();
	cout << r.to_txt();
	temp.temp_regex = r;
	data[in.get_initial()].push_back(temp);
	for (int i = 0; i < in.get_states_size(); i++) {
		State a = in.get_state(i);
		if (a.is_terminal) {
			endstate.push_back(i);
		}
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
					tempdata.push_back(temp);
				}
			} else {
				tempdata.push_back(data[i][j]);
			}
		}

		sort(data[i].begin(), data[i].end(), compare);
		data[i] = tempdata;
		data[i] = arden_minimize(data[i]);

		data[i] = arden(data[i], i);
		cout << i << " ";
		for (int j = 0; j < data[i].size(); j++) {

			cout << data[i][j].condition << "-"
				 << data[i][j].temp_regex.to_txt() << " ";
		}
		cout << "\n";
	}

	if (data[0].size() > 1) {
		cout << "error";
		Regex f;
		return f;
	}
	for (int i = 0; i < data.size(); i++) {
		for (int j = 0; j < data[i].size(); j++) {
			if (data[i][j].condition != -1) {
				Regex r;
				r.regex_union(data[data[i][j].condition][0].temp_regex,
							  data[i][j].temp_regex);
				data[i][j].condition = -1;
				data[i][j].temp_regex = r;
			}
		}
		data[i] = arden_minimize(data[i]);
		for (int j = 0; j < data[i].size(); j++) {

			/*cout << data[i][j].condition << "-"
				 << data[i][j].temp_regex.to_txt() << " \n";*/
		}
	}
	if (endstate.size() == 0) {
		Regex f;
		return f;
	}
	if (endstate.size() < 2) {
		return data[endstate[0]][0].temp_regex;
	}
	Regex r1;
	r1.regex_alt(data[endstate[0]][0].temp_regex,
				 data[endstate[1]][0].temp_regex);
	for (int i = 2; i < endstate.size(); i++) {
		Regex temp;
		temp.regex_alt(r1, data[endstate[i]][0].temp_regex);
		r1 = temp;
	}
	return r1;
}
//На нефтеперерабатывающем заводе один мужик зажигалкой нашёл утечку газа.