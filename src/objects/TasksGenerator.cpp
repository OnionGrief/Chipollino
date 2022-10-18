#include "TasksGenerator.h"

TasksGenerator::TasksGenerator() {}

string TasksGenerator::generate_task(int opNum, int maxLength_,
									 bool for_static_Tpchkr_,
									 bool for_dinamic_Tpchkr_) {
	res_str = "";
	idNum = 0;
	maxLength = maxLength_;
	for_static_Tpchkr = for_static_Tpchkr_;
	for_dinamic_Tpchkr = for_dinamic_Tpchkr_;

	distribute_functions();

	for (int i = 0; i < opNum; i++) {
		res_str += generate_op() + "\n";
	}

	return res_str;
}

string TasksGenerator::generate_op() {
	string str = "";
	int op = rand() % 3;
	op = 0; // пока что
	if (op == 0) {
		str = generate_declaration();
	}
	if (op == 1) {
		str = "Test";
	}
	if (op == 2) {
		str = "t/f";
	}
	return str;
}

string TasksGenerator::generate_declaration() {
	string str = "";
	idNum++;
	str += "N" + to_string(idNum) + " = ";
	int funcNum = rand() % maxLength;

	string prevOutput;
	string func_str = "";

	if (funcNum > 0) {
		function first_func = rand_func();
		while (first_func.output == "Int") { //исправить
			first_func = rand_func();
		}
		func_str += first_func.name;
		int objectsNum = first_func.input.size();
		for (int i = 0; i < objectsNum; i++) {
			if (first_func.input[i] == "Regex") {
				// сгенерировать регулярку или найти идентификатор
			}
			if (first_func.input[i] == "Int") {
				// сгенерировать число или найти идентификатор (так можно??)
			}
			// надо подумать чу делац если пока еще нет идентификаторов с типами
			// автоматов (пересоздавать first_func)
		}
		prevOutput = first_func.output;
	}

	// при finc_num = 0 генерация объекта

	for (int i = 1; i < funcNum; i++) {
		function func = generate_next_func(prevOutput);
		prevOutput = func.output;
		func_str = func.name + '.' + func_str;
	}

	str += func_str;

	if (rand() % 2 && funcNum > 0) str += " !!";
	return str;
}

function TasksGenerator::generate_next_func(string prevOutput) {
	function str;
	if (for_static_Tpchkr)
		str = rand_func();
	else {
		vector<function> possible_functions = funcInput[prevOutput];
		str = possible_functions[rand() % possible_functions.size()];
		if (str.output == "Int")
			str = generate_next_func(prevOutput); // исправить
	}
	return str;
}

void TasksGenerator::distribute_functions() {
	for (int i = 0; i < functions.size(); i++) {
		if (functions[i].input.size() == 1) {
			funcInput[functions[i].input[0]].push_back(functions[i]);
		}
	}
}

string TasksGenerator::to_txt() {
	return "Ответ убил...";
}

function TasksGenerator::rand_func() {
	return functions[rand() % functions.size()];
}