#include "Example.h"
#include <iostream>
#include "Regex.h"
#include "Interpreter.h"
using namespace std;

int main() {
	cout << "Chipollino :-)\n";
    ofstream f("im.here");
	Interpreter interpreter;
	interpreter.load_file("test.txt");
	interpreter.run_all();
}