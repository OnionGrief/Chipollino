#include "Objects/AbstractMachine.h"

AbstractMachine::AbstractMachine(int initial_state) : initial_state(initial_state) {}

AbstractMachine::AbstractMachine(int initial_state, std::shared_ptr<Language> language)
	: BaseObject(language), initial_state(initial_state) {}

AbstractMachine::AbstractMachine(int initial_state, set<alphabet_symbol> alphabet)
	: BaseObject(alphabet), initial_state(initial_state) {}
