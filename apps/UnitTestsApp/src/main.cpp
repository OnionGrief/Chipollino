#include <iostream>
#include <thread>
#include "UnitTestsApp/Example.h"
#include "gtest/gtest.h"

int main(int argc, char** argv) {
	cout << "Unit Tests\n";
	// Тестирование
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
