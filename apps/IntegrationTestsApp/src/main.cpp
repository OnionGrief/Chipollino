#include "gtest/gtest.h"
#include <iostream>
#include <thread>
using namespace std;

int main(int argc, char** argv) {
	cout << "Integration Tests\n";
	// Тестирование
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}