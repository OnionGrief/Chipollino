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
		expression_arden temp;
		temp.temp_regex = in[i].temp_regex->copy();
		temp.condition = in[i].condition;
		out.push_back(temp);
		while ((j < in.size()) && in[i].condition == in[j].condition) {
			cond = true;
			Regex* r = new Regex();
			Regex* s1 = new Regex();
			Regex* s2 = new Regex();
			// cout << in[j].temp_regex.to_txt() << in[i].temp_regex.to_txt();
			s1->regex_star(temp.temp_regex);
			s2->regex_star(in[j].temp_regex);
			r->regex_union(s1, s2);
			// delete temp.temp_regex;
			temp.temp_regex = r;
			// in[i].temp_regex = r;
			// delete r;
			delete s1;
			delete s2;
			j++;
		}
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
		Regex* r = new Regex();
		r->regex_star(in[0].temp_regex);
		expression_arden temp;
		temp.temp_regex = r;
		temp.condition = -1;
		out.push_back(temp);
		return out;
	}
	for (int i = 0; i < in.size(); i++) {
		if (i != indexcur) {
			Regex* r = new Regex();
			r->regex_star(in[indexcur].temp_regex);
			Regex* k = new Regex();
			k->regex_union(in[i].temp_regex, r);
			expression_arden temp;
			temp.temp_regex = k;
			delete r;
			temp.condition = in[i].condition;
			out.push_back(temp);
		}
	}
	// cout << "arden";
	return out;
}
Regex* nfa_to_regex(FiniteAutomaton in) {
	vector<int> endstate;
	vector<vector<expression_arden>> data;
	set<alphabet_symbol> alphabet = in.get_alphabet();

	for (int i = 0; i < in.get_states_size(); i++) {
		vector<expression_arden> temp;
		data.push_back(temp);
	}
	Regex* r = new Regex;
	// r->clear();
	// delete r;
	// Regex r ;
	expression_arden temp;
	temp.condition = -1;
	string str = "";
	r->regex_eps();
	// cout << r.to_txt();
	temp.temp_regex = r;
	data[in.get_initial()].push_back(temp);
	for (int i = 0; i < in.get_states_size(); i++) {
		State a = in.get_state(i);
		if (a.is_terminal) {
			endstate.push_back(i);
		}
		for (set<alphabet_symbol>::iterator it = alphabet.begin();
			 it != alphabet.end(); it++) {
			//}
			// for (int j = 0; j < alphabet.size(); j++) {

			if (a.transitions[*it].size()) {
				set<int> trans = a.transitions.at(*it);
				for (set<int>::iterator itt = trans.begin(); itt != trans.end();
					 itt++) {
					Regex* r = new Regex;
					expression_arden temp;
					temp.condition = i;
					string str = "";
					str += *it;
					r->from_string(str);
					temp.temp_regex = r;
					data[*itt].push_back(temp);
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
					Regex* r = new Regex;
					// Regex r ;
					r->regex_union(data[data[i][j].condition][k].temp_regex,
								   data[i][j].temp_regex);
					temp.temp_regex = r;

					temp.condition = data[data[i][j].condition][k].condition;
					// delete temp.temp_regex;
					tempdata.push_back(temp);
				}
			} else {
				expression_arden temp;
				Regex* r = new Regex(*data[i][j].temp_regex);
				temp.temp_regex = r;
				temp.condition = data[i][j].condition;
				tempdata.push_back(temp);
			}
		}

		sort(data[i].begin(), data[i].end(), compare);
		for (int o = 0; o < tempdata.size(); o++) {
			if (tempdata[o].temp_regex) {

				// cout << tempdata[o].temp_regex->to_txt() << "\n";
			}
			// delete tempdata[o].temp_regex;
		}
		for (int o = 0; o < tempdata.size(); o++) {
			// cout << tempdata[o].temp_regex->to_txt() << "\n";
			// data[i].push_back(tempdata[o]);
			// delete tempdata[o].temp_regex;
		}
		for (int o = 0; o < data[i].size(); o++) {
			delete data[i][o].temp_regex;
		}
		data[i].clear();
		// data[i] = tempdata;
		for (int o = 0; o < data[i].size(); o++) {
			//	delete data[i][o].temp_regex;
		}
		// data[i] = arden_minimize(data[i]);
		tempdata = arden_minimize(tempdata);

		vector<expression_arden> tempdata1 = arden(tempdata, i);
		for (int o = 0; o < tempdata.size(); o++) {
			// cout << tempdata[o].temp_regex->to_txt() << "\n";
			// data[i].push_back(tempdata[o]);
			// delete tempdata[o].temp_regex;
		}
		data[i] = tempdata1;
		/*cout << i << " ";
		for (int j = 0; j < data[i].size(); j++) {

			cout << data[i][j].condition << "-"
				 << data[i][j].temp_regex.to_txt() << " ";
		}
		cout << "\n";*/
	}

	if (data[0].size() > 1) {
		cout << "error";

		Regex* f = new Regex;
		return f;
	}
	for (int i = 0; i < data.size(); i++) {
		for (int j = 0; j < data[i].size(); j++) {
			if (data[i][j].condition != -1) {

				Regex* r = new Regex;
				r->regex_union(data[data[i][j].condition][0].temp_regex,
							   data[i][j].temp_regex);
				data[i][j].condition = -1;
				delete data[i][j].temp_regex;
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
		Regex* f = new Regex;
		return f;
	}
	if (endstate.size() < 2) {
		return data[endstate[0]][0].temp_regex;
	}
	Regex* r1 = new Regex;
	r1->regex_alt(data[endstate[0]][0].temp_regex,
				  data[endstate[1]][0].temp_regex);
	for (int i = 2; i < endstate.size(); i++) {
		Regex temp;
		temp.regex_alt(r1, data[endstate[i]][0].temp_regex);
		r1->clear();
		r1 = temp.copy();
	}
	for (int i = 0; i < data.size(); i++) {
		for (int j = 0; j < data[i].size(); j++) {
			delete data[i][j].temp_regex;
		}
	}
	// cout << r1->to_txt();

	return r1;
}
//На нефтеперерабатывающем заводе один мужик зажигалкой нашёл утечку газа.