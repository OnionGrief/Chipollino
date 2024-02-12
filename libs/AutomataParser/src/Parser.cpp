#include <AutomataParser/Parser.h>

using namespace std;

vector<lexy_ascii_child> Parser::find_children(lexy_ascii_tree& tree, set<string> names, set<string> exclude) {
    vector<lexy_ascii_child> result;
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

string Parser::first_child(lexy::_pt_node<lexy::_bra, void>::children_range::iterator it) {
    return lexy::as_string<string, lexy::ascii_encoding>(it->children().begin()->token().lexeme());
}

string Parser::first_child(lexy::_pt_node<lexy::_bra, void> it) {
    return lexy::as_string<string, lexy::ascii_encoding>(it.children().begin()->token().lexeme());
}

void Parser::parse_states(lexy_ascii_tree& tree, set<string>& names, map<string, string>& labels) {
    auto nodes = find_children(tree, {AutomataParser::node_id}, {AutomataParser::state_label});
    for (int i = 0; i < nodes.size(); i++) {
        auto name = first_child(nodes[i]);

        names.insert(name);
        labels[name] = name;
    }
}

void Parser::parse_descriptions(lexy_ascii_tree& tree, map<string, string>& labels, map<string, bool>& is_terminal, string& initial) {
    auto descriptions = find_children(tree, {AutomataParser::state_description});
    for (int i = 0; i < descriptions.size(); i++) {
        string name = "";
        for (auto desc : descriptions[i].children()) {
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
            if (string(desc.kind().name()) == AutomataParser::initial_state) {
                if (initial == "") {
                    initial = name;
                } else {
                    throw runtime_error("AutomataParser::Parser::parse_descriptions ERROR(second initial state found)");
                }
            }
        }
    }
}

void Parser::parse_FA_transitions(lexy_ascii_tree& tree, vector<FATransition_info>& trans) {
    auto transitions = find_children(tree, {AutomataParser::statement});
    for (int i = 0; i < transitions.size(); i++) {
        auto it = transitions[i].children().begin();
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

void Parser::parse_MFA_transitions(lexy_ascii_tree& tree, vector<Parser::MFATransition_info>& transitions) {
    auto edges = find_children(tree, {AutomataParser::MFA_edge});
    vector<MFATransition_info> mfat_info;
    for (int i = 0; i < edges.size(); i++) {
        auto edge_child = edges[i].children().begin();
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

        ;
        mfat_info.emplace_back(beg, end, symb, open, close);
    }

    transitions = mfat_info;
}

MemoryFiniteAutomaton Parser::parse_MFA(string filename) {
    lexy_ascii_tree tree;
    auto file = lexy::read_file<lexy::ascii_encoding>(filename.c_str());
    auto input = file.buffer();

    Lexer::parse_buffer(tree, input);

    set<string> names;
    map<string, string> labels;
    map<string, bool> is_terminal;
    string initial = "";

    parse_states(tree, names, labels);
    parse_descriptions(tree, labels, is_terminal, initial);

    vector<MFAState> states;
    map<string, int> states_id;

    int k = 0;
    int initial_state = 0;
    for (auto name : names) {
        if (name == initial)
            initial_state = k;
        states_id[name] = k;
        states.push_back(MFAState(k++, labels[name], is_terminal[name]));
    }

    set<Symbol> alphabet;

    vector<MFATransition_info> mfat_info;
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

FiniteAutomaton Parser::parse_FA(string filename) {
    lexy_ascii_tree tree;
    auto file = lexy::read_file<lexy::ascii_encoding>(filename.c_str());
    auto input = file.buffer();

    Lexer::parse_buffer(tree, input);

    set<string> names;
    map<string, string> labels;
    map<string, bool> is_terminal;
    string initial = "";

    parse_states(tree, names, labels);
    parse_descriptions(tree, labels, is_terminal, initial);

    vector<FAState> states;
    map<string, int> states_id;

    int k = 0;
    int initial_state = 0;
    for (auto name : names) {
        if (name == initial)
            initial_state = k;
        states_id[name] = k;
        states.push_back(FAState(k++, labels[name], is_terminal[name]));
    }

    set<Symbol> alphabet;
    vector<FATransition_info> trans;

    parse_FA_transitions(tree, trans);

    for (int i = 0; i < trans.size(); i++) {
        if (trans[i].symb == AutomataParser::epsilon) {
            states[states_id[trans[i].beg]].set_transition(states_id[trans[i].end], Symbol::Epsilon);
        } else {
            states[states_id[trans[i].beg]].set_transition(states_id[trans[i].end], trans[i].symb);
            alphabet.insert(Symbol(trans[i].symb));
        }
    }

    auto fa = FiniteAutomaton(initial_state, states, alphabet);

    return fa;
}