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

void Parser::read_symbols(int num) {
	cur_pos += num;
	while (cur_pos < file.size() && (file[cur_pos] == ' ' || file[cur_pos] == '\n')) {
		cur_pos++;
	}
}

bool Parser::parse_reserved(const std::string& res_case) {
	if (res_case == "EPS")
		return true;

	if (cur_pos == file.size())
		return false;

	// if (res_case == "LETTER") {
	// 	if ((file[cur_pos] >= 'a' && file[cur_pos] <= 'z') || (file[cur_pos] >= 'A' && file[cur_pos] <= 'Z')) {
	// 		LETTER = file[cur_pos];
	// 		read_symbols(1);
	// 		return true;
	// 	}
	// }
	int beg_pos = cur_pos;
	if (res_case == "LETTER") {
		while (cur_pos < file.size() &&
			   ((file[cur_pos] >= 'a' && file[cur_pos] <= 'z') ||
				(file[cur_pos] >= 'A' && file[cur_pos] <= 'Z') ||
				(file[cur_pos] >= '0' && file[cur_pos] <= '9') ||
				file[cur_pos] == '.' || file[cur_pos] == ',')) {
			cur_pos++;
		}
		if (beg_pos != cur_pos)
			LETTER = Symbol(file.substr(beg_pos, cur_pos - beg_pos));
		read_symbols(0);
		return (beg_pos != cur_pos);
	}
	if (res_case == "DIGIT") {
		if (file[cur_pos] >= '0' && file[cur_pos] <= '9') {
			NUMBER = int(file[cur_pos] - '0');
			read_symbols(1);
			return true;
		}
	}
	if (res_case == "STRING") {
		while (cur_pos < file.size() &&
			   ((file[cur_pos] >= 'a' && file[cur_pos] <= 'z') ||
				(file[cur_pos] >= 'A' && file[cur_pos] <= 'Z') ||
				(file[cur_pos] >= '0' && file[cur_pos] <= '9') ||
				(file[cur_pos] >= '(' && file[cur_pos] <= '.') || file[cur_pos] == '|' ||
				file[cur_pos] == '^' || file[cur_pos] == '&' || file[cur_pos] == ':' ||
				file[cur_pos] == '[' || file[cur_pos] == ']')) {
			cur_pos++;
		}
		if (beg_pos != cur_pos)
			STRING = file.substr(beg_pos, cur_pos - beg_pos);
		read_symbols(0);
		return (beg_pos != cur_pos);
	}
	if (res_case == "NUMBER") {
		if (file[cur_pos] == '0') {
			NUMBER = std::stoi(file.substr(beg_pos, 1));
			read_symbols(1);
			return true;
		}
		while (cur_pos < file.size() && (file[cur_pos] >= '0' && file[cur_pos] <= '9')) {
			cur_pos++;
		}
		if (beg_pos != cur_pos)
			NUMBER = std::stoi(file.substr(beg_pos, cur_pos - beg_pos));
		read_symbols(0);
		return (beg_pos != cur_pos);
	}
	return false;
}

bool Parser::parse_nonterminal(lexy::_pt_node<lexy::_bra, void> ref) {
	return parse_transition(first_child(ref));
}

bool Parser::parse_terminal(lexy::_pt_node<lexy::_bra, void> ref) {
	auto it = ref.children().begin();
	it++;
	std::string to_read = lexy::as_string<string, lexy::ascii_encoding>(it->token().lexeme());
	auto res = (file.size() - cur_pos >= to_read.size() &&
				file.substr(cur_pos, to_read.size()) == to_read);
	read_symbols(to_read.size());
	if (res)
		TERMINAL = to_read;
	return res;
}

void Parser::parse_attribute(lexy::_pt_node<lexy::_bra, void> ref) {
	auto it = ref.children().begin();
	while (std::string(it->kind().name()) != "nonterminal")
		it++;
	attributes.insert(first_child(it));
}

bool Parser::parse_alternative(lexy::_pt_node<lexy::_bra, void> ref) {
	bool read = true;
	int beg_pos = cur_pos;
	for (auto it = ref.children().begin(); it != ref.children().end(); it++) {
		if (it->kind().is_token() &&
			lexy::as_string<string, lexy::ascii_encoding>(it->token().lexeme()) == "|") {
			if (read)
				return true;
			cur_pos = beg_pos;
			read = true;
			continue;
		}
		if (!read) {
			continue;
		}
		if (it->kind().is_token() &&
			lexy::as_string<string, lexy::ascii_encoding>(it->token().lexeme()) == "(") {
			it++;
			if (attributes.count(first_child(it->children().begin()))) {
				while (it->kind().is_token() || std::string(it->kind().name()) != "alternative")
					it++;
				parse_alternative(*it);
			} else {
				while (!it->kind().is_token() ||
					   lexy::as_string<string, lexy::ascii_encoding>(it->token().lexeme()) != ":")
					it++;

				while (it->kind().is_token() || std::string(it->kind().name()) != "alternative")
					it++;
				parse_alternative(*it);
			}
			while (it->kind().is_token() &&
				   lexy::as_string<string, lexy::ascii_encoding>(it->token().lexeme()) != ")")
				it++;
		}
		if (std::string(it->kind().name()) == "terminal") {
			read &= parse_terminal(*it);
			continue;
		}
		if (std::string(it->kind().name()) == "nonterminal") {
			read &= parse_nonterminal(*it);
			continue;
		}
		if (std::string(it->kind().name()) == "reserved") {
			parse_reserved(first_child(it));
			continue;
		}
		if (std::string(it->kind().name()) == "attribute") {
			parse_attribute(*it);
			continue;
		}
	}
	return read;
}

bool Parser::parse_transition(const std::string& name) {
	if (!rewriting_rules.count(name)) {
		return false;
	}

	if (!parse_func.count(name)) {
		auto transition = rewriting_rules[name];
		return parse_alternative(*transition);
	}

	return parse_func[name]();
}

std::variant<FiniteAutomaton, MemoryFiniteAutomaton> Parser::parse(lexy_ascii_tree& grammar,
																   const std::string& filename) {
	auto transitions = find_children(grammar, {"transition"}, {});
	for (auto transition : transitions) {
		// итератор по описанию перехода
		auto it = transition.children().begin();
		// имя нетерминала
		std::string nonterminal = first_child(it);
		while (std::string(it->kind().name()) != "alternative") {
			it++;
		}
		rewriting_rules[nonterminal] = it;
	}

	std::ifstream t(filename);
	if (!t) {
		t.close();
		t.open("test_data/tmp/" + filename);
		if (!t) {
			std::cerr << "failed to open " + filename;
			exit(1);
		}
	}
	std::stringstream buffer;
	buffer << t.rdbuf();
	file = buffer.str();
	read_symbols(0);

	if (!parse_transition("production")) {
		throw(std::runtime_error("Parser: error occurred while parsing FA"));
	}

	if (!attributes.count("initial_set")) {
		throw(std::runtime_error("Parser: initial state is not set"));
	}

	for (const auto& transition : FAtransitions) {
		states.insert(transition.beg);
		states.insert(transition.end);
	}

	if (attributes.count("MFA")) {
		vector<MFAState> MFAstates;
		map<std::string, int> name_to_ind;
		int cnt = 0;
		for (const auto& state : states) {
			std::string name = state;
			if (labels.count(state))
				name = labels[state];
			MFAstates.emplace_back(cnt, name, final_states.count(state));
			name_to_ind[state] = cnt++;
		}

		Alphabet alphabet;

		for (const auto& transition : FAtransitions) {
			MFAstates[name_to_ind[transition.beg]].add_transition(
				MFATransition(name_to_ind[transition.end],
							  transition.open,
							  transition.close,
							  transition.reset),
				transition.symbol);
			if (!transition.symbol.is_epsilon() && !transition.symbol.is_ref())
				alphabet.insert(transition.symbol);
		}

		auto mfa = MemoryFiniteAutomaton(name_to_ind[initial], MFAstates, alphabet);

		if (!mfa.check_memory_correctness()) {
			throw(std::runtime_error("Parser: incorrect memory usage in MFA"));
		}

		return mfa;
	}

	if (attributes.count("NFA") || attributes.count("DFA")) {
		vector<FAState> FAstates;
		map<std::string, int> name_to_ind;
		int cnt = 0;
		for (const auto& state : states) {
			std::string name = state;
			if (labels.count(state))
				name = labels[state];
			FAstates.emplace_back(cnt, name, final_states.count(state));
			name_to_ind[state] = cnt++;
		}

		Alphabet alphabet;

		for (const auto& transition : FAtransitions) {
			FAstates[name_to_ind[transition.beg]].add_transition(name_to_ind[transition.end],
																 transition.symbol);
			if (!transition.symbol.is_epsilon())
				alphabet.insert(transition.symbol);
		}

		auto fa = FiniteAutomaton(name_to_ind[initial], FAstates, alphabet);

		if (attributes.count("DFA") && !fa.is_deterministic()) {
			throw(std::runtime_error("Parser: FA expected to be deterministic"));
		}

		return fa;
	}
}

FiniteAutomaton Parser::parse_NFA(const std::string& automaton_file,
								  const std::string& grammar_file) {
	lexy_ascii_tree grammar;

	auto file = lexy::read_file<lexy::ascii_encoding>(grammar_file.c_str());
	auto input = file.buffer();
	Lexer::parse_buffer(grammar, input);
	auto res = parse(grammar, automaton_file);

	if (attributes.count("DFA") || attributes.count("NFA"))
		return std::get<FiniteAutomaton>(res);
	throw(std::runtime_error("Parse: parsed automaton is not NFA"));
}

FiniteAutomaton Parser::parse_DFA(const std::string& automaton_file,
								  const std::string& grammar_file) {
	lexy_ascii_tree grammar;

	auto file = lexy::read_file<lexy::ascii_encoding>(grammar_file.c_str());
	auto input = file.buffer();
	Lexer::parse_buffer(grammar, input);

	auto res = parse(grammar, automaton_file);

	if (attributes.count("DFA"))
		return std::get<FiniteAutomaton>(res);
	throw(std::runtime_error("Parse: parsed automaton is not DFA"));
}

MemoryFiniteAutomaton Parser::parse_MFA(const std::string& automaton_file,
										const std::string& grammar_file) {
	lexy_ascii_tree grammar;

	auto file = lexy::read_file<lexy::ascii_encoding>(grammar_file.c_str());
	auto input = file.buffer();
	Lexer::parse_buffer(grammar, input);

	auto res = parse(grammar, automaton_file);

	if (attributes.count("MFA"))
		return std::get<MemoryFiniteAutomaton>(res);
	throw(std::runtime_error("Parse: parsed automaton is not MFA"));
}