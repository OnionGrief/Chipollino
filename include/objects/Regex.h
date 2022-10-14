#pragma once
#include "BaseObject.h"
#include "FiniteAutomat.h"
#include <string>
#include <vector>
#include <iostream>
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
	int number = 0;
	Lexem(Type type = error, char symbol = 0, int number = 0);
};

class Regex : BaseObject {
private:
	enum Type {
		// Error
		error,
		// Binary:
		alt,
		conc,
		// Unary:
		star,
		// Terminal:
		symb
	};

	Type type = error;
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
	Regex(string);
	string to_txt() override;
	void pre_order_travers();
	void clear();
	Regex* copy();
	FiniteAutomat to_tompson(int);
	FiniteAutomat to_glushkov();
	vector<Lexem>* first_state();
	int L();
	vector<Lexem>* end_state();
	map<int, vector<int>> pairs();
	vector<Regex*> pre_order_travers_vect();
	bool is_term(int, vector<Lexem>);
	// TODO: there may be some *to-automat* methods
	// like to_glushkov, to_antimirov, etc
};