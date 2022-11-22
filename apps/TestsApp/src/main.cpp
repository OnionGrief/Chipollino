#include "TestsApp/Example.h"
#include <iostream>
using namespace std;

int main() {
	cout << "Test\n";
	Logger::init();
	Logger::activate();
	// Тестирование
	Example::test_all();
	// Example::all_examples();
	Logger::finish();
	Logger::deactivate();
}