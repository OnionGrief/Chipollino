#pragma once

#include "BaseObject.h"
#include "iLogTemplate.h"

class AbstractMachine : public BaseObject {
  protected:
	int initial_state = 0;

  public:
	AbstractMachine(int initial_state = 0);
	AbstractMachine(int initial_state, shared_ptr<Language>);
	AbstractMachine(int initial_state, set<alphabet_symbol>);
};