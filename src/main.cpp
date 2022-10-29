#include "Example.h"
#include <iostream>
#include "Regex.h"
#include "Interpreter.h"
using namespace std;

int main() {
	cout << "Chipollino :-)\n";
	Interpreter interpreter;
	interpreter.load_file("test.txt");
	interpreter.run_all();
}