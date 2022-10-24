#pragma once
#include "AlphabetSymbol.h"
#include "BaseObject.h"
#include <iostream>
#include <map>
#include <set>
#include <stack>
#include <string>
#include <vector>
using namespace std;

class Regex;
class Language;

struct State {
	int index;
	// используется для объединения состояний в процессе работы алгоритмов
	// преобразования автоматов возможно для визуализации
	vector<int> label;
	string identifier;
	bool is_terminal;
	map<alphabet_symbol, vector<int>> transitions;
	State();
	State(int index, vector<int> label, string identifier, bool is_terminal,
		  map<alphabet_symbol, vector<int>> transitions);
	void set_transition(int, alphabet_symbol);
};

class FiniteAutomaton : public BaseObject {
  private:
	int initial_state = 0;
	Language* language = nullptr;
	vector<State> states;
	bool is_deterministic = 0;
	int max_index; // max индекс в автомате "q11" => 11

	// поиск множества состояний НКА, достижимых из множества состояний по
	// eps-переходам (если флаг установлен в 0 - по всем переходам)
	vector<int> closure(const vector<int>&, bool) const;

	enum AmbiguityValue {
		exponentially_ambiguous,
		unambigious,
		polynomially_ambigious
	};

  public:
	FiniteAutomaton();
	FiniteAutomaton(int initial_state, Language* language, vector<State> states,
					bool is_deterministic = false);
	FiniteAutomaton(const FiniteAutomaton& other);
	// визуализация автомата
	string to_txt() const override;
	// детерминизация ДКА
	FiniteAutomaton determinize() const;
	// построение eps-замыкания
	FiniteAutomaton remove_eps() const;
	// минимизация ДКА (по Майхиллу-Нероуда)
	FiniteAutomaton minimize() const;
	// пересечение ДКА (на выходе - автомат, распознающий слова пересечения
	// языков L1 и L2)
	static FiniteAutomaton intersection(const FiniteAutomaton&,
										const FiniteAutomaton&);
	// объединение ДКА (на выходе - автомат, распознающий слова объединенеия
	// языков L1 и L2)
	static FiniteAutomaton uunion(const FiniteAutomaton&,
								  const FiniteAutomaton&);
	// разность ДКА (на выходе - автомат, распознающий слова разности языков L1
	// и L2)
	FiniteAutomaton difference(const FiniteAutomaton&) const;
	// дополнение ДКА (на выходе - автомат, распознающий язык L' = Σ* - L)
	FiniteAutomaton complement(Language*) const; // меняет язык
	// обращение НКА (на выходе - автомат, распознающий язык, обратный к L)
	FiniteAutomaton reverse(Language*) const; // меняет язык
	// добавление ловушки в ДКА(нетерминальное состояние с переходами только в
	// себя)
	FiniteAutomaton add_trap_state() const;
	// удаление ловушки
	FiniteAutomaton remove_trap_state() const;
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
	// проверка автоматов на вложенность (аргумент вложен в this)
	bool subset(const FiniteAutomaton&) const; // TODO
											   // и тд
	// определяет меру неоднозначности
	AmbiguityValue ambiguity() const;
	friend class Regex;
};