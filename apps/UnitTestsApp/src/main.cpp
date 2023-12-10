#include <iostream>

#include "Objects/BackRefRegex.h"
#include "Objects/FiniteAutomaton.h"
#include "Objects/Grammar.h"
#include "Objects/Language.h"
#include "Objects/MemoryFiniteAutomaton.h"
#include "Objects/Regex.h"
#include "Objects/TransformationMonoid.h"
#include "UnitTestsApp/UnitTests.h"

using std::cout;

int main(int argc, char** argv) {
	cout << "Unit Tests\n";
	// Тестирование
	return UnitTests::RunTests(argc, argv);
}
