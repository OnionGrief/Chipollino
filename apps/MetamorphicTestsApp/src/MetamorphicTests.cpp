#include "gtest/gtest.h"

#include "InputGenerator/RegexGenerator.h"
#include "InputGenerator/AutomatonGenerator.h"
#include "AutomataParser/Parser.h"
#include "Objects/FiniteAutomaton.h"
#include "Objects/Regex.h"

using std::string;

const int RegexNumber = 30;

// TEST(TestParsing, RandomRegexParsing) {
// 	RegexGenerator rg(15, 10, 5, 3);
//     //rg.set_neg_chance(2); // для отрицания
// 	for (int i = 0; i < RegexNumber; i++) {
// 		string str = rg.generate_regex();
// 		Regex r1(str);
// 		string r1_str = r1.to_txt();
// 		Regex r2(r1_str);
// 		ASSERT_EQ(true, Regex::equivalent(r1, r2)) << str;
// 	}
// }

// TEST(TestArden, RandomRegexEquivalence) {
// 	RegexGenerator rg;
// 	for (int i = 0; i < RegexNumber; i++) {
// 		std::cout << "RandomRegexEquivalence: regex number " << i << std::endl;
// 		string rgx_str = rg.generate_regex();
// 		Regex r1(rgx_str), r2(rgx_str);
// 		//		ASSERT_TRUE(Regex::equivalent(r1, r2.to_thompson().to_regex()));
// 		ASSERT_TRUE(Regex::equivalent(r1, r2.to_glushkov().to_regex())) << rgx_str;
// 		ASSERT_TRUE(Regex::equivalent(r1, r2.to_ilieyu().to_regex())) << rgx_str;
// 		ASSERT_TRUE(Regex::equivalent(r1, r2.to_antimirov().to_regex())) << rgx_str;
// 	}
// }

// TEST(TestEqual, ThompsonGlushkov) {
// 	RegexGenerator rg;
// 	for (int i = 0; i < RegexNumber; i++) {
// 		string rgx_str = rg.generate_regex();
// 		Regex r(rgx_str);
// 		ASSERT_TRUE(FiniteAutomaton::equal(r.to_thompson().remove_eps(), r.to_glushkov()))
// 			<< rgx_str;
// 	}
// }

// TEST(TestNFA, Test_equivalent_nfa_negative) {
// 	RegexGenerator rg(5, 2, 2, 2);
// 	rg.set_neg_chance(2); // для отрицания
// 	for (int i = 0; i < 30; i++) {
// 		string str = rg.generate_regex();
// 		Regex r1(str), r2(str);
// 		ASSERT_TRUE(FiniteAutomaton::equivalent(r1.to_thompson(), r2.to_antimirov())) << str;
// 	}
// }

TEST(IsDeterministic, Test_is_deterministic) {
	for (int i = 0; i < 1000; i++) {
		AutomatonGenerator a(FA_type::FA);
		a.write_to_file("./TestData/tmp/test.txt");
		auto FA = Parser::parse_FA("./TestData/tmp/test.txt");
		auto FAd = FA.determinize();
		ASSERT_TRUE(FAd.is_deterministic()); 
	}
}

// TEST(Statistics, Test_statistics) {

// 	for (int term = 5; term <= 100; term = term + 5) {
// 		AutomatonGenerator::set_terminal_probability(term);
// 		int count = 0;
// 		int ALL = 10000;
// 		for (int i = 0; i < ALL; i++) {
// 			AutomatonGenerator a(FA_type::FA);
// 			a.write_to_file("./TestData/tmp/test.txt");
// 			auto FA = Parser::parse_FA("./TestData/tmp/test.txt");
// 			if (FA.is_finite()) {
// 				count++;
// 			}
// 		}
// 		std::cout << "terminal_probability = " << term << " : " << float(count) / float(ALL) << "%" << std::endl;
// 	}
// }

// TEST(Statistics, Test_dfa) {
// 	for (int term = 5; term <= 50; term = term + 5) {
// 		AutomatonGenerator::set_terminal_probability(20);
// 		int count = 0;
// 		int ALL = 10000;
// 		for (int i = 0; i < ALL; i++) {
// 			AutomatonGenerator a(FA_type::DFA);
// 			a.write_to_file("./TestData/tmp/test.txt");
// 			auto FA = Parser::parse_DFA("./TestData/tmp/test.txt");
// 			if (FA.is_deterministic() && FA.is_finite()) {
// 				count++;
// 			}
// 		}
// 		std::cout << "terminal_probability = " << term << " : " << float(count) / float(ALL) * 100<< "%" << std::endl;
// 	}
// }

TEST(Statistics, Test_fa) {
	for (int term = 5; term <= 50; term = term + 5) {
		AutomatonGenerator::set_terminal_probability(20);
		int count = 0;
		int ALL = 10000;
		for (int i = 0; i < ALL; i++) {
			AutomatonGenerator a(FA_type::FA);
			a.write_to_file("./TestData/tmp/test.txt");
			auto FA = Parser::parse_FA("./TestData/tmp/test.txt");
			std::cout << i << " " << std::endl;
			if (FA.is_deterministic() && FA.is_finite()) {
				count++;
			}
			std::cout << i << " " << std::endl;
		}
		std::cout << "terminal_probability = " << term << " : " << float(count) / float(ALL) * 100 << "%" << std::endl;
	}
}