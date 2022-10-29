#pragma once
#include "BaseObject.h"
#include "Regex.h"
#include "RegexGenerator.h"
#include <map>
#include <string>
#include <time.h>
#include <vector>
using namespace std;

class TasksGenerator {

  public:
	struct Function {
		string name;
		vector<string> input;
		string output;
	};

	struct Id {
		int num;
		string type;
	};

  private:
	RegexGenerator regex_generator;

	string res_str = "";
	int max_num_of_func_in_seq = 5; // максимальное кол-во функций в посл-ти
	int id_num = 0; // кол-во объявленных идентификаторов
	bool for_static_Tpchkr = false,
		 for_dinamic_Tpchkr =
			 false; //для статического тайпчекера - генерируем все функции
					//подряд, для динамического - dfa = nfa, если оба false, то
					//генерируем гарантированно правильные последовательности
					//команд
	map<string, vector<Function>>
		funcInput; // разделение функций (с единственным аргументом) по
				   // принимаемым значениям
	map<string, vector<Id>> ids; // поиск идентификатора по его типу

	vector<Function> functions = {{"Thompson", {"Regex"}, "NFA"},
								  {"IlieYu", {"Regex"}, "NFA"},
								  {"Antimirov", {"Regex"}, "NFA"},
								  {"Arden", {"NFA"}, "Regex"},
								  {"Glushkov", {"Regex"}, "NFA"},
								  {"Determinize", {"NFA"}, "DFA"},
								  {"RemEps", {"NFA"}, "NFA"},
								  {"Linearize", {"Regex"}, "Regex"},
								  {"Minimize", {"NFA"}, "DFA"},
								  {"Reverse", {"NFA"}, "NFA"},
								  {"Annote", {"NFA"}, "DFA"},
								  {"DeLinearize", {"NFA"}, "NFA"},
								  {"DeLinearize", {"Regex"}, "Regex"},
								  {"Complement", {"DFA"}, "DFA"},
								  {"DeAnnote", {"NFA"}, "NFA"},
								  {"DeAnnote", {"Regex"}, "Regex"},
								  {"MergeBisim", {"NFA"}, "NFA"},
								  {"PumpLength", {"Regex"}, "Int"},
								  {"ClassLength", {"DFA"}, "Int"},
								  // TODO:
								  //{"KSubSet", {"Int", "NFA"}, "NFA"}, // пока
								  //не используется, исправить если будет
								  {"Normalize", {"Regex", "FileName"}, "Regex"},
								  {"States", {"NFA"}, "Int"},
								  {"ClassCard", {"DFA"}, "Int"},
								  {"Ambiguity", {"NFA"}, "Value"},
								  {"Width", {"NFA"}, "Int"},
								  {"MyhillNerode", {"DFA"}, "Int"},
								  {"Simplify", {"Regex"}, "Regex"}};

	vector<Function> predicates = {
		{"Bisimilar", {"NFA", "NFA"}, "Boolean"},
		{"Minimal", {"DFA"}, "Boolean"},
		{"Subset", {"Regex", "Regex"}, "Boolean"},
		{"Equiv", {"NFA", "NFA"}, "Boolean"},
		{"Minimal", {"NFA"}, "Boolean"},
		{"Equal", {"NFA", "NFA"}, "Boolean"},
		{"SemDet", {"NFA"}, "Boolean"},
	};

	void distribute_functions();
	Function generate_next_func(string, int);
	string generate_op();
	Function rand_func();
	Function rand_pred();

  public:
	TasksGenerator();

	/* создает рандомный список операций, которые могут иметь один из трёх
	видов: Объявление: [идентификатор] = ([функция].)*[функция]? [объект]+ (!!)?
	Специальная форма test
	Предикат [предикат] [объект]+ */
	string generate_task(int opNum, int max_num_of_func_in_seq_,
						 bool for_static_Tpchkr_, bool for_dinamic_Tpchkr_);
	/* генерирует объявление:
	[идентификатор] = ([функция].)*[функция]? [объект]+ (!!)? */
	string generate_declaration();
	/* генерирует предикат */
	string generate_predicate();
	/* генерирует метод:
	test (НКА | рег. выр-е, рег. выр-е без альтернатив, шаг итерации) */
	string generate_test();
};

// TODO: убедиться, что интерпретатор + тайпчекер правильно обрабатывают
// неверное кол-во элементов;
// N3 = <object> <object>; N1 = Glushkov <object> <object>;
// и несоотвествие типов