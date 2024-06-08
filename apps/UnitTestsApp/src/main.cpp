#include <iostream>

#include "Objects/BackRefRegex.h"
#include "Objects/FiniteAutomaton.h"
#include "Objects/Grammar.h"
#include "Objects/Language.h"
#include "Objects/MemoryFiniteAutomaton.h"
#include "Objects/Regex.h"
#include "Objects/TransformationMonoid.h"
#include "UnitTestsApp/UnitTests.h"

#include "InputGenerator/AutomatonGenerator.h"

using std::cout;
using std::string;

int main(int argc, char** argv) {
	cout << "Unit Tests\n";
	// Тестирование
	Language::disable_retrieving_from_cache();
	return UnitTests::RunTests(argc, argv);
}
