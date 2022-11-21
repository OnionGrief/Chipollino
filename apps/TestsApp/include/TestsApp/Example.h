#include "AutomatonToImage/AutomatonToImage.h"
#include "InputGenerator/RegexGenerator.h"
#include "InputGenerator/TasksGenerator.h"
#include "Objects/FiniteAutomaton.h"
#include "Objects/Language.h"
#include "Objects/Logger.h"
#include "Objects/Regex.h"
#include "Objects/TransformationMonoid.h"
#include "Tester/Tester.h"
#include "Interpreter/Interpreter.h"
#include <cassert>
#include <iostream>
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
	static void random_regex_parsing();
	static void tasks_generating();
	static void parsing_regex(string);
	static void transformation_monoid_example();
	static void normalize_regex();
	static void step();
	static void parsing_nfa();
	static void fa_subset_check();
	static void arden_test();
	static void to_image();
	static void tester();
	static void step_interection();
	static void table();
	static void fa_semdet_check();
	// запуск всех тестов
	static void test_all();
	static void test_fa_equal();
	static void test_fa_equiv();
	static void test_bisimilar();
	static void test_regex_subset();
	static void test_merge_bisimilar();
	static void test_regex_equal();
	static void test_ambiguity();
	static void test_arden();
	static void test_pump_length();
	static void test_is_one_unambiguous();
	static void test_interpreter();
};