#include <iostream>

#include "InputGenerator/TasksGenerator.h"
#include "Interpreter/Interpreter.h"

int main() {

	/*
	// Если кому-то вдруг нужна подборка трешовых регулярок
	// RegexGenerator(int regex_length, int star_num, int star_nesting, int alphabet_size)
	RegexGenerator RG(100, 20, 4, 4);

	std::ofstream out;
	out.open("test_regexex.txt", std::ofstream::trunc);
	for (int i = 0; i < 10; i++) {
		if (out.is_open())
			out << RG.generate_regex() << "\n";
	}
	out.close();
	*/

	std::cout << "Input generator\n";

	// Инициализируем интерпретатор
	Interpreter interpreter;
	interpreter.set_log_mode(Interpreter::LogMode::all);

	// Используем сгенерированный тест
	TasksGenerator TG;
	TG.generate_task(3, 5, false, false);
	// TG.generate_test_for_all_functions();
	TG.write_to_file("test.txt");

	// Загружаем в интерпретатор файл с коммандами
	if (interpreter.run_file("test.txt"))
		interpreter.generate_log("./resources/report.tex");
}