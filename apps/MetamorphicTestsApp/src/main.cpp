#include <iostream>
#include "Objects/FiniteAutomaton.h"
#include "Objects/Regex.h"
#include "Objects/Grammar.h"
#include "gtest/gtest.h"

using std::cout;

int main(int argc, char** argv) {
	cout << "Metamorphic Tests\n";
	// Тестирование
	std::string s = "b(^(a|^a))*", s2 = ("^(a(^(b^(|^a)||||^b))*)"); // ^(a(^(b^(|^a)||||^b))*)
	Regex r1(s2), r2(s2);
	std::cout << "Томпсон:\n" << r1.to_thompson().to_txt() << std::endl << "Антимирова:\n" << r2.to_antimirov().to_txt() << std::endl;
	std::cout << r1.to_thompson().minimize().to_txt() << std::endl;
	std::cout << r2.to_antimirov().minimize().to_txt() << std::endl;


	return 0;
	// ::testing::InitGoogleTest(&argc, argv);
	// return RUN_ALL_TESTS();
}
