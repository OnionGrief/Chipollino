#pragma once
#include "BaseObject.h"
#include "Regex.h"
#include <map>
#include <string>
#include <vector>

using namespace std;

struct function {
	string name;
	vector<string> input;
	string output;
};

struct id {
	int num;
	string type;
};

class TasksGenerator : BaseObject {
  private:
	string res_str = "";
	int maxLength = 5; // максимальное кол-во функций подряд
	int idNum = 0; // кол-во объявленных идентификаторов
	bool for_static_Tpchkr = false,
		 for_dinamic_Tpchkr =
			 false; //для статического тайпчекера - генерируем все функции
					//подряд, для динамического - dfa = nfa, если оба false, то
					//генерируем гарантированно правильные последовательности
					//команд
	map<string, vector<function>> funcInput;
	map<string, vector<id>> ids; // поиск идентификатора по его типу

	vector<function> functions = {
		{"Thompson", {"Regex"}, "NFA"},
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
		{"KSubSet", {"Int", "NFA"}, "NFA"},
		{"Normalize", {"Regex", "FileName"}, "Regex"}, // fileName
		{"States", {"NFA"}, "Int"},
		{"ClassCard", {"DFA"}, "Int"},
		{"Ambiguity", {"NFA"}, "Value"}, // value???
		{"Width", {"NFA"}, "Int"},
		{"MyhillNerode", {"DFA"}, "Int"},
		{"Simplify", {"Regex"}, "Regex"}};

	vector<string> predicates = {
		"Bisimilar", "Minimal", "Subset", "Equiv", "Minimal", "Equal", "SemDet",
	};

	void distribute_functions();
    function generate_next_func(string);

  public:
	TasksGenerator();
	string to_txt() override;

	string generate_task(int, int, bool, bool);
	string generate_op();
	string generate_declaration();
	function rand_func();
};

// напоминание: убедиться, что интерпретатор + тайпчекер правильно обрабатывают
// неверное кол-во элементов, случаи N1 = <regex>; N2 = N1;
// N3 = <object> <object>; N1 = Glushkov <object> <object>;
// и несоотвествие типов