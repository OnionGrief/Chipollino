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
#include <cassert>
#include <functional>
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
	static void arden_example();
	static void to_image();
	static void tester();
	static void step_interection();
	static void table();
	static void fa_semdet_check();
	static void classes_number_GlaisterShallit();
	static void testing_with_generator(int regex_length, int star_num, int star_nesting,
									   int alphabet_size,
									   const function<void(string& rgx_str)>& check_function);
	static void arden_lemma_testing();
	static void fa_to_pgrammar();
	static void logger_test();

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
	static void test_get_one_unambiguous_regex();
	static void test_interpreter();
	static void test_TransformationMonoid();
	static void test_GlaisterShallit();
	static void test_fa_to_pgrammar();
};