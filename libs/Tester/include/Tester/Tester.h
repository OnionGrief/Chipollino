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
  private:
	static bool parsing_by_regex(const string&, const string&);

	using ParseDevice = std::variant<FiniteAutomaton, Regex>;

  public:
	/* проверяет на принадлежность языку (1 аргумент)
	 * слова из тестового сета (генерируется по 2 и 3 арг-там) */
	static void test(const ParseDevice& language, const Regex& regex, int iteration_step,
					 iLogTemplate* log = nullptr);
};