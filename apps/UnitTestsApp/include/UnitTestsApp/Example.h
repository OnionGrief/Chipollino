#ifndef APPS_UNITTESTSAPP_INCLUDE_UNITTESTSAPP_EXAMPLE_H_
#define APPS_UNITTESTSAPP_INCLUDE_UNITTESTSAPP_EXAMPLE_H_

#include <cassert>
#include <functional>
#include <iostream>
#include <string>

#include "AutomatonToImage/AutomatonToImage.h"
#include "InputGenerator/RegexGenerator.h"
#include "InputGenerator/TasksGenerator.h"
#include "Interpreter/Interpreter.h"
#include "Objects/FiniteAutomaton.h"
#include "Objects/Grammar.h"
#include "Objects/Language.h"
#include "Objects/Regex.h"
#include "Objects/TransformationMonoid.h"
#include "Objects/iLogTemplate.h"
#include "Tester/Tester.h"
/*
Это статический класс, где вы можете писать примеры
использования функций и, соответственно, их тестить.
Чтобы оставлять чистым main
*/
class Example {
  public:
	/*запуск всех примеров*/
	static void all_examples();
	static void determinize();
	static void remove_eps();
	static void minimize();
	static void intersection();
	static void regex_parsing();
	static void regex_generating();
	static void tasks_generating();
	static void parsing_regex(string);
	static void transformation_monoid_example();
	static void normalize_regex();
	static void step();
	static void parsing_nfa();
	static void fa_subset_check();
	static void arden_example();
	static void to_image();
	static void tester();
	static void step_interection();
	static void table();
	static void fa_semdet_check();
	static void classes_number_GlaisterShallit();
	static void testing_with_generator(int regex_length, int star_num, int star_nesting,
									   int alphabet_size,
									   const std::function<void(string& rgx_str)>& check_function);
	static void arden_lemma_testing();
	static void fa_to_pgrammar();
	static void logger_test();
};
#endif // APPS_UNITTESTSAPP_INCLUDE_UNITTESTSAPP_EXAMPLE_H_