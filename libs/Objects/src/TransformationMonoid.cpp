#include "Objects/TransformationMonoid.h"
#include "Objects/FiniteAutomaton.h"
#include "Objects/Language.h"

#include <iostream>
using namespace std;
vector<alphabet_symbol> union_words(vector<alphabet_symbol> a,
									vector<alphabet_symbol> b) {
	vector<alphabet_symbol> newword;
	for (int i = 0; i < a.size(); i++) {
		newword.push_back(a[i]);
	}
	for (int i = 0; i < b.size(); i++) {
		newword.push_back(b[i]);
	}
	return newword;
}

// vector<string> to string
string to_str(vector<alphabet_symbol> in) {
	string out = "";
	for (int i = 0; i < in.size(); i++) {
		out += in[i];
	}
	return out;
}
//получаем	все	перестановки	алфавита	длины	len
vector<vector<alphabet_symbol>> get_comb_alphabet(
	int len, const set<alphabet_symbol>& alphabet) {

	vector<vector<alphabet_symbol>> newcomb;
	if (len == 0) {
		return newcomb;
	}
	for (set<alphabet_symbol>::iterator it = alphabet.begin();
		 it != alphabet.end(); it++) {
		vector<alphabet_symbol> new_symbol;
		new_symbol.push_back(*it);
		newcomb.push_back(new_symbol);
	}
	if (len == 1) {
		return newcomb;
	}
	vector<vector<alphabet_symbol>> comb;
	vector<vector<alphabet_symbol>> oldcomb =
		get_comb_alphabet(len - 1, alphabet);
	for (int i = 0; i < newcomb.size(); i++) {
		for (int j = 0; j < oldcomb.size(); j++) {
			comb.push_back(union_words(newcomb[i], oldcomb[j]));
		}
	}
	return comb;
}

//Проверяем	встречался	ли	терм	раньше
vector<alphabet_symbol> was_term(
	vector<TransformationMonoid::Term> all_terms,
	vector<TransformationMonoid::Transition> cur_transition) {
	bool met_term = true;
	for (int i = 0; i < all_terms.size(); i++) {
		met_term = true;
		if (all_terms[i].transitions.size() != cur_transition.size()) {
			continue;
		}
		for (int j = 0; j < all_terms[i].transitions.size(); j++) {
			if ((all_terms[i].transitions[j].first ==
				 cur_transition[j].first) &&
				(all_terms[i].transitions[j].second ==
				 cur_transition[j].second)) {
				//	continue;
			} else {
				met_term = false;
			}
		}
		if (met_term) {
			return all_terms[i].name;
		}
	}
	return {};
}

TransformationMonoid::TransformationMonoid(){};

//переписывание терма
vector<alphabet_symbol> rewriting(
	vector<alphabet_symbol> in,
	map<vector<alphabet_symbol>, vector<vector<alphabet_symbol>>> rules) {
	if (in.size() < 2) {
		return in;
	}
	vector<alphabet_symbol> out;
	vector<alphabet_symbol> out1;
	bool not_rewrite = true;
	int counter = 0;
	for (int k = 2; not_rewrite && (k <= in.size()); k++) {
		vector<alphabet_symbol> new_symbol;
		for (int y = 0; y < k; y++) {
			new_symbol.push_back(in[y]);
		}
		if ((rules.count(new_symbol)) &&
			(rules.at(new_symbol)[0] != new_symbol)) {
			for (int y = 0; y < rules.at(new_symbol)[0].size(); y++) {
				out.push_back(rules.at(new_symbol)[0][y]);
			}
			counter = k;
			not_rewrite = false;
		}
	}
	if (!not_rewrite) {
		vector<alphabet_symbol> rec_in = {in.begin() + counter, in.end()};
		out1 = rewriting(rec_in, rules);
		for (int y = 0; y < out1.size(); y++) {
			out.push_back(out1[y]);
		}
		return out;
	} else {
		vector<alphabet_symbol> rec_in = {in.begin() + 1, in.end()};
		out.push_back(in[0]);
		out1 = rewriting(rec_in, rules);
		for (int y = 0; y < out1.size(); y++) {
			out.push_back(out1[y]);
		}
		return out;
	}
	return in;
}

//Получаем ДКА и строим моноид
TransformationMonoid::TransformationMonoid(const FiniteAutomaton& in) {
	Logger::activate_step_counter();
	automat = in.remove_trap_states();
	automat.remove_unreachable_states();
	Logger::deactivate_step_counter();
	int i = 0;
	bool cond_get_transactions = true;
	while (cond_get_transactions) {
		i++;
		vector<vector<alphabet_symbol>> various =
			get_comb_alphabet(i, automat.language->get_alphabet());
		bool cond_rule_len = true;
		for (int j = 0; j < various.size(); j++) //Для	всех	комбинаций
		{
			Term current;
			current.name = various[j];
			current.name = rewriting(various[j], rules);
			if (current.name.size() != i) {
				cond_rule_len = false;
			}
			for (int t = 0; t < automat.states.size(); t++) {
				int final_state = -1;
				Transition g;
				g.first = t;
				bool not_final_state = true;
				for (int k = 0; k < current.name.size();
					 k++) //Для	каждого	символа	перехода
				{
					State a;
					if (final_state == -1) {
						a = automat.states[g.first];
					} else {
						a = automat.states[final_state];
					}
					if (a.transitions.count(current.name[k])) {
						set<int> temp_transitions =
							a.transitions.at(current.name[k]);
						if (temp_transitions.size()) {
							final_state = *temp_transitions.begin();
						} else {
							not_final_state = false;
						}
					} else {
						not_final_state = false;
					}
				}
				if (final_state != -1) {
					g.second = final_state;
					if (not_final_state) {
						current.transitions.push_back(g);
					}
				}
			}
			vector<alphabet_symbol> eqv = was_term(terms, current.transitions);
			if (eqv.size() == 0) //Если	не	встретился	в
								 //Эквивалентных классах
			{
				for (int i = 0; i < current.transitions.size(); i++) {
					if (automat.states[current.transitions[i].second]
							.is_terminal &&
						current.transitions[i].first == automat.initial_state) {
						current.isFinal = true;
					}
				}
				terms.push_back(current);
			} else {
				if (!rules.count(current.name) && current.name != eqv) {
					rules[current.name].push_back(eqv);
				}
			}
		}
		if (!cond_rule_len) {
			cond_get_transactions = false;
		}
	}
}

string TransformationMonoid::to_txt() const {
	return automat.to_txt();
}

vector<TransformationMonoid::Term> TransformationMonoid::
	get_equalence_classes() {
	return terms;
}

map<vector<alphabet_symbol>, vector<vector<alphabet_symbol>>>
TransformationMonoid::get_rewriting_rules() {
	return rules;
}

string TransformationMonoid::get_equalence_classes_txt() {
	stringstream ss;
	Logger::init_step("Equivalence classes");
	for (int i = 0; i < terms.size(); i++) {

		Logger::log("class " + to_str(terms[i].name),
					terms[i].isFinal ? "in lang" : "not in lang");
		ss << "Term	" << to_str(terms[i].name) << "	in	language	"
		   << terms[i].isFinal << "\n";
		for (int j = 0; j < terms[i].transitions.size(); j++) {
			Logger::log(
				"transition " +
					automat.states[terms[i].transitions[j].first].identifier,
				automat.states[terms[i].transitions[j].second].identifier);
			ss << automat.states[terms[i].transitions[j].first].identifier
			   << "	->	"
			   << automat.states[terms[i].transitions[j].second].identifier
			   << "\n";
		}
	}
	Logger::finish_step();
	return ss.str();
}

string TransformationMonoid::get_rewriting_rules_txt() {
	stringstream ss;
	Logger::init_step("Rewriting Rules");
	for (auto& item : rules) {
		for (int i = 0; i < item.second.size(); i++) {
			Logger::log("rewriting " + to_str(item.first),
						to_str(item.second[i]));
			ss << to_str(item.first) << "	->	" << to_str(item.second[i])
			   << "\n";
		}
	}
	Logger::finish_step();
	return ss.str();
}

vector<TransformationMonoid::Term> TransformationMonoid::
	get_equalence_classes_vw(const Term& w) {
	vector<Term> out;
	for (int i = 0; i < terms.size(); i++) {
		vector<Transition> transitions;
		for (int j = 0; j < terms[i].transitions.size(); j++) {
			for (int k = 0; k < w.transitions.size(); k++) {
				if (terms[i].transitions[j].second == w.transitions[k].first) {
					Transition new_transition;
					new_transition.second = w.transitions[k].second;
					new_transition.first = terms[i].transitions[j].first;
					transitions.push_back(new_transition);
				}
			}
		}
		if (transitions.size() > 0) {
			for (int j = 0; j < transitions.size(); j++) {
				if (automat.states[transitions[j].second].is_terminal &&
					transitions[j].first == automat.initial_state) {
					out.push_back(terms[i]);
				}
			}
		}
	}
	return out;
}

vector<TransformationMonoid::Term> TransformationMonoid::
	get_equalence_classes_wv(const Term& w) {
	vector<Term> out;
	for (int i = 0; i < terms.size(); i++) {
		vector<Transition> transitions;
		for (int j = 0; j < terms[i].transitions.size(); j++) {
			for (int k = 0; k < w.transitions.size(); k++) {
				if (terms[i].transitions[j].first == w.transitions[k].second) {
					Transition new_transition;
					new_transition.first = w.transitions[k].first;
					new_transition.second = terms[i].transitions[j].second;
					transitions.push_back(new_transition);
				}
			}
		}
		if (transitions.size() > 0) {
			for (int j = 0; j < transitions.size(); j++) {
				if (automat.states[transitions[j].second].is_terminal &&
					transitions[j].first == automat.initial_state) {
					out.push_back(terms[i]);
				}
			}
		}
	}
	return out;
}
bool wasTransition(vector<TransformationMonoid::Transition> mas,
				   TransformationMonoid::Transition b) {
	for (int i = 0; i < mas.size(); i++) {
		if ((mas[i].first == b.first) && (mas[i].second == b.second)) {
			return true;
		}
	}
	return false;
}

vector<TransformationMonoid::TermDouble> TransformationMonoid::
	get_equalence_classes_vwv(const Term& w) {
	vector<TermDouble> out;
	for (int i1 = 0; i1 < terms.size(); i1++) {
		for (int i2 = 0; i2 < terms.size(); i2++) {
			vector<Transition> transitions;
			for (int j1 = 0; j1 < terms[i1].transitions.size(); j1++) {
				for (int j2 = 0; j2 < terms[i2].transitions.size(); j2++) {
					for (int k = 0; k < w.transitions.size(); k++) {
						if ((terms[i1].transitions[j1].second ==
							 w.transitions[k].first) &&
							(w.transitions[k].second ==
							 terms[i2].transitions[j2].first)) {
							Transition new_transition;
							new_transition.first =
								terms[i1].transitions[j1].first;
							new_transition.second =
								terms[i2].transitions[j2].second;
							if (!wasTransition(transitions, new_transition)) {
								transitions.push_back(new_transition);
							}
						}
					}
				}
			}
			if (transitions.size() > 0) {
				for (int j = 0; j < transitions.size(); j++) {
					if (automat.states[transitions[j].second].is_terminal &&
						transitions[j].first == automat.initial_state) {
						TermDouble new_transition_double;
						new_transition_double.first = terms[i1];
						new_transition_double.second = terms[i2];
						out.push_back(new_transition_double);
					}
				}
			}
		}
	}
	return out;
}

//Вернет	-1	если	не	синхронизирован	или	номер
//состояния	с	которым синхронизирован
int TransformationMonoid::is_synchronized(const Term& w) {
	Logger::init_step("Is synchronized word?");
	Logger::log("word " + to_str(w.name));

	if (w.transitions.size() == 0) {
		Logger::log("not synchronized");
		Logger::finish_step();
		return -1;
	}
	int state = w.transitions[0].second;
	for (int i = 1; i < w.transitions.size(); i++) {
		if (w.transitions[i].second != state) {
			Logger::log("not synchronized");
			Logger::finish_step();
			return -1;
		}
	}
	Logger::log("synchronized");
	Logger::finish_step();
	return state;
}

//Вернет число классов эквивалентности
int TransformationMonoid::class_card() {
	Logger::init_step("Number of equivalence classes");
	Logger::log(to_string(terms.size()));
	Logger::finish_step();
	return terms.size();
}

//Вернет самое длинное слово в классе
int TransformationMonoid::class_length() {
	Logger::init_step("Longest word in the class");
	Logger::log("Size", to_string(terms[terms.size() - 1].name.size()));
	Logger::log("One of the correct words",
				to_str(terms[terms.size() - 1].name));
	Logger::finish_step();
	return terms[terms.size() - 1].name.size();
}

//Вычисление
int TransformationMonoid::get_classes_number_MyhillNerode() {
	int sum = 0;
	for (int i = 0; i < equivalence_classes_table.size(); i++) {
		for (int j = 0; j < equivalence_classes_table[i].size(); j++) {
			if (equivalence_classes_table[i][j]) {
				sum++;
			}
		}
	}
	Logger::init_step("Myhill-Nerode сlasses number");
	Logger::log(to_string(sum));
	Logger::finish_step();
	return sum;
}

//Вычисление Минимальности (1 если минимальный)
bool TransformationMonoid::is_minimal() {
	map<vector<alphabet_symbol>, int>
		data; //храним ссылку на Терм (быстрее и проще искать)
	for (int i = 0; i < terms.size(); i++) {
		data[terms[i].name] = i;
	}
	for (int i = 0; i <= terms.size(); i++) { //заполняем матрицу нулями
		vector<bool> vector_first(terms.size() + 1);
		equivalence_classes_table.push_back(vector_first);
	}
	for (int i = 0; i < terms.size(); i++) {
		if (terms[i].isFinal) {
			equivalence_classes_table[0][i + 1] = true;
		}
	}
	for (int i = 0; i < terms.size(); i++) {
		vector<Term> cur = this->get_equalence_classes_vw(terms[i]);
		for (int j = 0; j < cur.size(); j++) {
			equivalence_classes_table[i + 1][data.at(cur[j].name) + 1] = true;
		}
	}
	map<vector<bool>, bool> wasvec;
	int counter = 0;
	for (int i = 0; i < equivalence_classes_table.size(); i++) {
		if (!wasvec.count(equivalence_classes_table[i])) {
			wasvec[equivalence_classes_table[i]] = true;
			counter++;
		}
	}

	Logger::init_step("Is minimal");

	Logger::log(((log2(terms.size()) + 1) <= counter) ? "True" : "False");
	Logger::finish_step();
	return (log2(terms.size()) + 1) <= counter;
}

string TransformationMonoid::to_txt_MyhillNerode() {
	stringstream ss;
	ss << "    e   ";
	for (int i = 0; i < terms.size(); i++) {
		ss << to_str(terms[i].name) << string(4 - terms[i].name.size(), ' ');
	}
	ss << "\n";
	for (int i = 0; i < equivalence_classes_table.size(); i++) { //вывод матрицы
		if (i == 0) {
			ss << "e   ";
		} else {
			ss << to_str(terms[i - 1].name)
			   << string(4 - terms[i - 1].name.size(), ' ');
		}
		for (int j = 0; j < equivalence_classes_table[0].size();
			 j++) { //вывод матрицы
			ss << equivalence_classes_table[j][i] << "   ";
		}
		ss << "\n";
	}
	Logger::init_step("MyhillNerode TABLE");
	Logger::log(ss.str());
	Logger::finish_step();
	return ss.str();
}

const vector<vector<bool>>& TransformationMonoid::
	get_equivalence_classes_table() {
	return equivalence_classes_table;
}

//В психиатрической больнице люди по настоящему заботятся о своём здоровье. Они
//переходят с электронных сигарет на воображаемые.