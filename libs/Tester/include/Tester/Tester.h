#pragma once
#include <chrono>
#include <iostream>
#include <regex>
#include <string>
#include <variant>
#include <vector>

#include "Objects/BaseObject.h"
#include "Objects/FiniteAutomaton.h"
#include "Objects/Regex.h"
#include "Objects/iLogTemplate.h"

class Tester {
  public:
	struct word {			// доступ извне Tester::word
		int iterations_num; // сколько проведено итераций
		long long time;		// время парсинга в секундах
		bool belongs;		// принадлежность языку
	};

	struct table {
		std::string r2; // слова порождаются регуляркой r2 и шагом итерации
		int step;				 // шаг итерации
		std::vector<word> words; // таблица
	};

  private:
	static bool parsing_by_regex(const std::string&, const std::string&);

	using ParseDevice = std::variant<FiniteAutomaton, Regex>;

  public:
	static void test(const ParseDevice& language, const Regex& regex, int iteration_step,
					 iLogTemplate* log = nullptr);
	/*	static void test(const Regex& language, const Regex& regex,
						 int iteration_step, iLogTemplate* log = nullptr); */
};