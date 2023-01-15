#include "Objects/Grammar.h"
#include "Objects/iLogTemplate.h"
#include <sstream>

PrefixGrammarItem::PrefixGrammarItem() : state_index(-1) {}

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
	for (alphabet_symbol alpha : alphabet) {
		fa_items[item_ind] = (GrammarItem(GrammarItem::terminal, alpha));
		terminals.push_back(&fa_items[item_ind]);
		terminal_index[alpha] = item_ind - nonterminals.size();
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
const int Grammar::fa_to_g(const FiniteAutomaton& fa, alphabet_symbol w,
						   int index, int index_back,
						   const vector<PrefixGrammarItem*>& grammar_items,
						   const set<string>& monoid_rules, string word) {
	const State& st = fa.states[index];
	const State& st_back = fa.states[index_back];
	PrefixGrammarItem* g = grammar_items[index];
	const set<string>& equivalence_class_back =
		grammar_items[index_back]->equivalence_class;

	g->rules[w].insert(index_back);
	if (g->is_visit) {
		return 0;
	}
	g->is_visit = true;
	for (const auto& equ : equivalence_class_back) {
		if (monoid_rules.find(equ) == monoid_rules.end() || st.is_terminal) {
			string l = string(w);
			g->equivalence_class.insert(word + l);
		}
	}

	for (const auto& elem : st.transitions) {
		alphabet_symbol alpha = elem.first;
		// if st.is_terminal то учитываем только переходы в себя
		for (const auto& ind : elem.second) {
			if (alpha.is_epsilon()) {
				alpha = "";
			}
			if (index != ind) {
				string l = string(w);
				fa_to_g(fa, alpha, ind, index, grammar_items, monoid_rules,
						word + l);

			} else {
				g->rules[alpha].insert(index);
			}
		}
	}
	return 0;
}

void Grammar::fa_to_prefix_grammar(const FiniteAutomaton& fa,
								   iLogTemplate* log) {
	// Logger::init_step("PrefixGrammar");
	// Logger::log("Автомат", fa);
	if (log) {
		log->set_parameter("automaton", fa);
	}
	const vector<State>& states = fa.states;
	TransformationMonoid a(fa.minimize());
	map<vector<alphabet_symbol>, vector<vector<alphabet_symbol>>> monoid_rules =
		a.get_rewriting_rules();
	set<string> m_r;
	for (auto& item : monoid_rules) {
		m_r.insert(alphabet_symbol::vector_to_str(item.first));
	}

	const State& st0 = states[fa.initial_state];
	vector<PrefixGrammarItem*> grammar_items;
	prefix_grammar.resize(states.size());
	fill(prefix_grammar.begin(), prefix_grammar.end(), PrefixGrammarItem());

	for (size_t i = 0; i < states.size(); i++) {
		grammar_items.push_back(&prefix_grammar[i]);
		grammar_items[states[i].index]->is_terminal = states[i].is_terminal;
		grammar_items[states[i].index]->state_index = states[i].index;
		grammar_items[states[i].index]->equivalence_class = {};
		grammar_items[states[i].index]->rules = {};
		if (i == fa.initial_state) {
			grammar_items[states[i].index]->is_started = true;
		}
	}
	PrefixGrammarItem* g = grammar_items[fa.initial_state];
	g->equivalence_class.insert("");
	for (const auto& elem : st0.transitions) {
		alphabet_symbol alpha = elem.first;
		// if st.is_terminal то учитываем только переходы в себя
		for (const auto& ind : elem.second) {
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
		// if st.is_terminal то учитываем только переходы в себя
		for (const auto& ind : elem.second) {
			if (fa.initial_state != ind) {
				if (alpha.is_epsilon()) {
					alpha = "";
				}
				fa_to_g(fa, alpha, ind, fa.initial_state, grammar_items, m_r,
						"");
			}
		}
	}

	for (const auto& elem : st0.transitions) {
		alphabet_symbol alpha = elem.first;
		for (const auto& ind : elem.second) {
			if (fa.initial_state == ind) {
				g->equivalence_class.insert(alpha);
				g->equivalence_class.erase("");
			}
		}
	}

	set<string> equal_classes;
	int count = 0;
	for (size_t i = 0; i < prefix_grammar.size(); i++) {
		for (const auto& elem : prefix_grammar[i].equivalence_class) {
			equal_classes.insert(elem);
			count++;
		}
	}

	if (count != equal_classes.size()) {
		// в логер то что неопределенность и детерминизируем
		// Logger::log("Неопределенность");
		// Logger::log("Детерминизируем");
		fa_to_prefix_grammar(fa.determinize());
		// Logger::log("Построенная по нему префиксная грамматика:");
		// Logger::log(pg_to_txt());
		// Logger::finish_step();
		if (log) {
			log->set_parameter("text1", "Неопределенность");
			log->set_parameter("text2", "Детерминизируем");
			log->set_parameter("result", pg_to_txt());
		}
		return;
	}
	vector<State> states_not_trap = fa.remove_trap_states().states;

	for (size_t i = 0; i < prefix_grammar.size(); i++) {
		bool check = false;
		for (size_t j = 0; j < states_not_trap.size(); j++) {
			if (states_not_trap[j].identifier == states[i].identifier) {
				check = true;
				break;
			}
		}
		if (!check) {
			prefix_grammar[i].equivalence_class = {};
		}
	}
	// Logger::log(pg_to_txt());
	// Logger::finish_step();
	if (log) {
		log->set_parameter("result", pg_to_txt());
	}
	return;
}

string Grammar::pg_to_txt() const {
	set<string> out;
	stringstream ss;
	// vector<PrefixGrammarItem> prefix_grammar = prefix_grammar;
	for (int i = 0; i < prefix_grammar.size(); i++) {
		const PrefixGrammarItem& g = prefix_grammar[i];
		for (const auto& w : g.equivalence_class) {
			for (const auto& elem : g.rules) {
				alphabet_symbol a = elem.first;
				// int index = elem.second;
				for (const auto& w_back : elem.second) {
					for (const auto& eq_back :
						 prefix_grammar[w_back].equivalence_class) {
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
						if (!g.is_started &&
							prefix_grammar[w_back].is_started) {
							eq = "eps";
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
		ss << elem << endl << endl;
	}
	ss << "------------ base words ------------" << endl << endl;

	for (int i = 0; i < prefix_grammar.size(); i++) {
		if (prefix_grammar[i].is_terminal) {
			const PrefixGrammarItem& g = prefix_grammar[i];
			for (const auto& w : g.equivalence_class) {
				if (w == "") {
					ss << "eps"
					   << " ";
				} else {
					ss << w << " ";
				}
			}
			ss << endl << endl;
		}
	}
	return ss.str();
}

FiniteAutomaton Grammar::prefix_grammar_to_automaton(iLogTemplate* log) const {
	// Logger::init_step("PrefixGrammar -> NFA");
	// // TODO:
	// Logger::log("Префиксная грамматика:");
	// Logger::log(pg_to_txt());
	if (log) {
		log->set_parameter("grammar", pg_to_txt());
	}
	set<alphabet_symbol> symbols;
	vector<State> states;
	int initial_state;
	for (int i = 0; i < prefix_grammar.size(); i++) {
		const PrefixGrammarItem& gr = prefix_grammar[i];
		bool is_terminal = false;
		if (gr.is_started) {
			initial_state = i;
		}

		if (gr.is_terminal) {
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
		const PrefixGrammarItem& gr = prefix_grammar[i];

		for (const auto& elem : gr.rules) {
			alphabet_symbol alpha = elem.first;
			for (const auto& trans : elem.second) {
				states[trans].transitions[alpha].insert(i);
			}
			if (alpha == "") {
				alpha = alphabet_symbol::epsilon();
			} else {
				symbols.insert(alpha);
			}
		}
	}
	FiniteAutomaton res = FiniteAutomaton(initial_state, states, symbols);
	// Logger::log("Построенный по ней автомат", res);
	// Logger::finish_step();
	if (log) {
		log->set_parameter("result", res);
	}
	return res;
}

const int Grammar::fa_to_g_TM(const FiniteAutomaton& fa, string w, int index,
							  int index_back,
							  const vector<PrefixGrammarItem*>& grammar_items,
							  const set<string>& monoid_rules, string word) {
	const State& st = fa.states[index];
	PrefixGrammarItem* g = grammar_items[index];

	g->rules[w].insert(index_back);
	if (g->is_visit) {
		return 0;
	}
	g->is_visit = true;
	for (const auto& elem : st.transitions) {
		alphabet_symbol alpha = elem.first;
		// if st.is_terminal то учитываем только переходы в себя
		for (const auto& ind : elem.second) {
			if (alpha.is_epsilon()) {
				alpha = "";
			}
			if (index != ind) {
				fa_to_g_TM(fa, alpha, ind, index, grammar_items, monoid_rules,
						   word + w);

			} else {
				g->rules[alpha].insert(index);
			}
		}
	}
	return 0;
}

void Grammar::fa_to_prefix_grammar_TM(const FiniteAutomaton& fa) {
	const vector<State>& states = fa.states;
	TransformationMonoid a(fa);
	map<vector<alphabet_symbol>, vector<vector<alphabet_symbol>>> monoid_rules =
		a.get_rewriting_rules();
	set<string> m_r;
	for (auto& item : monoid_rules) {
		m_r.insert(alphabet_symbol::vector_to_str(item.first));
	}

	map<string, vector<string>> terms = a.get_equalence_classes_map();
	const State& st0 = states[fa.initial_state];
	vector<PrefixGrammarItem*> grammar_items;
	prefix_grammar.resize(states.size());
	fill(prefix_grammar.begin(), prefix_grammar.end(), PrefixGrammarItem());

	for (size_t i = 0; i < states.size(); i++) {
		grammar_items.push_back(&prefix_grammar[i]);
		grammar_items[states[i].index]->is_terminal = states[i].is_terminal;
		if (i == fa.initial_state) {
			grammar_items[states[i].index]->is_started = true;
		}
		for (const auto& elem : terms) {
			string term = elem.first;
			for (size_t j = 0; j < elem.second.size(); j += 2) {
				if (elem.second[j] == st0.identifier &&
					elem.second[j + 1] == states[i].identifier) {
					grammar_items[states[i].index]->equivalence_class.insert(
						term);
				}
			}
		}
		if (i == fa.initial_state) {
			grammar_items[states[i].index]->is_started = true;
		}
	}
	//---------------------------
	set<string> equal_classes;
	int count = 0;
	for (size_t i = 0; i < prefix_grammar.size(); i++) {
		for (const auto& elem : prefix_grammar[i].equivalence_class) {
			equal_classes.insert(elem);
			count++;
		}
	}
	if (count != equal_classes.size()) {
		// в логер то что неопределенность и детерменизируем
		fa_to_prefix_grammar_TM(fa.determinize());
		return;
	}
	//----------------------------
	PrefixGrammarItem* g = grammar_items[fa.initial_state];
	g->equivalence_class.insert("");
	for (const auto& elem : st0.transitions) {
		alphabet_symbol alpha = elem.first;
		// if st.is_terminal то учитываем только переходы в себя
		for (const auto& ind : elem.second) {
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
		// if st.is_terminal то учитываем только переходы в себя
		for (const auto& ind : elem.second) {
			if (fa.initial_state != ind) {
				if (alpha.is_epsilon()) {
					alpha = "";
				}
				fa_to_g_TM(fa, alpha, ind, fa.initial_state, grammar_items, m_r,
						   "");
			}
		}
	}

	for (const auto& elem : st0.transitions) {
		for (const auto& ind : elem.second) {
			if (fa.initial_state == ind) {
				g->equivalence_class.erase("");
			}
		}
	}
	return;
}
