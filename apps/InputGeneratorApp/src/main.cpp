#include <iostream>

#include "InputGenerator/TasksGenerator.h"
#include "Interpreter/Interpreter.h"

int main(int argc, char* argv[]) {

	/*
	// Если кому-то вдруг нужна подборка трешовых регулярок
	// RegexGenerator(int regex_length, int star_num, int star_nesting, int alphabet_size)
	RegexGenerator RG(100, 20, 4, 4);

	std::ofstream out;
	out.open("test_regexes.txt", std::ofstream::trunc);
	for (int i = 0; i < 10; i++) {
		if (out.is_open())
			out << RG.generate_regex() << "\n";
	}
	out.close();
	*/

	std::cout << "Input generator\n";

	std::string task = "Test";
	bool run = true;

	if (argc > 1)
		task = argv[1];
	if (argc > 2 && std::string(argv[2]) == "false")
		run = false;

	TasksGenerator TG;
	if (task == "Test") {
		std::string res = TG.generate_task(3, 5, false, false);
		// TG.generate_test_for_all_functions();
		if (run) {
			TG.write_to_file("test.txt");
			// Инициализируем интерпретатор
			Interpreter interpreter;
			interpreter.set_log_mode(Interpreter::LogMode::all);
			// Используем сгенерированный тест
			if (interpreter.run_file("test.txt"))
				interpreter.generate_log("./resources/report.tex");
		} else {
			std::cout << res;
		}
	} else if (task == "Regex") {
		std::cout << TG.generate_regex() << "\n";
	} else if (task == "BackRefRegex") {
		std::cout << TG.generate_brefregex() << "\n";
	} else if (task == "NFA") {
		std::cout << TG.generate_NFA() << "\n";
	} else if (task == "MFA") {
		std::cout << TG.generate_MFA() << "\n";
	}
}