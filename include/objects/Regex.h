#pragma once
#include "BaseObject.h"
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <optional>
using namespace std;

struct Lexem {
	enum Type
	{
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
	enum Type
	{
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

	// Принадлежит ли эпсилон языку регулярки
	bool is_eps_possible();
	// Множество префиксов длины len
	void get_prefix(int len, std::set<std::string>* prefs) const;
	// Производная по символу
	bool derevative_with_respect_to_sym(Regex* respected_sym, const Regex* reg_e,
										Regex* result) const;
	// Производная по префиксу
	bool derevative_with_respect_to_str(std::string str, const Regex* reg_e,
										Regex* result) const;

  public:
	Regex();
	string to_txt() override;
	void pre_order_travers();
	void clear();
	~Regex();
	Regex* copy() const;
	Regex(const Regex&);
	bool from_string(string);
	// проверка регулярок на равентсво(буквальное)
	static bool equal(Regex* r1, Regex* r2);

	// Производная по символу
	std::optional<Regex> symbol_derevative(const Regex &respected_sym) const;
	// Производная по префиксу
	std::optional<Regex> prefix_derevative(std::string respected_str) const;
	// Длина накачки
	int pump_length() const;

	// TODO: there may be some *to-automat* methods
	// like to_glushkov, to_antimirov, etc
};