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

void AutomatonGenerator::add_terminality() {
    std::vector<int> unreachable;
    for (int i = 0; i < states_number; i++)
        if (!finaity_coloring[i])
            unreachable.push_back(i);

    while (!unreachable.empty()) {
        unreachable.clear();
        for (int i = 0; i < states_number; i++)
            if (!finaity_coloring[i])
                unreachable.push_back(i);
        int vertex = unreachable[rand() % unreachable.size()];
        stateDescriptions[vertex].terminal = true;
        std::queue<int> bfs;
        bfs.push(vertex);
        while (!bfs.empty()) {
            finaity_coloring[vertex] = 1;
            for (int i = 0; i < regraph[bfs.front()].size(); i++) {
                if (!finaity_coloring[regraph[bfs.front()][i]])
                    bfs.push(regraph[bfs.front()][i]);
            }
            bfs.pop();
        }
    }
    for (int i = 0; i < states_number; i++) {
        if (!stateDescriptions[i].terminal && dice_throwing(terminal_probability))
            stateDescriptions[i].terminal = true;
    }
}

bool AutomatonGenerator::coloring_MFA_transition(int beg, FAtransition& trans, int color) {
    // red
    if (MFA_coloring[color][trans.end] == 1)
        return false;
    // set color memory cell idle
    trans.close.erase(color);
    // not yellow
    if (MFA_coloring[color][beg] != 2) {
        MFA_coloring[color][beg] = 1;
        trans.open.insert(color);
    }
    // closing color memory cell for all transitions from the end vertex
    for (auto t : graph[trans.end]) {
        t.close.insert(color);
    }
    // yellow
    MFA_coloring[color][trans.end] = 2;
    return true;
}

void AutomatonGenerator::generate_symbol(int beg, FAtransition& trans) {
    vector<int> possible_colors;
    for (int color = 0; color < colors; color++) {
        if (!trans.open.count(color) && MFA_coloring[color][beg] != 2)
            possible_colors.push_back(color);
    }

    if (dice_throwing(ref_probability) && !possible_colors.empty()) {
        trans.symbol = Symbol::Ref(possible_colors[rand() % possible_colors.size()]);
    } else {
        if (dice_throwing(epsilon_probability))
            trans.symbol = Symbol::Epsilon;
        else
            trans.symbol = alphabet[rand() % alphabet.size()];
    }
}

void AutomatonGenerator::generate_graph() {
    max_edges_number = states_number * (states_number - 1) / 2 + 3;
    edges_number = rand() % (max_edges_number - states_number + 1) + states_number - 1;
    colors_tries = edges_number;

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
        cur.pop = "$";

        graph[beg].push_back(cur);
    }

    for (int i = 0; i < states_number; i++) {
        stateDescriptions.emplace_back(i, 0, i == initial);
    }

    regraph.resize(states_number);
    for (int i = 0; i < states_number; i++) {
        for (int j = 0; j < graph[i].size(); j++) {
            regraph[graph[i][j].end].push_back(i);
        }
    }

    add_terminality();

    for (int i = 0; i < colors_tries; i++) {
        int color = rand() % colors;
        int beg = included_states[rand() % included_states.size()];
        while (graph[beg].empty())
            beg = included_states[rand() % included_states.size()];
        int transition_num = rand() % graph[beg].size();
        coloring_MFA_transition(beg, graph[beg][transition_num], color);
    }

    for (int i = 0; i < states_number; i++) {
        for (auto trans : graph[i]) {
            generate_symbol(i, trans);
        }
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
    return parse_transition(Parser::first_child(ref));
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
    attributes.insert(Parser::first_child(it));
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
            if (attributes.count(Parser::first_child(it->children().begin()))) {
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
            parse_reserved(Parser::first_child(it));
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

AutomatonGenerator::AutomatonGenerator(std::string grammar_file, FA_type type) {
    lexy_ascii_tree grammar;
    
    auto file = lexy::read_file<lexy::ascii_encoding>(grammar_file.c_str());
    auto input = file.buffer();
    Lexer::parse_buffer(grammar, input);

    auto transitions = Parser::find_children(grammar, {"transition"}, {});
    for (auto transition : transitions) {
        // итератор по описанию перехода
        auto it = transition.children().begin();
        // имя нетерминала
        std::string nonterminal = Parser::first_child(it);
        while (std::string(it->kind().name()) != "alternative") {
            it++;
        }
        rewriting_rules[nonterminal] = it;    
    }

    switch (type)
    {
    case FA_type::MFA:
        TERMINAL.push("MFA");
        break;
    case FA_type::NFA:
        TERMINAL.push("NFA");
        colors = 0;
        break;
    case FA_type::DFA:
        TERMINAL.push("DFA");
        colors = 0;
        epsilon_probability = 0;
        break;
    }

    generate_graph();
    parse_transition("production");
}