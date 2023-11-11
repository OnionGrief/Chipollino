#include <iostream>

#include "UnitTestsApp/UnitTests.h"

using std::cout;

int main(int argc, char** argv) {
	cout << "Unit Tests\n";
	// Тестирование
	return UnitTests::RunTests(argc, argv);
}
