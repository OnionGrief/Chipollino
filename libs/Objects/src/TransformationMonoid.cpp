#include "Objects/TransformationMonoid.h"
#include "Objects/FiniteAutomaton.h"
#include "Objects/Language.h"
#include <algorithm>
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

// получаем	все	перестановки	алфавита	длины	len
vector<vector<alphabet_symbol>> get_comb_alphabet(
	int len, const set<alphabet_symbol>& alphabet) {

	vector<vector<alphabet_symbol>> newcomb;
	if (len == 0) {
		return newcomb;
	}
	for (alphabet_symbol as : alphabet) {
		vector<alphabet_symbol> new_symbol;
		new_symbol.push_back(as);
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

// Проверяем	встречался	ли	терм	раньше
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

// переписывание терма
vector<alphabet_symbol> TransformationMonoid::rewriting(
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
			break;
		}
	}
	if (!not_rewrite) {
		for (int i = counter; i < in.size(); i++) {
			out.push_back(in[i]);
		}
		out1 = rewriting(out, rules);
		return out1;
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
bool wasrewrite(vector<alphabet_symbol> a, vector<alphabet_symbol> b) {
	for (int i = 0; i < a.size(); i++) {
		for (int j = 0; i + j < a.size() && j < b.size(); j++) {
			if (b[j] != a[i + j]) {
				break;
			}
			if (j == b.size() - 1) {
				return true;
			}
		}
	}
	return false;
}
bool TransformationMonoid::searchrewrite(vector<alphabet_symbol> in) {
	for (auto currules : rules) {
		for (vector<alphabet_symbol> rule : currules.second) {
			if (wasrewrite(in, rule)) {
				return 1;
			}
		}
	}
	return 0;
}
set<int> TransformationMonoid::search_transition_by_word(
	vector<alphabet_symbol> word, int init_state) {
	set<int> out = {};
	if (word.size() <= 0) {
		return {init_state};
	}
	alphabet_symbol symbol_transition = word[0];
	word.erase(word.begin());
	for (auto temp :
		 automat.states[init_state].transitions[symbol_transition]) {
		set<int> res = search_transition_by_word(word, temp);
		out.insert(res.begin(), res.end());
	}
	return out;
}
vector<int> addtransition(vector<int> in, int from, int to, int size) {
	// добавляем переход
	int curtransition = from * size + to;
	in[curtransition / size] = in[curtransition / size] | curtransition % size;
	return in;
}
vector<int> showtransitions(vector<int> in, int from, int to, int size) {
	// добавляем переход
	int curtransition = from * size + to;
	in[curtransition / size] = in[curtransition / size] | curtransition % size;
	return in;
}
// Получаем ДКА и строим моноид
void TransformationMonoid::get_transition_by_symbol(
	vector<TransformationMonoid::Transition> in, vector<alphabet_symbol> word,
	const set<alphabet_symbol>& alphabet) {
	for (const alphabet_symbol& as : alphabet) { // для каждого символа
		set<TransformationMonoid::Transition> out;
		for (TransformationMonoid::Transition temp : in) {
			set<int> tostate = automat.states[temp.second].transitions[as];
			for (int outstate : tostate) {
				TransformationMonoid::Transition curtransition;
				curtransition.first = temp.first;
				curtransition.second = outstate;
				out.insert(curtransition);
			}
		}
		// получили новые переходы
		Term curTerm;

		std::vector<TransformationMonoid::Transition> v(out.size());
		std::copy(out.begin(), out.end(), v.begin());
		curTerm.transitions = v;
		vector<alphabet_symbol> tempword = word;
		tempword.push_back(*it);
		curTerm.name = tempword;
		queueTerm.push(curTerm);
	}
}

TransformationMonoid::TransformationMonoid(const FiniteAutomaton& in) {
	int states_counter_old = 0;
	int states_counter_new = 0;
	states_counter_old = in.states.size();

	automat = in.remove_trap_states();
	states_counter_new = automat.states.size();
	if (states_counter_old - states_counter_new > 1) {
		trap_not_minimal = true;
		states_counter_old = states_counter_new;
	}

	automat.remove_unreachable_states();
	states_counter_new = automat.states.size();
	if (states_counter_old - states_counter_new > 1) {
		trap_not_minimal = true;
	}
	// cout << automat.to_txt();
	vector<TransformationMonoid::Transition> initperehods;
	for (int i = 0; i < automat.states.size(); i++) {
		TransformationMonoid::Transition temp;
		temp.first = i;
		temp.second = i;
		initperehods.push_back(temp);
	}

	get_transition_by_symbol(initperehods, {},
							 automat.language->get_alphabet());
	while (queueTerm.size() > 0) {
		TransformationMonoid::Term cur = queueTerm.front();
		queueTerm.pop();
		// cout << "\n " << rules.size();
		// cout << "\nnew ";
		// for (auto w : cur.name) {
		// 	cout << w;
		// }
		// cout << "\n";

		// for (auto temp : cur.transitions) {
		// 	cout << "(" << temp.first << " " << temp.second << ")";
		// }
		// cout << "\n";
		if (!searchrewrite(cur.name)) {
			std::vector<TransformationMonoid::Term>::iterator rewritein =
				std::find(terms.begin(), terms.end(), cur);
			if (rewritein != terms.end()) {
				// cout << "\trewrite ";
				rules[(*rewritein).name].push_back(cur.name);

			} else {
				for (int i = 0; i < cur.transitions.size(); i++) {
					if (automat.states[cur.transitions[i].second].is_terminal &&
						cur.transitions[i].first == automat.initial_state) {
						cur.isFinal = true;
					}
				}

				// cout << "\tnewterm ";
				terms.push_back(cur);
				get_transition_by_symbol(cur.transitions, cur.name,
										 automat.language->get_alphabet());
			}
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

		Logger::log("class " + alphabet_symbol::vector_to_str(terms[i].name),
					terms[i].isFinal ? "in lang" : "not in lang");
		ss << "Term	" << alphabet_symbol::vector_to_str(terms[i].name)
		   << "	in	language	" << terms[i].isFinal << "\n";
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

map<string, vector<string>> TransformationMonoid::get_equalence_classes_map() {
	map<string, vector<string>> ss;
	Logger::init_step("Equivalence classes");
	for (int i = 0; i < terms.size(); i++) {

		// Logger::log("class " + to_str(terms[i].name),
		//			terms[i].isFinal ? "in lang" : "not in lang");
		// ss << "Term	" << to_str(terms[i].name) << "	in	language	"
		//   << terms[i].isFinal << "\n";
		string term = alphabet_symbol::vector_to_str(terms[i].name);
		for (int j = 0; j < terms[i].transitions.size(); j++) {
			// Logger::log(
			//	"transition " +
			//		automat.states[terms[i].transitions[j].first].identifier,
			//	automat.states[terms[i].transitions[j].second].identifier);
			// ss <<
			// automat.states[terms[i].transitions[j].first].identifier
			//   << "	->	"
			//   <<
			//   automat.states[terms[i].transitions[j].second].identifier
			//   << "\n";
			ss[term].push_back(
				automat.states[terms[i].transitions[j].first].identifier);
			ss[term].push_back(
				automat.states[terms[i].transitions[j].second].identifier);
		}
	}
	// Logger::finish_step();
	return ss;
}

string TransformationMonoid::get_rewriting_rules_txt() {
	stringstream ss;
	Logger::init_step("Rewriting Rules");
	for (auto& item : rules) {
		for (int i = 0; i < item.second.size(); i++) {
			Logger::log("rewriting " +
							alphabet_symbol::vector_to_str(item.second[i]),
						alphabet_symbol::vector_to_str(item.first));
			ss << alphabet_symbol::vector_to_str(item.second[i]) << "	->	"
			   << alphabet_symbol::vector_to_str(item.first) << "\n";
		}
	}
	Logger::finish_step();
	return ss.str();
}

vector<TransformationMonoid::Term> TransformationMonoid::
	get_equalence_classes_vw(const Term& w) {
	vector<Term> out;
	for (int i = 0; i < terms.size(); i++) {
		// vector<Transition> transitions;

		set<TransformationMonoid::Transition> transitions;
		for (int j = 0; j < terms[i].transitions.size(); j++) {
			for (int k = 0; k < w.transitions.size(); k++) {
				if (terms[i].transitions[j].second == w.transitions[k].first) {
					Transition new_transition;
					new_transition.second = w.transitions[k].second;
					new_transition.first = terms[i].transitions[j].first;
					transitions.insert(new_transition);
				}
			}
		}
		if (transitions.size() > 0) {
			for (TransformationMonoid::Transition tr : transitions) {
				if (automat.states[(tr).second].is_terminal &&
					(tr).first == automat.initial_state) {
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
		// vector<Transition> transitions;

		set<TransformationMonoid::Transition> transitions;
		for (int j = 0; j < terms[i].transitions.size(); j++) {
			for (int k = 0; k < w.transitions.size(); k++) {
				if (terms[i].transitions[j].first == w.transitions[k].second) {
					Transition new_transition;
					new_transition.first = w.transitions[k].first;
					new_transition.second = terms[i].transitions[j].second;
					transitions.insert(new_transition);
				}
			}
		}
		if (transitions.size() > 0) {
			for (TransformationMonoid::Transition tr : transitions) {
				if (automat.states[(tr).second].is_terminal &&
					(tr).first == automat.initial_state) {
					out.push_back(terms[i]);
				}
			}
		}
	}
	return out;
}
bool wasTransition(set<TransformationMonoid::Transition> mas,
				   TransformationMonoid::Transition b) {
	for (TransformationMonoid::Transition maselem : mas) {
		if (((maselem).first == b.first) && ((maselem).second == b.second)) {
			return true;
		}
	}
	return false;
}
void TransformationMonoid::OutAllTransformationMonoid() {
	cout << "Equivalence classes:\n";
	cout << get_equalence_classes_txt();
	cout << "Rewriting rules:\n";
	cout << get_rewriting_rules_txt();
	cout << "Information for class w:\n";

	for (int i = 0; i < terms.size(); i++) {
		cout << "  class " << alphabet_symbol::vector_to_str(terms[i].name)
			 << "\n";
		vector<TransformationMonoid::Term> vw =
			get_equalence_classes_vw(terms[i]);
		cout << "\t equivalence classes v such that  accepts vw: ";
		for (TransformationMonoid::Term CurTerm : vw) {
			cout << alphabet_symbol::vector_to_str(CurTerm.name) << ", ";
		}
		cout << "\n";
		vector<TransformationMonoid::Term> wv =
			get_equalence_classes_wv(terms[i]);
		cout << "\t equivalence classes v such that  accepts wv: ";
		for (TransformationMonoid::Term CurTerm : wv) {
			cout << alphabet_symbol::vector_to_str(CurTerm.name) << ", ";
		}
		cout << "\n";
		vector<TransformationMonoid::TermDouble> vwv =
			get_equalence_classes_vwv(terms[i]);
		cout << "\t equivalence classes v such that  accepts wv: ";
		for (TransformationMonoid::TermDouble CurTerm : vwv) {
			cout << alphabet_symbol::vector_to_str(CurTerm.first.name) << " - "
				 << alphabet_symbol::vector_to_str(CurTerm.second.name) << ", ";
		}
		cout << "\n";
		int sync = is_synchronized(terms[i]);
		if (sync == -1) {
			cout << "word not synchronizing\n";
		} else {
			cout << "word synchronizing " << automat.states[sync].identifier
				 << "\n";
		}
	}
	cout << "isminimal " << is_minimal() << "\n";
	cout << to_txt_MyhillNerode();
}
vector<TransformationMonoid::TermDouble> TransformationMonoid::
	get_equalence_classes_vwv(const Term& w) {
	vector<TermDouble> out;
	for (int i1 = 0; i1 < terms.size(); i1++) {
		for (int i2 = 0; i2 < terms.size(); i2++) {
			// vector<Transition> transitions;
			set<TransformationMonoid::Transition> transitions;
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
								transitions.insert(new_transition);
							}
						}
					}
				}
			}
			if (transitions.size() > 0) {
				for (TransformationMonoid::Transition tr : transitions) {
					if (automat.states[(tr).second].is_terminal &&
						(tr).first == automat.initial_state) {
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

// Вернет	-1	если	не	синхронизирован	или	номер
// состояния	с	которым синхронизирован
int TransformationMonoid::is_synchronized(const Term& w) {
	Logger::init_step("Is synchronized word?");
	Logger::log("word " + alphabet_symbol::vector_to_str(w.name));

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

// Вернет число классов эквивалентности
int TransformationMonoid::class_card() {
	Logger::init_step("Number of equivalence classes");
	Logger::log("Number of equivalence classes ", to_string(terms.size()));
	Logger::finish_step();
	return terms.size();
}

// Вернет самое длинное слово в классе
int TransformationMonoid::class_length() {
	Logger::init_step("Longest word in the class");
	Logger::log("Size", to_string(terms[terms.size() - 1].name.size()));
	Logger::log("One of the longest words",
				alphabet_symbol::vector_to_str(terms[terms.size() - 1].name));
	Logger::finish_step();
	return terms[terms.size() - 1].name.size();
}

// Вычисление
int TransformationMonoid::get_classes_number_MyhillNerode() {
	if (equivalence_classes_table_bool.size() == 0) {
		is_minimal();
	}
	Logger::init_step("Myhill-Nerode сlasses number");
	Logger::log(to_string(equivalence_classes_table_bool.size()));
	Logger::finish_step();
	return equivalence_classes_table_bool.size();
}

// Вычисление Минимальности (1 если минимальный)
bool TransformationMonoid::is_minimal() {
	// временные данные
	vector<Term> table_classes;
	vector<vector<bool>> equivalence_classes_table_temp;
	if (trap_not_minimal) {
		return false;
	}
	if (equivalence_classes_table_bool.size() == 0) {
		map<vector<alphabet_symbol>, int>
			data; // храним ссылку на Терм (быстрее и проще искать)
		for (int i = 0; i < terms.size(); i++) {
			data[terms[i].name] = i;
		}
		int sizetable = 0;
		set<int> templeft;
		for (int i = 0; i < terms.size(); i++) {
			if (terms[i].isFinal) {
				templeft.insert(data[terms[i].name]);
			}
			vector<Term> cur = get_equalence_classes_vw(terms[i]);
			for (int j = 0; j < cur.size(); j++) {
				templeft.insert(data[cur[j].name]);
				// table_classes.insert(cur[j]);
			}
		}
		for (auto i : templeft) {
			table_classes.push_back(terms[i]);
		}
		map<vector<alphabet_symbol>, int>
			data_table; // храним ссылку на Терм из таблицы М-Н (быстрее и
						// проще искать)
		for (int i = 0; i < table_classes.size(); i++) {
			data_table[table_classes[i].name] = i;
		}
		for (int i = 0; i <= table_classes.size();
			 i++) { // заполняем матрицу нулями
			vector<bool> vector_first(terms.size() + 1);
			equivalence_classes_table_temp.push_back(vector_first);
		}

		// заполняем с eps
		if (automat.states[automat.initial_state].is_terminal) {
			equivalence_classes_table_temp[0][0] = true;
		}
		int i = 1;
		for (Term t : table_classes) {
			if ((t).isFinal) {
				equivalence_classes_table_temp[i][0] = true;
			}
			i++;
		}
		for (int i = 0; i < terms.size(); i++) {
			if (terms[i].isFinal) {
				equivalence_classes_table_temp[0][i + 1] = true;
			}
		}
		for (int i = 0; i < terms.size(); i++) {
			vector<Term> cur = get_equalence_classes_vw(terms[i]);
			for (int j = 0; j < cur.size(); j++) {
				equivalence_classes_table_temp[data_table.at(cur[j].name) + 1]
											  [i + 1] = true;
			}
		}

		map<vector<bool>, bool> wasvec;
		for (int i = 0; i < equivalence_classes_table_temp.size(); i++) {
			if (!wasvec.count(equivalence_classes_table_temp[i])) {
				wasvec[equivalence_classes_table_temp[i]] = true;
				equivalence_classes_table_bool.push_back(
					equivalence_classes_table_temp[i]);
				if (i == 0) {
					equivalence_classes_table_left.push_back(" ");
				} else {
					equivalence_classes_table_left.push_back(
						alphabet_symbol::vector_to_str(
							table_classes[i - 1].name));
				}
			}
		}
		equivalence_classes_table_top.push_back(" ");
		for (int i = 0; i < terms.size(); i++) {
			equivalence_classes_table_top.push_back(
				alphabet_symbol::vector_to_str(terms[i].name));
		}
		// проходим по таблице и удаляем одинаковые столбцы
		vector<int> delete_column_index;
		set<vector<bool>> for_find_same_column;
		for (int j = 0; j < equivalence_classes_table_bool[0].size(); j++) {
			vector<bool> temp;
			int size_set = for_find_same_column.size();
			for (int i = 0; i < equivalence_classes_table_bool.size(); i++) {
				temp.push_back(equivalence_classes_table_bool[i][j]);
			}
			for_find_same_column.insert(temp);
			if (size_set == for_find_same_column.size()) {
				delete_column_index.push_back(j);
			}
		}
		for (int i = delete_column_index.size() - 1; i >= 0; i--) {
			equivalence_classes_table_top.erase(
				equivalence_classes_table_top.begin() + delete_column_index[i]);
			for (int j = 0; j < equivalence_classes_table_bool.size(); j++) {
				equivalence_classes_table_bool[j].erase(
					equivalence_classes_table_bool[j].begin() +
					delete_column_index[i]);
			}
		}
	}
	// не уверен что правильно
	bool is_minimal_bool = (log2(automat.states.size()) + 1) <=
						   equivalence_classes_table_bool.size();
	Logger::init_step("Is minimal");
	Logger::log(is_minimal_bool ? "true" : "false");
	Logger::finish_step();
	return is_minimal_bool;
}

string TransformationMonoid::to_txt_MyhillNerode() {
	if (equivalence_classes_table_bool.size() == 0) {
		is_minimal();
	}
	stringstream ss;
	int maxlen = terms[terms.size() - 1].name.size();
	ss << string(maxlen + 2, ' ');
	for (int i = 0; i < equivalence_classes_table_top.size(); i++) {
		ss << equivalence_classes_table_top[i]
		   << string(maxlen + 2 - equivalence_classes_table_top[i].size(), ' ');
	}
	ss << "\n";

	for (int i = 0; i < equivalence_classes_table_left.size(); i++) {
		ss << equivalence_classes_table_left[i]
		   << string(maxlen + 2 - equivalence_classes_table_left[i].size(),
					 ' ');
		for (int j = 0; j < equivalence_classes_table_bool[i].size();
			 j++) { // вывод матрицы
			ss << equivalence_classes_table_bool[i][j]
			   << string(maxlen + 1, ' ');
		}
		ss << "\n";
	}

	Logger::init_step("MyhillNerode TABLE");
	Logger::log(ss.str());
	Logger::finish_step();
	return ss.str();
}

vector<vector<bool>> TransformationMonoid::get_equivalence_classes_table(
	vector<string>& table_rows, vector<string>& table_columns) {
	table_rows = equivalence_classes_table_left;
	table_columns = equivalence_classes_table_top;
	return equivalence_classes_table_bool;
}

// В психиатрической больнице люди по настоящему заботятся о своём здоровье.
// Они переходят с электронных сигарет на воображаемые.
