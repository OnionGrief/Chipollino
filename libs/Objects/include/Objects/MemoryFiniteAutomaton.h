#pragma once
#include <memory>
#include <set>
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
	MFATransition(int, const std::unordered_set<int>&, const std::unordered_set<int>&,
				  const std::unordered_set<int>&, const std::unordered_set<int>&,
				  const std::unordered_set<int>&);

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
	MFAState(int index, std::string identifier, bool is_terminal);
	MFAState(int index, std::string identifier, bool is_terminal, Transitions transitions);

	std::string to_txt() const override;
	void set_transition(const MFATransition&, const Symbol&);
	bool operator==(const MFAState& other) const;
};

struct ParingState {
	int pos;
	const MFAState* state;
	std::unordered_set<int> opened_cells;
	std::unordered_map<int, std::pair<int, int>> memory; // значение - начало и конец подстроки

	ParingState(int pos, const MFAState* state, std::unordered_set<int>& opened_cells,
				std::unordered_map<int, std::pair<int, int>>& memory);
};

class Matcher {
  protected:
	const std::string* s;

  public:
	Matcher(const std::string&);

	virtual void match(
		const ParingState&, MFAState::Transitions&,		 // NOLINT(runtime/references)
		std::vector<std::tuple<Symbol, int, int>>&) = 0; // NOLINT(runtime/references)
};

class MemoryFiniteAutomaton : public AbstractMachine {
  private:
	std::vector<MFAState> states;

	static MemoryFiniteAutomaton get_just_one_total_trap(const std::shared_ptr<Language>& language);

	std::pair<int, bool> _parse_by_mfa(const std::string&, Matcher*) const;

  public:
	MemoryFiniteAutomaton();
	MemoryFiniteAutomaton(int initial_state, std::vector<MFAState> states,
						  std::shared_ptr<Language> language);
	MemoryFiniteAutomaton(int initial_state, std::vector<MFAState> states,
						  std::set<Symbol> alphabet);

	// dynamic_cast unique_ptr к типу MemoryFiniteAutomaton*
	template <typename T> MemoryFiniteAutomaton* cast(std::unique_ptr<T>&& uptr);
	// визуализация автомата
	std::string to_txt() const override;

	std::vector<MFAState> get_states() const;
	size_t size(iLogTemplate* log = nullptr) const override;

	bool is_deterministic(iLogTemplate* log = nullptr) const override;
	// добавление ловушки
	// (нетерминальное состояние с переходами только в себя)
	MemoryFiniteAutomaton add_trap_state(iLogTemplate* log = nullptr) const;
	// дополнение ДКА (на выходе - автомат, распознающий язык L' = Σ* - L)
	MemoryFiniteAutomaton complement(iLogTemplate* log = nullptr) const; // меняет язык
	// проверяет, распознаёт ли автомат слово
	std::pair<int, bool> parse_by_mfa(const std::string&) const;
};
