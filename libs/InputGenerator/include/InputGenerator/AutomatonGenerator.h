#pragma once
#include <string>
#include <sstream>
#include <ctime>
#include <optional>
#include <vector>

class AutomatonGenerator {
public:
    enum class FA_type { MFA, FA };
private:
    int seed_it, memory_cells_number;
    std::vector<char> alphabet;

    void change_seed();

    std::stringstream output;

    void generate_alphabet(int max_alphabet_size);

    void generate_states_description(int states_number);

    void generate_transitions(int transitions_number, int states_number, FA_type type);

public:
    AutomatonGenerator(FA_type type = FA_type::FA);
};