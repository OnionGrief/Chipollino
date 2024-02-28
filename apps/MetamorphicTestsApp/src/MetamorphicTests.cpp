#include "gtest/gtest.h"

#include "InputGenerator/RegexGenerator.h"
#include "InputGenerator/AutomatonGenerator.h"
#include "AutomataParser/Parser.h"
#include "Objects/FiniteAutomaton.h"
#include "Objects/Regex.h"

using std::string;

const int RegexNumber = 30;

TEST(TestParsing, RandomRegexParsing) {
	RegexGenerator rg(15, 10, 5, 3);
    //rg.set_neg_chance(2); // для отрицания
	for (int i = 0; i < RegexNumber; i++) {
		string str = rg.generate_regex();
		Regex r1(str);
		string r1_str = r1.to_txt();
		Regex r2(r1_str);
		ASSERT_EQ(true, Regex::equivalent(r1, r2)) << str;
	}
}

TEST(TestArden, RandomRegexEquivalence) {
	RegexGenerator rg;
	for (int i = 0; i < RegexNumber; i++) {
		std::cout << "RandomRegexEquivalence: regex number " << i << std::endl;
		string rgx_str = rg.generate_regex();
		Regex r1(rgx_str), r2(rgx_str);
		//		ASSERT_TRUE(Regex::equivalent(r1, r2.to_thompson().to_regex()));
		ASSERT_TRUE(Regex::equivalent(r1, r2.to_glushkov().to_regex())) << rgx_str;
		ASSERT_TRUE(Regex::equivalent(r1, r2.to_ilieyu().to_regex())) << rgx_str;
		ASSERT_TRUE(Regex::equivalent(r1, r2.to_antimirov().to_regex())) << rgx_str;
	}
}

TEST(TestEqual, ThompsonGlushkov) {
	RegexGenerator rg;
	for (int i = 0; i < RegexNumber; i++) {
		string rgx_str = rg.generate_regex();
		Regex r(rgx_str);
		ASSERT_TRUE(FiniteAutomaton::equal(r.to_thompson().remove_eps(), r.to_glushkov()))
			<< rgx_str;
	}
}

TEST(TestNFA, Test_equivalent_nfa_negative) {
	RegexGenerator rg(5, 2, 2, 2);
	rg.set_neg_chance(2); // для отрицания
	for (int i = 0; i < 30; i++) {
		string str = rg.generate_regex();
		Regex r1(str), r2(str);
		ASSERT_TRUE(FiniteAutomaton::equivalent(r1.to_thompson(), r2.to_antimirov())) << str;
	}
}

TEST(IsDeterministic, Test_is_deterministic) {
    std::string grammar_path = "./TestData/grammar.txt";
    std::string test_path = "./TestData/MetamorphicTest/test1.txt";
	for (int i = 0; i < 1000; i++) {
		AutomatonGenerator a(grammar_path, FA_type::NFA);
		a.write_to_file(test_path);
        Parser parser;
		auto FA = parser.parse_NFA(grammar_path, test_path);
		auto FAd = FA.determinize();
		ASSERT_TRUE(FAd.is_deterministic()); 
	}
}

// TEST(Statistics, Test_statistics) {
//     std::string grammar_path = "./TestData/grammar.txt";
//     std::string test_path = "./TestData/MetamorphicTest/test1.txt";
//     std::vector<int> OX;
//     std::vector<float> OY;
//     AutomatonGenerator::set_initial_state_not_terminal(true);
//     for (int term = 5; term <= 100; term = term + 5) {
//         AutomatonGenerator::set_terminal_probability(term);
//         int count = 0;
//         int ALL = 10000;
//         for (int i = 0; i < ALL; i++) {
//             AutomatonGenerator a(grammar_path, FA_type::NFA);
//             a.write_to_file(test_path);
//             Parser parser;
//             FiniteAutomaton FA;
//             try {
//                 FA = parser.parse_NFA(grammar_path, test_path); 
//             } catch (const std::runtime_error& re) {
//                 std::ifstream t(test_path);
//                 std::stringstream buffer;
//                 buffer << t.rdbuf();
//                 std::string file = buffer.str();
//                 throw(std::runtime_error(file));
//             }
//             if (FA.is_finite()) {
//                 count++;
//             }
//         }
//         std::cout << "terminal_probability = " << term << " : " << float(count) / float(ALL) << "%" << std::endl;
//         OX.push_back(term);
//         OY.push_back(float(count) / float(ALL));
//     }
//     std::cout << "OX = [";
//     for (int i = 0; i < OX.size() - 1; i++) {
//         std::cout << OX[i] << ",";
//     }
//     std::cout << OX[OX.size() - 1] << "]\n";

//     std::cout << "OY = [";
//     for (int i = 0; i < OY.size() - 1; i++) {
//         std::cout << OY[i] << ",";
//     }
//     std::cout << OY[OY.size() - 1] << "]\n";
// }

TEST(AutomatonGenerator, Test_MergeBisim_equivalent) {
    int ALL = 10000;
    for (int i = 0; i < ALL; i++) {
        std::string grammar_path = "./TestData/grammar.txt";
        std::string test_path = "./TestData/MetamorphicTest/test1.txt";
        AutomatonGenerator a(grammar_path, FA_type::NFA, 5);
        a.write_to_file(test_path);
        Parser parser;
        FiniteAutomaton FA;
        FA = parser.parse_NFA(grammar_path, test_path);
        auto first = FA.merge_bisimilar();
        auto second = FA;

        std::ifstream t(test_path);
        std::stringstream buffer;
        buffer << t.rdbuf();
        std::string file = buffer.str();

        auto equality = FiniteAutomaton::equivalent(first, second);
        ASSERT_TRUE(equality) << file << "\n" << first.to_txt() << "\n" << second.to_txt() << "\n" << FA.to_regex().to_txt();
    }
}

TEST(AutomatonGenerator, Test_Arden_Glushkov_equivalent) {
    int ALL = 10000;
    for (int i = 0; i < ALL; i++) {
        std::string grammar_path = "./TestData/grammar.txt";
        std::string test_path = "./TestData/MetamorphicTest/test1.txt";
        AutomatonGenerator a(grammar_path, FA_type::NFA, 5);
        a.write_to_file(test_path);
        Parser parser;
        FiniteAutomaton FA;
        FA = parser.parse_NFA(grammar_path, test_path);
		auto ard =  FA.to_regex().to_glushkov();
        auto first = ard;
        auto second = FA;

        std::ifstream t(test_path);
        std::stringstream buffer;
        buffer << t.rdbuf();
        std::string file = buffer.str();

        auto equality = FiniteAutomaton::equivalent(first, second);
        ASSERT_TRUE(equality) << file << "\n" << FA.minimize().to_txt() << "\n" << ard.minimize().to_txt() << "\n" << FA.to_regex().to_txt();
    }
}

TEST(AutomatonGenerator, Test_Arden_Glushkov_Ambiguity_equivalent) {
    int ALL = 10000;
    for (int i = 0; i < ALL; i++) {
        std::string grammar_path = "./TestData/grammar.txt";
        std::string test_path = "./TestData/MetamorphicTest/test1.txt";
        AutomatonGenerator a(grammar_path, FA_type::NFA, 5);
        a.write_to_file(test_path);
        Parser parser;
        FiniteAutomaton FA;
        FA = parser.parse_NFA(grammar_path, test_path);
		auto ard =  FA.to_regex().to_glushkov();
        auto first = ard.ambiguity();
        auto second = ard.to_regex().to_glushkov().ambiguity();

        std::ifstream t(test_path);
        std::stringstream buffer;
        buffer << t.rdbuf();
        std::string file = buffer.str();

        ASSERT_EQ(first,second) << file << "\n" << FA.minimize().to_txt() << "\n" << ard.minimize().to_txt() << "\n" << FA.to_regex().to_txt();
    }
}

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

// TEST(Statistics, Test_fa) {
// 	std::cout << "TEST\n";
// 	for (int term = 5; term <= 50; term = term + 5) {
// 		AutomatonGenerator::set_terminal_probability(20);
// 		int count = 0;
// 		int ALL = 10000;
// 		for (int i = 0; i < ALL; i++) {
// 			AutomatonGenerator a(FA_type::FA);
// 			std::cout << "write_to_file START\n";
// 			a.write_to_file("./TestData/tmp/test.txt");
// 			std::cout << "write_to_file DONE\n";
// 			auto FA = Parser::parse_FA("./TestData/tmp/test.txt");
// 			std::cout << i << " " << std::endl;
// 			if (FA.is_deterministic() && FA.is_finite()) {
// 				count++;
// 			}
// 			std::cout << i << " " << std::endl;
// 		}
// 		std::cout << "terminal_probability = " << term << " : " << float(count) / float(ALL) * 100 << "%" << std::endl;
// 	}
// }