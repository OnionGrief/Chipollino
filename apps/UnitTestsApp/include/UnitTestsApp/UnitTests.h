#pragma once

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
#include <functional>

class UnitTests
{
public:
    UnitTests(){};
    
    static int  InitTests(int argc, char** argv) {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    };

    using Lexeme = AlgExpression::Lexeme;
    using LexemeType = AlgExpression::Lexeme::Type;
    static std::vector<Lexeme> parse_string(std::string str) {return AlgExpression::parse_string(str);};
};
