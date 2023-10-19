#pragma once

#include "BaseObject.h"

class AlgExpression : public BaseObject {
  protected:
	struct Lexeme {
		enum Type {
			error,
			parL, // (
			parR, // )
			alt,  // |
			conc, // .
			star, // *
			symb, // alphabet symbol
			eps,  // Epsilon
		};

		Type type = error;
		alphabet_symbol symbol = "";
		int number = 0; // Нужно для линеаризации в глушкове
		Lexeme(Type type = error, const alphabet_symbol& symbol = "",
			   int number = 0);
	};

	enum Type {
		// Epsilon
		eps,
		// Binary:
		alt,
		conc,
		// Unary:
		star,
		// Terminal:
		symb
	};

	set<alphabet_symbol> alphabet;
	Type type;
	Lexeme value;

  public:
	AlgExpression() = default;
	AlgExpression(shared_ptr<Language>, Type, const Lexeme&,
				  const set<alphabet_symbol>&);
	AlgExpression(set<alphabet_symbol>);

	// создает новый язык с алфавитом
	void set_language(const set<alphabet_symbol>& _alphabet);
};