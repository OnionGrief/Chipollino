#pragma once
#include "AbstractMachine.h"
using namespace std;

class Language;

class MemoryFiniteAutomaton : public AbstractMachine {
  public:
	MemoryFiniteAutomaton() = default;
	MemoryFiniteAutomaton(const MemoryFiniteAutomaton& other);

	// dynamic_cast unique_ptr к типу MemoryFiniteAutomaton*
	template <typename T> MemoryFiniteAutomaton* cast(unique_ptr<T>&& uptr);
	// визуализация автомата
	string to_txt() const override;
};
