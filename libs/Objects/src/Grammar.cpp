#include <sstream>

#include "Objects/Grammar.h"
#include "Objects/Language.h"
#include "Objects/iLogTemplate.h"

using std::map;
using std::pair;
using std::set;
using std::string;
using std::stringstream;
using std::to_string;
using std::vector;

RLGrammar::Item::Item() : type(terminal), state_index(-1), class_number(-1) {}

RLGrammar::Item::Item(Type type, string name, int state_index, int class_number)
	: type(type), name(name), state_index(state_index), class_number(class_number) {}

RLGrammar::Item::Item(Type type, string name, int state_index)
	: type(type), name(name), state_index(state_index) {}

RLGrammar::Item::Item(Type type, string name) : type(type), name(name) {}

bool RLGrammar::Item::operator!=(const Item& other) const {
	return type != other.type || state_index != other.state_index ||
		   class_number != other.class_number || name != other.name;
}

std::ostream& operator<<(std::ostream& os, const RLGrammar::Item& item) {
	if (item.type == RLGrammar::Item::terminal) {
		return os << item.name;
	} else {
		return os << "S" << item.state_index;
	}
}

void RLGrammar::update_classes(set<int>& checker,
							   map<set<string>, vector<Item*>>& classes_check_map) {
	int classNum = 0;
	checker.clear();
	for (const auto& elem : classes_check_map) {
		checker.insert(elem.second[0]->state_index);
		for (Item* nont : elem.second) {
			nont->class_number = classNum;
		}
		classNum++;
	}
}

void RLGrammar::check_classes(const vector<vector<vector<Item*>>>& rules,
							  map<set<string>, vector<Item*>>& classes_check_map,
							  vector<Item*>& nonterminals) {
	classes_check_map.clear();
	for (int i = 0; i < nonterminals.size(); i++) {
		set<string> temp_rules;
		for (const vector<Item*>& rule : rules[i]) {
			string newRule;

			for (Item* t : rule) {
				if (t->type == Item::terminal)
					newRule += t->name;
				else
					newRule += to_string(t->class_number);
			}

			temp_rules.insert(newRule);
		}
		classes_check_map[temp_rules].push_back(nonterminals[i]);
	}
}

vector<vector<vector<RLGrammar::Item*>>> RLGrammar::get_bisimilar_grammar(
	vector<vector<vector<Item*>>>& rules, vector<Item*>& nonterminals,
	vector<Item*>& bisimilar_nonterminals, map<int, vector<Item*>>& class_to_nonterminals) {
	class_to_nonterminals.clear();
	map<set<string>, vector<Item*>> classes_check_map;
	set<int> checker;
	// checker
	while (true) {
		set<int> temp = checker;
		check_classes(rules, classes_check_map, nonterminals);
		update_classes(checker, classes_check_map);
		if (checker == temp)
			break;
	}
	// формирование бисимилярной грамматики
	for (const auto& elem : classes_check_map)
		for (Item* t : elem.second)
			class_to_nonterminals[elem.second[0]->class_number].push_back(t);
	vector<vector<vector<Item*>>> bisimilar_rules;
	for (const auto& elem : classes_check_map) {
		Item* curNonterm = elem.second[0];
		vector<vector<Item*>> temp_rules;
		for (const vector<Item*>& rule : rules[curNonterm->state_index]) {
			vector<Item*> tempRule;
			for (Item* item : rule) {
				if (item->type == Item::nonterminal) {
					tempRule.push_back(class_to_nonterminals[item->class_number][0]);
				} else {
					tempRule.push_back(item);
				}
			}
			temp_rules.push_back(tempRule);
		}
		bisimilar_nonterminals.push_back(curNonterm);
		bisimilar_rules.push_back(temp_rules);
	}
	return bisimilar_rules;
}

vector<vector<vector<RLGrammar::Item*>>> RLGrammar::fa_to_grammar(const vector<FAState>& states,
																  const set<Symbol>& alphabet,
																  vector<Item>& fa_items, vector<Item*>& nonterminals, vector<Item*>& terminals) {
	vector<vector<vector<Item*>>> rules(states.size());
	fa_items.resize(states.size() + alphabet.size() + 2);
	int item_ind = 0;
	while (item_ind < states.size()) {
		fa_items[item_ind] = Item(Item::nonterminal, states[item_ind].identifier, item_ind, 0);
		nonterminals.push_back(&fa_items[item_ind]);
		item_ind++;
	}
	map<Symbol, int> terminal_indexes;
	fa_items[item_ind] = Item(Item::terminal, Symbol::epsilon());
	terminals.push_back(&fa_items[item_ind]);
	terminal_indexes[Symbol::epsilon()] = 0;
	item_ind++;
	for (const Symbol& symbol : alphabet) {
		fa_items[item_ind] = Item(Item::terminal, symbol);
		terminals.push_back(&fa_items[item_ind]);
		terminal_indexes[symbol] = item_ind - nonterminals.size();
		item_ind++;
	}

	for (int i = 0; i < states.size(); i++) {
		for (const auto& [symb, to_states] : states[i].transitions) {
			for (int transition_to : to_states)
				rules[i].push_back(
					{terminals[terminal_indexes[symb]], nonterminals[transition_to]});
		}
		if (states[i].is_terminal)
			rules[i].push_back({terminals[0]});
	}

	return rules;
}

vector<vector<vector<RLGrammar::Item*>>> RLGrammar::tansitions_to_grammar(
	const vector<FAState>& states, const vector<Item*>& fa_nonterminals,
	vector<pair<Item, map<Symbol, vector<Item>>>>& fa_items, vector<Item*>& nonterminals,
	vector<Item*>& terminals) {
	// fa_items вектор пар <терминал (состояние), map нетерминалов (переходов)>
	fa_items.resize(states.size());
	int ind = 0;
	for (int i = 0; i < states.size(); i++) {
		// имя терминала - класс эквивалентности состояния
		fa_items[i].first = Item(Item::terminal, to_string(fa_nonterminals[i]->class_number));
		terminals.push_back(&fa_items[i].first);
		for (const auto& elem : states[i].transitions) {
			vector<Item>& item_vec = fa_items[i].second[elem.first];
			item_vec.resize(elem.second.size());
			for (int j = 0; j < elem.second.size(); j++) {
				item_vec[j] = (Item(Item::nonterminal, "Tr" + to_string(ind), ind, 0));
				nonterminals.push_back(&item_vec[j]);
				ind++;
			}
		}
	}

	vector<vector<vector<Item*>>> rules(nonterminals.size());
	ind = 0;
	for (int i = 0; i < states.size(); i++) {
		for (const auto& elem : states[i].transitions) {
			for (int transition_to : elem.second) {
				// смотрим все переходы из состояния transition_to
				for (const auto& transition_elem : states[transition_to].transitions) {
					for (int k = 0; k < transition_elem.second.size(); k++) {
						int nonterm_ind =
							fa_items[transition_to].second[transition_elem.first][k].state_index;
						rules[ind].push_back({terminals[transition_to], nonterminals[nonterm_ind]});
					}
				}
				if (states[transition_to].is_terminal)
					rules[ind].push_back({terminals[transition_to]}); // переход в финальное
																	  // состояние
				ind++;
			}
		}
	}

	return rules;
}

vector<vector<vector<RLGrammar::Item*>>> RLGrammar::get_reverse_grammar(
	vector<vector<vector<Item*>>>& rules, vector<Item*>& nonterminals, vector<Item*>& terminals,
	int initial_state) {
	vector<vector<vector<Item*>>> reverse_rules(rules.size());
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

//------------------------------------------Prefix Grammar------------------------------------------

PrefixGrammar::Item::Item() : state_index(-1) {}

int PrefixGrammar::fa_to_g(const FiniteAutomaton& fa, Symbol w, int index, int index_back,
						   const vector<Item*>& grammar_items, const set<string>& monoid_rules,
						   string word) {
	const FAState& st = fa.states[index];
	const FAState& st_back = fa.states[index_back];
	Item* g = grammar_items[index];
	const set<string>& equivalence_class_back = grammar_items[index_back]->equivalence_class;

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
		Symbol alpha = elem.first;
		// if st.is_terminal то учитываем только переходы в себя
		for (const auto& ind : elem.second) {
			if (alpha.is_epsilon()) {
				alpha = "";
			}
			if (index != ind) {
				string l = string(w);
				fa_to_g(fa, alpha, ind, index, grammar_items, monoid_rules, word + l);

			} else {
				g->rules[alpha].insert(index);
			}
		}
	}
	return 0;
}

void PrefixGrammar::fa_to_prefix_grammar(const FiniteAutomaton& fa, iLogTemplate* log) {
	if (log) {
		log->set_parameter("oldautomaton", fa);
	}
	const vector<FAState>& states = fa.states;

	if (!fa.language->is_min_dfa_cached() && log) {
		log->set_parameter("cachedMINDFA", "Минимальный автомат сохранен в кэше");
	}

	TransformationMonoid a(fa.minimize());
	map<vector<Symbol>, vector<vector<Symbol>>> monoid_rules = a.get_rewriting_rules();
	set<string> m_r;
	for (auto& item : monoid_rules) {
		m_r.insert(Symbol::vector_to_str(item.first));
	}

	const FAState& st0 = states[fa.initial_state];
	vector<Item*> grammar_items;
	prefix_grammar.resize(states.size());
	fill(prefix_grammar.begin(), prefix_grammar.end(), Item());

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
	Item* g = grammar_items[fa.initial_state];
	g->equivalence_class.insert("");
	for (const auto& elem : st0.transitions) {
		Symbol alpha = elem.first;
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
		Symbol alpha = elem.first;
		// if st.is_terminal то учитываем только переходы в себя
		for (const auto& ind : elem.second) {
			if (fa.initial_state != ind) {
				if (alpha.is_epsilon()) {
					alpha = "";
				}
				fa_to_g(fa, alpha, ind, fa.initial_state, grammar_items, m_r, "");
			}
		}
	}

	for (const auto& elem : st0.transitions) {
		Symbol alpha = elem.first;
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
		fa_to_prefix_grammar(fa.determinize());
		if (log) {
			log->set_parameter("text1", "Неопределенность");
			log->set_parameter("text2", "Детерминизируем");
			log->set_parameter("result", pg_to_txt());
		}
		return;
	}
	vector<FAState> states_not_trap = fa.remove_trap_states().states;

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
	if (log) {
		log->set_parameter("result", pg_to_txt());
	}
	return;
}

string PrefixGrammar::pg_to_txt() const {
	set<string> out;
	stringstream ss;
	// vector<Item> prefix_grammar = prefix_grammar;
	for (const auto& item : prefix_grammar) {
		for (const auto& w : item.equivalence_class) {
			for (const auto& elem : item.rules) {
				Symbol a = elem.first;
				// int index = elem.second;
				for (const auto& w_back : elem.second) {
					for (const auto& eq_back : prefix_grammar[w_back].equivalence_class) {
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
						if (!item.is_started && prefix_grammar[w_back].is_started) {
							eq = "eps";
						}
						if (wt == "eps") {
							continue;
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
		ss << elem << "\\\\";
	}
	ss << "Базисные слова: "
	   << "\\\\";

	for (const auto& item : prefix_grammar) {
		if (item.is_terminal) {
			const Item& g = item;
			for (const auto& w : g.equivalence_class) {
				if (w == "") {
					ss << "eps"
					   << " ";
				} else {
					ss << w << " ";
				}
			}
			ss << "\\\\";
		}
	}
	return ss.str();
}

FiniteAutomaton PrefixGrammar::prefix_grammar_to_automaton(iLogTemplate* log) const {
	// TODO:
	if (log) {
		log->set_parameter("grammar", pg_to_txt());
	}
	set<Symbol> symbols;
	vector<FAState> states;
	int initial_state;
	for (int i = 0; i < prefix_grammar.size(); i++) {
		const Item& gr = prefix_grammar[i];
		bool is_terminal = false;
		if (gr.is_started) {
			initial_state = i;
		}

		if (gr.is_terminal) {
			is_terminal = true;
		}

		FAState s = {i, {i}, to_string(i), is_terminal, FAState::Transitions()};
		states.push_back(s);
	}

	for (size_t i = 0; i < states.size(); i++) {
		FAState s = states[i];
		const Item& gr = prefix_grammar[i];

		for (const auto& elem : gr.rules) {
			Symbol alpha = elem.first;
			for (const auto& trans : elem.second) {
				states[trans].transitions[alpha].insert(i);
			}
			if (alpha == "") {
				alpha = Symbol::epsilon();
			} else {
				symbols.insert(alpha);
			}
		}
	}
	FiniteAutomaton res = FiniteAutomaton(initial_state, states, symbols);
	if (log) {
		log->set_parameter("result", res);
	}
	return res;
}

int PrefixGrammar::fa_to_g_TM(const FiniteAutomaton& fa, string w, int index, int index_back,
							  const vector<Item*>& grammar_items, const set<string>& monoid_rules,
							  string word) {
	const FAState& st = fa.states[index];
	Item* g = grammar_items[index];

	g->rules[w].insert(index_back);
	if (g->is_visit) {
		return 0;
	}
	g->is_visit = true;
	for (const auto& elem : st.transitions) {
		Symbol alpha = elem.first;
		// if st.is_terminal то учитываем только переходы в себя
		for (const auto& ind : elem.second) {
			if (alpha.is_epsilon()) {
				alpha = "";
			}
			if (index != ind) {
				fa_to_g_TM(fa, alpha, ind, index, grammar_items, monoid_rules, word + w);

			} else {
				g->rules[alpha].insert(index);
			}
		}
	}
	return 0;
}

void PrefixGrammar::fa_to_prefix_grammar_TM(const FiniteAutomaton& fa, iLogTemplate* log) {
	if (log) {
		log->set_parameter("oldautomaton", fa);
	}
	const vector<FAState>& states = fa.states;
	TransformationMonoid a(fa);
	map<vector<Symbol>, vector<vector<Symbol>>> monoid_rules = a.get_rewriting_rules();
	set<string> m_r;
	for (auto& item : monoid_rules) {
		m_r.insert(Symbol::vector_to_str(item.first));
	}

	map<string, vector<string>> terms = a.get_equalence_classes_map();
	const FAState& st0 = states[fa.initial_state];
	vector<Item*> grammar_items;
	prefix_grammar.resize(states.size());
	fill(prefix_grammar.begin(), prefix_grammar.end(), Item());

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
					grammar_items[states[i].index]->equivalence_class.insert(term);
				}
			}
		}
		if (i == fa.initial_state) {
			grammar_items[states[i].index]->is_started = true;
		}
	}
	if (st0.is_terminal) {
		grammar_items[st0.index]->equivalence_class.insert("");
	}
	//---------------------------
	set<string> equal_classes;
	int count = 0;
	for (auto& item : prefix_grammar) {
		for (const auto& elem : item.equivalence_class) {
			equal_classes.insert(elem);
			count++;
		}
	}
	if (count != equal_classes.size()) {
		// в логер то что неопределенность и детерменизируем
		fa_to_prefix_grammar_TM(fa.determinize());
		if (log) {
			log->set_parameter("text1", "Неопределенность");
			log->set_parameter("text2", "Детерминизируем");
			log->set_parameter("result", pg_to_txt());
		}
		return;
	}
	//----------------------------
	Item* g = grammar_items[fa.initial_state];
	g->equivalence_class.insert("");
	for (const auto& elem : st0.transitions) {
		Symbol alpha = elem.first;
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
		Symbol alpha = elem.first;
		// if st.is_terminal то учитываем только переходы в себя
		for (const auto& ind : elem.second) {
			if (fa.initial_state != ind) {
				if (alpha.is_epsilon()) {
					alpha = "";
				}
				fa_to_g_TM(fa, alpha, ind, fa.initial_state, grammar_items, m_r, "");
			}
		}
	}

	for (const auto& elem : st0.transitions) {
		for (const auto& ind : elem.second) {
			if (fa.initial_state == ind) {
				// g->equivalence_class.erase("");
			}
		}
	}
	if (log) {
		log->set_parameter("result", pg_to_txt());
	}
	return;
}
