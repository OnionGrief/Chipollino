#include "TestsApp/Example.h"
// #include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <iostream>
using namespace std;

int main(int argc, char **argv) {
	cout << "Test\n";
	// Тестирование
	// Example::test_all();
	// Example::all_examples();
	// Example::logger_test();
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}


