#include "Interpreter/Interpreter.h"
#include "Objects/Regex.h"
#include <iostream>
using namespace std;

int main(int argc, char* argv[]) {
	// Приветсвие
	cout << "Chipollino :-)\n";

	// Инициализируем интерпретатор
	Interpreter interpreter;
	interpreter.set_log_mode(Interpreter::LogMode::all);

	// Загружаем в интерпретатор файл с коммандами
	string load_file = "test.txt";
	if (argc > 1) load_file = argv[1];
	interpreter.run_file(load_file);
	interpreter.generate_log("log.tex");

	// Гененрируем выходной документ, завершаем работу логгера
	//Logger::finish();
	//Logger::deactivate();
}