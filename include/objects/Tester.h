#pragma once
#include "BaseObject.h"
#include "FiniteAutomaton.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Tester {
  public:
	struct word {			// доступ извне Tester::word
		int iterations_num; // сколько проведено итераций
		long long time; // время парсинга в ms скорее всего
		bool is_belongs; // принадлежность языку
	};

	struct tableRegex {
		string r1; // язык, основанный на регулярке
		string r2; // слова порождаются регуляркой r2 и шагом итерации
		int step;  // шаг итерации
		vector<word> words; // таблица
	};

	struct tableFA {
		FiniteAutomaton automaton; // автомат // хз как логгировать, мб
								   // передавать еще строку (как объект N1, н-р)
		string r2; // слова порождаются регуляркой r2 и шагом итерации
		int step;  // шаг итерации
		vector<word> words1; // таблица по алгоритму с возвратами
		vector<word> words2; // таблица по параллельному алгоритму
	};

  private:
  public:
	static void test(FiniteAutomaton language, string regex, int iteration_step);
	static void test(string language, string regex, int iteration_step);
};