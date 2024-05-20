#pragma once
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <stack>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "AbstractMachine.h"
#include "MemoryCommon.h"
#include "iLogTemplate.h"

class Regex;
class MetaInfo;
class Language;
class MFAState;
class TransformationMonoid;
class MemoryFiniteAutomaton;

class FAState : public State {
  public:
	using Transitions = std::map<Symbol, std::set<int>>;

	Transitions transitions;
	// используется для объединения состояний в процессе работы алгоритмов
	// преобразования автоматов возможно для визуализации
	std::set<int> label;

	FAState() = default;
	FAState(int index, bool is_terminal);
	FAState(int index, std::string identifier, bool is_terminal);
	FAState(int index, bool is_terminal, Transitions transitions);
	FAState(int index, std::string identifier, bool is_terminal, Transitions transitions);
	FAState(int index, std::set<int> label, std::string identifier, bool is_terminal);
	FAState(int index, std::set<int> label, std::string identifier, bool is_terminal,
			Transitions transitions);
	// принимает алфавит, чтобы дополнить его символами переходов
	explicit FAState(const MFAState& state, Alphabet& alphabet); // NOLINT(runtime/references)

	std::string to_txt() const override;
	void add_transition(int, const Symbol&);
};

// TODO если меняешь структуру, поменяй FA_model в TransformationMonoid.h
class FiniteAutomaton : public AbstractMachine {
  public:
	enum AmbiguityValue {
		exponentially_ambiguous,
		almost_unambigious,
		unambigious,
		polynomially_ambigious
	};

  private:
	std::vector<FAState> states;

	// Если режим isTrim включён (т.е. по умолчанию), то на всех подозрительных
	// преобразованиях всегда удаляем в конце ловушки.
	// Если isTrim = false, тогда после удаления ловушки в результате
	// преобразований добавляем её обратно
	//	bool is_trim = true;

	void dfs(int index, std::set<int>& reachable, // NOLINT(runtime/references)
			 bool use_epsilons_only) const;

	// поиск множества состояний НКА, достижимых из множества состояний по
	// eps-переходам (если флаг установлен в 0 - по всем переходам)
	std::set<int> closure(const std::set<int>&, bool) const;
	static bool equality_checker(const FiniteAutomaton& fa1, const FiniteAutomaton& fa2);
	// дополнительно возвращает в векторах номера классов состояний каждого автомата
	static std::pair<bool, std::vector<std::vector<int>>> bisimilarity_checker(
		const FiniteAutomaton& fa1, const FiniteAutomaton& fa2);
	// принимает в качестве лимита максимальное количество цифр в
	// числителе + знаменателе дроби, которая может встретиться при вычислениях
	AmbiguityValue get_ambiguity_value(
		int digits_number_limit,
		std::optional<int>& word_length) // NOLINT(runtime/references)
		const;
	std::optional<bool> get_nfa_minimality_value() const;

	// поиск префикса из состояния state_beg в состояние state_end
	std::optional<std::string> get_prefix(
		int state_beg, int state_end, std::map<int, bool>& was) const; // NOLINT(runtime/references)

	// функция проверки на семантическую детерминированность
	bool semdet_entry(bool annoted = false, iLogTemplate* log = nullptr) const;

	// меняет местами состояние под индексом 0 с начальным
	// используется в томпсоне
	void set_initial_state_to_zero();

	std::vector<FAState::Transitions> get_reversed_transitions() const;

	void fill_order(int state_index, std::vector<bool>& visited, // NOLINT(runtime/references)
					std::stack<int>& order						 // NOLINT(runtime/references)
	);
	// возвращает компоненты сильной связности
	std::vector<std::unordered_set<int>> get_SCCs();

	FiniteAutomaton get_subautomaton(const CaptureGroup&);

  public:
	FiniteAutomaton();
	FiniteAutomaton(int initial_state, std::vector<FAState> states,
					std::shared_ptr<Language> language);
	FiniteAutomaton(int initial_state, std::vector<FAState> states, Alphabet alphabet);
	FiniteAutomaton(const FiniteAutomaton& other);

	// dynamic_cast unique_ptr к типу FiniteAutomaton*
	template <typename T> static FiniteAutomaton* cast(std::unique_ptr<T>&& uptr);
	// визуализация автомата
	std::string to_txt() const override;

	std::vector<FAState> get_states() const;
	size_t size(iLogTemplate* log = nullptr) const override;

	// детерминизация ДКА
	FiniteAutomaton determinize(bool is_trim = false, iLogTemplate* log = nullptr) const;
	// удаление недостижимых из начального состояний
	// TODO: на самом деле должен быть приватным, но почему-то понадобился ТМ
	FiniteAutomaton remove_unreachable_states() const;
	// удаление eps-переходов (построение eps-замыканий)
	FiniteAutomaton remove_eps(iLogTemplate* log = nullptr) const;
	// удаление eps-переходов (доп. вариант)
	FiniteAutomaton remove_eps_additional(iLogTemplate* log = nullptr) const;
	// минимизация ДКА (по Майхиллу-Нероуда)
	FiniteAutomaton minimize(bool is_trim = false, iLogTemplate* log = nullptr) const;
	// пересечение НКА (на выходе - автомат, распознающий слова пересечения
	// языков L1 и L2)
	static FiniteAutomaton intersection(const FiniteAutomaton&, const FiniteAutomaton&,
										iLogTemplate* log = nullptr); // меняет язык
	// объединение НКА (на выходе - автомат, распознающий слова объединения
	// языков L1 и L2)
	static FiniteAutomaton uunion(const FiniteAutomaton&, const FiniteAutomaton&,
								  iLogTemplate* log = nullptr); // меняет язык
	// разность НКА (на выходе - автомат, распознающий слова разности языков L1
	// и L2)
	static FiniteAutomaton difference(const FiniteAutomaton&, const FiniteAutomaton&,
									  iLogTemplate* log = nullptr); // меняет язык
	// дополнение ДКА (на выходе - автомат, распознающий язык L' = Σ* - L)
	FiniteAutomaton complement(iLogTemplate* log = nullptr) const; // меняет язык
	// обращение НКА (на выходе - автомат, распознающий язык, обратный к L)
	FiniteAutomaton reverse(iLogTemplate* log = nullptr) const; // меняет язык
	// добавление ловушки в ДКА
	// (нетерминальное состояние с переходами только в себя)
	FiniteAutomaton add_trap_state(iLogTemplate* log = nullptr) const;
	// удаление ловушек
	FiniteAutomaton remove_trap_states(iLogTemplate* log = nullptr) const;
	// навешивание разметки на все буквы в автомате, стоящие на
	// недетерминированных переходах (если ветвление содержит eps-переходы, то
	// eps размечаются как буквы). ДКА не меняется
	FiniteAutomaton annote(iLogTemplate* log = nullptr) const;
	// снятие разметки с букв
	FiniteAutomaton deannote(iLogTemplate* log = nullptr) const;
	FiniteAutomaton delinearize(iLogTemplate* log = nullptr) const;
	// объединение эквивалентных классов (принимает на вход вектор размера
	// states.size()) i-й элемент хранит номер класса i-го состояния
	FiniteAutomaton merge_equivalent_classes(std::vector<int>) const;
	// объединение эквивалентных по бисимуляции состояний
	FiniteAutomaton merge_bisimilar(iLogTemplate* log = nullptr) const;
	// проверка автоматов на эквивалентность
	static bool equivalent(const FiniteAutomaton&, const FiniteAutomaton&,
						   iLogTemplate* log = nullptr);
	// проверка автоматов на равенство(буквальное)
	static bool equal(const FiniteAutomaton&, const FiniteAutomaton&, iLogTemplate* log = nullptr);
	// проверка автоматов на бисимилярность
	static bool bisimilar(const FiniteAutomaton&, const FiniteAutomaton&,
						  iLogTemplate* log = nullptr);
	bool is_deterministic(iLogTemplate* log = nullptr) const override;
	// проверка НКА на семантический детерминизм
	bool semdet(iLogTemplate* log = nullptr) const;
	std::pair<int, bool> parse(const std::string&) const override;
	// проверка автоматов на вложенность (проверяет вложен ли аргумент в this)
	bool subset(const FiniteAutomaton&, iLogTemplate* log = nullptr) const;
	// определяет меру неоднозначности
	AmbiguityValue ambiguity(iLogTemplate* log = nullptr) const;
	// проверка на детерминированность методом орбит Брюггеманн-Вуда
	bool is_one_unambiguous(iLogTemplate* log = nullptr) const;
	// проверка на пустоту
	bool is_empty() const;
	// проверка автомата на финальность
	bool is_finite() const;
	// метод Arden
	Regex to_regex(iLogTemplate* log = nullptr) const;
	// возвращает число диагональных классов по методу Глейстера-Шаллита
	int get_classes_number_GlaisterShallit(iLogTemplate* log = nullptr) const;
	// построение синтаксического моноида по автомату
	TransformationMonoid get_syntactic_monoid() const;
	// проверка на минимальность для нка
	std::optional<bool> is_nfa_minimal(iLogTemplate* log = nullptr) const;
	// проверка на минимальность для дка
	bool is_dfa_minimal(iLogTemplate* log = nullptr) const;
	MemoryFiniteAutomaton to_mfa() const;

	friend class Regex;
	friend class MemoryFiniteAutomaton;
	friend class MetaInfo;
	friend class RLGrammar;
	friend class PrefixGrammar;
};