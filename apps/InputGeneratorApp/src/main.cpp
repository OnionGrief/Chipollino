#include <iostream>
#include "InputGenerator/TasksGenerator.h"
#include "Interpreter/Interpreter.h"
using namespace std;

int main() {
	cout << "Input generator\n";
	Logger::init();
	Logger::activate();

	// Инициализируем интерпретатор
	Interpreter interpreter;
	interpreter.set_log_mode(Interpreter::LogMode::all);
	
	// Используем сгенерированный тест
	TasksGenerator TG;
	TG.generate_task(3, 5, false, false);
	TG.write_to_file("test.txt");

	// Загружаем в интерпретатор файл с коммандами
	interpreter.load_file("test.txt");

	// Выполняем коммнады
	interpreter.run_all();

	// Гененрируем выходной документ, завершаем работу логгера
	Logger::finish();
	Logger::deactivate();
}