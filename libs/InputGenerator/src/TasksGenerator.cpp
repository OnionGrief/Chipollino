#include "InputGenerator/TasksGenerator.h"

TasksGenerator::TasksGenerator() {
	change_seed();
	distribute_functions();
}

void TasksGenerator::change_seed() {
	seed_it++;
	srand((size_t)time(nullptr) + seed_it + rand());
}

string TasksGenerator::generate_task(int op_num, int max_num_of_func_in_seq_,
									 bool for_static_Tpchkr_, bool for_dinamic_Tpchkr_) {
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
	std::ofstream out;
	out.open(filename, std::ofstream::trunc);
	if (out.is_open())
		out << res_str;
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
		// ф/я подходит, если отключена проверка на соответствие типов
		!for_static_Tpchkr &&
		// ф/я не подходит, если в памяти нет типа данных, ожидаемого на
		// вход ф/и (исключение - REGEX и INT, т.к. их можно сгенерировать)
		(!ids.count(input_type) && input_type != REGEX && input_type != INT) &&
		// ф/я подходит, если на вход требуется DFA, включен дин. тайпчек,
		// и в памяти есть либо NFA либо DFA
		!(input_type == DFA && ids.count(NFA_DFA) && for_dinamic_Tpchkr) &&
		// ф/я подходит, если на вход требуется NFA, и
		// в памяти есть либо NFA либо DFA
		!(input_type == NFA && ids.count(NFA_DFA))) {
		predicate = rand_pred();
		input_type = predicate.input[0];
	}
	str += predicate.name;
	str += generate_arguments(predicate);
	return str;
}

// TODO: для дин и стат тайпчека
string TasksGenerator::generate_test() {
	change_seed();
	string str = "Test ";

	if (rand() % 2 && ids.count(NFA_DFA))
		str += "N" + get_random_id_by_type(NFA_DFA);
	else if (rand() % 2 && ids.count(REGEX))
		str += "N" + get_random_id_by_type(REGEX);
	else
		str += regex_generator.generate_framed_regex();

	str += " ";

	str += regex_generator.generate_framed_regex();

	int rand_num = rand() % 5 + 1; // шаг итерации - пусть будет до 5..
	str += " " + std::to_string(rand_num);

	return str;
}

string TasksGenerator::generate_arguments(Function first_func) {
	string func_str = "";
	for (int i = 0; i < first_func.input.size(); i++) {
		if (for_static_Tpchkr) {
			if (rand() % 2 && id_num > 1) {
				int rand_id = rand() % (id_num - 1) + 1;
				func_str += " N" + std::to_string(rand_id);
			} else {
				func_str += " " + regex_generator.generate_framed_regex();
			}
		} else {

			string input_type = first_func.input[i];

			if ((for_dinamic_Tpchkr && input_type == DFA) || input_type == NFA) {
				input_type = NFA_DFA;
			}

			if (input_type == REGEX && (!ids.count(REGEX) || rand() % 2)) {
				// сгенерировать регулярку
				func_str += " " + regex_generator.generate_framed_regex();
			} else if (input_type == INT && (!ids.count(INT) || rand() % 2)) {
				// сгенерировать число
				int rand_num = rand() % 5; // пусть будет до 5..
				func_str += " " + std::to_string(rand_num);
			} else if (input_type == ARRAY) {
				func_str += " [[{a} {b}]]";
			} else if (ids.count(input_type)) {
				func_str += " N" + get_random_id_by_type(input_type);
			} else {
				cout << "generator error: there is no id with type" + input_type << endl;
			}
		}
	}
	return func_str;
}

string TasksGenerator::get_random_id_by_type(string type) {
	vector<Id> possible_ids = ids[type];
	Id rand_id = possible_ids[rand() % possible_ids.size()];
	return std::to_string(rand_id.num);
}

string TasksGenerator::generate_declaration() {
	change_seed();
	id_num++;
	string str = "N" + std::to_string(id_num) + " = ";
	int funcNum = max_num_of_func_in_seq > 0 ? rand() % (max_num_of_func_in_seq + 1) : 0;

	string prevOutput;
	string func_str = "";

	if (funcNum > 0) {
		Function first_func = rand_func();
		// TODO:
		string input_type = first_func.input[0]; // исправить для normalize!!!

		while (
			// ф/я подходит, если отключена проверка на соответствие типов
			!for_static_Tpchkr &&
			// ф/я не подходит, если на выходе не REGEX / FA / PG, т.к. ни одна
			// ф/я не принимает другие типы данных на вход
			((first_func.output != REGEX && first_func.output != DFA && first_func.output != NFA &&
			  first_func.output != PG && funcNum > 1) ||
			 // ф/я не подходит, если в памяти нет типа данных, ожидаемого на
			 // вход ф/и (исключение - REGEX и INT, т.к. их можно сгенерировать)
			 ((!ids.count(input_type) && input_type != REGEX && input_type != INT) &&
			  // ф/я подходит, если на вход требуется DFA, включен дин. тайпчек,
			  // и в памяти есть FA
			  !(input_type == DFA && ids.count(NFA_DFA) && for_dinamic_Tpchkr) &&
			  // ф/я подходит, если на вход требуется NFA, и
			  // в памяти есть FA
			  !(input_type == NFA && ids.count(NFA_DFA))))) {
			first_func = rand_func();
			input_type = first_func.input[0];
		} // вроде работает

		func_str += first_func.name;
		func_str += generate_arguments(first_func);

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
			string id_output = NFA_DFA;
			map<string, vector<Id>>::iterator it;
			while (id_output == NFA_DFA) {
				// поиск по ключу
				it = ids.begin();
				advance(it, (rand() % ids.size()));
				id_output = it->first;
			}
			str += "N" + get_random_id_by_type(id_output);
			prevOutput = id_output;
		} else {
			str += regex_generator.generate_framed_regex();
			prevOutput = REGEX;
		}
	}

	// запоминаем идентификатор N#
	ids[prevOutput].push_back({id_num, prevOutput});
	if (prevOutput == DFA || prevOutput == NFA)
		ids[NFA_DFA].push_back({id_num, prevOutput});

	if (/*rand() % 2 && */ funcNum > 0)
		str += " !!";

	return str;
}

TasksGenerator::Function TasksGenerator::generate_next_func(string prevOutput, int funcNum) {
	Function str;
	if (for_static_Tpchkr) {
		str = rand_func();
	} else if ((for_dinamic_Tpchkr && prevOutput == NFA) || prevOutput == DFA) {
		vector<Function> possible_functions = funcInput[NFA_DFA];
		str = possible_functions[rand() % possible_functions.size()];
		if ((str.output == INT || str.output == VALUE) && funcNum != 0)
			str = generate_next_func(prevOutput, funcNum);
	} else {
		vector<Function> possible_functions = funcInput[prevOutput];
		str = possible_functions[rand() % possible_functions.size()];
		if ((str.output == INT || str.output == VALUE) &&
			funcNum != 0) // может возвращать int если последняя функция в посл-ти
			str = generate_next_func(prevOutput, funcNum);
	}
	return str;
}

void TasksGenerator::distribute_functions() {
	for (int i = 0; i < functions.size(); i++) {
		if (functions[i].input.size() == 1) {
			string input_type = functions[i].input[0];
			funcInput[input_type].push_back(functions[i]);
			if (input_type == NFA || input_type == DFA)
				funcInput[NFA_DFA].push_back(functions[i]);
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