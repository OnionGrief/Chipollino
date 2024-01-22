#include <Objects/FiniteAutomaton.h>
#include <Lexer.h>
#include <Parser.h>

int main() {
    utf8_tree tree;

    auto input2 = lexy::read_file<lexy::ascii_encoding>("input.txt");

    auto input = input2.buffer();


    Lexer::prod(tree, input);
    Parser::parse_MFA(tree);


}