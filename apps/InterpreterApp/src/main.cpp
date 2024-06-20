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
	std::string user_name = "";
	if (argc > 1)
		load_file = argv[1];
	if (argc > 2)
		user_name = argv[2];

	try {
		if (interpreter.run_file(load_file, user_name)) {
			interpreter.generate_log("./resources/" + user_name + "report.tex");
		} else {
			exit(1);
		}
	} catch (const std::exception& e) {
		std::cerr << "Run error: " << e.what() << "\n";
	}
}