#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "AbstractMachine.h"

class Language;

class MemoryFiniteAutomaton : public AbstractMachine {
  public:
	struct Transition {
		enum MemoryAction {
			// idle, ◇
			open,  // o
			close, // c
		};

		int to;
		std::unordered_map<int, MemoryAction> memory_actions;

		explicit Transition(int to);
		Transition(int to, const std::unordered_set<int>& opens);
		Transition(
			int to,
			std::pair<const std::unordered_set<int>&, const std::unordered_set<int>&> opens_closes);

		std::string get_actions_str() const;
	};

	using Transitions = std::unordered_map<Symbol, std::vector<Transition>, SymbolHasher>;

	struct State : AbstractMachine::State {
		Transitions transitions;
		State(int index, std::string identifier, bool is_terminal, Transitions transitions);
		void set_transition(const Transition&, const Symbol&);
	};

  private:
	std::vector<State> states;

  public:
	MemoryFiniteAutomaton();
	MemoryFiniteAutomaton(int initial_state, std::vector<State> states,
						  std::shared_ptr<Language> language);
	//	MemoryFiniteAutomaton(const MemoryFiniteAutomaton& other);

	// dynamic_cast unique_ptr к типу MemoryFiniteAutomaton*
	template <typename T> MemoryFiniteAutomaton* cast(std::unique_ptr<T>&& uptr);
	// визуализация автомата
	std::string to_txt(bool eps_is_empty = true) const override;
};
