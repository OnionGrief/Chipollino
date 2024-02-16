#include "InputGenerator/AutomatonGenerator.h"

using std::ofstream;
using std::string;
using std::to_string;
using std::rand;

namespace AutomatonGeneratorConstants {
    int terminal_probability = 20;
    int max_memory_cells_number = 10;
    int max_states_number = 10;
    int alphabet_size = 52;
    // макс кол-во переходов = кол-во рёбер в полном графе + additional_max_transitions_number
    int additional_max_transitions_number = 10;
    // вероятность генерации открытия/закрытия для ячейки
    int action_probability = 50;
    // вероятность генерации закрытия для ячейки, при генерации взаимодействия
    int action_closing_probability = 50;
} // AutomatonGeneratorConstants

void AutomatonGenerator::change_seed() {
	seed_it++;
	srand((size_t)time(nullptr) + seed_it + rand());
}

bool AutomatonGenerator::dice_throwing(int percentage) {
    change_seed();
    if (percentage < rand() % 100) {
        return true;
    }

    return false;
}

void AutomatonGenerator::calculate_open_cells(int state, std::map<int, std::set<int>>& _open_cells) {
    std::set<int> states_to_check;
    states_to_check.insert(state);
    
    while (!states_to_check.empty()) {
        int cur_state = *states_to_check.begin();
        for (auto transition : MFA_graph[cur_state]) {
            for (auto cell : _open_cells[cur_state]) {
                // Если в текущем состоянии ячейка открыта для записи и переход её не закрывает
                bool pass = !transition.close.count(cell);
                // Если раньше в open_cells для состояния transition.to ячейка была закрыта, то это состояние надо проверить
                if (!_open_cells[transition.to].count(cell) && pass) {
                    _open_cells[transition.to].insert(cell);
                    states_to_check.insert(transition.to);
                }
            }
            // Для всех ячеек, открываемых переходом
            for (auto cell : transition.open) {
                // Если раньше в open_cells для состояния transition.to ячейка была закрыта, то это состояние надо проверить
                if (!_open_cells[transition.to].count(cell)) {
                    _open_cells[transition.to].insert(cell);
                    states_to_check.insert(transition.to);
                }
            }
        }
    }
}

bool AutomatonGenerator::is_MFA_transition_legal(int _state, MFA_edge edge) {
    // Чтение из открывающейся для записи ячейки
    if (edge.open.count(edge.ref))
        return false;

    auto _open_cells = open_cells;

    MFA_graph[_state].push_back(edge);

    calculate_open_cells(_state, _open_cells);

    for (auto state : _open_cells) {
        for (auto transition : MFA_graph[state.first]) {
            for (auto cell : state.second) {
                // Ячейка может быть открыта и она открывается повторно или по ней существует переход
                if (transition.open.count(cell) || transition.ref == cell) {
                    MFA_graph[_state].pop_back();
                    return false;
                }
            }
        }
    }

    MFA_graph[_state].pop_back();
    return true;    
}

void AutomatonGenerator::add_MFA_transition(int state, MFA_edge edge) {
    MFA_graph[state].push_back(edge);

    calculate_open_cells(state, open_cells);
}

void AutomatonGenerator::generate_transitions(int transitions_number, int states_number, FA_type type) {
    for (int i = 0; i < transitions_number; i++) {
        MFA_edge new_edge;

        change_seed();
        int beg = rand() % states_number;

        change_seed();
        int end = rand() % states_number;

        new_edge.to = end;

        change_seed();
        string transition_sym;
        bool generation_success = true;
        do {
            transition_sym = "";
            int transition_symbol_num = rand() % (alphabet.size() + memory_cells_number);
            if (transition_symbol_num < alphabet.size()) {
                transition_sym.push_back(alphabet[i]);
                new_edge.ref = -1;
            }
            else {
                transition_sym = "&" + to_string(transition_symbol_num - alphabet.size());
                new_edge.ref = transition_symbol_num - alphabet.size();
            }
            if (type == FA_type::MFA)
                generation_success = is_MFA_transition_legal(beg, new_edge);

            if (transition_symbol_num < alphabet.size() && !generation_success)
                break;
        } while (!generation_success);


        if (!generation_success)
            continue;

        output << to_string(beg) << " " << to_string(end) << " " << transition_sym;
        
        if (type == FA_type::MFA) {
			for (int j = 0; j < memory_cells_number; j++) {
				if (dice_throwing(AutomatonGeneratorConstants::action_probability)) {
					if (dice_throwing(AutomatonGeneratorConstants::action_closing_probability)) {
                        new_edge.close.insert(j);
                        output << " " << j << " c";
                    } else {
                        new_edge.open.insert(j);
                        if (is_MFA_transition_legal(beg, new_edge)) {
                            output << " " << j << " o";
                        } else {
                            new_edge.open.erase(j);
                        }
                    }
                }
            }
        }

        if (i + 1 != transitions_number)
            output << ";\n";
        else
            output << "\n";

        add_MFA_transition(beg, new_edge);
    }
}

void AutomatonGenerator::generate_states_description(int states_number) {
    change_seed();
    int initial = rand() % states_number;

    std::set<int> terminal_states;

    for (int i = 0; i < states_number; i++) {
        if (initial == i) {
            output << to_string(i) << " initial_state";

            change_seed();
            if (dice_throwing(AutomatonGeneratorConstants::terminal_probability)) {
                output << " terminal";
            }
        } else {
            if (dice_throwing(AutomatonGeneratorConstants::terminal_probability)) {
                terminal_states.insert(i);
            }
        }
    }

    for (auto state : terminal_states) {
        output << ";\n" << to_string(state) << " terminal";
    }

    output << "\n...\n";
}

void AutomatonGenerator::generate_alphabet(int max_alphabet_size) {
    change_seed();
    max_alphabet_size = max_alphabet_size > 52 ? 52 : max_alphabet_size;
    int alphabet_size = 0;
    if (max_alphabet_size)
        alphabet_size = rand() % max_alphabet_size;

    for (char i = 'a'; i < 'a' + alphabet_size && i <= 'z'; i++) {
        alphabet.push_back(i);
    }
    for (char i = 'A'; i < 'A' + alphabet_size - 26 && i <= 'Z'; i++) {
        alphabet.push_back(i);
    }
}

AutomatonGenerator::AutomatonGenerator(FA_type type) {
    if (type == FA_type::FA) {
        output << "FA {\n";
        memory_cells_number = 0;
    } else {
        output << "MFA {\n";

        change_seed();
        memory_cells_number = rand() % AutomatonGeneratorConstants::max_memory_cells_number;
    }

    change_seed();
    int states_number = rand() % AutomatonGeneratorConstants::max_states_number;

    generate_states_description(states_number);

    generate_alphabet(AutomatonGeneratorConstants::alphabet_size);

    change_seed();
    int transitions_number = rand() % (states_number * (states_number - 1) / 2 +
     AutomatonGeneratorConstants::additional_max_transitions_number);

    generate_transitions(transitions_number, states_number, type);

    output << "}";
}

void AutomatonGenerator::write_to_file(string filename) {
    ofstream out;
	out.open(filename, ofstream::trunc);
	if (out.is_open())
		out << output.str();
	out.close();
}