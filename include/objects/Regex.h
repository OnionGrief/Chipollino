#pragma once
#include "BaseObject.h"
#include <iostream>
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

class Regex : BaseObject {
  private:
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

	Type type;
	Lexem value;
	Regex* term_p = nullptr;
	Regex* term_l = nullptr;
	Regex* term_r = nullptr;
	// Turns string into lexem vector
	vector<Lexem> parse_string(string);
	Regex* expr(vector<Lexem>, int, int);
	Regex* scan_conc(vector<Lexem>, int, int);
	Regex* scan_star(vector<Lexem>, int, int);
	Regex* scan_alt(vector<Lexem>, int, int);
	Regex* scan_symb(vector<Lexem>, int, int);
	Regex* scan_par(vector<Lexem>, int, int);

  public:
	Regex();

	string to_txt() override;
	void pre_order_travers();
	void clear();
	Regex* copy();
	bool from_string(string);
	// TODO: there may be some *to-automat* methods
	// like to_glushkov, to_antimirov, etc
};