#include "AutomataParser/ParseTreeDescend.h"

using std::map;
using std::runtime_error;
using std::set;
using std::string;
using std::unordered_set;
using std::vector;

vector<lexy_ascii_child> ParseTreeDescend::find_children(lexy_ascii_tree& tree, const set<string>& names,
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

string ParseTreeDescend::first_child(lexy::_pt_node<lexy::_bra, void>::children_range::iterator it) {
	return lexy::as_string<string, lexy::ascii_encoding>(it->children().begin()->token().lexeme());
}

string ParseTreeDescend::first_child(lexy::_pt_node<lexy::_bra, void> it) {
	return lexy::as_string<string, lexy::ascii_encoding>(it.children().begin()->token().lexeme());
}

bool ParseTreeDescend::parse_nonterminal(lexy::_pt_node<lexy::_bra, void> ref) {
    return parse_transition(first_child(ref));
}

void ParseTreeDescend::parse_attribute(lexy::_pt_node<lexy::_bra, void> ref) {
    auto it = ref.children().begin();
    while(std::string(it->kind().name()) != "nonterminal")
        it++;
    attributes.insert(first_child(it));
}

bool ParseTreeDescend::parse_alternative(lexy::_pt_node<lexy::_bra, void> ref) {
    bool read = true;
    int beg_pos = cur_pos;
    for (auto it = ref.children().begin(); it != ref.children().end(); it++) {
        if (it->kind().is_token() && lexy::as_string<string, lexy::ascii_encoding>(it->token().lexeme()) == "|") {
            if (read)
                return true;
            cur_pos = beg_pos;
            read = true;
            continue;
        }
        if (!read) {
            continue;
        }
        if (it->kind().is_token() && lexy::as_string<string, lexy::ascii_encoding>(it->token().lexeme()) == "(") {
            it++;
            if (attributes.count(first_child(it->children().begin()))) {
                while (it->kind().is_token() || std::string(it->kind().name()) != "alternative")
                    it++;
                parse_alternative(*it);
            } else {
                while (!it->kind().is_token() || lexy::as_string<string, lexy::ascii_encoding>(it->token().lexeme()) != ":")
                    it++;

                while (it->kind().is_token() || std::string(it->kind().name()) != "alternative")
                    it++;
                parse_alternative(*it);
            }
            while (it->kind().is_token() && lexy::as_string<string, lexy::ascii_encoding>(it->token().lexeme()) != ")")
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

bool ParseTreeDescend::parse_transition(std::string name) {
    if (!rewriting_rules.count(name)) {
        return false;
    }

    if (!parse_func.count(name)) {
        auto transition = rewriting_rules[name];
        return parse_alternative(*transition);  
    }

    return parse_func[name]();
}