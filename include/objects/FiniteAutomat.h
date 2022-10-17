#pragma once
#include "BaseObject.h"
#include <iostream>
#include <map>
#include <set>
#include <stack>
#include <string>
#include <vector>
#include <optional>
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
	optional<bool> deterministic = nullopt;
	int initial_state = 0;
	vector<char> alphabet;
	vector<State> states;

  public:
	FiniteAutomat();
	FiniteAutomat(int initial_state, vector<char> alphabet,
				  vector<State> states, bool deterministic);
	// визуализация автомата
	string to_txt() override;
	// поиск множества состояний НКА, достижимых из множества состояний по
	// eps-переходам
	vector<int> closure(vector<int>);
	//
	void is_deterministic();
	// детерминизация ДКА
	FiniteAutomat determinize();
	// построение eps-замыкания
	FiniteAutomat remove_eps();
	// минимизация ДКА
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
	// и тд
};