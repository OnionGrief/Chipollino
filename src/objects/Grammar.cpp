#include "Grammar.h"

ostream& operator<<(ostream& os, const GrammarItem& item) {
	if (item.type == GrammarItem::terminal) {
		if (item.term_name == "\0") return os << "eps";
		if (item.term_name == "\1") return os << "init";
		return os << item.term_name;
	} else
		return os << "S" << item.state_index;
}

void update_classes(set<int>& checker,
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

void check_classes(vector<vector<vector<GrammarItem*>>>& rules,
				   map<set<string>, vector<GrammarItem*>>& classes_check_map,
				   vector<GrammarItem*>& nonterminals) {
	classes_check_map.clear();
	for (int i = 0; i < nonterminals.size(); i++) {
		set<string> temp_rules;
		for (vector<GrammarItem*> rule : rules[i]) {
			string newRule;

			for (GrammarItem* t : rule) {
				if (t->type == GrammarItem::terminal)
					newRule += t->term_name;
				else
					newRule += to_string(t->class_number);
			}

			temp_rules.insert(newRule);
		}
		classes_check_map[temp_rules].push_back(nonterminals[i]);
	}
}

vector<vector<vector<GrammarItem*>>> get_bisimilar_grammar(
	vector<vector<vector<GrammarItem*>>>& rules,
	vector<GrammarItem*>& nonterminals,
	vector<GrammarItem*>& bisimilar_nonterminals) {
	map<set<string>, vector<GrammarItem*>> classes_check_map;
	set<int> checker;
	// checker
	while (true) {
		set<int> temp = checker;
		check_classes(rules, classes_check_map, nonterminals);
		update_classes(checker, classes_check_map);
		if (checker == temp) break;
	}
	/*for (auto& elem : classes_check_map) {
		cout << "{";
		for (int i = 0; i < elem.second.size() - 1; i++)
			cout << *elem.second[i] << ",";
		cout << *elem.second[elem.second.size() - 1] << "}";
	}
	cout << endl;*/
	// формирование бисимилярной грамматики
	map<int, GrammarItem*> class_to_nonterm;
	for (const auto& elem : classes_check_map)
		class_to_nonterm[elem.second[0]->class_number] = elem.second[0];
	vector<vector<vector<GrammarItem*>>> bisimilar_rules;
	for (const auto& elem : classes_check_map) {
		GrammarItem* curNonterm = elem.second[0];
		vector<vector<GrammarItem*>> temp_rules;
		for (vector<GrammarItem*> rule : rules[curNonterm->state_index]) {
			vector<GrammarItem*> tempRule;
			for (GrammarItem* item : rule) {
				if (item->type == GrammarItem::nonterminal) {
					tempRule.push_back(class_to_nonterm[item->class_number]);
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

vector<vector<vector<GrammarItem*>>> fa_to_grammar(
	const vector<State>& states, const set<alphabet_symbol>& alphabet,
	int initial_state, vector<GrammarItem>& fa_items,
	vector<GrammarItem*>& nonterminals, vector<GrammarItem*>& terminals) {
	vector<vector<vector<GrammarItem*>>> rules(states.size());
	fa_items.resize(states.size() + alphabet.size() + 2);
	int item_ind = 0;
	while (item_ind < states.size()) {
		fa_items[item_ind] = GrammarItem(GrammarItem::nonterminal, item_ind, 0);
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
	fa_items[item_ind] = (GrammarItem(GrammarItem::terminal,
									  "\1")); // обозначает начальное состояние
	terminals.push_back(&fa_items[item_ind]);

	for (int i = 0; i < states.size(); i++) {
		for (const auto& elem : states[i].transitions) {
			for (int transition_to : elem.second)
				rules[i].push_back({terminals[terminal_index[elem.first]],
									nonterminals[transition_to]});
		}
		if (states[i].is_terminal) rules[i].push_back({terminals[0]});
	}
	rules[initial_state].push_back({terminals[alphabet.size() + 1]});

	return rules;
}
// имя терминала - строка (тк может представлять собой число)
// нужно из имени получать alphabet_symbol
alphabet_symbol to_alphabet_symbol(string s) {
	if (s == "\0")
		return epsilon();
	else
		return s[0];
}

vector<vector<vector<GrammarItem*>>> tansitions_to_grammar(
	const vector<State>& states, int initial_state,
	const vector<GrammarItem*>& fa_nonterminals,
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
				item_vec[j] = (GrammarItem(GrammarItem::nonterminal, ind, 0));
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

vector<vector<vector<GrammarItem*>>> get_reverse_grammar(
	vector<vector<vector<GrammarItem*>>>& rules,
	vector<GrammarItem*>& nonterminals, vector<GrammarItem*>& terminals) {
	vector<vector<vector<GrammarItem*>>> reverse_rules(rules.size());
	for (int i = 0; i < rules.size(); i++) {
		for (int j = 0; j < rules[i].size(); j++) {
			if (rules[i][j].size() == 1) {
				if (rules[i][j][0]->term_name == "\1")
					reverse_rules[i].push_back({terminals[0]});
				if (rules[i][j][0]->term_name == "\0")
					reverse_rules[i].push_back(
						{terminals[terminals.size() - 1]});
			} else {
				reverse_rules[rules[i][j][1]->state_index].push_back(
					{rules[i][j][0], nonterminals[i]});
			}
		}
	}
	return reverse_rules;
}