#pragma once
#include <string>

#include "Objects/BackRefRegex.h"
#include "Objects/BaseObject.h"
#include "Objects/FiniteAutomaton.h"
#include "Objects/MemoryFiniteAutomaton.h"
#include <Objects/PushdownAutomaton.h>
#include "Objects/Regex.h"
#include "Objects/iLogTemplate.h"


class Tester {
  private:
	static bool parsing_by_regex(const std::string&, const std::string&);

	using ParseDevice = std::variant<const FiniteAutomaton*, const Regex*,
									 const MemoryFiniteAutomaton*, const BackRefRegex*, const PushdownAutomaton*>;

  public:
	/* проверяет на принадлежность языку (1 аргумент)
	 * слова из тестового сета (генерируется по 2 и 3 арг-там) */
	static void test(const ParseDevice& language, const Regex& regex, int iteration_step,
					 iLogTemplate* log = nullptr);
};