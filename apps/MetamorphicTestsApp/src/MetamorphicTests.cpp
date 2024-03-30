#include "gtest/gtest.h"

#include "InputGenerator/RegexGenerator.h"
#include "Objects/BackRefRegex.h"
#include "Objects/FiniteAutomaton.h"
#include "Objects/MemoryFiniteAutomaton.h"
#include "Objects/Regex.h"

using std::string;
using std::unordered_set;
using std::vector;

const int RegexNumber = 30;

TEST(TestRegex, ToTxt) {
	RegexGenerator rg(15, 10, 5, 3);
	// rg.set_neg_chance(50); // для отрицания
	for (int i = 0; i < RegexNumber; i++) {
		string rgx_str = rg.generate_regex();
		SCOPED_TRACE("Regex: " + rgx_str);
		Regex r1(rgx_str);
		Regex r2(r1.to_txt());
		ASSERT_TRUE(Regex::equivalent(r1, r2));
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
	for (int i = 0; i < RegexNumber; i++) {
		string rgx_str = rg.generate_regex();
		SCOPED_TRACE("Regex: " + rgx_str);
		Regex r(rgx_str);
		ASSERT_TRUE(FiniteAutomaton::equal(r.to_thompson().remove_eps(), r.to_glushkov()));
	}
}

TEST(TestNFA, NegativeNFAEquivalence) {
	RegexGenerator rg(5, 2, 2, 2);
	rg.set_neg_chance(50); // для отрицания
	for (int i = 0; i < RegexNumber; i++) {
		string rgx_str = rg.generate_regex();
		SCOPED_TRACE("Regex: " + rgx_str);
		Regex r1(rgx_str), r2(rgx_str);
		ASSERT_TRUE(FiniteAutomaton::equivalent(r1.to_thompson(), r2.to_antimirov()));
	}
}

TEST(TestMFA, Fuzzing) {
	RegexGenerator rg(5, 3, 2, 3);
	for (int i = 0; i < RegexNumber; i++) {
		string rgx_str = rg.generate_regex();
		SCOPED_TRACE("Regex: " + rgx_str);
		MemoryFiniteAutomaton mfa1 = BackRefRegex(rgx_str).to_mfa();
		MemoryFiniteAutomaton mfa2 = BackRefRegex(rgx_str).to_mfa_additional();

		int len = std::max(mfa1.size(), mfa2.size());
		auto test_set1 = mfa1.generate_test_set(len);
		auto test_set2 = mfa2.generate_test_set(len);
		ASSERT_EQ(test_set1.first, test_set2.first);

		test_set1.second.insert(test_set2.second.begin(), test_set2.second.end());
		for (const auto& mutated_word : test_set1.second) {
			auto res1 = mfa1.parse(mutated_word);
			auto res2 = mfa2.parse(mutated_word);
			ASSERT_EQ(res1.second, res2.second);
		}
	}
}

TEST(TestParsing, RandomBrefRegexParsing) {
	RegexGenerator rg(15, 5, 3, 3);
	for (int i = 0; i < RegexNumber; i++) {
		string str = rg.generate_brefregex(2, 40, 40);
		BackRefRegex r1(str);
		string r1_str = r1.to_txt();
		BackRefRegex r2(r1_str);
		// ASSERT_TRUE(BackRefRegex::equal(r1, r2)) << str;
	}
}