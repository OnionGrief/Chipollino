#pragma once
#include "BaseObject.h"
#include <iostream>
#include <map>
#include <set>
#include <stack>
#include <string>
#include <vector>
using namespace std;

struct State {
	int index;
	// используется для объединения состояний в процессе работы алгоритмов
	// преобразования автоматов возможно для визуализации
	vector<int> label;
	string identifier;
	bool is_terminal;
	map<char, vector<int>> transitions;
	State();
	State(int index, vector<int> label, string identifier, bool is_terminal,
		  map<char, vector<int>> transitions);
	void set_transition(int, char);
};

class FiniteAutomat : public BaseObject {
  private:
	bool is_deterministic = 0;
	int initial_state = 0;
	vector<char> alphabet;
	vector<State> states;

  public:
	FiniteAutomat();
	FiniteAutomat(int initial_state, vector<char> alphabet,
				  vector<State> states, bool is_deterministic = false);
	// визуализация автомата
	string to_txt() override;
	// поиск множества состояний НКА, достижимых из множества состояний по
	// eps-переходам (если флаг установлен в 1 - по всем переходам)
	vector<int> closure(vector<int>, bool);
	// детерминизация ДКА
	FiniteAutomat determinize();
	// построение eps-замыкания
	FiniteAutomat remove_eps();
	// минимизация ДКА (по Майхиллу-Нероуда)
	FiniteAutomat minimize();
	// пересечение ДКА (на выходе - автомат, распознающий слова пересечения
	// языков L1 и L2)
	static FiniteAutomat intersection(const FiniteAutomat&,
									  const FiniteAutomat&);
	// объединение ДКА (на выходе - автомат, распознающий слова объединенеия
	// языков L1 и L2)
	static FiniteAutomat uunion(const FiniteAutomat&, const FiniteAutomat&);
	// разность ДКА (на выходе - автомат, распознающий слова разности языков L1
	// и L2)
	FiniteAutomat difference(const FiniteAutomat&);
	// дополнение ДКА (на выходе - автомат, распознающий язык L' = Σ* - L)
	FiniteAutomat complement();
	// обращение НКА (на выходе - автомат, распознающий язык, обратный к L)
	FiniteAutomat reverse();
	// добавление ловушки в ДКА(нетерминальное состояние с переходами только в
	// себя)
	FiniteAutomat add_trap_state();
	// удаление ловушки
	FiniteAutomat remove_trap_state();
	// объединение эквивалентных по бисимуляции состояний
	FiniteAutomat merge_bisimilar();
	// проверка автоматов на эквивалентность
	static bool equivalent(const FiniteAutomat&, const FiniteAutomat&); //TODO
	// проверка автоматов на равентсво(буквальное)
	static bool equal(const FiniteAutomat&, const FiniteAutomat&);
	// проверка автоматов на бисимилярность
	static bool bisimilar(const FiniteAutomat&, const FiniteAutomat&);
	// проверка автоматов на вложенность (аргумент вложен в this) 
	bool subset(const FiniteAutomat&); //TODO
	// и тд
};