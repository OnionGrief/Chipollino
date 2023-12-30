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

	using MemoryActions = std::unordered_map<int, MemoryAction>;

	int to;
	MemoryActions memory_actions;

	explicit MFATransition(int to);
	MFATransition(int, MemoryActions);
	MFATransition(int, const std::unordered_set<int>&, const std::unordered_set<int>&);

	std::string get_actions_str() const;
	bool operator==(const MFATransition& other) const;

	struct Hasher {
		std::size_t operator()(const MFATransition& t) const;
	};
};

class MFAState : public State {
  public:
	using Transitions =
		std::unordered_map<Symbol, std::unordered_set<MFATransition, MFATransition::Hasher>,
						   Symbol::Hasher>;

	Transitions transitions;
	explicit MFAState(bool is_terminal);
	MFAState(int index, std::string identifier, bool is_terminal, Transitions transitions);

	std::string to_txt() const override;
	void set_transition(const MFATransition&, const Symbol&);
	bool operator==(const MFAState& other) const;
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
	std::string to_txt(bool eps_is_empty = true) const override;

	// возвращает количество состояний (метод States)
	size_t size(iLogTemplate* log = nullptr) const;
	std::vector<MFAState> get_states() const;
};
