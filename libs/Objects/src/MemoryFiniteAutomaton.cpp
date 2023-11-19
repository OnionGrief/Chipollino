#include <sstream>
#include <utility>

#include "Objects/MemoryFiniteAutomaton.h"

using std::pair;
using std::string;
using std::stringstream;
using std::unordered_map;
using std::unordered_set;
using std::vector;

MemoryFiniteAutomaton::State::State(int index, string identifier, bool is_terminal,
									Transitions transitions)
	: AbstractMachine::State::State(index, std::move(identifier), is_terminal),
	  transitions(std::move(transitions)) {}

void MemoryFiniteAutomaton::State::set_transition(const Transition& to,
												  const alphabet_symbol& symbol) {
	transitions[symbol].push_back(to);
}

MemoryFiniteAutomaton::Transition::Transition(int to) : to(to) {}

MemoryFiniteAutomaton::Transition::Transition(int to, const unordered_set<int>& opens) : to(to) {
	for (auto num : opens)
		memory_actions[num] = Transition::open;
}

MemoryFiniteAutomaton::Transition::Transition(
	int to, pair<const unordered_set<int>&, const unordered_set<int>&> opens_closes)
	: Transition(to, opens_closes.first) {
	for (auto num : opens_closes.second) {
		if (memory_actions.count(num))
			std::cerr << "!!! Конфликт действий с ячейкой памяти !!!";
		memory_actions[num] = Transition::close;
	}
}

string MemoryFiniteAutomaton::Transition::get_actions_str() const {
	stringstream ss;
	unordered_set<int> opens;
	unordered_set<int> closes;

	for (const auto& [num, action] : memory_actions) {
		switch (action) {
		case MemoryFiniteAutomaton::Transition::open:
			opens.insert(num);
			break;
		case MemoryFiniteAutomaton::Transition::close:
			closes.insert(num);
			break;
		}
	}

	size_t count = 0;
	char memory_actions_separator = ';';
	if (!opens.empty()) {
		ss << memory_actions_separator << " o: ";
		count = 0;
		for (int num : opens) {
			ss << num;
			if (++count < opens.size()) {
				ss << ", ";
			}
		}
	}

	if (!closes.empty()) {
		ss << memory_actions_separator << " c: ";
		count = 0;
		for (int num : closes) {
			ss << num;
			if (++count < closes.size()) {
				ss << ", ";
			}
		}
	}

	return ss.str();
}

MemoryFiniteAutomaton::MemoryFiniteAutomaton() : AbstractMachine() {}

MemoryFiniteAutomaton::MemoryFiniteAutomaton(int initial_state, vector<State> states,
											 std::shared_ptr<Language> language)
	: AbstractMachine(initial_state, std::move(language)), states(std::move(states)) {}

template <typename T>
MemoryFiniteAutomaton* MemoryFiniteAutomaton::cast(std::unique_ptr<T>&& uptr) {
	auto* mfa = dynamic_cast<MemoryFiniteAutomaton*>(uptr.get());
	if (!mfa) {
		throw std::runtime_error("Failed to cast to MemoryFiniteAutomaton");
	}

	return mfa;
}

string MemoryFiniteAutomaton::to_txt() const {
	stringstream ss;
	ss << "digraph {\n\trankdir = LR\n\tdummy [label = \"\", shape = none]\n\t";
	for (int i = 0; i < states.size(); i++) {
		ss << i << " [label = \"" << states[i].identifier << "\", shape = ";
		ss << (states[i].is_terminal ? "doublecircle]\n\t" : "circle]\n\t");
	}
	if (states.size() > initial_state)
		ss << "dummy -> " << states[initial_state].index << "\n";

	for (const auto& state : states) {
		for (const auto& elem : state.transitions) {
			for (const auto& transition : elem.second) {
				ss << "\t" << state.index << " -> " << transition.to << " [label = \""
				   << string(elem.first) << transition.get_actions_str() << "\"]\n";
			}
		}
	}

	ss << "}\n";
	return ss.str();
}