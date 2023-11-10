#include "UnitTestsApp/Example.h"
#include "UnitTestsApp/UnitTests.h"
#include "gtest/gtest.h"
#include <iostream>
#include <thread>
using namespace std;

int main(int argc, char** argv) {
	cout << "Unit Tests\n";
	// Тестирование
	return UnitTests::RunTests(argc, argv);
}
