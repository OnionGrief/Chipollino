#pragma once
#include <string>
#include <sstream>
#include <ctime>
#include <optional>
#include <vector>
#include <fstream>

enum class FA_type { MFA, FA };

namespace GeneratorConstants {
    int terminal_probability = 20;
    int max_memory_cells_number = 10;
    int max_states_number = 10;
    int alphabet_size = 52;
    // макс кол-во переходов = кол-во рёбер в полно графе + additional_max_transitions_number
    int additional_max_transitions_number = 10;
} // GeneratorConstants

class AutomatonGenerator {
private:
    int seed_it, memory_cells_number;
    std::vector<char> alphabet;

    void change_seed();

    // возращает true с вероятностью percentage%
    bool dice_throwing(int percentage);

    std::stringstream output;

    void generate_alphabet(int max_alphabet_size);

    void generate_states_description(int states_number);

    void generate_transitions(int transitions_number, int states_number, FA_type type);

public:
    explicit AutomatonGenerator(FA_type type = FA_type::FA);

    void write_to_file(std::string filename);
};