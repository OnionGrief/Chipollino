#pragma once
#include "BaseObject.h"
#include "FiniteAutomaton.h"
#include "Logger.h"
#include "Regex.h"
#include <chrono>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

using namespace std;
struct word {			// доступ извне Tester::word
	int iterations_num; // сколько проведено итераций
	long long time;		// время парсинга в секундах
	bool is_belongs;	// принадлежность языку
};

class Tester {
  public:
	// struct word {			// доступ извне Tester::word
	// 	int iterations_num; // сколько проведено итераций
	// 	long long time;		// время парсинга в секундах
	// 	bool is_belongs;	// принадлежность языку
	// };

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
	static bool parsing_by_regex(string, string);

  public:
	static void test(FiniteAutomaton language, string regex,
					 int iteration_step);
	static void test(string language, string regex, int iteration_step);
};