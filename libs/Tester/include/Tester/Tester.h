#pragma once
#include "Objects/BaseObject.h"
#include "Objects/FiniteAutomaton.h"
#include "Objects/Regex.h"
#include "Objects/iLogTemplate.h"
#include <chrono>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

using namespace std;

class Tester {
  public:
	struct word {			// доступ извне Tester::word
		int iterations_num; // сколько проведено итераций
		long long time;		// время парсинга в секундах
		bool is_belongs;	// принадлежность языку
	};

	struct table {
		string r2; // слова порождаются регуляркой r2 и шагом итерации
		int step;  // шаг итерации
		vector<word> words; // таблица
	};

  private:
	static bool parsing_by_regex(string, string);

  public:
	static void test(const FiniteAutomaton& language, const Regex& regex,
					 int iteration_step, iLogTemplate* log = nullptr);
	static void test(const Regex& language, const Regex& regex,
					 int iteration_step, iLogTemplate* log = nullptr);
};