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
#include "MemoryCommon.h"
#include "iLogTemplate.h"

class Language;
class FAState;
class FiniteAutomaton;

class MFAState : public State {
  public:
	using SymbolTransitions = std::unordered_set<MFATransition, MFATransition::Hasher>;
	using Transitions = std::unordered_map<Symbol, SymbolTransitions, Symbol::Hasher>;

	Transitions transitions;
	explicit MFAState(bool is_terminal);
	MFAState(int index, std::string identifier, bool is_terminal);
	MFAState(int index, std::string identifier, bool is_terminal, Transitions transitions);

	bool operator==(const MFAState& other) const;
	std::string to_txt() const override;
	void add_transition(const MFATransition&, const Symbol&);
};

using MemoryConfiguration = std::unordered_set<int>;
using MemoryContents = std::unordered_map<int, std::pair<int, int>>;

// состояние идентифицирующее шаг парсинга по MFA
struct ParingState {
	int pos;
	const MFAState* state;
	MemoryConfiguration opened_cells;
	MemoryContents memory; // значение - начало и конец подстроки

	ParingState(int pos, const MFAState* state, const MemoryConfiguration& opened_cells,
				const MemoryContents& memory);
	bool operator==(const ParingState& other) const;

	struct Hasher {
		std::size_t operator()(const ParingState&) const;
	};
};

struct MutationHasher {
	std::size_t operator()(const std::tuple<int, int, int>& m) const;
};

// состояние идентифицирующее шаг обхода MFA
struct TraversalState {
	std::string str;
	const MFAState* state;
	MemoryConfiguration opened_cells;
	MemoryContents memory; // значение - начало и конец подстроки

	// последовательность посещенных состояний
	std::vector<int> visited_path;
	// {индекс состояния, {индекс в visited_path, размер строки на момент последнего посещения}}
	std::unordered_map<int, std::pair<int, int>> visited_states;
	int last_memory_reading = 0; // индекс в строке, на котором последний раз происходило чтение
	// номер(индекс) состояния и пара индексов, ограничивающих подстроку для мутации
	std::unordered_set<std::tuple<int, int, int>, MutationHasher> substrs_to_mutate;

	TraversalState() = default;
	explicit TraversalState(const MFAState* state);
	TraversalState(const std::string& str, const MFAState* state, MemoryConfiguration opened_cells,
				   std::unordered_map<int, std::pair<int, int>> memory,
				   const TraversalState& previous_state, bool memory_used = false);
	bool operator==(const TraversalState& other) const;

	void process_mutations();

	struct Hasher {
		std::size_t operator()(const TraversalState&) const;
	};
};

// переход дополняется информацией о состоянии памяти
struct ParseTransition : MFATransition {
	MemoryConfiguration opened_cells;
	MemoryContents memory;

  private:
	// применяет действия над памятью
	void do_memory_actions(int pos);

  public:
	ParseTransition(const MFATransition& transition, const ParingState& parsing_state);
	ParseTransition(const MFATransition& transition, const TraversalState& traversal_state);
	// "записывает" символы в открытые ячейки
	void update_memory_contents(const Symbol& symbol);
};

struct ParseTransitions {
	std::unordered_map<Symbol, std::unordered_set<ParseTransition, MFATransition::Hasher>,
					   Symbol::Hasher>
		transitions;

	void add_transitions(const Symbol& symbol,
						 const MFAState::SymbolTransitions& symbol_transitions,
						 const ParingState& parsing_state);

	void add_transitions(const Symbol& symbol,
						 const MFAState::SymbolTransitions& symbol_transitions,
						 const TraversalState& traversal_state);

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

	std::pair<int, bool> _parse_slow(const std::string&, Matcher*) const;
	std::pair<int, bool> _parse(const std::string&, Matcher*) const;

	// поиск множества состояний НКА,
	// достижимых из множества состояний по eps-переходам
	std::tuple<std::set<int>, std::unordered_set<int>, MFATransition::MemoryActions>
	get_eps_closure(const std::set<int>& indices) const;
	void dfs_by_eps(int, std::set<int>&, const int&, int&, // NOLINT(runtime/references)
					MFATransition::MemoryActions&) const;  // NOLINT(runtime/references)
	// раскрашивает состояния в соответствии с открытыми на момент их посещения ячейками памяти
	void color_mem_dfs(
		int state_index,
		std::vector<bool>& visited, // NOLINT(runtime/references)
		const MemoryConfiguration& opened_cells,
		std::vector<std::unordered_set<int>>& state_colors, // NOLINT(runtime/references)
		const std::vector<int>& ab_classes,
		std::unordered_map<int, int>& ab_class_to_first_state // NOLINT(runtime/references)
	) const;

	MemoryFiniteAutomaton get_subautomaton(const std::vector<int>& state_indexes,
										   int sub_initial_state) const;
	std::vector<MFAState::Transitions> get_reversed_transitions() const;

	std::pair<std::vector<std::vector<int>>, std::vector<std::vector<int>>> find_cg_paths(
		int state_index, std::unordered_set<int> visited, int cell, int opening_state) const;
	std::vector<CaptureGroup> find_capture_groups_backward(
		int ref_incoming_state, int cell,
		const std::vector<MFAState::Transitions>& reversed_transitions,
		const std::vector<int>& fa_classes) const;

	bool find_decisions(int state_index,
						std::vector<int>& visited, // NOLINT(runtime/references)
						const std::unordered_set<int>& states_to_check) const;
	bool states_have_decisions(const std::unordered_set<int>& states_to_check) const;

	FiniteAutomaton get_cg_fa(const CaptureGroup&) const;

	static std::optional<bool> bisimilarity_checker(const MemoryFiniteAutomaton&,
													const MemoryFiniteAutomaton&);

	// объединение эквивалентных классов (принимает на вход вектор размера states.size())
	// на i-й позиции номер класса i-го состояния
	std::tuple<MemoryFiniteAutomaton, std::unordered_map<int, int>> merge_equivalent_classes(
		const std::vector<int>&) const;

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
	// возвращает множество уникальных слов длины <= max_len, распознаваемых автоматом
	// и множество тестовых слов с мутациями
	std::pair<std::unordered_set<std::string>, std::unordered_set<std::string>> generate_test_set(
		int max_len) const;
	// ссылки считаются символами алфавита, операции над памятью игнорируются
	FiniteAutomaton to_action_fa(iLogTemplate* log = nullptr) const;
	// ссылки считаются символами алфавита, операции над памятью преобразуются в переходы Oi, Ci, Ri
	// (первые size() состояний - состояния исходного mfa)
	FiniteAutomaton to_symbolic_fa(iLogTemplate* log = nullptr) const;
	// проверка автоматов на равенство (буквальное + строгое равенство номеров ячеек)
	static bool equal(const MemoryFiniteAutomaton&, const MemoryFiniteAutomaton&,
					  iLogTemplate* log = nullptr);
	// проверка автоматов на бисимилярность
	static std::optional<bool> bisimilar(const MemoryFiniteAutomaton&, const MemoryFiniteAutomaton&,
										 iLogTemplate* log = nullptr);
	static bool action_bisimilar(const MemoryFiniteAutomaton&, const MemoryFiniteAutomaton&,
								 iLogTemplate* log = nullptr);
	static bool symbolic_bisimilar(const MemoryFiniteAutomaton&, const MemoryFiniteAutomaton&,
								   iLogTemplate* log = nullptr);
	// объединение эквивалентных по бисимуляции состояний
	MemoryFiniteAutomaton merge_bisimilar(iLogTemplate* log = nullptr) const;
};
