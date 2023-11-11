#include <iostream>
#include <thread>
#include "gtest/gtest.h"

using std::cout;

int main(int argc, char** argv) {
	cout << "Integration Tests\n";
	// Тестирование
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
