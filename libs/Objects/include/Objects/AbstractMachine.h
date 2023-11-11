#pragma once

#include <memory>
#include <set>
#include "BaseObject.h"
#include "iLogTemplate.h"

class AbstractMachine : public BaseObject {
  protected:
	int initial_state = 0;

  public:
	AbstractMachine(int initial_state = 0);	// NOLINT(runtime/explicit)
	AbstractMachine(int initial_state, std::shared_ptr<Language>);
	AbstractMachine(int initial_state, set<alphabet_symbol>);
};