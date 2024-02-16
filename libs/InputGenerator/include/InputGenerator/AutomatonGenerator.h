#pragma once
#include <string>
#include <sstream>
#include <ctime>
#include <optional>
#include <vector>
#include <fstream>
#include <map>
#include <set>

enum class FA_type { MFA, FA };

class AutomatonGenerator {
private:
    struct MFA_edge {
        int ref;
        int to;

        std::set<int> open;
        std::set<int> close;
    };

    int seed_it, memory_cells_number;
    std::vector<char> alphabet;
    std::map<int, std::set<int>> open_cells;
    std::map<int, std::vector<MFA_edge>> MFA_graph;

    void calculate_open_cells(int state, std::map<int, std::set<int>>& _open_cells); // NOLINT(runtime/references)
    bool is_MFA_transition_legal(int state, MFA_edge edge);
    void add_MFA_transition(int state, MFA_edge edge);

    void change_seed();

	// возвращает true с вероятностью percentage%
	bool dice_throwing(int percentage);

    std::stringstream output;

    void generate_alphabet(int max_alphabet_size);

    void generate_states_description(int states_number);

    void generate_transitions(int transitions_number, int states_number, FA_type type);

public:
    explicit AutomatonGenerator(FA_type type = FA_type::FA);

    void write_to_file(std::string filename);
};