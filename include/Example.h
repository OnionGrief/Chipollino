#include "FiniteAutomaton.h"
#include "Language.h"
#include "Regex.h"
#include "AutomatonToImage.h"
#include "Logger.h"
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
	static void fa_equal_check();
	static void fa_bisimilar_check();
	static void fa_merge_bisimilar();
	static void to_image();
	static void step();
};