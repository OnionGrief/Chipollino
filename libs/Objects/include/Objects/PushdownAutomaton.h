#pragma once
#include "AbstractMachine.h"
#include "FiniteAutomaton.h"
#include "Symbol.h"

#include <stack>
#include <unordered_map>
#include <unordered_set>

struct PDATransition {
	int to;
	Symbol input_symbol;
	Symbol pop;
	std::vector<Symbol> push;

	PDATransition(int to, const Symbol& input, const Symbol& pop, const std::vector<Symbol>& push);

	bool operator==(const PDATransition& other) const;

	struct Hasher {
		std::size_t operator()(const PDATransition& t) const;
	};
};

class PDAState : public State {
  public:
	using SymbolTransitions = std::unordered_set<PDATransition, PDATransition::Hasher>;
	using Transitions = std::unordered_map<Symbol, SymbolTransitions, Symbol::Hasher>;

	Transitions transitions;

	explicit PDAState(bool is_terminal);
	PDAState(int index, bool is_terminal);
	PDAState(int index, std::string identifier, bool is_terminal);
	PDAState(int index, std::string identifier, bool is_terminal, Transitions transitions);
	explicit PDAState(const FAState& state);

	std::string to_txt() const override;
	void set_transition(const PDATransition& to, const Symbol& input_symbol);
};

struct ParsingState {
	int pos;
	const PDAState* state;
	std::stack<Symbol> stack;

	ParsingState(int pos, const PDAState* state, const std::stack<Symbol>& stack)
		: pos(pos), state(state), stack(stack){};
};

struct EqualityCheckerState {
	int index1, index2;
	std::unordered_map<Symbol, Symbol, Symbol::Hasher> stack_mapping;
	std::unordered_map<int, int> states_mapping;

	EqualityCheckerState() = default;
	EqualityCheckerState(int index1, int index2,
						 const std::unordered_map<Symbol, Symbol, Symbol::Hasher>& stack_mapping,
						 const std::unordered_map<int, int>& states_mapping)
		: index1(index1), index2(index2), stack_mapping(stack_mapping),
		  states_mapping(states_mapping) {}
	EqualityCheckerState& operator=(const EqualityCheckerState& other) {
		if (this == &other)
			return *this;
		index1 = other.index1;
		index2 = other.index2;
		stack_mapping = other.stack_mapping;
		states_mapping = other.states_mapping;
		return *this;
	}
	bool can_map_stack(const Symbol& s1, const Symbol& s2) {
		return stack_mapping.count(s1) == 0 || stack_mapping[s1] == s2;
	};
	void map_stack(const Symbol& s1, const Symbol& s2) {
		stack_mapping[s1] = s2;
	};
	bool can_map_states(const int i1, const int i2) {
		return states_mapping.count(i1) == 0 || states_mapping[i1] == i2;
	}
	void map_states(const int i1, const int i2) {
		states_mapping[i1] = i2;
	}
};

class PushdownAutomaton : public AbstractMachine {
  private:
	std::vector<PDAState> states;
	PushdownAutomaton _remove_unreachable_states(iLogTemplate* log);
	std::pair<MetaInfo, PushdownAutomaton> _add_trap_state(iLogTemplate* log);
	PushdownAutomaton _remove_bad_epsilon();
	PushdownAutomaton _add_trap_state();
	void _dfs(int index, std::unordered_set<int>& reachable) const;
	[[nodiscard]] std::unordered_map<PDATransition, std::unordered_set<int>, PDATransition::Hasher>
	closure(const int index) const;
	[[nodiscard]] std::pair<bool, EqualityCheckerState> _equality_dfs(
		const PushdownAutomaton& pda1, const PushdownAutomaton& pda2,
		EqualityCheckerState chst) const;
	static std::pair<bool, std::unordered_map<Symbol, Symbol, Symbol::Hasher>> _equality_checker(PushdownAutomaton pda1, PushdownAutomaton pda2);
	[[nodiscard]] std::unordered_map<int, std::unordered_set<PDATransition, PDATransition::Hasher>>
	_find_problematic_epsilon_transitions() const;
	[[nodiscard]] std::vector<std::pair<int, PDATransition>> _find_transitions_to(int index) const;
	[[nodiscard]] std::unordered_set<Symbol, Symbol::Hasher> _get_stack_symbols() const;

  public:
	PushdownAutomaton();
	PushdownAutomaton(int initial_state, std::vector<PDAState> states,
					  std::shared_ptr<Language> language);
	PushdownAutomaton(int initial_state, std::vector<PDAState> states, Alphabet alphabet);

	template <typename T> PushdownAutomaton* cast(std::unique_ptr<T>&& uptr);
	// визуализация автомата
	std::string to_txt() const override;

	std::vector<PDAState> get_states() const;
	size_t size(iLogTemplate* log = nullptr) const override;
	bool is_deterministic(iLogTemplate* log = nullptr) const override;
	// меняет язык
	PushdownAutomaton complement(iLogTemplate* log = nullptr) const;
	static bool equal(PushdownAutomaton pda1, PushdownAutomaton pda2, iLogTemplate* log);
	// пересечение с регуляркой
	PushdownAutomaton regular_intersect(const Regex& re, iLogTemplate* log);
	// проверяет, распознаёт ли автомат слово (использует BasicMatcher)
	std::pair<int, bool> parse(const std::string&) const override;
};
