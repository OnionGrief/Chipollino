#pragma once

#include <functional>
#include <string>
#include <vector>

#include "AutomatonToImage/AutomatonToImage.h"
#include "InputGenerator/RegexGenerator.h"
#include "Interpreter/Interpreter.h"
#include "Objects/AlgExpression.h"
#include "Objects/FiniteAutomaton.h"
#include "Objects/Grammar.h"
#include "Objects/Language.h"
#include "Objects/Regex.h"
#include "Objects/TransformationMonoid.h"
#include "Tester/Tester.h"
#include "gtest/gtest.h"

class UnitTests {
  public:
	UnitTests() {}

	static int RunTests(int argc, char** argv) {
		::testing::InitGoogleTest(&argc, argv);
		return RUN_ALL_TESTS();
	}

	using Lexeme = AlgExpression::Lexeme;
	using LexemeType = AlgExpression::Lexeme::Type;
	static std::vector<Lexeme> parse_string(const std::string& str, bool allow_ref,
											bool allow_negation) {
		return AlgExpression::parse_string(str, allow_ref, allow_negation);
	}
};
