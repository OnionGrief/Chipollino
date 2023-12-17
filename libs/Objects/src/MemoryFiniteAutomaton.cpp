#include <cassert>
#include <sstream>
#include <utility>

#include "Objects/Language.h"
#include "Objects/MemoryFiniteAutomaton.h"
#include "Objects/iLogTemplate.h"

using std::pair;
using std::set;
using std::string;
using std::stringstream;
using std::unordered_map;
using std::unordered_set;
using std::vector;

MFATransition::MFATransition(int to) : to(to) {}

MFATransition::MFATransition(int to, MemoryActions memory_actions)
	: to(to), memory_actions(std::move(memory_actions)) {}

MFATransition::MFATransition(int to, const unordered_set<int>& opens,
							 const unordered_set<int>& closes)
	: MFATransition(to) {
	for (auto num : opens)
		memory_actions[num] = MFATransition::open;
	for (auto num : closes) {
		if (memory_actions.count(num))
			std::cerr << "!!! Конфликт действий с ячейкой памяти !!!";
		memory_actions[num] = MFATransition::close;
	}
}

MFATransition::MFATransition(int to, const unordered_set<int>& destination_first,
							 const unordered_set<int>& iteration_over_cells,
							 const unordered_set<int>& source_last,
							 const unordered_set<int>& destination_in_cells)
	: MFATransition(to) {
	for (auto num : destination_first) {
		if (destination_in_cells.count(num) && !iteration_over_cells.count(num))
			continue;
		memory_actions[num] = MFATransition::open;
	}
	for (auto num : source_last) {
		if (destination_in_cells.count(num))
			continue;
		memory_actions[num] = MFATransition::close;
	}
}

bool MFATransition::operator==(const MFATransition& other) const {
	return (to == other.to) && (memory_actions == other.memory_actions);
}

std::size_t MFATransition::Hasher::operator()(const MFATransition& t) const {
	std::size_t result = std::hash<int>{}(t.to);
	for (const auto& pair : t.memory_actions) {
		result ^= std::hash<int>{}(pair.first) + std::hash<int>{}(static_cast<int>(pair.second));
	}
	return result;
}

MFAState::MFAState(bool is_terminal) : State::State(0, {}, is_terminal) {}

MFAState::MFAState(int index, std::string identifier, bool is_terminal)
	: State::State(index, std::move(identifier), is_terminal) {}

MFAState::MFAState(int index, std::string identifier, bool is_terminal,
				   MFAState::Transitions transitions)
	: State::State(index, std::move(identifier), is_terminal), transitions(std::move(transitions)) {
}

void MFAState::set_transition(const MFATransition& to, const Symbol& symbol) {
	transitions[symbol].insert(to);
}

string MFAState::to_txt() const {
	return {};
}

bool MFAState::operator==(const MFAState& other) const {
	return State::operator==(other) && transitions == other.transitions;
}

string MFATransition::get_actions_str() const {
	stringstream ss;
	unordered_set<int> opens;
	unordered_set<int> closes;

	for (const auto& [num, action] : memory_actions) {
		switch (action) {
		case MFATransition::open:
			opens.insert(num);
			break;
		case MFATransition::close:
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

MemoryFiniteAutomaton::MemoryFiniteAutomaton(int initial_state, vector<MFAState> states,
											 std::shared_ptr<Language> language)
	: AbstractMachine(initial_state, std::move(language)), states(std::move(states)) {
	for (int i = 0; i < this->states.size(); i++) {
		if (this->states[i].index != i)
			throw std::logic_error(
				"State.index must correspond to its ordinal number in the states vector");
	}
}

MemoryFiniteAutomaton::MemoryFiniteAutomaton(int initial_state, std::vector<MFAState> states,
											 set<Symbol> alphabet)
	: AbstractMachine(initial_state, std::move(alphabet)), states(std::move(states)) {
	for (int i = 0; i < this->states.size(); i++) {
		if (this->states[i].index != i)
			throw std::logic_error(
				"State.index must correspond to its ordinal number in the states vector");
	}
}

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

size_t MemoryFiniteAutomaton::size(iLogTemplate* log) const {
	return states.size();
}

std::vector<MFAState> MemoryFiniteAutomaton::get_states() const {
	return states;
}

bool MemoryFiniteAutomaton::is_deterministic(iLogTemplate* log) const {
	if (log) {
		//		log->set_parameter("oldautomaton", *this);
	}
	bool result = true;
	for (const auto& state : states) {
		for (const auto& [symbol, states_to] : state.transitions) {
			if ((symbol.is_epsilon() || symbol.is_ref()) && state.transitions.size() > 1) {
				result = false;
				break;
			}
			if (states_to.size() > 1) {
				result = false;
				break;
			}
		}
	}
	if (log) {
		log->set_parameter("result", result ? "True" : "False");
	}
	return result;
}

MemoryFiniteAutomaton MemoryFiniteAutomaton::add_trap_state(iLogTemplate* log) const {
	if (!is_deterministic())
		throw std::logic_error("add_trap_state: mfa must be deterministic");

	vector<MFAState> new_states = states;
	bool add_trap = false;
	MetaInfo new_meta;
	int count = static_cast<int>(size());
	for (auto& state : new_states) {
		for (const Symbol& symb : language->get_alphabet()) {
			if (state.transitions.size() == 1 && (state.transitions.begin()->first.is_epsilon() ||
												  state.transitions.begin()->first.is_ref()))
				continue;
			if (!state.transitions.count(symb)) {
				state.set_transition(MFATransition(count), symb);
				new_meta.upd(EdgeMeta{state.index, count, symb, MetaInfo::trap_color});
				add_trap = true;
			}
		}
	}

	if (add_trap) {
		new_states.emplace_back(count, "", false);
		for (const Symbol& symb : language->get_alphabet()) {
			new_states[count].set_transition(MFATransition(count), symb);
			new_meta.upd(EdgeMeta{count, count, symb, MetaInfo::trap_color});
		}
	}

	MemoryFiniteAutomaton new_mfa(initial_state, new_states, language);
	if (log) {
		//		log->set_parameter("oldautomaton", *this);
		//		log->set_parameter("result", new_mfa, new_meta);
	}
	return new_mfa;
}

MemoryFiniteAutomaton MemoryFiniteAutomaton::get_just_one_total_trap(
	const std::shared_ptr<Language>& language) {
	vector<MFAState> states;
	states.emplace_back(0, "", false);
	for (const Symbol& symb : language->get_alphabet()) {
		states[0].set_transition(MFATransition(0), symb);
	}

	return {0, states, language};
}

MemoryFiniteAutomaton MemoryFiniteAutomaton::complement(iLogTemplate* log) const {
	if (!is_deterministic())
		throw std::logic_error("add_trap_state: automaton must be deterministic");

	MemoryFiniteAutomaton new_mfa(initial_state, states, language->get_alphabet());
	new_mfa = new_mfa.add_trap_state();
	int final_states_counter = 0;
	for (int i = 0; i < new_mfa.size(); i++) {
		new_mfa.states[i].is_terminal = !new_mfa.states[i].is_terminal;
		if (new_mfa.states[i].is_terminal)
			final_states_counter++;
	}
	if (!final_states_counter)
		new_mfa = MemoryFiniteAutomaton::get_just_one_total_trap(new_mfa.language);

	if (log) {
		//		log->set_parameter("oldautomaton", *this);
		//		log->set_parameter("result", new_mfa);
	}
	return new_mfa;
}
