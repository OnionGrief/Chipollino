#include <iostream>
#include "TestsApp/Example.h"
using namespace std;

int main() {
	cout << "Test\n";
	// Тестирование
	Example::test_all();
	Example::all_examples();
}