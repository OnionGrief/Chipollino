#include "InputGenerator/TasksGenerator.h"

using std::cout;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::to_string;
using std::vector;

using FuncLib::Function;

TasksGenerator::TasksGenerator() {
	change_seed();
	distribute_functions();
}

void TasksGenerator::change_seed() {
	seed_it++;
	srand(static_cast<unsigned int>((size_t)time(nullptr) + seed_it + rand()));
}

bool TasksGenerator::check_probability(int percentage) {
	if (rand() % 100 <= percentage)
		return true;
	return false;
}

string TasksGenerator::generate_regex() {
	return "{" + regex_generator.generate_regex() + "}";
}

string TasksGenerator::generate_brefregex() {
	return "{" + regex_generator.generate_brefregex() + "}";
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

bool TasksGenerator::arguments_exist(vector<ObjectType> args) {
	for (ObjectType arg_type : args) {
		// аргумент можно подобрать, если его тип есть в памяти, либо его можно сгенерировать
		if (!(ids_by_type.count(arg_type) || Typization::is_belong(generated_types, arg_type)))
			return false;
	}
	return true;
}

string TasksGenerator::generate_expression() {
	change_seed();

	int funcNum = max_num_of_func_in_seq > 0 ? rand() % (max_num_of_func_in_seq + 1) : 0;

	string str = "";

	if (funcNum == 0) {
		if (check_probability(30) && ids_by_type.size() > 0) {
			/* N# = id */
			map<ObjectType, vector<Id>>::iterator it = ids_by_type.begin();
			advance(it, (rand() % ids_by_type.size()));
			cur_type = it->first;
			str += "N" + get_random_id_by_type(cur_type);
		} else {
			/* N# = {regex}*/
			str += generate_regex();
			cur_type = REGEX;
		}
	} else {

		/* генерация первой функции в посл-ти */
		Function first_func;

		do {
			first_func = rand_func();
		} while (
			// ф/я не подходит, если она не последняя в последовательности и на выходе не
			// принимаемый другими функциями тип данных
			((!funcInput.count(first_func.output) && funcNum > 1) ||
			 // ф/я не подходит, если невозможно подобрать для нее аргументы
			 !arguments_exist(first_func.input)));

		str += first_func.name;
		str += generate_arguments(first_func);

		cur_type = first_func.output;

		/* генерация посл-ти функций */
		for (int i = 1; i < funcNum; i++) {
			Function func = generate_next_func(cur_type, funcNum - 1 - i);
			cur_type = func.output;
			str = func.name + '.' + str;
		}

		str += " !!";
	}
	return str;
}

string TasksGenerator::generate_test() {
	change_seed();
	string str = "Test ";

	if (check_probability(50) && ids_by_type.count(NFA))
		str += "N" + get_random_id_by_type(NFA);
	else if (check_probability(50) && ids_by_type.count(REGEX))
		str += "N" + get_random_id_by_type(REGEX);
	else
		str += generate_regex();

	str += " " + generate_regex();

	int rand_num = rand() % 5 + 1; // шаг итерации - пусть будет до 5..
	str += " " + to_string(rand_num);

	return str;
}

string TasksGenerator::generate_arguments(Function first_func) {
	string args_str = "";
	for (auto input_type : first_func.input) {
		// сгенерировать идентификатор
		if (ids_by_type.count(input_type) &&
			!(Typization::is_belong(generated_types, input_type) && check_probability(50))) {
			args_str += " N" + get_random_id_by_type(input_type);
			/* генерируемые типы: */
		} else if (input_type == NFA) {
			std::string filename = std::to_string(automata_id++) + ".txt";
			AutomatonGenerator(FA_type::FA).write_to_file(filename);
			args_str += " (getNFA \"" + filename + "\")";
		} else if (input_type == MFA) {
			std::string filename = std::to_string(automata_id++) + ".txt";
			AutomatonGenerator(FA_type::MFA).write_to_file(filename);
			args_str += " (getMFA \"" + filename + "\")";
		} else if (input_type == REGEX) {
			args_str += " " + generate_regex();
		} else if (input_type == BRefRegex) {
			args_str += " " + generate_brefregex();
		} else if (input_type == INT) {
			int rand_num = rand() % 5; // пусть будет до 5..
			args_str += " " + to_string(rand_num);
		} else if (input_type == ARRAY) {
			args_str += " [[{a} {b}]]";
		} else {
			cout << "generator error: there is no id with type " +
						Typization::types_to_string.at(input_type) + "\n";
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
	string str = "N" + to_string(id_num) + " = " + generate_expression();

	// запоминаем идентификатор N#
	// (DFA может быть подан в качестве NFA)
	for (ObjectType type : Typization::get_types(cur_type, Typization::types_parents))
		ids_by_type[type].push_back({id_num, cur_type});

	return str;
}

Function TasksGenerator::generate_next_func(ObjectType prevOutput, int funcNum) {
	Function func;
	if (for_static_tpchkr) {
		func = rand_func();
	} else {
		vector<Function> possible_functions = funcInput[prevOutput];
		func = possible_functions[rand() % possible_functions.size()];
		// на выходе дб принимаемый тип данных, если ф/я явл-ся не последней в посл-ти
		if (!funcInput.count(func.output) && funcNum != 0)
			func = generate_next_func(prevOutput, funcNum);
	}
	return func;
}

void TasksGenerator::distribute_functions() {
	for (Function func : FuncLib::functions) {
		if (func.input.size() == 1) {
			ObjectType input_type = func.input[0];
			/* ф/я может принимать свой входной тип и его подтипы*/
			for (ObjectType type : Typization::get_types(input_type, Typization::types_children))
				funcInput[type].push_back(func);
		}
	}
}

Function TasksGenerator::rand_func() {
	return FuncLib::functions[rand() % FuncLib::functions.size()];
}

void TasksGenerator::generate_test_for_all_functions() {
	ofstream outfile("./resources/all_functions.txt");
	outfile << "Set log_theory true\n";
	outfile << "R = {a*b*}\n";
	outfile << "BR = {[a*]:1b&1}\n";
	outfile << "A = Determinize.Glushkov R\n";
	outfile << "V = Ambiguity A\n";
	outfile << "B = Deterministic A\n";
	outfile << "P = PrefixGrammar A\n";
	for (const auto& function : FuncLib::functions) {
		string func_id = function.name;
		outfile << "N = " << func_id;
		for (const auto& arg : function.input) {
			if (arg == NFA || arg == DFA) {
				outfile << " A";
			} else if (arg == VALUE) {
				outfile << " V";
			} else if (arg == BOOLEAN || arg == ObjectType::OptionalBool) {
				outfile << " B";
			} else if (arg == PG) {
				outfile << " P";
			} else if (arg == REGEX) {
				outfile << " R";
			} else if (arg == BRefRegex) {
				outfile << " BR";
			} else if (arg == ARRAY) {
				outfile << " [[{a} {b}]]";
			} else if (arg == INT) {
				outfile << " 1";
			} else {
				cout << "generator error: can't generate arg " +
							Typization::types_to_string.at(arg) + '\n';
			}
		}
		outfile << "\n";
	}
}

/*
– По радио сообщили о переходе на зимнее время, сказав, что «этой ночью, ровно в
03:00 нужно перевести стрелку часов на один час назад, на 02:00».
– У всех программистов эта ночь зависла в бесконечном цикле.
*/