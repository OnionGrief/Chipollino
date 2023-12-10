#include <iostream>

#include "UnitTestsApp/UnitTests.h"

using std::cout;

int main(int argc, char** argv) {
	cout << "Unit Tests\n";
	// Тестирование
	Language::disable_retrieving_from_cache();
	return UnitTests::RunTests(argc, argv);
}
