#include "gtest/gtest.h"

#include "InputGenerator/RegexGenerator.h"
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
		ASSERT_EQ(true, Regex::equivalent(r1, r2));
	}
}

TEST(TestArden, RandomRegexEquivalence) {
	RegexGenerator rg;
	for (int i = 0; i < RegexNumber; i++) {
		std::cout << "RandomRegexEquivalence: regex number " << i << std::endl;
		string rgx_str = rg.generate_regex();
		Regex r1(rgx_str), r2(rgx_str);
		//		ASSERT_TRUE(Regex::equivalent(r1, r2.to_thompson().to_regex()));
		ASSERT_TRUE(Regex::equivalent(r1, r2.to_glushkov().to_regex()));
		ASSERT_TRUE(Regex::equivalent(r1, r2.to_ilieyu().to_regex()));
		ASSERT_TRUE(Regex::equivalent(r1, r2.to_antimirov().to_regex()));
	}
}

TEST(TestEqual, ThompsonGlushkov) {
	RegexGenerator rg;
	for (int i = 0; i < RegexNumber; i++) {
		string rgx_str = rg.generate_regex();
		Regex r(rgx_str);
		ASSERT_TRUE(FiniteAutomaton::equal(r.to_thompson().remove_eps(), r.to_glushkov()));
	}
}

TEST(TestNFA, Test_equivalent_nfa_negative) {
	RegexGenerator rg(5, 2, 2, 2);
	rg.set_neg_chance(2); // для отрицания
	for (int i = 0; i < 30; i++) {
		string str = rg.generate_regex();
		Regex r1(str), r2(str);
		ASSERT_TRUE(FiniteAutomaton::equivalent(r1.to_thompson(), r2.to_antimirov()));
	}
}