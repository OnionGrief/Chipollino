#include "Example.h"
#include <iostream>
using namespace std;

int main() {
	Example::tester();
	cout << "Chipollino :-)\n";
	Logger::init();
	Logger::activate();
	Example::tester();
	Logger::deactivate();
	Logger::finish();
}