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
	terminal_index[epsilon()] = 0;
	item_ind++;
	for (alphabet_symbol symb : alphabet) {
		fa_items[item_ind] =
			(GrammarItem(GrammarItem::terminal, to_string(symb)));
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
int Grammar::fa_to_g(const FiniteAutomaton& fa, string w, int index,
					 int index_back, map<int, GrammarItem*> grammer_items,
					 set<string> monoid_rules, string word,
					 map<vector<string>, vector<vector<string>>> rules,
					 map<int, bool> is_visit) {
	// int index = st.index;
	State st = fa.states[index];
	State st_back = fa.states[index_back];
	// GrammarItem gr(GrammarItem::nonterminal, w, index);
	GrammarItem* g = grammer_items[index];
	set equivalence_class_back = grammer_items[index_back]->equivalence_class;
	/*
	for (const auto& equ : equivalence_class_back) {
		if (monoid_rules.find(equ) != monoid_rules.end()) {
			return 0;
		}
	}
	*/
	// cout << st.identifier << word << endl;
	vector<string> test;
	for (size_t i = 0; i < word.size(); i++) {
		test.push_back(string(1, word[i]));
	}
	// TransformationMonoid q;
	//  for (const auto& equ : g->equivalence_class) {
	//   cout << equ << " ";
	//  cout << this->to_str(q.rewriting(test, rules)) << " " <<
	//  this->to_str(test)
	//	 << endl;
	if (/*monoid_rules.find(equ) != monoid_rules.end() and*/
		/*monoid_rules.find(word) != monoid_rules.end()*/
		TransformationMonoid::rewriting(test, rules) != test) {
		// return 0;
	}
	//}
	// cout << "----" << endl;
	// if (is_visit[index]) {
	//	return 0;
	//}
	if (is_visit[index]) {
		return 0;
	}
	g->rules[w].insert(index_back);
	for (const auto& equ : equivalence_class_back) {
		if (monoid_rules.find(equ) == monoid_rules.end()) {
			// cout << equ + w << " TT";
			//  g->equivalence_class.insert(equ + w);
			g->equivalence_class.insert(word + w);
		}
	}
	// cout << "----" << endl;

	for (const auto& elem : st.transitions) {
		alphabet_symbol alpha = elem.first;
		set<int> transitions = elem.second;
		// if st.is_terminal то учитываем только переходы в себя
		for (const auto& ind : transitions) {
			if (is_epsilon(alpha)) {
				alpha = "";
			}
			if (st.is_terminal && false) {
				if (index == ind) {
					g->rules[alpha].insert(index);
				}
			} else {
				if (index != ind) {
					// if (is_epsilon(alpha)) {
					//	alpha = "";
					// }
					// cout << alpha << endl;

					is_visit[index] = true;
					fa_to_g(fa, alpha, ind, index, grammer_items, monoid_rules,
							word + w, rules, is_visit);

				} else {
					g->rules[alpha].insert(index);
				}
			}
		}
	}
	return 0;
}

string Grammar::to_str(vector<string> in) {
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
	// cout << fa.minimize().to_txt();
	map<vector<string>, vector<vector<string>>> monoid_rules =
		a.get_rewriting_rules();
	set<string> m_r;
	// cout << a.get_equalence_classes_txt() << "\n--------" << endl;
	//   cout << a.get_rewriting_rules_txt() << "\n--------" << endl;
	// a.get_equalence_classes();
	for (auto& item : monoid_rules) {
		// for (int i = 0; i < item.second.size(); i++) {
		//	Logger::log("rewriting " + to_str(item.first),
		//				to_str(item.second[i]));
		//	ss << to_str(item.first) << "	->	" << to_str(item.second[i])
		//	   << "\n";
		// }
		// cout << this->to_str(item.first) << endl;
		m_r.insert(this->to_str(item.first));
	}
	// cout << "Test\n";
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
		// grammer_items[states[i].index]->state_index = i; // states[i].index;
	}
	GrammarItem* g = grammer_items[fa.initial_state];
	g->equivalence_class.insert("");
	for (const auto& elem : st0.transitions) {
		alphabet_symbol alpha = elem.first;
		set<int> transitions = elem.second;
		// if st.is_terminal то учитываем только переходы в себя
		for (const auto& ind : transitions) {
			if (fa.initial_state == ind) {
				if (is_epsilon(alpha)) {
					alpha = "";
				}
				g->rules[alpha].insert(fa.initial_state);
				// g->equivalence_class.insert("");
			}
		}
	}
	for (const auto& elem : st0.transitions) {
		alphabet_symbol alpha = elem.first;
		set<int> transitions = elem.second;
		// if st.is_terminal то учитываем только переходы в себя
		for (const auto& ind : transitions) {
			if (fa.initial_state != ind) {
				if (is_epsilon(alpha)) {
					alpha = "";
				}
				fa_to_g(fa, alpha, ind, fa.initial_state, grammer_items, m_r,
						"", monoid_rules, is_visit);
			}
		}
	}

	for (const auto& elem : st0.transitions) {
		alphabet_symbol alpha = elem.first;
		set<int> transitions = elem.second;
		// if st.is_terminal то учитываем только переходы в себя
		for (const auto& ind : transitions) {
			if (fa.initial_state == ind) {
				// g->rules[alpha].insert(fa.initial_state);
				g->equivalence_class.insert(alpha);
				g->equivalence_class.erase("");
			}
		}
	}

	// cout << "---------" << endl;
	set<string> out;
	for (int i = 0; i < gr_it.size(); i++) {
		GrammarItem g = gr_it[i];
		for (const auto& w : g.equivalence_class) {
			for (const auto& elem : g.rules) {
				alphabet_symbol a = elem.first;
				// int index = elem.second;
				for (const auto& w_back : elem.second) {
					for (const auto& eq_back :
						 grammer_items[w_back]->equivalence_class) {
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
						// cout << wt << " -> " << eq << " " << a << endl;
						if (m_r.find(wt) == m_r.end() &&
							!(wt == eq && a == "eps")) {
							out.insert(wt + " -> " + eq + " " + a);
						}
					}
				}
			}
		}
	}
	for (const auto& elem : out) {
		cout << elem << endl;
	}
	cout << "base words" << endl;

	for (int i = 0; i < gr_it.size(); i++) {
		if (/*states[i].is_terminal*/ gr_it[i].type == GrammarItem::terminal) {
			GrammarItem g = gr_it[i];
			cout << states[i].identifier << ":\n";
			for (const auto& w : g.equivalence_class) {
				cout << w << " ";
			}
			cout << endl;
		}
	}
	this->prefix_grammar = gr_it;
	return {};
}

string Grammar::pg_to_txt() {
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
						// cout << wt << " -> " << eq << " " << a << endl;
						if (/*m_r.find(wt) == m_r.end() &&*/
							!(wt == eq && a == "eps")) {
							out.insert(wt + " -> " + eq + " " + a);
						}
					}
				}
			}
		}
	}
	for (const auto& elem : out) {
		ss << elem << endl;
	}
	out.insert("base words");
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