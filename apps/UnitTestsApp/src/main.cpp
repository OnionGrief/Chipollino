#include "UnitTestsApp/UnitTests.h"
#include <iostream>
using namespace std;

int main(int argc, char** argv) {
	cout << "Unit Tests\n";
	// Тестирование
	return UnitTests::RunTests(argc, argv);
}
