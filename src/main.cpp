#include "Example.h"
#include <iostream>
#include "Regex.h"
#include "Interpreter.h"
using namespace std;

int main() {
	cout << "Chipollino :-)\n";
	Interpreter interpreter;
	interpreter.set_log_mode(Interpreter::LogMode::all);
	interpreter.load_file("test.txt");
	interpreter.run_all();
}