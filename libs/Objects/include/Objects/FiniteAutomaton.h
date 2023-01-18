#pragma once
#include "AlphabetSymbol.h"
#include "BaseObject.h"
#include <functional>
#include <iostream>
#include <map>
#include <math.h>
#include <optional>
#include <set>
#include <stack>
#include <string>
#include <vector>
using namespace std;

class Regex;
class Language;
class Grammar;
class TransformationMonoid;

struct State {
	int index;
	// используется для объединения состояний в процессе работы алгоритмов
	// преобразования автоматов возможно для визуализации
	set<int> label;
	string identifier;
	bool is_terminal;
	map<alphabet_symbol, set<int>> transitions;
	State();
	State(int index, set<int> label, string identifier, bool is_terminal,
		  map<alphabet_symbol, set<int>> transitions);
	void set_transition(int, const alphabet_symbol&);
};

class FiniteAutomaton : public BaseObject {
  public:
	enum AmbiguityValue {
		exponentially_ambiguous,
		almost_unambigious,
		unambigious,
		polynomially_ambigious
	};

  private:
	int initial_state = 0;
	vector<State> states;

	// Если режим isTrim включён (т.е. по умолчанию), то на всех подозрительных
	// преобразованиях всегда удаляем в конце ловушки.
	// Если isTrim = false, тогда после удаления ловушки в результате
	// преобразований добавляем её обратно
	bool is_trim = true;

	bool parsing_nfa(const string&, int) const; // парсинг слова в нка
	bool parsing_nfa_for(const string&) const;

	// поиск множества состояний НКА, достижимых из множества состояний по
	// eps-переходам (если флаг установлен в 0 - по всем переходам)
	set<int> closure(const set<int>&, bool) const;
	// удаление недостижимых из начального состояний
	FiniteAutomaton remove_unreachable_states() const;
	static bool equality_checker(const FiniteAutomaton& fa1,
								 const FiniteAutomaton& fa2);
	static bool bisimilarity_checker(const FiniteAutomaton& fa1,
									 const FiniteAutomaton& fa2);
	// принимает в качетве лимита максимальное количество цифр в
	// числителе + знаменателе дроби, которая может встретиться при вычислениях
	AmbiguityValue get_ambiguity_value(int digits_number_limit,
									   optional<int>& word_length) const;
	optional<bool> get_nfa_minimality_value() const;

	// поиск префикса из состояния state_beg в состояние state_end
	std::optional<std::string> get_prefix(int state_beg, int state_end,
										  map<int, bool>& was) const;

	// функция проверки на семантическую детерминированность
	bool semdet_entry(bool annoted = false) const;

  public:
	FiniteAutomaton();
	FiniteAutomaton(int initial_state, vector<State> states,
					shared_ptr<Language> language);
	FiniteAutomaton(int initial_state, vector<State> states,
					set<alphabet_symbol> alphabet);
	FiniteAutomaton(const FiniteAutomaton& other);
	// визуализация автомата
	string to_txt() const override;
	// детерминизация ДКА
	FiniteAutomaton determinize(bool is_trim = true) const;
	// построение eps-замыкания
	FiniteAutomaton remove_eps() const;
	// минимизация ДКА (по Майхиллу-Нероуда)
	FiniteAutomaton minimize(bool is_trim = true) const;
	// пересечение НКА (на выходе - автомат, распознающий слова пересечения
	// языков L1 и L2)
	static FiniteAutomaton intersection(const FiniteAutomaton&,
										const FiniteAutomaton&); // меняет язык
	// объединение НКА (на выходе - автомат, распознающий слова объединенеия
	// языков L1 и L2)
	static FiniteAutomaton uunion(const FiniteAutomaton&,
								  const FiniteAutomaton&); // меняет язык
	// разность НКА (на выходе - автомат, распознающий слова разности языков L1
	// и L2)
	static FiniteAutomaton difference(const FiniteAutomaton&,
									  const FiniteAutomaton&); // меняет язык
	// дополнение ДКА (на выходе - автомат, распознающий язык L' = Σ* - L)
	FiniteAutomaton complement() const; // меняет язык
	// обращение НКА (на выходе - автомат, распознающий язык, обратный к L)
	FiniteAutomaton reverse() const; // меняет язык
	// добавление ловушки в ДКА(нетерминальное состояние с переходами только в
	// себя)
	FiniteAutomaton add_trap_state() const;
	// удаление ловушек
	FiniteAutomaton remove_trap_states() const;
	// навешивание разметки на все буквы в автомате, стоящие на
	// недетерминированных переходах (если ветвление содержит eps-переходы, то
	// eps размечаются как буквы). ДКА не меняется
	FiniteAutomaton annote() const;
	// снятие разметки с букв
	FiniteAutomaton deannote() const;
	FiniteAutomaton delinearize() const;
	// объединение эквивалентных классов (принимает на вход вектор размера
	// states.size()) i-й элемент хранит номер класса i-го состояния
	FiniteAutomaton merge_equivalent_classes(vector<int>) const;
	// объединение эквивалентных по бисимуляции состояний
	FiniteAutomaton merge_bisimilar() const;
	// проверка автоматов на эквивалентность
	static bool equivalent(const FiniteAutomaton&,
						   const FiniteAutomaton&); // TODO
	// проверка автоматов на равентсво(буквальное)
	static bool equal(const FiniteAutomaton&, const FiniteAutomaton&);
	// проверка автоматов на бисимилярность
	static bool bisimilar(const FiniteAutomaton&, const FiniteAutomaton&);
	// проверка автомата на детерминированность
	bool is_deterministic() const;
	// проверка НКА на семантический детерминизм
	bool semdet() const;
	// проверяет, распознаёт ли автомат слово
	bool parsing_by_nfa(const string&) const;
	// проверка автоматов на вложенность (проверяет вложен ли аргумент в this)
	bool subset(const FiniteAutomaton&) const; // TODO
											   // и тд
	// начальное состояние
	int get_initial();
	// определяет меру неоднозначности
	AmbiguityValue ambiguity() const;
	// проверка на детерминированность методом орбит Брюггеманн-Вуда
	bool is_one_unambiguous() const;
	// возвращает количество состояний (пердикат States)
	int states_number() const;
	// метод Arden
	Regex to_regex() const;
	// возвращает число диагональных классов по методу Глейстера-Шаллита
	int get_classes_number_GlaisterShallit() const;
	// построение синтаксического моноида по автомату
	TransformationMonoid get_syntactic_monoid() const;
	// проверка на минимальность для нка
	optional<bool> is_nfa_minimal() const;
	// проверка на минимальность для дка
	bool is_dfa_minimal() const;

	friend class Regex;
	friend class TransformationMonoid;
	friend class Grammar;
};