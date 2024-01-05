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
	State(int index, bool is_terminal);

	virtual std::string to_txt() const = 0;
	bool operator==(const State& other) const;
};

// TODO если меняешь структуру, поменяй FA_model в TransformationMonoid.h
class AbstractMachine : public BaseObject {
  protected:
	int initial_state = 0;

  public:
	AbstractMachine() = default;
	AbstractMachine(int initial_state, std::shared_ptr<Language>);
	AbstractMachine(int initial_state, std::set<Symbol>);
	
	// начальное состояние
	int get_initial() const;
	// возвращает количество состояний (метод States)
	virtual size_t size(iLogTemplate* log = nullptr) const = 0;

	// проверка автомата на детерминированность
	virtual bool is_deterministic(iLogTemplate* log = nullptr) const = 0;
};