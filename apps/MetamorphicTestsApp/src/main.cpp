#include <iostream>

#include "gtest/gtest.h"

using std::cout;

int main(int argc, char** argv) {
	cout << "Metamorphic Tests\n";
	// Тестирование
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
