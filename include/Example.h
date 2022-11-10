#include "AutomatonToImage.h"
#include "FiniteAutomaton.h"
#include "Language.h"
#include "Logger.h"
#include "Regex.h"
#include "RegexGenerator.h"
#include "TasksGenerator.h"
#include "Tester.h"
#include "TransformationMonoid.h"
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
};