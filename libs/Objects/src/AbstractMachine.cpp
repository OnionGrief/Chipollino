#include <utility>

#include "Objects/AbstractMachine.h"

using std::set;
using std::string;

State::State() : index(0), is_terminal(false) {}

State::State(int index, bool is_terminal)
	: index(index), identifier(""), is_terminal(is_terminal) {}

State::State(int index, string identifier, bool is_terminal)
	: index(index), identifier(std::move(identifier)), is_terminal(is_terminal) {}

bool State::operator==(const State& other) const {
	return index == other.index && identifier == other.identifier &&
		   is_terminal == other.is_terminal;
}

AbstractMachine::AbstractMachine(int initial_state, std::shared_ptr<Language> language)
	: BaseObject(std::move(language)), initial_state(initial_state) {}

AbstractMachine::AbstractMachine(int initial_state, Alphabet alphabet)
	: BaseObject(std::move(alphabet)), initial_state(initial_state) {}

int AbstractMachine::get_initial() const {
	return initial_state;
}
