#pragma once
#include "BaseObject.h"
#include <iostream>
#include <set>
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
		eps,  // Epsilon
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
	Regex* expr(const vector<Lexem>&, int, int);
	Regex* scan_conc(const vector<Lexem>&, int, int);
	Regex* scan_star(const vector<Lexem>&, int, int);
	Regex* scan_alt(const vector<Lexem>&, int, int);
	Regex* scan_symb(const vector<Lexem>&, int, int);
	Regex* scan_eps(const vector<Lexem>&, int, int);
	Regex* scan_par(const vector<Lexem>&, int, int);
	bool is_eps_possible();
	void get_prefix(int len, std::set<std::string>* prefs);

  public:
	Regex();
	string to_txt() override;
	void pre_order_travers();
	void clear();
	Regex* copy();
	bool from_string(string);

	bool derevative_with_respect_to_sym(Regex* respected_sym, Regex* reg_e,
										Regex* result);
	bool derevative_with_respect_to_str(std::string str, Regex* reg_e,
										Regex* result);
	void pump_lenght(Regex* reg_e);

	// TODO: there may be some *to-automat* methods
	// like to_glushkov, to_antimirov, etc
};