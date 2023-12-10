#pragma once
#include <memory>
#include <set>
#include <string>

#include "BaseObject.h"
#include "iLogTemplate.h"

class State {
  public:
	int index;
	std::string identifier;
	bool is_terminal;
	State();
	State(int index, std::string identifier, bool is_terminal);

	virtual std::string to_txt() const = 0;
	bool operator==(const State& other) const;
};

class AbstractMachine : public BaseObject {
  protected:
	int initial_state = 0;

  public:
	AbstractMachine() = default;
	AbstractMachine(int initial_state, std::shared_ptr<Language>);
	AbstractMachine(int initial_state, std::set<Symbol>);
};