#include "Objects/PushdownAutomaton.h"

#include <AutomataParser/Parser.h>

using std::map;
using std::runtime_error;
using std::set;
using std::string;
using std::unordered_set;
using std::vector;

vector<lexy_ascii_child> Parser::find_children(lexy_ascii_tree& tree, const set<string>& names,
											   const set<string>& exclude) {
	vector<lexy_ascii_child> result;
	int skip = 0;
	for (auto [event, node] : tree.traverse()) {
		auto depth = 0;
		switch (event) {
		case lexy::traverse_event::enter:
			if (!skip && names.count(node.kind().name())) {
				result.push_back(node);
			}
			if (exclude.count(node.kind().name())) {
				skip++;
			}
			depth++;
			break;
		case lexy::traverse_event::exit:
			if (exclude.count(node.kind().name())) {
				skip--;
			}
			depth--;
			break;

		case lexy::traverse_event::leaf:
			if (!skip && names.count(node.kind().name())) {
				result.push_back(node);
			}
			break;
		}
	}
	return result;
}

string Parser::first_child(lexy::_pt_node<lexy::_bra, void>::children_range::iterator it) {
	return lexy::as_string<string, lexy::ascii_encoding>(it->children().begin()->token().lexeme());
}

string Parser::first_child(lexy::_pt_node<lexy::_bra, void> it) {
	return lexy::as_string<string, lexy::ascii_encoding>(it.children().begin()->token().lexeme());
}

void Parser::parse_states(lexy_ascii_tree& tree, set<string>& names, map<string, string>& labels) {
	auto nodes = find_children(tree,
							   {AutomataParser::node_id},
							   {AutomataParser::state_label, AutomataParser::stack_symbol});
	for (auto node : nodes) {
		auto name = first_child(node);

		names.insert(name);
		labels[name] = name;
	}
}

void Parser::parse_descriptions(lexy_ascii_tree& tree, map<string, string>& labels,
								map<string, bool>& is_terminal, string& initial) {
	auto descriptions = find_children(tree, {AutomataParser::state_description});
	for (auto description : descriptions) {
		string name;
		for (auto desc : description.children()) {
			if (string(desc.kind().name()) == AutomataParser::node_id) {
				name = first_child(desc);
			}
			if (string(desc.kind().name()) == AutomataParser::state_label) {
				for (auto alias_child : desc.children()) {
					if (string(alias_child.kind().name()) == AutomataParser::node_id) {
						labels[name] = first_child(alias_child);
					}
				}
			}
			if (string(desc.kind().name()) == AutomataParser::terminal_mark) {
				is_terminal[name] = true;
			}
			if (string(desc.kind().name()) == AutomataParser::initial_mark) {
				if (initial.empty()) {
					initial = name;
				} else {
					throw runtime_error("AutomataParser::Parser::parse_descriptions ERROR(second "
										"initial state found)");
				}
			}
		}
	}
}

void Parser::parse_FA_transitions(lexy_ascii_tree& tree, vector<FATransition_info>& trans) {
	auto transitions = find_children(tree, {AutomataParser::statement});
	for (auto transition : transitions) {
		auto it = transition.children().begin();
		auto beg = first_child(it);
		it++;
		auto end = first_child(it);
		it++;
		auto symb = first_child(it);
		if (first_child(it)[0] == '&') {
			throw runtime_error("AutomataParser::Parser::parse_FA ERROR(MFA transition found)");
		}

		trans.emplace_back(beg, end, symb);
	}
}

void Parser::parse_MFA_transitions(lexy_ascii_tree& tree,
								   vector<Parser::MFATransition_info>& transitions) {
	auto edges = find_children(tree, {AutomataParser::MFA_edge});
	vector<MFATransition_info> mfat_info;
	for (auto edge : edges) {
		auto edge_child = edge.children().begin();
		auto it = edge_child->children().begin();
		string beg = first_child(*it);
		it++;
		string end = first_child(*it);
		it++;
		Symbol symb = first_child(*it);

		if (symb == "&") {
			for (auto symb_child : it->children()) {
				if (string(symb_child.kind().name()) == AutomataParser::cell_id) {
					symb = Symbol::Ref(stoi(first_child(symb_child)));
				}
			}
		}
		if (symb == AutomataParser::epsilon)
			symb = Symbol::Epsilon;

		unordered_set<int> open;
		unordered_set<int> close;
		edge_child++;
		for (auto memory_cell : edge_child->children()) {
			if (string(memory_cell.kind().name()) == AutomataParser::memory_cell) {
				auto cell = memory_cell.children().begin();
				int cell_id = stoi(first_child(*cell));
				cell++;
				if (first_child(cell) == "c")
					close.insert(cell_id);
				if (first_child(cell) == "o")
					open.insert(cell_id);
			}
		}

		mfat_info.emplace_back(beg, end, symb, open, close);
	}

	transitions = mfat_info;
}

void Parser::parse_PDA_transitions(lexy_ascii_tree& tree, vector<PDATransition_info>& trans) {
	auto edges = find_children(tree, {AutomataParser::PDA_edge});
	vector<PDATransition_info> pda_info;

	for (auto edge : edges) {
		auto edge_child = edge.children().begin();
		auto it = edge_child->children().begin();
		string beg = first_child(*it);
		it++;
		string end = first_child(*it);
		it++;
		Symbol symb = first_child(*it);

		edge_child++;
		it = edge_child->children().begin();

		Symbol push = first_child(it->children().begin());
		it++;
		it++;
		Symbol pop = first_child(it->children().begin());

		// Create a PDATransition_info object and add it to the vector
		pda_info.emplace_back(beg, end, symb, push, pop);
	}

	// Assign parsed PDATransition_info objects to the output vector
	trans = pda_info;
}

MemoryFiniteAutomaton Parser::parse_MFA(const string& filename) {
	lexy_ascii_tree tree;
	auto file = lexy::read_file<lexy::ascii_encoding>(filename.c_str());
	auto input = file.buffer();

	Lexer::parse_buffer(tree, input);

	set<string> names;
	map<string, string> labels;
	map<string, bool> is_terminal;
	string initial;

	parse_states(tree, names, labels);
	parse_descriptions(tree, labels, is_terminal, initial);

	vector<MFAState> states;
	map<string, int> states_id;

	int k = 0;
	int initial_state = 0;
	for (const auto& name : names) {
		if (name == initial)
			initial_state = k;
		states_id[name] = k;
		states.emplace_back(k++, labels[name], is_terminal[name]);
	}

	set<Symbol> alphabet;

	vector<MFATransition_info> mfat_info;
	parse_MFA_transitions(tree, mfat_info);

	for (auto& i : mfat_info) {
		if (!i.symb.is_epsilon() && !i.symb.is_ref()) {
			alphabet.insert(i.symb);
		}

		auto x = MFATransition(states_id[i.end], i.open, i.close);
		states[states_id[i.beg]].set_transition(x, i.symb);
	}

	auto mfa = MemoryFiniteAutomaton(initial_state, states, alphabet);

	return mfa;
}

FiniteAutomaton Parser::parse_FA(const string& filename) {
	lexy_ascii_tree tree;
	auto file = lexy::read_file<lexy::ascii_encoding>(filename.c_str());
	auto input = file.buffer();

	Lexer::parse_buffer(tree, input);

	set<string> names;
	map<string, string> labels;
	map<string, bool> is_terminal;
	string initial;

	parse_states(tree, names, labels);
	parse_descriptions(tree, labels, is_terminal, initial);

	vector<FAState> states;
	map<string, int> states_id;

	int k = 0;
	int initial_state = 0;
	for (const auto& name : names) {
		if (name == initial)
			initial_state = k;
		states_id[name] = k;
		states.emplace_back(k++, labels[name], is_terminal[name]);
	}

	set<Symbol> alphabet;
	vector<FATransition_info> trans;

	parse_FA_transitions(tree, trans);

	for (auto& tran : trans) {
		if (tran.symb == AutomataParser::epsilon) {
			states[states_id[tran.beg]].set_transition(states_id[tran.end], Symbol::Epsilon);
		} else {
			states[states_id[tran.beg]].set_transition(states_id[tran.end], tran.symb);
			alphabet.insert(Symbol(tran.symb));
		}
	}

	auto fa = FiniteAutomaton(initial_state, states, alphabet);

	return fa;
}

PushdownAutomaton Parser::parse_PDA(const string& filename) {
	lexy_ascii_tree tree;
	auto file = lexy::read_file<lexy::ascii_encoding>(filename.c_str());
	auto input = file.buffer();

	Lexer::parse_buffer(tree, input);

	map<string, bool> is_terminal;
	set<string> names;
	map<string, string> labels;
	string initial;

	parse_states(tree, names, labels);
	parse_descriptions(tree, labels, is_terminal, initial);

	vector<PDAState> states;
	map<string, int> states_id;

	int k = 0;
	int initial_state = 0;
	for (const auto& name : names) {
		if (name == initial)
			initial_state = k;
		states_id[name] = k;
		states.emplace_back(k++, labels[name], is_terminal[name]);
	}

	set<Symbol> alphabet;
	vector<PDATransition_info> trans;
	parse_PDA_transitions(tree, trans);

	for (auto& rawTransition : trans) {
		auto transition = PDATransition(states_id[rawTransition.end], rawTransition.symb, rawTransition.push, rawTransition.pop);

		if (rawTransition.symb == AutomataParser::epsilon) {
			states[states_id[rawTransition.beg]].set_transition(transition, Symbol::Epsilon);
		} else {
			states[states_id[rawTransition.beg]].set_transition(transition, rawTransition.symb);
			alphabet.insert(Symbol(rawTransition.symb));
		}
	}

	auto pda = PushdownAutomaton(initial_state, states, alphabet);

	return pda;
}