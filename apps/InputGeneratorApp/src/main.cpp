#include "InputGenerator/TasksGenerator.h"
#include "Interpreter/Interpreter.h"
#include <iostream>
using namespace std;

int main() {

	/*
	// Если кому-то вдруг нужна подборка трешовых регулярок
	// RegexGenerator(int regex_length, int star_num, int star_nesting, int
	// alphabet_size)
	RegexGenerator RG(100, 20, 4, 4);

	ofstream out;
	out.open("test.txt", ofstream::trunc);
	for (int i = 0; i < 10; i++) {
		if (out.is_open()) out << RG.generate_regex() << "\n";
	}
	out.close();
	*/

	cout << "Input generator\n";
	//Logger::init();
	//Logger::activate();

	// Инициализируем интерпретатор
	Interpreter interpreter;
	interpreter.set_log_mode(Interpreter::LogMode::all);

	// Используем сгенерированный тест
	TasksGenerator TG;
	TG.generate_task(3, 5, false, false);
	TG.write_to_file("test.txt");

	// Загружаем в интерпретатор файл с коммандами
	interpreter.run_file("test.txt");
	interpreter.generate_log("./resources/report.tex");

	// Гененрируем выходной документ, завершаем работу логгера
	//Logger::finish();
	//Logger::deactivate();
}