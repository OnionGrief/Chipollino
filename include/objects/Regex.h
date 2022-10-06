#pragma once
#include "BaseObject.h"
#include <string>
#include <vector>
using namespace std;

struct Lexem {
	enum Type {
		error,
		parL, // (
		parR, // )
		alt,  // |
		conc, // .
		star, // *
		symb, // alphabet symbol
	};
	
	Type type = error;
	char symbol = 0;

	Lexem(Type type = error, char symbol = 0);
};

struct Term {
	enum Type {
	// Error
		error,
	// Binary:
		alt,
		conc,
	// Unary:
		star,
	// Terminal:
		symbol
	};

	Type type = error;
	Lexem symbol;
	Term* term_l = nullptr;
	Term* term_r = nullptr;

	Term(Type type, Lexem symbol)
};

class Regex : BaseObject {
private:
	vector<Lexem> lexems;
	// Turns string into lexem vector
	void parse_string(string);

	// Root of regexp
	Term* root;
public:
	Regex();
	Regex(string);
	string to_txt() override;
	// TODO: there may be some *to-automat* methods
	// like to_glushkov, to_antimirov, etc
}