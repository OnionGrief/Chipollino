#include "InputGenerator/AutomatonGenerator.h"

using std::ofstream;
using std::string;
using std::to_string;
using std::rand;

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

void AutomatonGenerator::generate_transitions(int transitions_number, int states_number, FA_type type) {
    for (int i = 0; i < transitions_number; i++) {
        change_seed();
        int beg = rand() % states_number;

        change_seed();
        int end = rand() % states_number;

        change_seed();
        string transition_sym;
        int transition_symbol_num = rand() % (alphabet.size() + memory_cells_number);
        if (transition_symbol_num < alphabet.size())
            transition_sym.push_back(alphabet[i]);
        else
            transition_sym = "&" + to_string(transition_symbol_num - alphabet.size());

        output << to_string(beg) << " " << to_string(end) << " " << transition_sym;
        
        if (type == FA_type::MFA) {
            // С вероятностью 50% открытие или закрытие ячейки
            for (int i = 0; i < memory_cells_number; i++) {
                if (dice_throwing(50)) {
                    output << " " << i;
                    if (dice_throwing(50)) {
                        output << " c";
                    } else {
                        output << " o";
                    }
                }
            }
        }

        if (i + 1 != transitions_number)
            output << ";\n";
        else
            output << "\n";
    }
}

void AutomatonGenerator::generate_states_description(int states_number) {
    change_seed();
    int initial = rand() % states_number;

    for (int i = 0; i < states_number; i++) {
        if (initial == i) {
            output << to_string(i) << " initial_state";

            // Пусть состояние терминально с вероятностью 20%
            change_seed();
            if (dice_throwing(GeneratorConstants::terminal_probability)) {
                output << " terminal";
            }

            output << "\n";
        } else {
            if (dice_throwing(GeneratorConstants::terminal_probability)) {
                output << to_string(i) << " terminal";
            }
        }
    }
    output << "...\n";
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

        // Пусть кол-во ячеек не больше 10;
        change_seed();
        memory_cells_number = rand() % GeneratorConstants::max_memory_cells_number;
    }

    change_seed();
    // Пусть в автомате будет не более 10 состояний
    int states_number = rand() % GeneratorConstants::max_states_number;

    generate_states_description(states_number);

    generate_alphabet(GeneratorConstants::alphabet_size);

    change_seed();
    int transitions_number = rand() % (states_number * (states_number - 1) / 2 +
     GeneratorConstants::additional_max_transitions_number);

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