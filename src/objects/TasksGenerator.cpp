#include "TasksGenerator.h"
#include "RegexGenerator.h"

TasksGenerator::TasksGenerator() {}

string TasksGenerator::generate_task(int opNum, int maxLength_,
									 bool for_static_Tpchkr_,
									 bool for_dinamic_Tpchkr_) {
	res_str = "";
	idNum = 0;
	ids.clear();
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
	int op = rand() % 5; // на объявление - вероятность 3 / 5;
						 // на test и предикаты 1 / 5
	if (op == 0) {
		str = generate_predicate();
	} else if (op == 1) {
		str = generate_test();
	} else {
		str = generate_declaration();
	}
	return str;
}

string TasksGenerator::generate_predicate() {
	string str = "";

	function predicate = rand_pred(); // не все может существовать
	string input_type = predicate.input[0];
	// да, не логично, но второй аргумент всегда повторяется,
	// а значит можно забить на их проверку

	while (
		!for_static_Tpchkr &&
		!(input_type == "DFA" && ids.count("NFA-DFA") && for_dinamic_Tpchkr) &&
		((input_type == "DFA" && !ids.count("DFA")) ||
		 (input_type == "NFA") && !(ids.count("NFA") || ids.count("DFA")))) {
		predicate = rand_pred();
		input_type = predicate.input[0];
	}
	str += predicate.name;

	int objectsNum = predicate.input.size();
	for (int i = 0; i < objectsNum; i++) {
		if (for_static_Tpchkr) {
			if (!(rand() % 4) && idNum > 1) {
				int rand_id = rand() % idNum;
				str += " N" + to_string(rand_id);
			} else {
				RegexGenerator rand_regex;
				str += " " + rand_regex.to_txt();
			}
		} else {
			if (predicate.input[i] == "Regex") {
				// сгенерировать регулярку или найти идентификатор
				//если есть идентификаторы с типом Regex
				if (ids.count("Regex") && rand() % 2) {
					vector<id> possible_ids = ids["Regex"];
					id rand_id = possible_ids[rand() % possible_ids.size()];
					str += " N" + to_string(rand_id.num);
				} else {
					RegexGenerator rand_regex;
					str += " " + rand_regex.to_txt();
				}
			}

			if (predicate.input[i] == "NFA" || predicate.input[i] == "DFA") {
				string id_type = predicate.input[i];
				if (for_dinamic_Tpchkr || id_type == "NFA") id_type = "NFA-DFA";
				vector<id> possible_ids = ids[id_type];
				int rand_num = rand() % possible_ids.size();
				id rand_id = possible_ids[rand_num];
				str += " N" + to_string(rand_id.num);
			}
		}
	}

	return str;
}

string TasksGenerator::generate_test() {
	string str = "";
	str += "Test ";

	if (rand() % 2 && ids.count("NFA-DFA")) {
		vector<id> possible_ids = ids["NFA-DFA"];
		id rand_id = possible_ids[rand() % possible_ids.size()];
		str += "N" + to_string(rand_id.num);
	} else if (rand() % 2 && ids.count("Regex")) {
		vector<id> possible_ids = ids["Regex"];
		id rand_id = possible_ids[rand() % possible_ids.size()];
		str += "N" + to_string(rand_id.num);
	} else {
		RegexGenerator rand_regex;
		str += rand_regex.to_txt();
	}

	// regex без альтернатив... надо наверно сделать
	RegexGenerator rand_regex;
	str += " " + rand_regex.to_txt();

	int rand_num = rand() % 5 + 1; // шаг итерации - пусть будет до 5..
	str += " " + to_string(rand_num);

	return str;
}

string TasksGenerator::generate_declaration() {
	string str = "";
	idNum++;
	str += "N" + to_string(idNum) + " = ";
	int funcNum = maxLength > 0 ? rand() % (maxLength + 1) : 0;

	string prevOutput;
	string func_str = "";

	if (funcNum > 0) {
		function first_func = rand_func();
		string input_type = first_func.input[0]; // исправить для ksubset!!!

		// Андрей не дает мне делать большие комменты((
		while ((!for_static_Tpchkr &&
				!(input_type == "DFA" && ids.count("NFA-DFA") &&
				  for_dinamic_Tpchkr) &&
				((input_type == "DFA" && !ids.count("DFA")) ||
				 (input_type == "NFA") &&
					 !(ids.count("NFA") || ids.count("DFA")))) ||
			   ((first_func.output == "Int" || first_func.output == "Value") &&
				!for_static_Tpchkr && funcNum > 1)) {
			first_func = rand_func();
			input_type = first_func.input[0];
		} // вроде работает

		func_str += first_func.name;

		int objectsNum = first_func.input.size();
		for (int i = 0; i < objectsNum; i++) {
			if (for_static_Tpchkr) {
				if (rand() % 2 && idNum > 1) {
					int rand_id = rand() % idNum;
					func_str += " N" + to_string(rand_id);
				} else {
					RegexGenerator rand_regex;
					func_str += " " + rand_regex.to_txt();
				}
			} else {
				if (first_func.input[i] == "Regex") {
					// сгенерировать регулярку или найти идентификатор
					//если есть идентификаторы с типом Regex
					if (ids.count("Regex") && rand() % 2) {
						vector<id> possible_ids = ids["Regex"];
						id rand_id = possible_ids[rand() % possible_ids.size()];
						func_str += " N" + to_string(rand_id.num);
					} else {
						RegexGenerator rand_regex;
						func_str += " " + rand_regex.to_txt();
					}
				}
				if (first_func.input[i] == "Int") {
					// сгенерировать число или найти идентификатор (так можно??)
					// таких функций пока нет
					int rand_num = rand() % 5; // пусть будет до 5..
					func_str += " " + to_string(rand_num);
				}

				if (first_func.input[i] == "NFA" ||
					first_func.input[i] == "DFA") {
					string id_type = first_func.input[i];
					if (for_dinamic_Tpchkr || id_type == "NFA")
						id_type = "NFA-DFA";
					vector<id> possible_ids = ids[id_type];
					int rand_num = rand() % possible_ids.size();
					id rand_id = possible_ids[rand_num];
					func_str += " N" + to_string(rand_id.num);
				}
			}
		}
		prevOutput = first_func.output;
	}

	for (int i = 1; i < funcNum; i++) {
		function func = generate_next_func(prevOutput, funcNum - 1 - i);
		prevOutput = func.output;
		func_str = func.name + '.' + func_str;
	}

	str += func_str;

	if (funcNum == 0) {
		if (rand() % 3 && ids.size() > 0) {
			string id_output = "NFA-DFA";
			map<string, vector<id>>::iterator it;
			while (id_output == "NFA-DFA") {
				it = ids.begin();
				advance(it, (rand() % ids.size()));
				pair<string, vector<id>> possible_ids = *it;
				id_output = possible_ids.first;
			}
			vector<id> possible_ids = it->second;
			id rand_id = possible_ids[rand() % possible_ids.size()];
			// int rand_id = rand() % (idNum - 1) + 1;
			str += "N" + to_string(rand_id.num);
			prevOutput = id_output;
		} else {
			RegexGenerator rand_regex;
			str += rand_regex.to_txt();
			prevOutput = "Regex";
		}
	}

	// запоминаем идентификатор N#
	ids[prevOutput].push_back({idNum, prevOutput});
	if (prevOutput == "DFA" || prevOutput == "NFA")
		ids["NFA-DFA"].push_back({idNum, prevOutput});

	if (rand() % 2 && funcNum > 0) str += " !!";

	return str;
}

function TasksGenerator::generate_next_func(string prevOutput, int funcNum) {
	function str;
	if (for_static_Tpchkr)
		str = rand_func();
	else if ((for_dinamic_Tpchkr && prevOutput == "NFA") ||
			 prevOutput == "DFA") {
		vector<function> possible_functions = funcInput["NFA-DFA"];
		str = possible_functions[rand() % possible_functions.size()];
		if ((str.output == "Int" || str.output == "Value") && funcNum != 0)
			str = generate_next_func(prevOutput, funcNum);
	} else {
		vector<function> possible_functions = funcInput[prevOutput];
		str = possible_functions[rand() % possible_functions.size()];
		if ((str.output == "Int" || str.output == "Value") &&
			funcNum != 0) // может возвращать int если
						  // последняя функция в посл-ти
			str = generate_next_func(prevOutput, funcNum);
	}
	return str;
}

void TasksGenerator::distribute_functions() {
	for (int i = 0; i < functions.size(); i++) {
		if (functions[i].input.size() == 1) {
			string input_type = functions[i].input[0];
			funcInput[input_type].push_back(functions[i]);
			if (input_type == "NFA" || input_type == "DFA")
				funcInput["NFA-DFA"].push_back(functions[i]);
		}
	}
}

string TasksGenerator::to_txt() {
	return "Ответ убил...";
}

function TasksGenerator::rand_func() {
	return functions[rand() % functions.size()];
}

function TasksGenerator::rand_pred() {
	return predicates[rand() % predicates.size()];
}