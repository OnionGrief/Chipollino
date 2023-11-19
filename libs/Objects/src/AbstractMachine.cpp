#include <utility>

#include "Objects/AbstractMachine.h"

using std::set;
using std::string;

AbstractMachine::State::State() : index(0), is_terminal(false) {}

AbstractMachine::State::State(int index, string identifier, bool is_terminal)
	: index(index), identifier(std::move(identifier)), is_terminal(is_terminal) {}

AbstractMachine::AbstractMachine(int initial_state, std::shared_ptr<Language> language)
	: BaseObject(std::move(language)), initial_state(initial_state) {}

AbstractMachine::AbstractMachine(int initial_state, set<alphabet_symbol> alphabet)
	: BaseObject(std::move(alphabet)), initial_state(initial_state) {}
