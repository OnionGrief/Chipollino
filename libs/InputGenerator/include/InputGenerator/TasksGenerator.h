#pragma once
#include "InputGenerator/RegexGenerator.h"
#include "Objects/BaseObject.h"
#include "Objects/Regex.h"
#include <fstream>
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
	size_t seed_it = 0; // итерация для рандома

	string res_str = "";
	int max_num_of_func_in_seq = 5; // максимальное кол-во функций в посл-ти
	int id_num = 0; // кол-во объявленных идентификаторов
	bool for_static_Tpchkr = false,
		 for_dinamic_Tpchkr =
			 false; // для статического тайпчекера - генерируем все функции
					// подряд, для динамического - dfa = nfa, если оба false, то
					// генерируем гарантированно правильные последовательности
					// команд
	map<string, vector<Function>>
		funcInput; // разделение функций (с единственным аргументом) по
				   // принимаемым значениям
	map<string, vector<Id>> ids; // поиск идентификатора по его типу

	string REGEX = "Regex", NFA = "NFA", DFA = "DFA", INT = "Int",
		   VALUE = "Value", BOOLEAN = "Boolean", NFA_DFA = "NFA-DFA",
		   FILENAME = "FileName", PG = "PG";
	vector<Function> functions = {
		{"Thompson", {REGEX}, NFA},
		{"IlieYu", {REGEX}, NFA},
		{"Antimirov", {REGEX}, NFA},
		//{"Arden", {NFA}, REGEX},
		{"Glushkov", {REGEX}, NFA},
		{"Determinize", {NFA}, DFA},
		{"RemEps", {NFA}, NFA},
		{"Linearize", {REGEX}, REGEX},
		{"Disambiguate", {REGEX}, REGEX},
		{"Minimize", {NFA}, DFA},
		{"Reverse", {NFA}, NFA},
		{"Annote", {NFA}, DFA},
		{"DeLinearize", {NFA}, NFA},
		{"DeLinearize", {REGEX}, REGEX},
		{"Complement", {DFA}, DFA},
		{"DeAnnote", {NFA}, NFA},
		{"DeAnnote", {REGEX}, REGEX},
		{"MergeBisim", {NFA}, NFA},
		{"PumpLength", {REGEX}, INT},
		{"ClassLength", {DFA}, INT},
		//{"Normalize", {REGEX, FILENAME}, REGEX},
		{"States", {NFA}, INT},
		{"ClassCard", {DFA}, INT},
		{"Ambiguity", {NFA}, VALUE},
		{"MyhillNerode", {DFA}, INT},
		{"GlaisterShallit", {DFA}, INT},
		{"PrefixGrammar", {NFA}, PG},
		{"PGtoNFA", {PG}, NFA},
		{"Intersect", {NFA, NFA}, NFA},
		{"Union", {NFA, NFA}, NFA},
		{"Difference", {NFA, NFA}, NFA},
	};

	vector<Function> predicates = {
		{"Subset", {REGEX, REGEX}, BOOLEAN},
		{"Subset", {NFA, NFA}, BOOLEAN},
		{"Equiv", {NFA, NFA}, BOOLEAN},
		{"Equiv", {REGEX, REGEX}, BOOLEAN},
		{"OneUnambiguity", {NFA}, BOOLEAN},
		{"OneUnambiguity", {REGEX}, BOOLEAN},
		{"Bisimilar", {NFA, NFA}, BOOLEAN},
		{"Minimal", {NFA}, BOOLEAN},
		{"Equal", {NFA, NFA}, BOOLEAN},
		{"SemDet", {NFA}, BOOLEAN},
	};

	void distribute_functions();
	Function generate_next_func(string, int);
	Function rand_func();
	Function rand_pred();
	void change_seed();

  public:
	TasksGenerator();

	/*создает рандомный список операций, которые могут иметь один из трёх видов:
	Объявление: [идентификатор] = ([функция].)*[функция]? [объект]+ (!!)?
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
	/* генерирует рандомную операцию: объявление, предикат или test*/
	string generate_op();
	/*запись теста в файл*/
	void write_to_file(string filename);
};

// TODO: убедиться, что интерпретатор + тайпчекер правильно обрабатывают
// неверное кол-во элементов;
// N3 = <object> <object>; N1 = Glushkov <object> <object>;
// и несоотвествие типов