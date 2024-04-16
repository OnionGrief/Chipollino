#pragma once

#include <string>
#include <vector>

#include "InputGenerator/RegexGenerator.h"
#include "MetamorphicTestsApp/MetamorphicTests.h"
#include "Objects/MemoryFiniteAutomaton.h"
#include "gtest/gtest.h"

class MetamorphicTests {
  public:
	MetamorphicTests() {}

	static int RunTests(int argc, char** argv) {
		::testing::InitGoogleTest(&argc, argv);
		return RUN_ALL_TESTS();
	}

	static void cmp_automatons(const MemoryFiniteAutomaton&, // NOLINT(runtime/references)
							   const MemoryFiniteAutomaton&);

	static std::string generate_bregex(RegexGenerator&, int); // NOLINT(runtime/references)
};
