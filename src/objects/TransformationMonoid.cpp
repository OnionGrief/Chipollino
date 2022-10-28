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
				(allTerms[i].Transitions[j].second ==
				 curTransition[j].second)) {
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

//переписывание терма
string rewriting(string in, map<string, vector<string>> rules) {
	if (in.size() < 3) {
		return in;
	}
	string out = "";
	bool cond = true;
	for (int k = 2; cond && (k < in.size()); k++) {
		for (int i = 0; cond && (i < in.size() - k + 1); i++) {
			string temp = "";
			for (int y = 0; y < k; y++) {
				temp += in[i + y];
			}
			cout << temp << " ";
			if ((rules.count(temp)) && (rules.at(temp)[0] != temp)) {
				cond = false;
				cout << "rulesat " << rules.at(temp)[0] << "\n";
				// out += rules.at(temp)[0];
			}
		}
	}

	if (cond) {
		out = in;
	} else {
		out = rewriting(out, rules);
	}
	return "";
}

TransformationMonoid::TransformationMonoid(FiniteAutomaton* in,
										   int transferlen) {
	automat = in;
	for (int i = 1; i <= transferlen; i++) {
		vector<string> various = get_comb_alphabet(i, in->get_state[0]._;
		for (int j = 0; j < various.size(); j++) //Для	всех	комбинаций
		{
			Term current;
			cout << various[j] << " 1 \n";
			current.name = rewriting(various[j], rules);

			cout << current.name << " 2 \n";

			//вставить переписывание слова

			for (int t = 0; t < automat->get_states_size();
				 t++) //Для	всех	состояний
			{
				//	cout	<<	current.name	<<	"	";
				int endsost = t;

				Transition g;
				g.first = endsost;
				bool cond = true;
				for (int k = 0; k < current.name.size();
					 k++) //Для	каждого	символа	перехода
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
				if (current.Transitions.size() != automat->get_states_size()) {
					cond = false;
				}
				for (int i = 0; i < current.Transitions.size(); i++) {
					if (!automat->get_state(current.Transitions[i].second)
							 .is_terminal) {
						cond = false;
					}
				}
				current.isFinal = cond;
				//	cout	<<	current.name	<<	"	"
				//<<	current.isFinal	<<	"\n";
				terms.push_back(current);
			} else {
				if (!rules.count(current.name) && current.name != eqv) {
					cout << current.name << " dad\n";
					rules[current.name].push_back(eqv);
				}
				//	cout	<<	current.name	<<	"->"	<<
				// eqv	<<	"\n";
			}
		}
	}
}

string TransformationMonoid::to_txt() const {
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
		ss << "Term	" << terms[i].name << "	in	language	"
		   << terms[i].isFinal << "\n";
		for (int j = 0; j < terms[i].Transitions.size(); j++) {
			ss << terms[i].Transitions[j].first << "	->	"
			   << terms[i].Transitions[j].second << "\n";
		}
	}
	return ss.str();
}

string TransformationMonoid::get_Rewriting_Rules_Txt() {
	stringstream ss;
	for (auto& item : rules) {
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
			if (Transitions.size() != automat->get_states_size()) {
				cond = false;
			} else {
				for (int j = 0; j < Transitions.size(); j++) {
					// cout << "\n t " << Transitions[j].first << " " <<
					// Transitions[j].second << "\n";
					if (!automat->get_state(Transitions[j].second)
							 .is_terminal) {
						cond = false;
					}
				}
			}
			if (cond) {
				out.push_back(terms[i]);
				// cout << terms[i].name << "	";
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
			if (Transitions.size() != automat->get_states_size()) {
				cond = false;
			} else {
				for (int j = 0; j < Transitions.size(); j++) {
					if (!automat->get_state(Transitions[j].second)
							 .is_terminal) {
						cond = false;
					}
				}
			}
			if (cond) {
				out.push_back(terms[i]);
				// cout << terms[i].name << "	";
			}
		}
	}
	return out;
}
bool wasTransition(vector<Transition> mas, Transition b) {
	for (int i = 0; i < mas.size(); i++) {
		if ((mas[i].first == b.first) && (mas[i].second == b.second)) {
			return true;
		}
	}
	return false;
}

vector<TermDouble> TransformationMonoid::get_Equalence_Classes_VWV(Term w) {
	vector<TermDouble> out;
	for (int i1 = 0; i1 < terms.size(); i1++) {
		for (int i2 = 0; i2 < terms.size(); i2++) {
			vector<Transition> Transitions;
			for (int j1 = 0; j1 < terms[i1].Transitions.size(); j1++) {
				for (int j2 = 0; j2 < terms[i1].Transitions.size(); j2++) {
					for (int k = 0; k < w.Transitions.size(); k++) {
						if ((terms[i1].Transitions[j1].second ==
							 w.Transitions[k].first) &&
							(w.Transitions[k].second ==
							 terms[i2].Transitions[j2].first)) {
							Transition temp;
							temp.first = terms[i1].Transitions[j1].first;
							temp.second = terms[i2].Transitions[j2].second;
							if (!wasTransition(Transitions, temp)) {
								Transitions.push_back(temp);
							}
						}
					}
				}
			}
			if (Transitions.size() > 0) {
				bool cond = true;
				if (Transitions.size() != automat->get_states_size()) {
					cond = false;
				} else {
					for (int j = 0; j < Transitions.size(); j++) {
						// cout << "\n t " << Transitions[j].first << " " <<
						// Transitions[j].second << "\n";
						if (!automat->get_state(Transitions[j].second)
								 .is_terminal) {
							cond = false;
						}
					}
				}
				if (cond) {
					TermDouble temp1;
					temp1.first = terms[i1];
					temp1.second = terms[i2];
					out.push_back(temp1);
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

//Вернет число классов эквивалентности
int TransformationMonoid::class_Card() {
	return terms.size();
}

//Вернет самое длинное слово в классе
Term TransformationMonoid::Class_Length() {
	return terms[terms.size() - 1];
}

//Вычисление
int TransformationMonoid::size_MyhillNerode() {
	int sum = 0;
	for (int i = 0; i < equivalence_class_table.size(); i++) {
		for (int j = 0; j < equivalence_class_table[i].size(); j++) {
			if (equivalence_class_table[i][j]) {
				sum++;
			}
		}
	}
	return sum;
}

//Вычисление Минимальности (1 если минимальный)
bool TransformationMonoid::is_minimality() {
	map<string, int> data; //храним ссылку на Терм (быстрее и проще искать)
	for (int i = 0; i < terms.size(); i++) {
		data[terms[i].name] = i;
	}
	for (int i = 0; i <= terms.size(); i++) { //заполняем матрицу нулями
		vector<bool> vector_first(terms.size() + 1);
		equivalence_class_table.push_back(vector_first);
	}
	for (int i = 0; i < terms.size(); i++) {
		if (terms[i].isFinal) {
			equivalence_class_table[0][i + 1] = true;
		}
	}
	for (int i = 0; i < terms.size(); i++) {
		vector<Term> cur = this->get_Equalence_Classes_VW(terms[i]);
		for (int j = 0; j < cur.size(); j++) {
			equivalence_class_table[i + 1][data.at(cur[j].name) + 1] = true;
		}
	}
	map<vector<bool>, bool> wasvec;
	int counter = 0;
	for (int i = 0; i < equivalence_class_table.size(); i++) {
		if (!wasvec.count(equivalence_class_table[i])) {
			wasvec[equivalence_class_table[i]] = true;
			counter++;
		}
	}
	return (log2(terms.size()) + 1) <= counter;
}

string TransformationMonoid::to_Txt_MyhillNerode() {

	stringstream ss;
	ss << "    e   ";
	for (int i = 0; i < terms.size(); i++) {
		ss << terms[i].name << string(4 - terms[i].name.size(), ' ');
	}
	ss << "\n";
	for (int i = 0; i < equivalence_class_table.size(); i++) { //вывод матрицы
		if (i == 0) {
			ss << "e   ";
		} else {
			ss << terms[i - 1].name
			   << string(4 - terms[i - 1].name.size(), ' ');
		}
		for (int j = 0; j < equivalence_class_table[0].size();
			 j++) { //вывод матрицы
			ss << equivalence_class_table[j][i] << "   ";
		}
		ss << "\n";
	}

	return ss.str();
}
//В психиатрической больнице люди по настоящему заботятся о своём здоровье. Они
//переходят с электронных сигарет на воображаемые.