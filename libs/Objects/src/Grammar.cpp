#include "Objects/Grammar.h"
#include "Objects/Logger.h"
#include <sstream>

GrammarItem::GrammarItem()
	: type(terminal), state_index(-1), class_number(-1), name("") {}

GrammarItem::GrammarItem(Type type, string name, int state_index,
						 int class_number)
	: type(type), name(name), state_index(state_index),
	  class_number(class_number) {}

GrammarItem::GrammarItem(Type type, string name, int state_index)
	: type(type), name(name), state_index(state_index) {}

GrammarItem::GrammarItem(Type type, string name) : type(type), name(name) {}

bool GrammarItem::operator!=(const GrammarItem& other) {
	return type != other.type || state_index != other.state_index ||
		   class_number != other.class_number || name != other.name;
}
void GrammarItem::operator=(const GrammarItem& other) {
	type = other.type;
	state_index = other.state_index;
	class_number = other.class_number;
	name = other.name;
}

ostream& operator<<(ostream& os, const GrammarItem& item) {
	if (item.type == GrammarItem::terminal) {
		if (item.name == "\0") return os << "eps";
		return os << item.name;
	} else
		return os << "S" << item.state_index;
}

void Grammar::update_classes(
	set<int>& checker,
	map<set<string>, vector<GrammarItem*>>& classes_check_map) {
	int classNum = 0;
	checker.clear();
	for (const auto& elem : classes_check_map) {
		checker.insert(elem.second[0]->state_index);
		for (GrammarItem* nont : elem.second) {
			nont->class_number = classNum;
		}
		classNum++;
	}
}

void Grammar::check_classes(
	vector<vector<vector<GrammarItem*>>>& rules,
	map<set<string>, vector<GrammarItem*>>& classes_check_map,
	vector<GrammarItem*>& nonterminals) {
	classes_check_map.clear();
	for (int i = 0; i < nonterminals.size(); i++) {
		set<string> temp_rules;
		for (vector<GrammarItem*> rule : rules[i]) {
			string newRule;

			for (GrammarItem* t : rule) {
				if (t->type == GrammarItem::terminal)
					newRule += t->name;
				else
					newRule += to_string(t->class_number);
			}

			temp_rules.insert(newRule);
		}
		classes_check_map[temp_rules].push_back(nonterminals[i]);
	}
}

vector<vector<vector<GrammarItem*>>> Grammar::get_bisimilar_grammar(
	vector<vector<vector<GrammarItem*>>>& rules,
	vector<GrammarItem*>& nonterminals,
	vector<GrammarItem*>& bisimilar_nonterminals,
	map<int, vector<GrammarItem*>>& class_to_nonterminals) {
	class_to_nonterminals.clear();
	map<set<string>, vector<GrammarItem*>> classes_check_map;
	set<int> checker;
	// checker
	while (true) {
		set<int> temp = checker;
		check_classes(rules, classes_check_map, nonterminals);
		update_classes(checker, classes_check_map);
		if (checker == temp) break;
	}
	// формирование бисимилярной грамматики
	for (const auto& elem : classes_check_map)
		for (GrammarItem* t : elem.second)
			class_to_nonterminals[elem.second[0]->class_number].push_back(t);
	vector<vector<vector<GrammarItem*>>> bisimilar_rules;
	for (const auto& elem : classes_check_map) {
		GrammarItem* curNonterm = elem.second[0];
		vector<vector<GrammarItem*>> temp_rules;
		for (vector<GrammarItem*> rule : rules[curNonterm->state_index]) {
			vector<GrammarItem*> tempRule;
			for (GrammarItem* item : rule) {
				if (item->type == GrammarItem::nonterminal) {
					tempRule.push_back(
						class_to_nonterminals[item->class_number][0]);
				} else
					tempRule.push_back(item);
			}
			temp_rules.push_back(tempRule);
		}
		bisimilar_nonterminals.push_back(curNonterm);
		bisimilar_rules.push_back(temp_rules);
	}
	return bisimilar_rules;
}

vector<vector<vector<GrammarItem*>>> Grammar::fa_to_grammar(
	const vector<State>& states, const set<alphabet_symbol>& alphabet,
	vector<GrammarItem>& fa_items, vector<GrammarItem*>& nonterminals,
	vector<GrammarItem*>& terminals) {
	vector<vector<vector<GrammarItem*>>> rules(states.size());
	fa_items.resize(states.size() + alphabet.size() + 2);
	int item_ind = 0;
	while (item_ind < states.size()) {
		fa_items[item_ind] = GrammarItem(
			GrammarItem::nonterminal, states[item_ind].identifier, item_ind, 0);
		nonterminals.push_back(&fa_items[item_ind]);
		item_ind++;
	}
	map<alphabet_symbol, int> terminal_index;
	fa_items[item_ind] = (GrammarItem(GrammarItem::terminal, "\0"));
	terminals.push_back(&fa_items[item_ind]);
	terminal_index[alphabet_symbol::epsilon()] = 0;
	item_ind++;
	for (alphabet_symbol symb : alphabet) {
		fa_items[item_ind] = (GrammarItem(GrammarItem::terminal, symb));
		terminals.push_back(&fa_items[item_ind]);
		terminal_index[symb] = item_ind - nonterminals.size();
		item_ind++;
	}

	for (int i = 0; i < states.size(); i++) {
		for (const auto& elem : states[i].transitions) {
			for (int transition_to : elem.second)
				rules[i].push_back({terminals[terminal_index[elem.first]],
									nonterminals[transition_to]});
		}
		if (states[i].is_terminal) rules[i].push_back({terminals[0]});
	}

	return rules;
}

vector<vector<vector<GrammarItem*>>> Grammar::tansitions_to_grammar(
	const vector<State>& states, const vector<GrammarItem*>& fa_nonterminals,
	vector<pair<GrammarItem, map<alphabet_symbol, vector<GrammarItem>>>>&
		fa_items,
	vector<GrammarItem*>& nonterminals, vector<GrammarItem*>& terminals) {
	// fa_items вектор пар <терминал (состояние), map нетерминалов (переходов)>
	fa_items.resize(states.size());
	int ind = 0;
	for (int i = 0; i < states.size(); i++) {
		// имя терминала - класс  эквивалентности состояния
		fa_items[i].first = GrammarItem(
			GrammarItem::terminal, to_string(fa_nonterminals[i]->class_number));
		terminals.push_back(&fa_items[i].first);
		for (const auto& elem : states[i].transitions) {
			vector<GrammarItem>& item_vec = fa_items[i].second[elem.first];
			item_vec.resize(elem.second.size());
			for (int j = 0; j < elem.second.size(); j++) {
				item_vec[j] = (GrammarItem(GrammarItem::nonterminal,
										   "Tr" + to_string(ind), ind, 0));
				nonterminals.push_back(&item_vec[j]);
				ind++;
			}
		}
	}

	vector<vector<vector<GrammarItem*>>> rules(nonterminals.size());
	ind = 0;
	for (int i = 0; i < states.size(); i++) {
		for (const auto& elem : states[i].transitions) {
			for (int transition_to : elem.second) {
				// смотрим все переходы из состояния transition_to
				for (auto transition_elem : states[transition_to].transitions) {
					for (int k = 0; k < transition_elem.second.size(); k++) {
						int nonterm_ind = fa_items[transition_to]
											  .second[transition_elem.first][k]
											  .state_index;
						rules[ind].push_back({terminals[transition_to],
											  nonterminals[nonterm_ind]});
					}
				}
				if (states[transition_to].is_terminal)
					rules[ind].push_back(
						{terminals[transition_to]}); // переход в финальное
													 // состояние
				ind++;
			}
		}
	}

	return rules;
}

vector<vector<vector<GrammarItem*>>> Grammar::get_reverse_grammar(
	vector<vector<vector<GrammarItem*>>>& rules,
	vector<GrammarItem*>& nonterminals, vector<GrammarItem*>& terminals,
	int initial_state) {
	vector<vector<vector<GrammarItem*>>> reverse_rules(rules.size());
	for (int i = 0; i < rules.size(); i++) {
		for (int j = 0; j < rules[i].size(); j++) {
			if (rules[i][j].size() == 2)
				reverse_rules[rules[i][j][1]->state_index].push_back(
					{rules[i][j][0], nonterminals[i]});
		}
	}
	reverse_rules[initial_state].push_back({terminals[0]});
	return reverse_rules;
}

// vector<vector<GrammarItem>>
const int Grammar::fa_to_g(const FiniteAutomaton& fa, string w, int index,
						   int index_back, map<int, GrammarItem*> grammer_items,
						   set<string> monoid_rules, string word,
						   map<int, bool> is_visit) {

	State st = fa.states[index];
	State st_back = fa.states[index_back];
	GrammarItem* g = grammer_items[index];
	set equivalence_class_back = grammer_items[index_back]->equivalence_class;

	g->rules[w].insert(index_back);
	if (is_visit[index]) {
		return 0;
	}
	for (const auto& equ : equivalence_class_back) {
		if (monoid_rules.find(equ) == monoid_rules.end()) {
			g->equivalence_class.insert(word + w);
		}
	}

	for (const auto& elem : st.transitions) {
		alphabet_symbol alpha = elem.first;
		set<int> transitions = elem.second;
		// if st.is_terminal то учитываем только переходы в себя
		for (const auto& ind : transitions) {
			if (alpha.is_epsilon()) {
				alpha = "";
			}
			if (index != ind) {
				is_visit[index] = true;
				fa_to_g(fa, alpha, ind, index, grammer_items, monoid_rules,
						word + w, is_visit);

			} else {
				g->rules[alpha].insert(index);
			}
		}
	}
	return 0;
}

const string Grammar::to_str(vector<alphabet_symbol> in) {
	string out = "";
	for (int i = 0; i < in.size(); i++) {
		out += in[i];
	}
	return out;
}

vector<vector<GrammarItem>> Grammar::fa_to_prefix_grammar(
	const FiniteAutomaton& fa) {
	vector<State> states = fa.states;
	TransformationMonoid a(fa.minimize());
	map<vector<alphabet_symbol>, vector<vector<alphabet_symbol>>> monoid_rules =
		a.get_rewriting_rules();
	set<string> m_r;
	for (auto& item : monoid_rules) {
		m_r.insert(this->to_str(item.first));
	}

	State st0 = states[fa.initial_state];
	map<int, GrammarItem*> grammer_items;
	map<int, GrammarItem> gr_it;
	map<int, bool> is_visit;
	for (size_t i = 0; i < states.size(); i++) {
		gr_it[states[i].index].state_index = i;
		if (!states[i].is_terminal) {
			gr_it[states[i].index].type = GrammarItem::nonterminal;
		}
		grammer_items[states[i].index] =
			&gr_it[states[i].index]; // new GrammarItem();

		is_visit[states[i].index] = false;
		if (i == fa.initial_state) {
			grammer_items[states[i].index]->is_started = true;
		}
	}
	GrammarItem* g = grammer_items[fa.initial_state];
	g->equivalence_class.insert("");
	for (const auto& elem : st0.transitions) {
		alphabet_symbol alpha = elem.first;
		set<int> transitions = elem.second;
		// if st.is_terminal то учитываем только переходы в себя
		for (const auto& ind : transitions) {
			if (fa.initial_state == ind) {
				if (alpha.is_epsilon()) {
					alpha = "";
				}
				g->rules[alpha].insert(fa.initial_state);
			}
		}
	}
	for (const auto& elem : st0.transitions) {
		alphabet_symbol alpha = elem.first;
		set<int> transitions = elem.second;
		// if st.is_terminal то учитываем только переходы в себя
		for (const auto& ind : transitions) {
			if (fa.initial_state != ind) {
				if (alpha.is_epsilon()) {
					alpha = "";
				}
				fa_to_g(fa, alpha, ind, fa.initial_state, grammer_items, m_r,
						"", is_visit);
			}
		}
	}

	for (const auto& elem : st0.transitions) {
		alphabet_symbol alpha = elem.first;
		set<int> transitions = elem.second;
		for (const auto& ind : transitions) {
			if (fa.initial_state == ind) {
				g->equivalence_class.insert(alpha);
				g->equivalence_class.erase("");
			}
		}
	}

	this->prefix_grammar = gr_it;
	return {};
}

const string Grammar::pg_to_txt() {
	set<string> out;
	stringstream ss;
	map<int, GrammarItem> gr_it = prefix_grammar;
	for (int i = 0; i < gr_it.size(); i++) {
		GrammarItem g = gr_it[i];
		for (const auto& w : g.equivalence_class) {
			for (const auto& elem : g.rules) {
				alphabet_symbol a = elem.first;
				// int index = elem.second;
				for (const auto& w_back : elem.second) {
					for (const auto& eq_back :
						 gr_it[w_back].equivalence_class) {
						string eq = eq_back;
						string wt = w;
						if (eq == "") {
							eq = "eps";
						}
						if (wt == "") {
							wt = "eps";
						}
						if (a == "") {
							a = "eps";
						}
						if (/*m_r.find(wt) == m_r.end() &&*/
							!(wt == eq && a == "eps")) {
							string test = wt;
							test += " -> ";
							test += eq + " ";
							test += a;
							out.insert(test); // wt + " -> " + eq + " " + a);
						}
					}
				}
			}
		}
	}
	for (const auto& elem : out) {
		ss << elem << endl;
	}
	ss << "------------ base words ------------" << endl;

	for (int i = 0; i < gr_it.size(); i++) {
		if (/*states[i].is_terminal*/ gr_it[i].type == GrammarItem::terminal) {
			GrammarItem g = gr_it[i];
			// cout << states[i].identifier << ":\n";
			for (const auto& w : g.equivalence_class) {
				ss << w << " ";
			}
			ss << endl;
		}
	}
	// this->prefix_grammar = gr_it;
	return ss.str();
}

FiniteAutomaton Grammar::prefix_grammar_to_automaton() {

	set<alphabet_symbol> symbols;
	vector<State> states;
	int initial_state;
	for (int i = 0; i < prefix_grammar.size(); i++) {
		GrammarItem gr = prefix_grammar[i];
		bool is_terminal = false;
		if (gr.is_started) {
			initial_state = i;
		}

		if (gr.type == GrammarItem::terminal) {
			is_terminal = true;
		}

		State s = {i,
				   {i},
				   to_string(i),
				   is_terminal,
				   map<alphabet_symbol, set<int>>()};
		states.push_back(s);
	}

	for (size_t i = 0; i < states.size(); i++) {
		State s = states[i];
		GrammarItem gr = prefix_grammar[i];

		for (const auto& elem : gr.rules) {
			alphabet_symbol alpha = elem.first;
			set<int> transition = elem.second;
			for (const auto& trans : transition) {
				states[trans].transitions[alpha].insert(i);
			}
			if (alpha == "") {
				alpha = alphabet_symbol::epsilon();
			}
			symbols.insert(alpha);
		}
	}

	return FiniteAutomaton(initial_state, states, symbols);
}

const int Grammar::fa_to_g_TM(const FiniteAutomaton& fa, string w, int index,
							  int index_back,
							  map<int, GrammarItem*> grammer_items,
							  set<string> monoid_rules, string word,
							  map<int, bool> is_visit) {

	State st = fa.states[index];
	GrammarItem* g = grammer_items[index];

	g->rules[w].insert(index_back);
	if (is_visit[index]) {
		return 0;
	}

	for (const auto& elem : st.transitions) {
		alphabet_symbol alpha = elem.first;
		set<int> transitions = elem.second;
		// if st.is_terminal то учитываем только переходы в себя
		for (const auto& ind : transitions) {
			if (alpha.is_epsilon()) {
				alpha = "";
			}
			if (index != ind) {
				is_visit[index] = true;
				fa_to_g_TM(fa, alpha, ind, index, grammer_items, monoid_rules,
						   word + w, is_visit);

			} else {
				g->rules[alpha].insert(index);
			}
		}
	}
	return 0;
}

vector<vector<GrammarItem>> Grammar::fa_to_prefix_grammar_TM(
	const FiniteAutomaton& fa) {
	vector<State> states = fa.states;
	TransformationMonoid a(fa);
	// cout << a.get_equalence_classes_txt();
	map<vector<alphabet_symbol>, vector<vector<alphabet_symbol>>> monoid_rules =
		a.get_rewriting_rules();
	set<string> m_r;
	for (auto& item : monoid_rules) {
		m_r.insert(this->to_str(item.first));
	}

	map<string, vector<string>> terms = a.get_equalence_classes_map();
	State st0 = states[fa.initial_state];
	map<int, GrammarItem*> grammer_items;
	map<int, GrammarItem> gr_it;
	map<int, bool> is_visit;
	for (size_t i = 0; i < states.size(); i++) {
		gr_it[states[i].index].state_index = i;
		if (!states[i].is_terminal) {
			gr_it[states[i].index].type = GrammarItem::nonterminal;
		}

		for (const auto& elem : terms) {
			string term = elem.first;
			for (size_t j = 0; j < elem.second.size(); j += 2) {
				if (elem.second[j] == st0.identifier &&
					elem.second[j + 1] == states[i].identifier) {
					gr_it[states[i].index].equivalence_class.insert(term);
				}
			}
		}

		grammer_items[states[i].index] =
			&gr_it[states[i].index]; // new GrammarItem();

		is_visit[states[i].index] = false;
		if (i == fa.initial_state) {
			grammer_items[states[i].index]->is_started = true;
		}
	}
	GrammarItem* g = grammer_items[fa.initial_state];
	g->equivalence_class.insert("");
	for (const auto& elem : st0.transitions) {
		alphabet_symbol alpha = elem.first;
		set<int> transitions = elem.second;
		// if st.is_terminal то учитываем только переходы в себя
		for (const auto& ind : transitions) {
			if (fa.initial_state == ind) {
				if (alpha.is_epsilon()) {
					alpha = "";
				}
				g->rules[alpha].insert(fa.initial_state);
			}
		}
	}
	for (const auto& elem : st0.transitions) {
		alphabet_symbol alpha = elem.first;
		set<int> transitions = elem.second;
		// if st.is_terminal то учитываем только переходы в себя
		for (const auto& ind : transitions) {
			if (fa.initial_state != ind) {
				if (alpha.is_epsilon()) {
					alpha = "";
				}
				fa_to_g_TM(fa, alpha, ind, fa.initial_state, grammer_items, m_r,
						   "", is_visit);
			}
		}
	}

	for (const auto& elem : st0.transitions) {
		alphabet_symbol alpha = elem.first;
		set<int> transitions = elem.second;
		for (const auto& ind : transitions) {
			if (fa.initial_state == ind) {
				//		g->equivalence_class.insert(alpha);
				g->equivalence_class.erase("");
			}
		}
	}

	this->prefix_grammar = gr_it;
	return {};
}
