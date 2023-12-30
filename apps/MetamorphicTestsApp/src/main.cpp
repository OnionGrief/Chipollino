#include <iostream>

#include "Objects/Language.h"
#include "gtest/gtest.h"

using std::cout;

int main(int argc, char** argv) {
	cout << "Metamorphic Tests\n";
	// Тестирование
	Language::disable_retrieving_from_cache();
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
