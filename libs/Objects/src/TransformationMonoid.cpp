#include <algorithm>
#include <iostream>

#include "Objects/FiniteAutomaton.h"
#include "Objects/Language.h"
#include "Objects/TransformationMonoid.h"

using std::cout;
using std::map;
using std::set;
using std::string;
using std::stringstream;
using std::to_string;
using std::vector;

FA_model::FA_model(int initial_state, vector<FAState> states, std::weak_ptr<Language> language)
	: initial_state(initial_state), states(states), language(language) {}

FiniteAutomaton FA_model::make_fa() {
	return FiniteAutomaton(initial_state, states, language.lock());
}

TransformationMonoid::TransformationMonoid(const FiniteAutomaton& in) {
	int states_counter_old = 0;
	int states_counter_new = 0;
	states_counter_old = in.size(); // для проверки ловушки на минимальность

	FiniteAutomaton temp_fa = in.remove_trap_states(); // удаляем ловушки
	states_counter_new = temp_fa.size();
	if (states_counter_old - states_counter_new > 1) {
		trap_not_minimal = true;
		states_counter_old = states_counter_new;
	}

	temp_fa.remove_unreachable_states(); // удаляем недостижимые ловушки
	states_counter_new = temp_fa.size();
	if (states_counter_old - states_counter_new > 1) {
		trap_not_minimal = true;
	}

	automaton = FA_model(temp_fa.initial_state, temp_fa.states, temp_fa.language);

	// cout << automaton.to_txt();
	vector<TransformationMonoid::Transition>
		init_transitions; // получаем состояния по eps переходу (из себя в себя)
	for (int i = 0; i < automaton.states.size(); i++) {
		TransformationMonoid::Transition temp;
		temp.first = i;
		temp.second = i;
		init_transitions.push_back(temp);
	}
	get_new_transition(init_transitions, {}, automaton.language.lock()->get_alphabet());
	while (!queueTerm.empty()) { // пока есть кандидаты
		TransformationMonoid::Term cur = queueTerm.front();
		queueTerm.pop();
		if (!searchrewrite(cur.name)) { // если не переписывается
			auto rewrite_in = std::find(terms.begin(), terms.end(), cur);

			if (rewrite_in != terms.end()) { // в правила переписывания
				rules[rewrite_in->name].push_back(cur.name);
			} else { // новый терм
				for (const auto& transition : cur.transitions) {
					if (automaton.states[transition.second].is_terminal &&
						transition.first == automaton.initial_state) {
						cur.isFinal = true;
						break;
					}
				}

				terms.push_back(cur);
				get_new_transition(
					cur.transitions, cur.name, automaton.language.lock()->get_alphabet());
			}
		}
	}
}

vector<Symbol> union_words(vector<Symbol> a, vector<Symbol> b) {
	vector<Symbol> newword;
	for (const auto& i : a) {
		newword.push_back(i);
	}
	for (const auto& i : b) {
		newword.push_back(i);
	}
	return newword;
}

// получаем все перестановки алфавита длины len
vector<vector<Symbol>> get_comb_alphabet(int len, const set<Symbol>& alphabet) {

	vector<vector<Symbol>> newcomb;
	if (len == 0) {
		return newcomb;
	}
	for (const Symbol& as : alphabet) {
		vector<Symbol> new_symbol;
		new_symbol.push_back(as);
		newcomb.push_back(new_symbol);
	}
	if (len == 1) {
		return newcomb;
	}
	vector<vector<Symbol>> comb;
	vector<vector<Symbol>> oldcomb = get_comb_alphabet(len - 1, alphabet);
	for (auto& i : newcomb) {
		for (auto& j : oldcomb) {
			comb.push_back(union_words(i, j));
		}
	}
	return comb;
}

vector<Symbol> TransformationMonoid::rewriting(
	const vector<Symbol>& in, const map<vector<Symbol>, vector<vector<Symbol>>>& rules) {
	if (in.size() < 2) {
		return in;
	}
	vector<Symbol> out;
	vector<Symbol> out1;
	bool not_rewrite = true;
	int counter = 0;
	for (int k = 2; not_rewrite && (k <= in.size()); k++) {
		vector<Symbol> new_symbol;
		for (int y = 0; y < k; y++) {
			new_symbol.push_back(in[y]);
		}
		if ((rules.count(new_symbol)) && (rules.at(new_symbol)[0] != new_symbol)) {
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
		vector<Symbol> rec_in = {in.begin() + 1, in.end()};
		out.push_back(in[0]);
		out1 = rewriting(rec_in, rules);
		for (const auto& y : out1) {
			out.push_back(y);
		}
		return out;
	}
	return in;
}

bool TransformationMonoid::was_rewrite(const vector<Symbol>& a, const vector<Symbol>& b) {
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

bool TransformationMonoid::searchrewrite(const vector<Symbol>& in) {
	for (const auto& currules : rules) {
		for (const vector<Symbol>& rule : currules.second) {
			if (was_rewrite(in, rule)) {
				return true;
			}
		}
	}
	return false;
}

void TransformationMonoid::get_new_transition(const vector<TransformationMonoid::Transition>& in,
											  const vector<Symbol>& word,
											  const set<Symbol>& alphabet) {
	for (const Symbol& as : alphabet) { // для каждого символа
		set<TransformationMonoid::Transition> out;
		for (const TransformationMonoid::Transition& temp : in) {
			set<int> tostate =
				automaton.states[temp.second].transitions[as]; // получаем все переходы
			for (int outstate : tostate) { // для каждого перехода
				TransformationMonoid::Transition curtransition;
				curtransition.first = temp.first;
				curtransition.second = outstate;
				out.insert(curtransition);
			}
		}
		// получили новые переходы
		Term curTerm;
		vector<TransformationMonoid::Transition> v(out.size());
		std::copy(out.begin(), out.end(), v.begin());
		curTerm.transitions = v;
		vector<Symbol> tempword = word;
		tempword.push_back(as);
		curTerm.name = tempword;
		queueTerm.push(curTerm);
	}
}


string TransformationMonoid::to_txt() {
	stringstream ss;
	ss << "Equivalence classes:\n";
	ss << get_equalence_classes_txt();
	ss << "Rewriting rules:\n";
	ss << get_rewriting_rules_txt();
	ss << "Information for class w:\n";

	for (auto& term : terms) {
		ss << "  class " << Symbol::vector_to_str(term.name) << "\n";
		vector<TransformationMonoid::Term> vw = get_equalence_classes_vw(term);
		ss << "\t equivalence classes v such that  accepts vw: ";
		for (const TransformationMonoid::Term& CurTerm : vw) {
			ss << Symbol::vector_to_str(CurTerm.name) << ", ";
		}
		ss << "\n";
		vector<TransformationMonoid::Term> wv = get_equalence_classes_wv(term);
		ss << "\t equivalence classes v such that  accepts wv: ";
		for (const TransformationMonoid::Term& CurTerm : wv) {
			ss << Symbol::vector_to_str(CurTerm.name) << ", ";
		}
		ss << "\n";
		vector<TransformationMonoid::TermDouble> vwv = get_equalence_classes_vwv(term);
		ss << "\t equivalence classes v such that  accepts wv: ";
		for (const TransformationMonoid::TermDouble& CurTerm : vwv) {
			ss << Symbol::vector_to_str(CurTerm.first.name) << " - "
			   << Symbol::vector_to_str(CurTerm.second.name) << ", ";
		}
		ss << "\n";
		int sync = is_synchronized(term);
		if (sync == -1) {
			ss << "word not synchronizing\n";
		} else {
			ss << "word synchronizing " << automaton.states[sync].identifier << "\n";
		}
	}
	ss << "isminimal " << is_minimal() << "\n";
	ss << to_txt_MyhillNerode();
	return ss.str();
}

vector<TransformationMonoid::Term> TransformationMonoid::get_equalence_classes() {
	return terms;
}

map<vector<Symbol>, vector<vector<Symbol>>> TransformationMonoid::get_rewriting_rules() {
	return rules;
}

string TransformationMonoid::get_equalence_classes_txt() {
	stringstream ss;
	for (auto& term : terms) {
		ss << "Term	" << Symbol::vector_to_str(term.name) << "	in	language	" << term.isFinal
		   << "\n";
		for (int j = 0; j < term.transitions.size(); j++) {
			ss << automaton.states[term.transitions[j].first].identifier << "	->	"
			   << automaton.states[term.transitions[j].second].identifier << "\n";
		}
	}
	return ss.str();
}

map<string, vector<string>> TransformationMonoid::get_equalence_classes_map() {
	map<string, vector<string>> ss;
	for (auto& i : terms) {
		string term = Symbol::vector_to_str(i.name);
		for (int j = 0; j < i.transitions.size(); j++) {
			ss[term].push_back(automaton.states[i.transitions[j].first].identifier);
			ss[term].push_back(automaton.states[i.transitions[j].second].identifier);
		}
	}
	return ss;
}

string TransformationMonoid::get_rewriting_rules_txt(iLogTemplate* log) {
	stringstream ss;
	for (auto& item : rules) {
		for (int i = 0; i < item.second.size(); i++) {
			ss << Symbol::vector_to_str(item.second[i]) << "	->	"
			   << Symbol::vector_to_str(item.first) << "\n";
		}
	}
	if (log) {
		log->set_parameter("rewriting rules", ss.str());
	}
	return ss.str();
}

vector<TransformationMonoid::Term> TransformationMonoid::get_equalence_classes_vw(const Term& w) {
	vector<Term> out;
	for (auto& term : terms) {
		set<TransformationMonoid::Transition> transitions;
		for (auto& j : term.transitions) {
			for (auto transition : w.transitions) {
				if (j.second == transition.first) {
					Transition new_transition;
					new_transition.second = transition.second;
					new_transition.first = j.first;
					transitions.insert(new_transition);
				}
			}
		}
		if (!transitions.empty()) {
			for (TransformationMonoid::Transition tr : transitions) {
				if (automaton.states[(tr).second].is_terminal &&
					(tr).first == automaton.initial_state) {
					out.push_back(term);
				}
			}
		}
	}
	return out;
}

vector<TransformationMonoid::Term> TransformationMonoid::get_equalence_classes_wv(const Term& w) {
	vector<Term> out;
	for (auto& term : terms) {
		set<TransformationMonoid::Transition> transitions;
		for (auto& j : term.transitions) {
			for (auto transition : w.transitions) {
				if (j.first == transition.second) {
					Transition new_transition;
					new_transition.first = transition.first;
					new_transition.second = j.second;
					transitions.insert(new_transition);
				}
			}
		}
		if (!transitions.empty()) {
			for (const TransformationMonoid::Transition& tr : transitions) {
				if (automaton.states[(tr).second].is_terminal &&
					(tr).first == automaton.initial_state) {
					out.push_back(term);
				}
			}
		}
	}
	return out;
}

bool TransformationMonoid::was_transition(const set<TransformationMonoid::Transition>& mas,
										  const TransformationMonoid::Transition& b) {
	for (const TransformationMonoid::Transition& maselem : mas) { // TODO: переделать плохой нейминг
		if (((maselem).first == b.first) && ((maselem).second == b.second)) {
			return true;
		}
	}
	return false;
}

vector<TransformationMonoid::TermDouble> TransformationMonoid::get_equalence_classes_vwv(
	const Term& w) {
	vector<TermDouble> out;
	for (auto& i1 : terms) {
		for (auto& i2 : terms) {
			// vector<MFATransition> transitions;
			set<TransformationMonoid::Transition> transitions;
			for (auto& j1 : i1.transitions) {
				for (auto& j2 : i2.transitions) {
					for (auto k : w.transitions) {
						if ((j1.second == k.first) && (k.second == j2.first)) {
							Transition new_transition;
							new_transition.first = j1.first;
							new_transition.second = j2.second;
							if (!was_transition(transitions, new_transition)) {
								transitions.insert(new_transition);
							}
						}
					}
				}
			}
			if (!transitions.empty()) {
				for (const TransformationMonoid::Transition& tr : transitions) {
					if (automaton.states[(tr).second].is_terminal &&
						(tr).first == automaton.initial_state) {
						TermDouble new_transition_double;
						new_transition_double.first = i1;
						new_transition_double.second = i2;
						out.push_back(new_transition_double);
					}
				}
			}
		}
	}
	return out;
}

int TransformationMonoid::is_synchronized(const Term& w) {
	if (w.transitions.size() == 0) {
		return -1;
	}
	int state = w.transitions[0].second;
	for (int i = 1; i < w.transitions.size(); i++) {
		if (w.transitions[i].second != state) {
			return -1;
		}
	}
	return state;
}

// Вернет число классов эквивалентности
int TransformationMonoid::class_card(iLogTemplate* log) {
	if (log)
		log->set_parameter("oldautomaton", automaton.make_fa());
	if (log) {
		log->set_parameter("result", to_string(terms.size()));
	}
	return terms.size();
}

// Вернет самое длинное слово в классе
int TransformationMonoid::class_length(iLogTemplate* log) {
	if (log)
		log->set_parameter("oldautomaton", automaton.make_fa());
	if (log) {
		log->set_parameter("result", to_string(terms[terms.size() - 1].name.size()));
		// TODO: logs
		log->set_parameter("One of the longest words",
						   Symbol::vector_to_str(terms[terms.size() - 1].name));
	}
	return terms[terms.size() - 1].name.size();
}

int TransformationMonoid::get_classes_number_MyhillNerode(iLogTemplate* log) {
	if (log)
		log->set_parameter("oldautomaton", automaton.make_fa());
	if (equivalence_classes_table_bool.size() == 0) {
		is_minimal();
	}
	iLogTemplate::Table t;
	t.columns = equivalence_classes_table_top;
	t.rows = equivalence_classes_table_left;
	for (int i = 0; i < equivalence_classes_table_left.size(); i++) {
		for (int j = 0; j < equivalence_classes_table_bool[i].size(); j++) { // вывод матрицы
			t.data.push_back(to_string(equivalence_classes_table_bool[i][j]));
		}
	}
	if (log) {
		/*TODO: logs */
		log->set_parameter("result", static_cast<int>(equivalence_classes_table_bool.size()));
		log->set_parameter("table", t);
	}
	return equivalence_classes_table_bool.size();
}

bool TransformationMonoid::is_minimal(iLogTemplate* log) {
	// временные данные
	vector<Term> table_classes;
	vector<vector<bool>> equivalence_classes_table_temp;
	if (trap_not_minimal) {
		return false;
	}
	if (equivalence_classes_table_bool.empty()) {
		map<vector<Symbol>, int> data; // храним ссылку на Терм (быстрее и проще искать)
		for (int i = 0; i < terms.size(); i++) {
			data[terms[i].name] = i;
		}
		int sizetable = 0;
		set<int> templeft;
		for (auto& term : terms) {
			if (term.isFinal) {
				templeft.insert(data[term.name]);
			}
			vector<Term> cur = get_equalence_classes_vw(term);
			for (auto& j : cur) {
				templeft.insert(data[j.name]);
				// table_classes.insert(cur[j]);
			}
		}
		for (int i : templeft) {
			table_classes.push_back(terms[i]);
		}
		map<vector<Symbol>, int> data_table; // храним ссылку на Терм из таблицы М-Н
											 // (быстрее и проще искать)
		for (int i = 0; i < table_classes.size(); i++) {
			data_table[table_classes[i].name] = i;
		}
		for (int i = 0; i <= table_classes.size(); i++) { // заполняем матрицу нулями
			vector<bool> vector_first(terms.size() + 1);
			equivalence_classes_table_temp.push_back(vector_first);
		}

		// заполняем с eps
		if (automaton.states[automaton.initial_state].is_terminal) {
			equivalence_classes_table_temp[0][0] = true;
		}
		int i = 1;
		for (const Term& t : table_classes) {
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
			for (auto& j : cur) {
				equivalence_classes_table_temp[data_table.at(j.name) + 1][i + 1] = true;
			}
		}

		map<vector<bool>, bool> wasvec;
		for (int i = 0; i < equivalence_classes_table_temp.size(); i++) {
			if (!wasvec.count(equivalence_classes_table_temp[i])) {
				wasvec[equivalence_classes_table_temp[i]] = true;
				equivalence_classes_table_bool.push_back(equivalence_classes_table_temp[i]);
				if (i == 0) {
					equivalence_classes_table_left.push_back(" ");
				} else {
					equivalence_classes_table_left.push_back(
						Symbol::vector_to_str(table_classes[i - 1].name));
				}
			}
		}
		equivalence_classes_table_top.push_back(" ");
		for (auto& term : terms) {
			equivalence_classes_table_top.push_back(Symbol::vector_to_str(term.name));
		}
		// проходим по таблице и удаляем одинаковые столбцы
		vector<int> delete_column_index;
		set<vector<bool>> for_find_same_column;
		for (int j = 0; j < equivalence_classes_table_bool[0].size(); j++) {
			vector<bool> temp;
			int size_set = for_find_same_column.size();
			for (auto& i : equivalence_classes_table_bool) {
				temp.push_back(i[j]);
			}
			for_find_same_column.insert(temp);
			if (size_set == for_find_same_column.size()) {
				delete_column_index.push_back(j);
			}
		}
		for (int i = delete_column_index.size() - 1; i >= 0; i--) {
			equivalence_classes_table_top.erase(equivalence_classes_table_top.begin() +
												delete_column_index[i]);
			for (int j = 0; j < equivalence_classes_table_bool.size(); j++) {
				equivalence_classes_table_bool[j].erase(equivalence_classes_table_bool[j].begin() +
														delete_column_index[i]);
			}
		}
	}
	// не уверен что правильно
	bool is_minimal_bool =
		(log2(automaton.states.size()) + 1) <= equivalence_classes_table_bool.size();
	if (log) {
		log->set_parameter("result", is_minimal_bool ? "true" : "false");
	}
	return is_minimal_bool;
}

string TransformationMonoid::to_txt_MyhillNerode() {
	if (equivalence_classes_table_bool.size() == 0) {
		is_minimal();
	}
	stringstream ss;
	// iLogTemplate::Table t;
	int maxlen = terms[terms.size() - 1].name.size();
	ss << string(maxlen + 2, ' ');
	for (auto& i : equivalence_classes_table_top) {
		ss << i << string(maxlen + 2 - i.size(), ' ');
		// t.columns.push_back(equivalence_classes_table_top[i]);
	}
	ss << "\n";

	for (int i = 0; i < equivalence_classes_table_left.size(); i++) {
		// t.rows.push_back(equivalence_classes_table_left[i]);
		ss << equivalence_classes_table_left[i]
		   << string(maxlen + 2 - equivalence_classes_table_left[i].size(), ' ');
		for (auto&& j : equivalence_classes_table_bool[i]) { // вывод матрицы
			// t.data.push_back(to_string(equivalence_classes_table_bool[i][j]));
			ss << j << string(maxlen + 1, ' ');
		}
		ss << "\n";
	}
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
