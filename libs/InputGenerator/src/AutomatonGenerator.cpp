#include "InputGenerator/AutomatonGenerator.h"

using std::ofstream;
using std::string;
using std::to_string;
using std::rand;
using std::vector;
using std::set;

void AutomatonGenerator::change_seed() {
	seed_it++;
	srand((size_t)time(nullptr) + seed_it + rand());
}

bool AutomatonGenerator::dice_throwing(int percentage) {
    change_seed();
    if (percentage > rand() % 100) {
        return true;
    }

    return false;
}

void AutomatonGenerator::generate_graph() {
    int initial = 0, states_number = 10;
    int max_edges_number = states_number * (states_number - 1) / 2 + 3;
    int edges_number = rand() % (max_edges_number - states_number + 1) + states_number - 1;

    graph.resize(states_number);

    std::vector<int> included_states, excluded_states;

    included_states.push_back(initial);
    for (int i = 1; i < states_number; i++) {
        excluded_states.push_back(i);
    }

    for (int i = 0; i < edges_number; i++) {
        FAtransition cur;
        int beg = included_states[rand() % included_states.size()];
        cur.end = included_states[rand() % included_states.size()];
        if (!excluded_states.empty()) {
            int ind = rand() % excluded_states.size();
            cur.end = excluded_states[ind];
            included_states.push_back(excluded_states[ind]);
            excluded_states.erase(excluded_states.begin() + ind);
        }
        cur.symbol = generate_symbol();
        cur.pop = "$";

        graph[beg].push_back(cur);
    }
}

void AutomatonGenerator::generate_alphabet(int max_alphabet_size) {
    change_seed();
    max_alphabet_size = max_alphabet_size > 52 ? 52 : max_alphabet_size;
    int alphabet_size = 0;
    if (max_alphabet_size)
        alphabet_size = rand() % max_alphabet_size + 1;

    for (char i = 'a'; i < 'a' + alphabet_size && i <= 'z'; i++) {
        alphabet.push_back(i);
    }
    for (char i = 'A'; i < 'A' + alphabet_size - 26 && i <= 'Z'; i++) {
        alphabet.push_back(i);
    }
}

AutomatonGenerator::AutomatonGenerator(FA_type type) {
    
}

void AutomatonGenerator::write_to_file(string filename) {
    ofstream out;
	out.open(filename, ofstream::trunc);
	if (out.is_open())
		out << output.str();
	out.close();
}

void AutomatonGenerator::set_terminal_probability(int elem) {
    AutomatonGeneratorConstants::terminal_probability = elem;
}

void AutomatonGenerator::set_initial_state_not_terminal(bool f)
{
    AutomatonGeneratorConstants::initial_state_not_terminal = f;
}

vector<lexy_ascii_child> AutomatonGenerator::find_children(lexy_ascii_tree& tree, const set<string>& names,
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

bool AutomatonGenerator::parse_reserved(std::string res_case) {
    // std::cout << "parse_reserved: " << res_case << "\n";
    if (res_case == "EPS")
        return true;

    if (res_case == "LETTER") {
        if (!LETTER.empty()) {
            output << " " << LETTER.front();
            return true;
        }
        return false;
    }
    if (res_case == "DIGIT") {
        if (!DIGIT.empty()) {
            output << " " << LETTER.front();
            return true;
        }
        return false;
    }
    if (res_case == "STRING") {
        if (!STRING.empty()) {
            output << " " << LETTER.front();
            return true;
        }
        return false;
    }
    if (res_case == "NUMBER") {
        if (!NUMBER.empty()) {
            output << " " << LETTER.front();
            return true;
        }
        return false;
    }
    return false;
}

bool AutomatonGenerator::parse_nonterminal(lexy::_pt_node<lexy::_bra, void> ref) {
    return parse_transition(first_child(ref));
}

bool AutomatonGenerator::parse_terminal(lexy::_pt_node<lexy::_bra, void> ref) {
    // std::cout << "parsing terminal\n";
    auto it = ref.children().begin();
    it++;
    std::string to_read = lexy::as_string<string, lexy::ascii_encoding>(it->token().lexeme());
    // std::cout << "\n" << to_read << " " << cur_pos << " " << file.size() << "\n";
    // std::cout << file.substr(cur_pos, to_read.size()) << "\n";
    auto res = (!TERMINAL.empty() && TERMINAL.front() == to_read);
    if (res) {
        output << TERMINAL.front();
        TERMINAL.pop();
    }
    // std::cout << "terminal? --> " << res << "\n";
    return res;
}

void AutomatonGenerator::parse_attribute(lexy::_pt_node<lexy::_bra, void> ref) {
    // std::cout << "\nparsing attribute\n";
    auto it = ref.children().begin();
    while(std::string(it->kind().name()) != "nonterminal")
        it++;
    attributes.insert(first_child(it));
}

bool AutomatonGenerator::parse_alternative(lexy::_pt_node<lexy::_bra, void> ref) {
    // std::cout << "Enter alternative\n";
    bool read = true;
    for (auto it = ref.children().begin(); it != ref.children().end(); it++) {
        // std::cout << "tring to parse: " << it->kind().name() << "\n";
        if (it->kind().is_token() && lexy::as_string<string, lexy::ascii_encoding>(it->token().lexeme()) == "|") {
            if (read)
                return true;
            read = true;
            continue;
        }
        if (!read) {
            // std::cout << "Oops... read failed\n";
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
        // std::cout << "terminal?\n";
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

bool AutomatonGenerator::parse_transition(std::string name) {
    // std::cout << name << "\n";

    if (!rewriting_rules.count(name)) {
        return false;
    }

    if (!parse_func.count(name)) {
        auto transition = rewriting_rules[name];
        return parse_alternative(*transition);  
    }

    return parse_func[name]();
}