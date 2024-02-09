#include <AutomataParser/Parser.h>

std::vector<lexy_ascii_child> Parser::find_children(lexy_ascii_tree& tree, std::set<std::string> names, std::set<std::string> exclude) {
    std::vector<lexy_ascii_child> result;
    int skip = 0;
    for (auto [event, node] : tree.traverse())
    {
        auto depth = 0;
        switch (event)
        {
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

std::string first_child(lexy::_pt_node<lexy::_bra, void>::children_range::iterator it) {
    return lexy::as_string<std::string, lexy::ascii_encoding>(it->children().begin()->token().lexeme());
}

std::string first_child(lexy::_pt_node<lexy::_bra, void> it) {
    return lexy::as_string<std::string, lexy::ascii_encoding>(it.children().begin()->token().lexeme());
}

void Parser::parse_states(lexy_ascii_tree& tree, std::set<std::string>& names, std::map<std::string, std::string>& labels) {
    auto nodes = find_children(tree, {"node_id"}, {"state_label"});
    for (int i = 0; i < nodes.size(); i++) {
        auto name = first_child(nodes[i]);

        names.insert(name);
        labels[name] = name;
    }
}

void Parser::parse_descriptions(lexy_ascii_tree& tree, std::map<std::string, std::string>& labels, std::map<std::string, bool>& is_terminal, std::string& initial) {
    auto descriptions = find_children(tree, {"state_description"});
    for (int i = 0; i < descriptions.size(); i++) {
        std::string name = "";
        for (auto desc : descriptions[i].children()) {
            if (std::string(desc.kind().name()) == "node_id") {
                name = first_child(desc);
            }
            if (std::string(desc.kind().name()) == "state_label") {
                for (auto alias_child : desc.children()) {
                    if (std::string(alias_child.kind().name()) == "node_id") {
                        labels[name] = first_child(alias_child);
                    }
                }
            }
            if (std::string(desc.kind().name()) == "terminal_mark") {
                is_terminal[name] = true;
            }
            if (std::string(desc.kind().name()) == "initial_mark") {
                if (initial == "") {
                    initial = name;
                } else {
                    throw std::runtime_error("AutomataParser::Parser::parse_descriptions ERROR(second initial state found)");
                }
            }
        }
    }
}

void Parser::parse_transitions(lexy_ascii_tree& tree, std::vector<std::string>& beg, std::vector<std::string>& end, std::vector<std::string>& symb) {
    auto transitions = find_children(tree, {"stmt"});
    for (int i = 0; i < transitions.size(); i++) {
        auto it = transitions[i].children().begin();
        beg.push_back(first_child(it));
        it++;
        end.push_back(first_child(it));
        it++;
        symb.push_back(first_child(it));
    }
}

void Parser::parse_MFA_transitions(lexy_ascii_tree& tree, std::vector<Parser::MFATransition_info>& transitions) {
    auto edges = find_children(tree, {"MFA_edge"});
    std::vector<MFATransition_info> mfat_info;
    for (int i = 0; i < edges.size(); i++) {
        auto edge_child = edges[i].children().begin();
        auto it = edge_child->children().begin();
        std::string beg = first_child(it);
        it++;
        std::string end = first_child(it);
        it++;
        Symbol symb = first_child(it);
        
        if (symb == "&") {
            for (auto symb_child : it->children()) {
                if (std::string(symb_child.kind().name()) == "cell_id") {
                    symb = Symbol::Ref(std::stoi(first_child(symb_child)));
                }
            }
        }
        if (symb == "eps")
            symb = Symbol::Epsilon;

        std::unordered_set<int> open;
        std::unordered_set<int> close;
        edge_child++;
        for (auto memory_cell : edge_child->children()) {
            if (std::string(memory_cell.kind().name()) == "memory_cell") {
                auto cell = memory_cell.children().begin();
                int cell_id = std::stoi(first_child(cell));
                cell++;
                if (first_child(cell) == "c")
                    close.insert(cell_id);
                if (first_child(cell) == "o")
                    open.insert(cell_id);
            }
        }

        mfat_info.push_back({beg, end, symb, open, close});
    }

    transitions = mfat_info;
}

MemoryFiniteAutomaton Parser::parse_MFA(std::string filename) {
    lexy_ascii_tree tree;
    auto file = lexy::read_file<lexy::ascii_encoding>(filename.c_str());
    auto input = file.buffer();

    Lexer::parse_buffer(tree, input);

    std::set<std::string> names;
    std::map<std::string, std::string> labels;
    std::map<std::string, bool> is_terminal;
    std::string initial = "";

    parse_states(tree, names, labels);
    parse_descriptions(tree, labels, is_terminal, initial);

    std::vector<MFAState> states;
    std::map<std::string, int> states_id;

    int k = 0;
    int initial_state = 0;
    for (auto name : names) {
        if (name == initial)
            initial_state = k;
        states_id[name] = k;
        states.push_back(MFAState(k++, labels[name], is_terminal[name]));
    }

    std::set<Symbol> alphabet;

    std::vector<MFATransition_info> mfat_info;
    parse_MFA_transitions(tree, mfat_info);

    for (int i = 0; i < mfat_info.size(); i++) {
        if (!mfat_info[i].symb.is_epsilon() && !mfat_info[i].symb.is_ref()) {
            alphabet.insert(mfat_info[i].symb);
        }

        auto x = MFATransition(states_id[mfat_info[i].end], mfat_info[i].open, mfat_info[i].close);
        states[states_id[mfat_info[i].beg]].set_transition(x, mfat_info[i].symb);
    }

    auto mfa = MemoryFiniteAutomaton(initial_state, states, alphabet);

    return mfa;
}

FiniteAutomaton Parser::parse_FA(std::string filename) {
    lexy_ascii_tree tree;
    auto file = lexy::read_file<lexy::ascii_encoding>(filename.c_str());
    auto input = file.buffer();

    Lexer::parse_buffer(tree, input);

    std::set<std::string> names;
    std::map<std::string, std::string> labels;
    std::map<std::string, bool> is_terminal;
    std::string initial = "";

    parse_states(tree, names, labels);
    parse_descriptions(tree, labels, is_terminal, initial);

    std::vector<FAState> states;
    std::map<std::string, int> states_id;

    int k = 0;
    int initial_state = 0;
    for (auto name : names) {
        if (name == initial)
            initial_state = k;
        states_id[name] = k;
        states.push_back(FAState(k++, labels[name], is_terminal[name]));
    }

    std::set<Symbol> alphabet;
    std::vector<std::string> beg, end, symb;

    parse_transitions(tree, beg, end, symb);

    for (int i = 0; i < beg.size(); i++) {
        if (symb[i] == "eps") {
            states[states_id[beg[i]]].set_transition(states_id[end[i]], Symbol::Epsilon);
        } else {
            if (symb[i][0] == '&') {
                throw std::runtime_error("AutomataParser::Parser::parse_FA ERROR(MFA transition found)");
            } else {
                states[states_id[beg[i]]].set_transition(states_id[end[i]], symb[i]);
                alphabet.insert(Symbol(symb[i]));
            }
        }
    }

    auto fa = FiniteAutomaton(initial_state, states, alphabet);

    return fa;
}