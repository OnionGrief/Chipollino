#include "InputGenerator/TasksGenerator.h"

TasksGenerator::TasksGenerator() {
	change_seed();
	distribute_functions();
}

void TasksGenerator::change_seed() {
	srand(time(nullptr) + rand() % 100);
}

string TasksGenerator::generate_task(int op_num, int max_num_of_func_in_seq_,
									 bool for_static_Tpchkr_,
									 bool for_dinamic_Tpchkr_) {
	change_seed();
	res_str = "";
	id_num = 0;
	ids.clear();
	max_num_of_func_in_seq = max_num_of_func_in_seq_;
	for_static_Tpchkr = for_static_Tpchkr_;
	for_dinamic_Tpchkr = for_dinamic_Tpchkr_;

	for (int i = 0; i < op_num; i++) {
		res_str += generate_op() + "\n";
	}

	return res_str;
}

void TasksGenerator::write_to_file(string filename) {
	ofstream out;
	out.open(filename, ofstream::trunc);
	if (out.is_open()) out << res_str;
	out.close();
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
	change_seed();
	string str = "";

	Function predicate = rand_pred();
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

	for (int i = 0; i < predicate.input.size(); i++) {
		if (for_static_Tpchkr) {
			if (rand() % 4 && id_num > 1) {
				int rand_id = rand() % id_num + 1;
				str += " N" + to_string(rand_id);
			} else {
				str += " " + regex_generator.generate_regex();
			}
		} else {
			if (predicate.input[i] == "Regex") {
				// сгенерировать регулярку или найти идентификатор
				// если есть идентификаторы с типом Regex
				if (ids.count("Regex") && rand() % 2) {
					vector<Id> possible_ids = ids["Regex"];
					Id rand_id = possible_ids[rand() % possible_ids.size()];
					str += " N" + to_string(rand_id.num);
				} else {
					str += " " + regex_generator.generate_regex();
				}
			}

			if (predicate.input[i] == "NFA" || predicate.input[i] == "DFA") {
				string id_type = predicate.input[i];
				if (for_dinamic_Tpchkr || id_type == "NFA") id_type = "NFA-DFA";
				vector<Id> possible_ids = ids[id_type];
				int rand_num = rand() % possible_ids.size();
				Id rand_id = possible_ids[rand_num];
				str += " N" + to_string(rand_id.num);
			}
		}
	}

	return str;
}

// TODO: для дин и стат тайпчека
string TasksGenerator::generate_test() {
	change_seed();
	string str = "";
	str += "Test ";

	if (rand() % 2 && ids.count("NFA-DFA")) {
		vector<Id> possible_ids = ids["NFA-DFA"];
		Id rand_id = possible_ids[rand() % possible_ids.size()];
		str += "N" + to_string(rand_id.num);
	} else if (rand() % 2 && ids.count("Regex")) {
		vector<Id> possible_ids = ids["Regex"];
		Id rand_id = possible_ids[rand() % possible_ids.size()];
		str += "N" + to_string(rand_id.num);
	} else {
		str += regex_generator.generate_regex();
	}

	str += " ";
	// TODO:
	// str += "((ab)*a)*";
	str += regex_generator.generate_regex();

	int rand_num = rand() % 5 + 1; // шаг итерации - пусть будет до 5..
	str += " " + to_string(rand_num);

	return str;
}

string TasksGenerator::generate_declaration() {
	change_seed();
	string str = "";
	id_num++;
	str += "N" + to_string(id_num) + " = ";
	int funcNum =
		max_num_of_func_in_seq > 0 ? rand() % (max_num_of_func_in_seq + 1) : 0;

	string prevOutput;
	string func_str = "";

	if (funcNum > 0) {
		Function first_func = rand_func();
		// TODO:
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

		for (int i = 0; i < first_func.input.size(); i++) {
			if (for_static_Tpchkr) {
				if (rand() % 2 && id_num > 1) {
					int rand_id = rand() % (id_num - 1) + 1;
					func_str += " N" + to_string(rand_id);
				} else {
					func_str += " " + regex_generator.generate_regex();
				}
			} else {

				if (first_func.input[i] == "Regex") {
					// сгенерировать регулярку или найти идентификатор
					// если есть идентификаторы с типом Regex
					if (ids.count("Regex") && rand() % 2) {
						vector<Id> possible_ids = ids["Regex"];
						Id rand_id = possible_ids[rand() % possible_ids.size()];
						func_str += " N" + to_string(rand_id.num);
					} else {
						func_str += " " + regex_generator.generate_regex();
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
					vector<Id> possible_ids = ids[id_type];
					int rand_num = rand() % possible_ids.size();
					Id rand_id = possible_ids[rand_num];
					func_str += " N" + to_string(rand_id.num);
				}

				if (first_func.input[i] == "FileName") {
					func_str += " Rules";
				}
			}
		}
		prevOutput = first_func.output;
	}

	for (int i = 1; i < funcNum; i++) {
		Function func = generate_next_func(prevOutput, funcNum - 1 - i);
		prevOutput = func.output;
		func_str = func.name + '.' + func_str;
	}

	str += func_str;

	if (funcNum == 0) {
		if (rand() % 3 && ids.size() > 0) {
			string id_output = "NFA-DFA";
			map<string, vector<Id>>::iterator it;
			while (id_output == "NFA-DFA") {
				it = ids.begin();
				advance(it, (rand() % ids.size()));
				pair<string, vector<Id>> possible_ids = *it;
				id_output = possible_ids.first;
			}
			vector<Id> possible_ids = it->second;
			Id rand_id = possible_ids[rand() % possible_ids.size()];
			str += "N" + to_string(rand_id.num);
			prevOutput = id_output;
		} else {
			str += regex_generator.generate_regex();
			prevOutput = "Regex";
		}
	}

	// запоминаем идентификатор N#
	ids[prevOutput].push_back({id_num, prevOutput});
	if (prevOutput == "DFA" || prevOutput == "NFA")
		ids["NFA-DFA"].push_back({id_num, prevOutput});

	if (/*rand() % 2 && */ funcNum > 0) str += " !!";

	return str;
}

TasksGenerator::Function TasksGenerator::generate_next_func(string prevOutput,
															int funcNum) {
	Function str;
	if (for_static_Tpchkr)
		str = rand_func();
	else if ((for_dinamic_Tpchkr && prevOutput == "NFA") ||
			 prevOutput == "DFA") {
		vector<Function> possible_functions = funcInput["NFA-DFA"];
		str = possible_functions[rand() % possible_functions.size()];
		if ((str.output == "Int" || str.output == "Value") && funcNum != 0)
			str = generate_next_func(prevOutput, funcNum);
	} else {
		vector<Function> possible_functions = funcInput[prevOutput];
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

TasksGenerator::Function TasksGenerator::rand_func() {
	return functions[rand() % functions.size()];
}

TasksGenerator::Function TasksGenerator::rand_pred() {
	return predicates[rand() % predicates.size()];
}

/*
– По радио сообщили о переходе на зимнее время, сказав, что «этой ночью, ровно в
03:00 нужно перевести стрелку часов на один час назад, на 02:00».
– У всех программистов эта ночь зависла в бесконечном цикле.
*/