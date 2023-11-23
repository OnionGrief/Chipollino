#include <iostream>

#include "Interpreter/Interpreter.h"

int main(int argc, char* argv[]) {
	// Приветствие
	std::cout << "Chipollino :-)\n";

	// Инициализируем интерпретатор
	Interpreter interpreter;
	interpreter.set_log_mode(Interpreter::LogMode::all);

	// Загружаем в интерпретатор файл с коммандами
	std::string load_file = "test.txt";
	if (argc > 1)
		load_file = argv[1];
	if (interpreter.run_file(load_file)) {
		interpreter.generate_log("./resources/report.tex");
	}
}