#pragma once
#include <algorithm>
#include <ctime>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "FuncLib/Functions.h"
#include "FuncLib/Typization.h"
#include "InputGenerator/RegexGenerator.h"
#include "Objects/Regex.h"

using namespace Typization; // NOLINT(build/namespaces)
using namespace FuncLib;	// NOLINT(build/namespaces)

class TasksGenerator {
  private:
	/* идентификатор */
	struct Id {
		int num;
		ObjectType type;
	};
	RegexGenerator regex_generator;
	size_t seed_it = 0; // итерация для рандома

	std::string res_str = "";
	int max_num_of_func_in_seq = 5; // максимальное кол-во функций в посл-ти
	int id_num = 0;		 // кол-во объявленных идентификаторов
	ObjectType cur_type; // выходный тип данных последней сгенерированной функции

	/* для статического тайпчекера - генерируем все функции подряд, для динамического - dfa = nfa,
	 * если оба false, то генерируем гарантированно правильные последовательности команд */
	bool for_static_tpchkr = false, for_dynamic_tpchkr = false;

	inline static const ObjectType REGEX = ObjectType::Regex, NFA = ObjectType::NFA,
								   DFA = ObjectType::DFA, INT = ObjectType::Int,
								   VALUE = ObjectType::AmbiguityValue,
								   BOOLEAN = ObjectType::Boolean, NFA_DFA = ObjectType::NFA,
								   ARRAY = ObjectType::Array, PG = ObjectType::PrefixGrammar;

	std::vector<ObjectType> generated_types = {REGEX, INT, ARRAY};
	std::map<ObjectType, std::vector<Id>> ids_by_type; // поиск идентификатора по его типу
	// разделение функций (с единственным аргументом) по принимаемым значениям
	std::map<ObjectType, std::vector<Function>> funcInput;

	void distribute_functions();
	bool arguments_are_exists(std::vector<ObjectType> args);
	Function generate_next_func(ObjectType prevOutput, int funcNum);
	std::string generate_arguments(Function first_func);
	std::string get_random_id_by_type(ObjectType type);
	Function rand_func();
	void change_seed();

  public:
	TasksGenerator();

	/*создает рандомный список операций, которые могут иметь один из трёх видов:
	Объявление: [идентификатор] = ([функция].)*[функция]? [объект]+ (!!)?
	Выражение: ([функция].)*[функция]? [объект]+ (!!)?
	Специальная форма test */
	std::string generate_task(int opNum, int max_num_of_func_in_seq_, bool for_static_tpchkr_,
							  bool for_dynamic_tpchkr_);
	/* генерирует объявление:
	[идентификатор] = ([функция].)*[функция]? [объект]+ (!!)? */
	std::string generate_declaration();
	/* генерирует выражение */
	std::string generate_expression();
	/* генерирует метод:
	test [НКА | рег. выр-е] [рег. выр-е] [шаг итерации]) */
	std::string generate_test();
	/* генерирует рандомную операцию: объявление, выражение или test*/
	std::string generate_op();
	/*запись теста в файл*/
	void write_to_file(std::string filename);
};