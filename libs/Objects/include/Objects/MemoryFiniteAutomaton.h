#pragma once
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "AbstractMachine.h"
#include "iLogTemplate.h"

class Language;
class FAState;
struct PairHasher;

struct MFATransition {
	enum MemoryAction {
		// idle, ◇
		open,  // o
		close, // c
		reset, // r
	};

	using MemoryActions = std::unordered_map<int, MemoryAction>;

	int to;
	MemoryActions memory_actions;

	explicit MFATransition(int to);
	MFATransition(int, MemoryActions);
	MFATransition(int, const std::unordered_set<int>&, const std::unordered_set<int>&);
	MFATransition(int, const std::unordered_set<int>&, const std::unordered_set<int>&,
				  const std::unordered_set<int>&);

	struct TransitionConfig {
		// пары {номер ячейки, линеаризованный номер оператора}
		const std::unordered_set<std::pair<int, int>, PairHasher>* destination_first;
		const std::unordered_set<int>* source_in_cells;
		const std::unordered_set<int>* iteration_over_cells;
		// пары {номер ячейки, линеаризованный номер оператора}
		const std::unordered_set<std::pair<int, int>, PairHasher>* source_last;
		const std::unordered_set<int>* destination_in_cells;
		const std::unordered_set<int>* to_reset;
	};
	MFATransition(int, const TransitionConfig& config);

	std::string get_actions_str() const;
	bool operator==(const MFATransition& other) const;

	struct Hasher {
		std::size_t operator()(const MFATransition& t) const;
	};
};

class MFAState : public State {
  public:
	using SymbolTransitions = std::unordered_set<MFATransition, MFATransition::Hasher>;
	using Transitions = std::unordered_map<Symbol, SymbolTransitions, Symbol::Hasher>;

	Transitions transitions;
	explicit MFAState(bool is_terminal);
	MFAState(int index, std::string identifier, bool is_terminal);
	MFAState(int index, std::string identifier, bool is_terminal, Transitions transitions);
	explicit MFAState(const FAState& state);

	std::string to_txt() const override;
	void set_transition(const MFATransition&, const Symbol&);
	bool operator==(const MFAState& other) const;
};

// состояние идентифицирующее шаг парсинга по MFA
struct ParingState {
	int pos;
	const MFAState* state;
	std::unordered_set<int> opened_cells;
	std::unordered_map<int, std::pair<int, int>> memory; // значение - начало и конец подстроки

	ParingState(int pos, const MFAState* state, const std::unordered_set<int>& opened_cells,
				const std::unordered_map<int, std::pair<int, int>>& memory);
};

// переход дополняется информацией о состоянии памяти
struct ParseTransition : MFATransition {
	std::unordered_set<int> opened_cells;
	std::unordered_map<int, std::pair<int, int>> memory;

  private:
	// применяет действия над памятью
	void do_memory_actions(int pos);

  public:
	ParseTransition(const MFATransition& transition, const ParingState& parsing_state);
	// "записывает" символы в открытые ячейки
	void update_memory(const Symbol& symbol);
};

struct ParseTransitions {
	std::unordered_map<Symbol, std::unordered_set<ParseTransition, MFATransition::Hasher>,
					   Symbol::Hasher>
		transitions;

	void add_transitions(const Symbol& symbol,
						 const MFAState::SymbolTransitions& symbol_transitions,
						 const ParingState& parsing_state);

	std::unordered_map<Symbol, std::unordered_set<ParseTransition, MFATransition::Hasher>,
					   Symbol::Hasher>::const_iterator
	begin() const;

	std::unordered_map<Symbol, std::unordered_set<ParseTransition, MFATransition::Hasher>,
					   Symbol::Hasher>::const_iterator
	end() const;
};

// предоставляет метод для сопоставления набора содержимого ячеек памяти с позицией в строке
class Matcher {
  protected:
	// строка, в которой производится сопоставление
	const std::string* s;

  public:
	explicit Matcher(const std::string&);

	virtual void match(const ParingState&,
					   ParseTransitions&, // NOLINT(runtime/references)
										  // кортеж {символ-ссылка, левая граница подстроки, правая
										  // граница подстроки}
					   std::vector<std::tuple<Symbol, MFAState::SymbolTransitions, int,
											  int>>&) = 0; // NOLINT(runtime/references)
};

class MemoryFiniteAutomaton : public AbstractMachine {
  private:
	std::vector<MFAState> states;

	static MemoryFiniteAutomaton get_just_one_total_trap(const std::shared_ptr<Language>& language);

	std::pair<int, bool> _parse(const std::string&, Matcher*) const;

	// поиск множества состояний НКА,
	// достижимых из множества состояний по eps-переходам
	std::tuple<std::set<int>, std::unordered_set<int>, MFATransition::MemoryActions>
	get_eps_closure(const std::set<int>& indices) const;
	void dfs_by_eps(int, std::set<int>&, const int&, int&, // NOLINT(runtime/references)
					MFATransition::MemoryActions&) const; // NOLINT(runtime/references)

  public:
	MemoryFiniteAutomaton();
	MemoryFiniteAutomaton(int initial_state, std::vector<MFAState> states,
						  std::shared_ptr<Language> language);
	MemoryFiniteAutomaton(int initial_state, std::vector<MFAState> states, Alphabet alphabet);

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
	// удаление eps-переходов (построение eps-замыканий)
	MemoryFiniteAutomaton remove_eps(iLogTemplate* log = nullptr) const;
	// проверяет, распознаёт ли автомат слово (использует BasicMatcher)
	std::pair<int, bool> parse(const std::string&) const override;
	// проверяет, распознаёт ли автомат слово (использует FastMatcher)
	std::pair<int, bool> parse_additional(const std::string&) const;
};
