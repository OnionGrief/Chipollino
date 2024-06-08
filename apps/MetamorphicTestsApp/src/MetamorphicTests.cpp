#include <random>
#include <regex>
#include <string>
#include <unordered_set>

#include "MetamorphicTestsApp/MetamorphicTests.h"
#include "AutomataParser/Parser.h"
#include "InputGenerator/AutomatonGenerator.h"
#include "InputGenerator/RegexGenerator.h"
#include "Objects/BackRefRegex.h"
#include "Objects/FiniteAutomaton.h"
#include "Objects/MemoryFiniteAutomaton.h"
#include "Objects/Regex.h"
#include "gtest/gtest.h"

using std::cout;
using std::string;
using std::unordered_set;
using std::vector;

const int RegexNumber = 50;
const int RegexNumberX10 = RegexNumber * 10;

TEST(TestRegex, ToTxt) {
	RegexGenerator rg(15, 10, 5, 3);
	for (int i = 0; i < RegexNumberX10; i++) {
		string rgx_str = rg.generate_regex();
		SCOPED_TRACE("Regex: " + rgx_str);
		Regex r1(rgx_str);
		Regex r2(r1.to_txt());
		ASSERT_TRUE(Regex::equivalent(r1, r2));
		ASSERT_TRUE(Regex::equivalent(r1, r2.rewrite_aci()));
	}
}

TEST(TestArden, RandomRegexEquivalence) {
	RegexGenerator rg;
	for (int i = 0; i < RegexNumber; i++) {
		string rgx_str = rg.generate_regex();
		SCOPED_TRACE("Regex: " + rgx_str);
		Regex r1(rgx_str), r2(rgx_str);
		//		ASSERT_TRUE(Regex::equivalent(r1, r2.to_thompson().to_regex()));
		ASSERT_TRUE(Regex::equivalent(r1, r2.to_glushkov().to_regex()));
		ASSERT_TRUE(Regex::equivalent(r1, r2.to_ilieyu().to_regex()));
		ASSERT_TRUE(Regex::equivalent(r1, r2.to_antimirov().to_regex()));
	}
}

TEST(TestEqual, ThompsonGlushkov) {
	RegexGenerator rg;
	for (int i = 0; i < RegexNumberX10; i++) {
		string rgx_str = rg.generate_regex();
		SCOPED_TRACE("Regex: " + rgx_str);
		Regex r(rgx_str);
		FiniteAutomaton fa1 = r.to_thompson().remove_eps();
		FiniteAutomaton fa2 = r.to_glushkov();
		ASSERT_TRUE(FiniteAutomaton::equal(fa1, fa2));
		ASSERT_TRUE(FiniteAutomaton::bisimilar(fa1, fa2));
	}
}

TEST(TestNFA, NegativeNFAEquivalence) {
	RegexGenerator rg(5, 3, 3, 2);
	rg.set_neg_chance(50); // для отрицания
	for (int i = 0; i < RegexNumber; i++) {
		string rgx_str = rg.generate_regex();
		SCOPED_TRACE("Regex: " + rgx_str);
		Regex r1(rgx_str), r2(rgx_str);
		ASSERT_TRUE(FiniteAutomaton::equivalent(r1.to_thompson(), r2.to_antimirov()));
	}
}

string patch(const string& s) {
	std::regex pattern("\\|\\|+");
	return std::regex_replace(s, pattern, "|");
}

std::string MetamorphicTests::generate_bregex(RegexGenerator& rg, int cells_num) {
	string rgx_str;
	BackRefRegex r;
	bool condition;
	do {
		condition = true;
		rgx_str = patch(BackRefRegex(rg.generate_brefregex(cells_num, 70, 60)).to_txt());
		r = BackRefRegex(rgx_str);

		condition &=
			(rgx_str.find('&') != std::string::npos && rgx_str.find('[') != std::string::npos);
		for (int i = 1; i <= cells_num && condition; i++)
			if ((rgx_str.find(":" + std::to_string(i)) == std::string::npos) !=
				(rgx_str.find("&" + std::to_string(i)) == std::string::npos)) {
				condition = false;
			}
		if (condition)
			condition &= r.check_refs();
	} while (!condition);

	return rgx_str;
}

void MetamorphicTests::cmp_automatons(const MemoryFiniteAutomaton& mfa1,
									  const MemoryFiniteAutomaton& mfa2) {
	int len = mfa2.size();
	auto test_set1 = mfa1.generate_test_set(len);
	auto test_set2 = mfa2.generate_test_set(len);
	std::stringstream debug;
	debug << "Set sizes 1) " << test_set1.first.size() << " " << test_set1.second.size() << " 2) "
		  << test_set2.first.size() << " " << test_set2.second.size();
	ASSERT_EQ(test_set1.first, test_set2.first) << debug.str();

	vector<string> random_elements;
	std::sample(test_set1.second.begin(),
				test_set1.second.end(),
				std::back_inserter(random_elements),
				500,
				std::mt19937{std::random_device{}()});
	std::sample(test_set2.second.begin(),
				test_set2.second.end(),
				std::back_inserter(random_elements),
				500,
				std::mt19937{std::random_device{}()});
	unordered_set<string> random_set(random_elements.begin(), random_elements.end());
	//	int a = 0, b = 0;
	for (const auto& mutated_word : random_set) {
		auto res1 = mfa1.parse(mutated_word);
		auto res2 = mfa2.parse(mutated_word);
		ASSERT_EQ(res1.second, res2.second) << debug.str();
		//		if (res1.second)
		//			a++;
		//		else
		//			b++;
	}
	//	std::cout << " " << a << " " << b << " " << double(b) / (a + b) << "\n";
}

TEST(TestMFA, Fuzzing) {
	RegexGenerator rg(5, 3, 3, 2);
	for (int i = 0; i < RegexNumberX10; i++) {
		string rgx_str = MetamorphicTests::generate_bregex(rg, 2);
		SCOPED_TRACE("Regex: " + rgx_str);
		MemoryFiniteAutomaton mfa1 = BackRefRegex(rgx_str).to_mfa();
		MemoryFiniteAutomaton mfa2 = BackRefRegex(rgx_str).to_mfa_additional();

		MetamorphicTests::cmp_automatons(mfa1, mfa2);
	}
}

// TEST(TestMFA, Fuzz) {
//	string rgx_str = "(a[[b|]:1|]:2*[[c|]:1|]:2*&1&2)*";
//	MemoryFiniteAutomaton mfa1 = BackRefRegex(rgx_str).to_mfa();
//	MemoryFiniteAutomaton mfa2 = BackRefRegex(rgx_str).to_mfa_additional();
//
//	std::cout << mfa1.to_txt() << mfa2.to_txt();
//
//	MetamorphicTests::cmp_automatons(mfa1, mfa2);
//}

TEST(TestMFA, ToTxt) {
	RegexGenerator rg(5, 3, 3, 2);
	for (int i = 0; i < RegexNumberX10; i++) {
		string rgx_str = MetamorphicTests::generate_bregex(rg, 2);
		//		std::cout << i << " " << rgx_str << "\n";
		SCOPED_TRACE("Regex: " + rgx_str);
		BackRefRegex r = BackRefRegex(rgx_str);
		MemoryFiniteAutomaton mfa1 = r.to_mfa_additional();
		MemoryFiniteAutomaton mfa2 = BackRefRegex(r.to_txt()).to_mfa_additional();

		MetamorphicTests::cmp_automatons(mfa1, mfa2);
	}
}

TEST(IsDeterministic, Test_is_deterministic) {
	string test_path = "./test_data/MetamorphicTest/test1.txt";
	for (int i = 0; i < RegexNumber; i++) {
		AutomatonGenerator a(FA_type::NFA);
		a.write_to_file(test_path);
		Parser parser;
		auto FA = parser.parse_NFA(test_path);
		auto FAd = FA.determinize();
		ASSERT_TRUE(FAd.is_deterministic());
	}
}

TEST(AutomatonGenerator, Test_MergeBisim_equivalent) {
	string test_path = "./test_data/MetamorphicTest/test1.txt";
	for (int i = 0; i < RegexNumber; i++) {
		AutomatonGenerator a(FA_type::NFA, 5);
		a.write_to_file(test_path);
		Parser parser1, parse2;
		FiniteAutomaton FA, second;
		FA = parser1.parse_NFA(test_path);
		auto first = FA.merge_bisimilar();
		second = parse2.parse_NFA(test_path);

		std::ifstream t(test_path);
		std::stringstream buffer;
		buffer << t.rdbuf();
		string file = buffer.str();

		auto equality = FiniteAutomaton::equivalent(first, second);
		ASSERT_TRUE(equality) << file << "\n"
							  << first.to_txt() << "\n"
							  << second.to_txt() << "\n"
							  << FA.to_regex().to_txt();
	}
}

/*
 TEST(Statistics, Test_statistics) {
     string test_path = "./TestData/MetamorphicTest/test1.txt";
     std::vector<int> OX;
     std::vector<float> OY;
     AutomatonGenerator::set_initial_state_not_terminal(true);
     for (int term = 5; term <= 100; term = term + 5) {
         AutomatonGenerator::set_terminal_probability(term);
         int count = 0;
         int ALL = 10000;
         for (int i = 0; i < ALL; i++) {
             AutomatonGenerator a(FA_type::NFA);
             a.write_to_file(test_path);
             Parser parser;
             FiniteAutomaton FA;
             try {
                 FA = parser.parse_NFA(test_path);
             } catch (const std::runtime_error& re) {
                 std::ifstream t(test_path);
                 stringstream buffer;
                 buffer << t.rdbuf();
                 string file = buffer.str();
                 throw(std::runtime_error(file));
             }
             if (FA.is_finite()) {
                 count++;
             }
         }
         std::cout << "terminal_probability = " << term << " : " << float(count) / float(ALL) <<
         "%" << std::endl; OX.push_back(term); OY.push_back(float(count) / float(ALL));
     }
     std::cout << "OX = [";
     for (int i = 0; i < OX.size() - 1; i++) {
         std::cout << OX[i] << ",";
     }
     std::cout << OX[OX.size() - 1] << "]\n";

     std::cout << "OY = [";
     for (int i = 0; i < OY.size() - 1; i++) {
         std::cout << OY[i] << ",";
     }
     std::cout << OY[OY.size() - 1] << "]\n";
 }


TEST(AutomatonGenerator, Test_Arden_Glushkov_equivalent) {
	int ALL = 50;
	for (int i = 0; i < ALL; i++) {
		string test_path = "./TestData/MetamorphicTest/test1.txt";
		AutomatonGenerator a(FA_type::NFA, 5);
		a.write_to_file(test_path);
		Parser parser;
		FiniteAutomaton FA;
		FA = parser.parse_NFA(test_path);
		auto ard =  FA.to_regex().to_glushkov();
		auto first = ard;
		auto second = parser.parse_NFA(test_path);

		std::ifstream t(test_path);
		stringstream buffer;
		buffer << t.rdbuf();
		string file = buffer.str();

		auto equality = FiniteAutomaton::equivalent(first, second);
		ASSERT_TRUE(equality) << file << "\n" << FA.minimize().to_txt() << "\n" <<
ard.minimize().to_txt() << "\n" << FA.to_regex().to_txt();
	}
}

TEST(AutomatonGenerator, Test_Arden_Glushkov_Ambiguity_equivalent) {
	int ALL = 50;
	for (int i = 0; i < ALL; i++) {
		string test_path = "./TestData/MetamorphicTest/test1.txt";
		AutomatonGenerator a(FA_type::NFA, 5);
		a.write_to_file(test_path);
		Parser parser;
		FiniteAutomaton FA;
		FA = parser.parse_NFA(test_path);
		auto ard =  FA.to_regex().to_glushkov();
		auto first = ard.ambiguity();
		auto second = ard.to_regex().to_glushkov().ambiguity();

		std::ifstream t(test_path);
		stringstream buffer;
		buffer << t.rdbuf();
		string file = buffer.str();

		ASSERT_EQ(first,second) << file << "\n" << FA.minimize().to_txt() << "\n" <<
ard.minimize().to_txt() << "\n" << FA.to_regex().to_txt();
	}
}

 TEST(Statistics, Test_dfa) {
 	for (int term = 5; term <= 50; term = term + 5) {
 		AutomatonGenerator::set_terminal_probability(20);
 		int count = 0;
 		int ALL = 10000;
 		for (int i = 0; i < ALL; i++) {
 			AutomatonGenerator a(FA_type::DFA);
 			a.write_to_file("./TestData/tmp/test.txt");
 			auto FA = Parser::parse_DFA("./TestData/tmp/test.txt");
 			if (FA.is_deterministic() && FA.is_finite()) {
 				count++;
 			}
 		}
 		std::cout << "terminal_probability = " << term << " : " << float(count) / float(ALL) * 100<<
 "%" << std::endl;
 	}
 }

 TEST(Statistics, Test_fa) {
 	std::cout << "TEST\n";
 	for (int term = 5; term <= 50; term = term + 5) {
 		AutomatonGenerator::set_terminal_probability(20);
 		int count = 0;
 		int ALL = 10000;
 		for (int i = 0; i < ALL; i++) {
 			AutomatonGenerator a(FA_type::FA);
 			std::cout << "write_to_file START\n";
 			a.write_to_file("./TestData/tmp/test.txt");
 			std::cout << "write_to_file DONE\n";
 			auto FA = Parser::parse_FA("./TestData/tmp/test.txt");
 			std::cout << i << " " << std::endl;
 			if (FA.is_deterministic() && FA.is_finite()) {
 				count++;
 			}
 			std::cout << i << " " << std::endl;
 		}
 		std::cout << "terminal_probability = " << term << " : " << float(count) / float(ALL) * 100
 << "%" << std::endl;
 	}
 }*/
TEST(TestBisimilar, MFA_Bisimilar) {
	RegexGenerator rg(5, 3, 3, 2);
	for (int i = 0; i < RegexNumber; i++) {
		string rgx_str = MetamorphicTests::generate_bregex(rg, 1);
		SCOPED_TRACE("Regex: " + rgx_str);
		BackRefRegex r = BackRefRegex(rgx_str);
		MemoryFiniteAutomaton mfa = r.to_mfa_additional();

		ASSERT_TRUE(MemoryFiniteAutomaton::action_bisimilar(mfa, mfa));
		ASSERT_TRUE(MemoryFiniteAutomaton::symbolic_bisimilar(mfa, mfa));
		ASSERT_TRUE(MemoryFiniteAutomaton::bisimilar(mfa, mfa).value());
	}
}

TEST(TestBisimilar, MFA_MergeBisimilar) {
	RegexGenerator rg(6, 3, 3, 2);
	for (int i = 0; i < RegexNumber; i++) {
		string rgx_str = MetamorphicTests::generate_bregex(rg, 2);
		SCOPED_TRACE("Regex: " + rgx_str);
		BackRefRegex r = BackRefRegex(rgx_str);
		MemoryFiniteAutomaton mfa = r.to_mfa_additional();

		MetamorphicTests::cmp_automatons(mfa.merge_bisimilar(), mfa);
	}
}
