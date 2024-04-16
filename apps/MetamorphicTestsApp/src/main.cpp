#include <iostream>

#include "MetamorphicTestsApp/MetamorphicTests.h"
#include "Objects/Language.h"

using std::cout;

int main(int argc, char** argv) {
	cout << "Metamorphic Tests\n";
	// Тестирование
	Language::disable_retrieving_from_cache();
	return MetamorphicTests::RunTests(argc, argv);
}
