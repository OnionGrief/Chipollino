#include <iostream>
#include "Regex.h"
#include "FiniteAutomat.h"

/* 
Ёто статический класс, где вы можете писать примеры 
использовани€ функций и, соответственно, их тестить. 
„тобы оставл€ть чистым main
*/
class Example {
public:
	// ѕример построени€ regex из строки
	static void regex_parsing();
	static void fa_equal_check();
	static void fa_bisimilar_check();
	static void fa_merge_bisimilar();
};