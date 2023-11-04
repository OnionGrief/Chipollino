#include "TestsApp/Example.h"
// #include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <iostream>
#include <thread>
using namespace std;


int main(int argc, char **argv) {
	cout << "Test\n";
	// Тестирование
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();;
}


