#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "AbstractMachine.h"

class Language;

struct MFATransition {
	enum MemoryAction {
		// idle, ◇
		open,  // o
		close, // c
	};

	int to;
	std::unordered_map<int, MemoryAction> memory_actions;

	explicit MFATransition(int to);
	MFATransition(int to, const std::unordered_set<int>& opens);
	MFATransition(
		int to,
		std::pair<const std::unordered_set<int>&, const std::unordered_set<int>&> opens_closes);

	std::string get_actions_str() const;
};

class MFAState : public State {
  public:
	using Transitions = std::unordered_map<Symbol, std::vector<MFATransition>, SymbolHasher>;

	Transitions transitions;
	explicit MFAState(int index, std::string identifier, bool is_terminal, Transitions transitions);
	void set_transition(const MFATransition&, const Symbol&);

	std::string to_txt() const override;
};

class MemoryFiniteAutomaton : public AbstractMachine {
  private:
	std::vector<MFAState> states;

  public:
	MemoryFiniteAutomaton();
	MemoryFiniteAutomaton(int initial_state, std::vector<MFAState> states,
						  std::shared_ptr<Language> language);
	//	MemoryFiniteAutomaton(const MemoryFiniteAutomaton& other);

	// dynamic_cast unique_ptr к типу MemoryFiniteAutomaton*
	template <typename T> MemoryFiniteAutomaton* cast(std::unique_ptr<T>&& uptr);
	// визуализация автомата
	std::string to_txt() const override;
};
