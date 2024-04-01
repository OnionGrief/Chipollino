#include <random>
#include <string>
#include <unordered_set>

#include "InputGenerator/RegexGenerator.h"
#include "Objects/BackRefRegex.h"
#include "Objects/FiniteAutomaton.h"
#include "Objects/MemoryFiniteAutomaton.h"
#include "Objects/Regex.h"
#include "gtest/gtest.h"

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
	RegexGenerator rg(6, 3, 3, 3);
	for (int i = 0; i < RegexNumber; i++) {
		string rgx_str = rg.generate_brefregex(2, 70, 50);
		SCOPED_TRACE("Regex: " + rgx_str);
		MemoryFiniteAutomaton mfa1 = BackRefRegex(rgx_str).to_mfa();
		MemoryFiniteAutomaton mfa2 = BackRefRegex(rgx_str).to_mfa_additional();

		int len = mfa2.size();
		auto test_set1 = mfa1.generate_test_set(len);
		auto test_set2 = mfa2.generate_test_set(len);
		ASSERT_EQ(test_set1.first, test_set2.first);

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
		for (const auto& mutated_word : random_set) {
			auto res1 = mfa1.parse(mutated_word);
			auto res2 = mfa2.parse(mutated_word);
			ASSERT_EQ(res1.second, res2.second);
		}
	}
}