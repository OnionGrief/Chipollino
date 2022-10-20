#include "FiniteAutomat.h"
#include "Regex.h"
#include <iostream>

/*
Это статический класс, где вы можете писать примеры
использования функций и, соответственно, их тестить.
Чтобы оставлять чистым main
*/
class Example {
  public:
	// Пример построения regex из строки
	static void regex_parsing();
	static void fa_equal_check();
	static void fa_bisimilar_check();
	static void fa_merge_bisimilar();
};