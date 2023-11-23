#pragma once
#include <memory>
#include <set>
#include <string>

#include "BaseObject.h"
#include "iLogTemplate.h"

class AbstractMachine : public BaseObject {
  protected:
	struct State {
		int index;
		std::string identifier;
		bool is_terminal;
		State();
		State(int index, std::string identifier, bool is_terminal);
	};

	int initial_state = 0;

  public:
	AbstractMachine() = default;
	AbstractMachine(int initial_state, std::shared_ptr<Language>);
	AbstractMachine(int initial_state, std::set<Symbol>);
};