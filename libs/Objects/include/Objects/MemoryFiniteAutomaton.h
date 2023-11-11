#pragma once
#include "AbstractMachine.h"
#include <memory>
#include <string>

class Language;

class MemoryFiniteAutomaton : public AbstractMachine {
  public:
	MemoryFiniteAutomaton() = default;
	MemoryFiniteAutomaton(const MemoryFiniteAutomaton& other);

	// dynamic_cast unique_ptr к типу MemoryFiniteAutomaton*
	template <typename T> MemoryFiniteAutomaton* cast(std::unique_ptr<T>&& uptr);
	// визуализация автомата
	string to_txt() const override;
};
