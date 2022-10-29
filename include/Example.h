#include "AutomatonToImage.h"
#include "FiniteAutomaton.h"
#include "Language.h"
#include "Logger.h"
#include "Regex.h"
#include "RegexGenerator.h"
#include "TasksGenerator.h"
#include <iostream>

/*
Это статический класс, где вы можете писать примеры
использования функций и, соответственно, их тестить.
Чтобы оставлять чистым main
*/
class Example {
  public:
	// Пример построения regex из строки
	static void determinize();
	static void remove_eps();
	static void minimize();
	static void intersection();
	static void regex_parsing();
	static void regex_generating();
	static void random_regex_parsing();
	static void tasks_generating();
	static void parsing_regex(string);
	static void fa_equal_check();
	static void fa_bisimilar_check();
	static void fa_merge_bisimilar();
	static void parsing_nfa();
	static void fa_equivalent_check();
	static void fa_subset_check();
	static void to_image();
	static void step();
	static void normalize_regex();
};