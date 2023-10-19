#pragma once
#include "AbstractMachine.h"
using namespace std;

class Language;

class MemoryFiniteAutomaton : public AbstractMachine {
  public:
	MemoryFiniteAutomaton() = default;
	MemoryFiniteAutomaton(const MemoryFiniteAutomaton& other);

	template <typename T>
	MemoryFiniteAutomaton* castToMFA(unique_ptr<T>&& uptr);
	// визуализация автомата
	string to_txt() const override;
};
