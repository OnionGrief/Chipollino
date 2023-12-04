#include "InputGenerator/TasksGenerator.h"

using std::cout;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::to_string;
using std::vector;

TasksGenerator::TasksGenerator() {
	change_seed();
	distribute_functions();
}

void TasksGenerator::change_seed() {
	seed_it++;
	srand((size_t)time(nullptr) + seed_it + rand());
}

string TasksGenerator::generate_task(int op_num, int max_num_of_func_in_seq_,
									 bool for_static_tpchkr_, bool for_dynamic_tpchkr_) {
	change_seed();
	id_num = 0;
	ids_by_type.clear();
	max_num_of_func_in_seq = max_num_of_func_in_seq_;
	for_static_tpchkr = for_static_tpchkr_;
	for_dynamic_tpchkr = for_dynamic_tpchkr_;

	res_str = "";
	for (int i = 0; i < op_num; i++) {
		res_str += generate_op() + "\n";
	}
	return res_str;
}

void TasksGenerator::write_to_file(string filename) {
	ofstream out;
	out.open(filename, ofstream::trunc);
	if (out.is_open())
		out << res_str;
	out.close();
}

bool is_belong(vector<ObjectType> vec, ObjectType value) {
	return std::find(vec.begin(), vec.end(), value) != vec.end();
}

string TasksGenerator::generate_op() {
	string str = "";
	int op = rand() % 5; // на объявление - вероятность 3 / 5;
						 // на test и выражение 1 / 5

	if (op == 0) {
		str = generate_expression();
	} else if (op == 1) {
		str = generate_test();
	} else {
		str = generate_declaration();
	}
	return str;
}

bool TasksGenerator::arguments_are_exists(vector<ObjectType> args) {
	for (ObjectType arg_type : args) {
		// аргумент можно подобрать, если его тип есть в памяти, либо его можно сгенерировать
		if (!(ids_by_type.count(arg_type) || is_belong(generated_types, arg_type)))
			return false;
	}
	return true;
}

string TasksGenerator::generate_expression() {
	change_seed();

	int funcNum = max_num_of_func_in_seq > 0 ? rand() % (max_num_of_func_in_seq + 1) : 0;

	string str = "";

	if (funcNum == 0) {
		if (rand() % 3 && ids_by_type.size() > 0) {
			/* N# = id */
			map<ObjectType, vector<Id>>::iterator it = ids_by_type.begin();
			advance(it, (rand() % ids_by_type.size()));
			cur_type = it->first;
			str += "N" + get_random_id_by_type(cur_type);
		} else {
			/* N# = {regex}*/
			str += regex_generator.generate_framed_regex();
			cur_type = REGEX;
		}
	} else {

		/* генерация первой функции в посл-ти */
		Function first_func;

		do {
			first_func = rand_func();
		} while (
			/*// ф/я подходит, если отключена проверка на соответствие типов
			!for_static_tpchkr &&*/
		// ф/я не подходит, если она не последняя в последовательности и на выходе не
		// принимаемый другими функциями тип данных
		((!funcInput.count(first_func.output) && funcNum > 1) ||
		// ф/я не подходит, если невозможно подобрать для нее аргументы
		!arguments_are_exists(first_func.input) /*&&
			  // ф/я подходит, если на вход требуется DFA, включен дин. тайпчек,
			  // и в памяти есть FA
			  !(input_type == DFA && ids_by_type.count(NFA_DFA) && for_dynamic_tpchkr) &&
			  // ф/я подходит, если на вход требуется NFA, и
			  // в памяти есть FA
			  !(input_type == NFA && ids_by_type.count(NFA_DFA))*/));

		str += first_func.name;
		str += generate_arguments(first_func);

		cur_type = first_func.output;

		/* генерация посл-ти функций */
		for (int i = 1; i < funcNum; i++) {
			Function func = generate_next_func(cur_type, funcNum - 1 - i);
			cur_type = func.output;
			str = func.name + '.' + str;
		}
	}
	return str;
}

string TasksGenerator::generate_test() {
	change_seed();
	string str = "Test ";

	if (rand() % 2 && ids_by_type.count(NFA_DFA))
		str += "N" + get_random_id_by_type(NFA_DFA);
	else if (rand() % 2 && ids_by_type.count(REGEX))
		str += "N" + get_random_id_by_type(REGEX);
	else
		str += regex_generator.generate_framed_regex();

	str += " ";

	str += regex_generator.generate_framed_regex();

	int rand_num = rand() % 5 + 1; // шаг итерации - пусть будет до 5..
	str += " " + to_string(rand_num);

	return str;
}

string TasksGenerator::generate_arguments(Function first_func) {
	string args_str = "";
	for (auto input_type : first_func.input) {
		if (for_static_tpchkr) {
			if (rand() % 2 && id_num > 1) {
				int rand_id = rand() % (id_num - 1) + 1;
				args_str += " N" + to_string(rand_id);
			} else {
				args_str += " " + regex_generator.generate_framed_regex();
			}
		} else {
			if ((for_dynamic_tpchkr && input_type == DFA) || input_type == NFA) {
				input_type = NFA_DFA;
			}

			if (input_type == REGEX && (!ids_by_type.count(REGEX) || rand() % 2)) {
				// сгенерировать регулярку
				args_str += " " + regex_generator.generate_framed_regex();
			} else if (input_type == INT && (!ids_by_type.count(INT) || rand() % 2)) {
				// сгенерировать число
				int rand_num = rand() % 5; // пусть будет до 5..
				args_str += " " + to_string(rand_num);
			} else if (input_type == ARRAY) {
				args_str += " [[{a} {b}]]";
			} else if (ids_by_type.count(input_type)) {
				args_str += " N" + get_random_id_by_type(input_type);
			} else {
				cout << "generator error: there is no id with type" +
							types_to_string.at(input_type) + "\n";
			}
		}
	}
	return args_str;
}

string TasksGenerator::get_random_id_by_type(ObjectType type) {
	vector<Id> possible_ids = ids_by_type[type];
	Id rand_id = possible_ids[rand() % possible_ids.size()];
	return to_string(rand_id.num);
}

string TasksGenerator::generate_declaration() {
	change_seed();
	id_num++;
	string str = "N" + to_string(id_num) + " = " + generate_expression() + " !!";

	// запоминаем идентификатор N#
	ids_by_type[cur_type].push_back({id_num, cur_type});
	if (cur_type == DFA || cur_type == NFA)
		ids_by_type[NFA_DFA].push_back({id_num, cur_type});

	return str;
}

Function TasksGenerator::generate_next_func(ObjectType prevOutput, int funcNum) {
	Function func;
	if (for_static_tpchkr) {
		func = rand_func();
	} else {
		if ((for_dynamic_tpchkr && prevOutput == NFA) || prevOutput == DFA)
			prevOutput = NFA_DFA;
		vector<Function> possible_functions = funcInput[prevOutput];
		func = possible_functions[rand() % possible_functions.size()];
		// может возвращать int, только если функция явл-ся последней в посл-ти
		if (!funcInput.count(func.output) && funcNum != 0)
			func = generate_next_func(prevOutput, funcNum);
	}
	return func;
}

void TasksGenerator::distribute_functions() {
	for (Function func : functions) {
		if (func.input.size() == 1) {
			ObjectType input_type = func.input[0];
			funcInput[input_type].push_back(func);
		}
	}
}

Function TasksGenerator::rand_func() {
	return functions[rand() % functions.size()];
}

/*
– По радио сообщили о переходе на зимнее время, сказав, что «этой ночью, ровно в
03:00 нужно перевести стрелку часов на один час назад, на 02:00».
– У всех программистов эта ночь зависла в бесконечном цикле.
*/