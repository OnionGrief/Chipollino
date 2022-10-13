#include "TransformationMonoid.h"

#include <iostream>
using namespace std;

//получаем	все	перестановки	алфавита	длины	len
vector<string> get_comb_alphabet(int len, vector<char> alphabet) {
	vector<string> newcomb;
	if (len == 0) {
		return newcomb;
	}
	for (int i = 0; i < alphabet.size(); i++) {
		string temp;
		temp += alphabet[i];
		newcomb.push_back(temp);
	}
	if (len == 1) {
		return newcomb;
	}
	vector<string> comb;
	vector<string> oldcomb = get_comb_alphabet(len - 1, alphabet);
	for (int i = 0; i < newcomb.size(); i++) {
		for (int j = 0; j < oldcomb.size(); j++) {
			comb.push_back(newcomb[i] + oldcomb[j]);
		}
	}
	return comb;
}

//Проверяем	встречался	ли	терм	раньше
string was_Term(vector<Term> allTerms, vector<Transition> curTransition) {
	bool cond = true;
	for (int i = 0; i < allTerms.size(); i++) {
		cond = true;
		if (allTerms[i].Transitions.size() != curTransition.size()) {
			continue;
		}
		for (int j = 0; j < allTerms[i].Transitions.size(); j++) {
			if ((allTerms[i].Transitions[j].first == curTransition[j].first) &&
				(allTerms[i].Transitions[j].second == curTransition[j].second)) {
				//	continue;
			} else {
				cond = false;
			}
		}
		if (cond) {
			return allTerms[i].name;
		}
	}
	return "";
}

TransformationMonoid::TransformationMonoid(){};

TransformationMonoid::TransformationMonoid(FiniteAutomat *in) {
	automat = in;
	for (int i = 1; i <= 3; i++) {
		vector<string> various = get_comb_alphabet(i, automat->get_alphabet());
		for (int j = 0; j < various.size(); j++) //Для	всех	комбинаций
		{
			Term current;
			current.name = various[j];
			for (int t = 0; t < automat->get_states_size(); t++) //Для	всех	состояний
			{
				//	cout	<<	current.name	<<	"	";
				int endsost = t;

				Transition g;
				g.first = endsost;
				bool cond = true;
				for (int k = 0; k < current.name.size(); k++) //Для	каждого	символа	перехода
				{
					State a = automat->get_state(endsost);
					if (a.transitions.count(current.name[k])) {
						vector<int> temp = a.transitions.at(current.name[k]);
						endsost = temp[0];
					} else {
						cond = false;
					}
				}
				g.second = endsost;
				if (cond) {
					current.Transitions.push_back(g);
				}
			}

			string eqv = was_Term(terms, current.Transitions);
			if (eqv == "") //Если	не	встретился	в
						   //Эквивалентных классах
			{
				bool cond = true;
				for (int i = 0; i < current.Transitions.size(); i++) {
					if (!automat->get_state(current.Transitions[i].second).is_terminal) {
						cond = false;
					}
				}
				current.isFinal = cond;
				//	cout	<<	current.name	<<	"	"
				//<<	current.isFinal	<<	"\n";
				terms.push_back(current);
			} else {
				rules[eqv].push_back(current.name);
				//	cout	<<	current.name	<<	"->"	<<
				// eqv	<<	"\n";
			}
		}
	}
}

string TransformationMonoid::to_txt() {
	return automat->to_txt();
}

vector<Term> TransformationMonoid::get_Equalence_Classes() {
	return terms;
}

map<string, vector<string>> TransformationMonoid::get_Rewriting_Rules() {
	return rules;
}

string TransformationMonoid::get_Equalence_Classes_Txt() {
	stringstream ss;
	for (int i = 0; i < terms.size(); i++) {
		ss << "Term	" << terms[i].name << "	in	language	" << terms[i].isFinal << "\n";
		for (int j = 0; j < terms[i].Transitions.size(); j++) {
			ss << terms[i].Transitions[j].first << "	->	" << terms[i].Transitions[j].second << "\n";
		}
	}
	return ss.str();
}

string TransformationMonoid::get_Rewriting_Rules_Txt() {
	stringstream ss;
	for (auto &item : rules) {
		for (int i = 0; i < item.second.size(); i++) {
			ss << item.first << "	->	" << item.second[i] << "\n";
		}
	}
	return ss.str();
}

vector<Term> TransformationMonoid::get_Equalence_Classes_VW(Term w) {
	vector<Term> out;
	for (int i = 0; i < terms.size(); i++) {
		vector<Transition> Transitions;

		for (int j = 0; j < terms[i].Transitions.size(); j++) {
			for (int k = 0; k < w.Transitions.size(); k++) {
				if (terms[i].Transitions[j].second == w.Transitions[k].first) {
					Transition temp;
					temp.first = terms[i].Transitions[j].first;
					temp.second = w.Transitions[k].second;
					Transitions.push_back(temp);
				}
			}
		}
		if (Transitions.size() > 0) {
			bool cond = true;
			for (int j = 0; j < Transitions.size(); j++) {
				if (!automat->get_state(Transitions[j].second).is_terminal) {
					cond = false;
				}
			}
			if (cond) {
				out.push_back(terms[i]);
				cout << terms[i].name << "	";
			}
		}
	}
	return out;
}

vector<Term> TransformationMonoid::get_Equalence_Classes_WV(Term w) {
	vector<Term> out;
	for (int i = 0; i < terms.size(); i++) {
		vector<Transition> Transitions;

		for (int j = 0; j < terms[i].Transitions.size(); j++) {
			for (int k = 0; k < w.Transitions.size(); k++) {
				if (terms[i].Transitions[j].first == w.Transitions[k].second) {
					Transition temp;
					temp.first = w.Transitions[k].first;
					temp.second = terms[i].Transitions[j].second;
					Transitions.push_back(temp);
				}
			}
		}
		if (Transitions.size() > 0) {
			bool cond = true;
			for (int j = 0; j < Transitions.size(); j++) {
				if (!automat->get_state(Transitions[j].second).is_terminal) {
					cond = false;
				}
			}
			if (cond) {
				out.push_back(terms[i]);
				cout << terms[i].name << "	";
			}
		}
	}
	return out;
}

vector<TermDouble> TransformationMonoid::get_Equalence_Classes_VWV(Term w) {
	vector<TermDouble> out;
	for (int i1 = 0; i1 < terms.size(); i1++) {
		for (int i2 = 0; i2 < terms.size(); i2++) {
			vector<Transition> Transitions;
			for (int j1 = 0; j1 < terms[i1].Transitions.size(); j1++) {
				for (int j2 = 0; j2 < terms[i1].Transitions.size(); j2++) {
					for (int k = 0; k < w.Transitions.size(); k++) {
						if ((terms[i1].Transitions[j1].second == w.Transitions[k].first) &&
							(w.Transitions[k].second == terms[i2].Transitions[j2].first)) {
							Transition temp;
							temp.first = terms[i1].Transitions[j1].first;
							temp.second = terms[i2].Transitions[j2].second;
							Transitions.push_back(temp);
						}
					}
				}
			}
			if (Transitions.size() > 0) {
				bool cond = true;
				for (int j = 0; j < Transitions.size(); j++) {
					if (!automat->get_state(Transitions[j].second).is_terminal) {
						cond = false;
					}
				}
				if (cond) {
					TermDouble temp1;
					temp1.first = terms[i1];
					temp1.second = terms[i2];
					out.push_back(temp1);
					cout << temp1.first.name << "	" << temp1.second.name << "\n";
				}
			}
		}
	}
	return out;
}

//Вернет	-1	если	не	синхронизирован	или	номер
//состояния	с	которым синхронизирован
int TransformationMonoid::is_Synchronized(Term w) {
	if (w.Transitions.size() == 0) {
		return -1;
	}
	int sost = w.Transitions[0].second;
	for (int i = 1; i < w.Transitions.size(); i++) {
		if (w.Transitions[i].second != sost) {
			return -1;
		}
	}
	return sost;
}