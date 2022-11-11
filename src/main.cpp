#include "Example.h"
#include "Interpreter.h"
#include "Regex.h"
#include <iostream>
using namespace std;

int main(int argc, char* argv[]) {
	// Приветсвие
	cout << "Chipollino :-)\n";

	// Тестирование
	// Example::test_all();
	// Example::all_examples();

	// Инициализируем логгер
	Logger::init();
	Logger::activate();

	// Инициализируем интерпретатор
	Interpreter interpreter;
	interpreter.set_log_mode(Interpreter::LogMode::all);

	// Используем сгенерированный тест
	TasksGenerator TG;
	TG.generate_task(3, 5, false, false);
	TG.write_to_file();

	// Загружаем в интерпретатор файл с коммандами
	string load_file = "test.txt";
	if (argc > 1) load_file = argv[1];
	interpreter.load_file(load_file);

	// Выполняем коммнады
	interpreter.run_all();

	// Гененрируем выходной документ, завершаем работу логгера
	Logger::finish();
	Logger::deactivate();
}