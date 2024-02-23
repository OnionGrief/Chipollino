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
    while (cur_pos < file.size() && file[cur_pos] == ' ' || file[cur_pos] == '\n') {
        cur_pos++;
    }
}

bool Parser::parse_reserved(std::string res_case) {
    if (res_case == "EPS")
        return true;

    if (cur_pos == file.size())
        return false;

    if (res_case == "LETTER") {
        if (file[cur_pos] >= 'a' && file[cur_pos] <= 'z') {
            read_symbols(1);
            return true;
        }
        if (file[cur_pos] >= 'A' && file[cur_pos] <= 'Z') {
            read_symbols(1);
            return true;
        }
    }
    if (res_case == "DIGIT") {
        if (file[cur_pos] >= '0' && file[cur_pos] <= '9') {
            read_symbols(1);
            return true;
        }
    }
    int beg_pos = cur_pos;
    if (res_case == "STRING") {
        while ((file[cur_pos] >= 'a' && file[cur_pos] <= 'z') ||
         (file[cur_pos] >= 'A' && file[cur_pos] <= 'Z') ||
         (file[cur_pos] >= '0' && file[cur_pos] <= '9')) {
            cur_pos++;
        }
        if (beg_pos != cur_pos)
            STRING = file.substr(beg_pos, cur_pos - beg_pos);
        read_symbols(0);
        return (beg_pos != cur_pos);
    }
    if (res_case == "NUMBER") {
        if (file[cur_pos] == '0') {
            read_symbols(1);
            return true;
        }
        while (file[cur_pos] >= '0' && file[cur_pos] <= '9') {
            cur_pos++;
        }
        if (beg_pos != cur_pos)
            NUMBER = std::stoi(file.substr(beg_pos, cur_pos - beg_pos));
        read_symbols(0);
        return (beg_pos != cur_pos);
    }
}

bool Parser::parse_nonterminal(lexy::_pt_node<lexy::_bra, void> ref) {
    return parse_transition(first_child(ref));
}

bool Parser::parse_terminal(lexy::_pt_node<lexy::_bra, void> ref) {
    auto it = ref.children().begin();
    it++;
    std::string to_read = lexy::as_string<string, lexy::ascii_encoding>(it->token().lexeme());
    TERMINAL = to_read;
    return (file.size() - cur_pos > to_read.size() && file.substr(cur_pos, to_read.size()) == to_read);
}

void Parser::parse_attribute(lexy::_pt_node<lexy::_bra, void> ref) {
    auto it = ref.children().begin();
    it++;
    attributes.insert(lexy::as_string<string, lexy::ascii_encoding>(it->token().lexeme()));
}

bool Parser::parse_alternative(lexy::_pt_node<lexy::_bra, void> ref) {
    bool read = true;
    int beg_pos = cur_pos;
    for (auto it = ref.children().begin(); it != ref.children().end(); it++) {
        if (lexy::as_string<string, lexy::ascii_encoding>(it->token().lexeme()) == "|") {
            if (read)
                return true;
            cur_pos = beg_pos;
            read = true;
            continue;
        }
        if (!read)
            continue;
        if (lexy::as_string<string, lexy::ascii_encoding>(it->token().lexeme()) == "(") {
            it++;
            if (attributes.count(first_child(it->children().begin()))) {
                while (it->kind().name() != "alternative")
                    it++;
                parse_alternative(*it);
            }
            else {
                while (lexy::as_string<string, lexy::ascii_encoding>(it->token().lexeme()) != ":")
                    it++;

                while (it->kind().name() != "alternative")
                    it++;
                parse_alternative(*it);
            }
            while (lexy::as_string<string, lexy::ascii_encoding>(it->token().lexeme()) != ")")
                it++;
        }
        if (it->kind().name() == "terminal") {
            read &= parse_terminal(*it);
        }
        if (it->kind().name() == "nonterminal") {
            read &= parse_nonterminal(*it);
        }
        if (it->kind().name() == "reserved") {
            parse_reserved(first_child(it));
        }
        if (it->kind().name() == "attribute") {
            parse_attribute(*it);
        }
    }
}

bool Parser::parse_transition(std::string name) {
    if (!parse_func.count(name)) {
        auto transition = rewriting_rules[name];
        return parse_alternative(transition);  
    }

    return parse_func[name]();
}

bool Parser::parse(lexy_ascii_tree& grammar, std::string filename) {
    auto transitions = find_children(grammar, {"transition"}, {});
    for (auto transition : transitions) {
        // итератор по описанию перехода
        auto it = transition.children().begin();
        // имя нетерминала
        std::string nonterminal = first_child(it);
        // -->
        it++;
        // альтернатива
        it++;
        rewriting_rules[nonterminal] = *it;
    }

    return parse_transition("production");
}