#include "Objects/AlgExpression.h"
#include "Objects/Language.h"

AlgExpression::AlgExpression(shared_ptr<Language> language, Type type,
							 const Lexeme& value,
							 const set<alphabet_symbol>& alphabet)
	: BaseObject(language), type(type), value(value), alphabet(alphabet) {}

AlgExpression::AlgExpression(set<alphabet_symbol> alphabet)
	: BaseObject(alphabet) {}

AlgExpression::Lexeme::Lexeme(Type type, const alphabet_symbol& symbol,
							  int number)
	: type(type), symbol(symbol), number(number) {}

void AlgExpression::set_language(const set<alphabet_symbol>& _alphabet) {
	alphabet = _alphabet;
	language = make_shared<Language>(alphabet);
}