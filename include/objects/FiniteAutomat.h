#pragma once
#include "BaseObject.h"
#include <string>
#include <vector>
#include <map>
#include <stack>
#include <set>
#include <iostream>
using namespace std;

class Regex;

struct State {
	int index;
	// используется для объединения состояний в процессе работы алгоритмов преобразования автоматов
	// возможно для визуализации
	vector<int> label;
	string identifier;
	bool is_terminal;
	map<char, vector<int>> transitions;
	State();
	State(int index, vector<int> label, string identifier, bool is_terminal, map<char, vector<int>> transitions);
	void set_transition(int, char);
};

class FiniteAutomat : public BaseObject {
private:
	bool is_deterministic = 0;
	int initial_state = 0;
	vector<char> alphabet;
	vector<State> states;
	int max_index;
public:
	FiniteAutomat();
	FiniteAutomat(int initial_state, vector<char> alphabet, vector<State> states, bool is_deterministic = false);
	// визуализация автомата
	string to_txt() override;
	// поиск множества состояний НКА, достижимых из множества состояний по eps-переходам
	vector<int> closure(vector<int>);
	// детерминизация ДКА
	FiniteAutomat determinize();
	// построение eps-замыкания
	FiniteAutomat remove_eps();
	// минимизация ДКА
	FiniteAutomat minimize();
	// пересечение ДКА (на выходе - автомат, распознающий слова пересечения языков L1 и L2)
	static FiniteAutomat intersection(const FiniteAutomat&, const FiniteAutomat&);
	// объединение ДКА (на выходе - автомат, распознающий слова объединенеия языков L1 и L2)
	static FiniteAutomat uunion(const FiniteAutomat&, const FiniteAutomat&);
	// разность ДКА (на выходе - автомат, распознающий слова разности языков L1 и L2)
	FiniteAutomat difference(const FiniteAutomat&);
	// дополнение ДКА (на выходе - автомат, распознающий язык L' = Σ* - L)
	FiniteAutomat complement();
	// и тд
	friend class Regex;
};