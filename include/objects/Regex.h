#pragma once
#include "AlphabetSymbol.h"
#include "BaseObject.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

using namespace std;

class Language;
class FiniteAutomaton;

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
	int number = 0;
	Lexem(Type type = error, char symbol = 0, int number = 0);
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
	Language* language = nullptr;
	set<alphabet_symbol> alphabet;
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

	// Принадлежит ли эпсилон языку регулярки
	bool is_eps_possible();
	// Множество префиксов длины len
	void get_prefix(int len, std::set<std::string>* prefs) const;
	// Производная по символу
	bool derevative_with_respect_to_sym(Regex* respected_sym,
										const Regex* reg_e,
										Regex* result) const;
	// Производная по префиксу
	bool derevative_with_respect_to_str(std::string str, const Regex* reg_e,
										Regex* result) const;
	FiniteAutomaton get_tompson(int);

	vector<Lexem>* first_state(); // начальные состояния для to_glushkov
	int L(); //проверяет, входит ли eps в дерево regex
	vector<Lexem>* end_state(); // конечные состояния для to_glushkov
	map<int, vector<int>> pairs();
	vector<Regex*> pre_order_travers_vect(); // список листьев дерева regex
	bool is_term(
		int, const vector<Lexem>&); // возвращает true, если состояние конечно

  public:
	Regex();
	Regex(Language*);
	string to_txt() const override;
	void pre_order_travers();
	void clear();
	FiniteAutomaton to_tompson();
	FiniteAutomaton to_glushkov();
	FiniteAutomaton to_ilieyu();

	~Regex();
	Regex* copy() const;
	Regex(const Regex&);

	bool from_string(string);
	// проверка регулярок на равентсво(буквальное)
	static bool equal(Regex*, Regex*);
	// проверка регулярок на эквивалентность
	static bool equivalent(Regex, Regex);

	// Производная по символу
	std::optional<Regex> symbol_derevative(const Regex& respected_sym) const;
	// Производная по префиксу
	std::optional<Regex> prefix_derevative(std::string respected_str) const;
	// Длина накачки
	int pump_length() const;

	// TODO: there may be some *to-automat* methods
	// like to_glushkov, to_antimirov, etc
};