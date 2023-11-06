#pragma once

#include "BaseObject.h"
#include "iLogTemplate.h"
#include <map>
#include <unordered_map>

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
		Lexeme(Type type = error, const alphabet_symbol& symbol = "", int number = 0);
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

	AlgExpression* term_l = nullptr;
	AlgExpression* term_r = nullptr;

	// возвращает указатель на 'new' объект соответствующего типа
	virtual AlgExpression* make() const = 0;

	void clear();

	// Создает новый язык с алфавитом
	void set_language(const set<alphabet_symbol>& _alphabet);
	// Рекурсивная генерация алфавита
	void generate_alphabet(set<alphabet_symbol>& _alphabet);
	// Генерация языка из алфавита
	void make_language();

	// для print_tree
	void print_subtree(AlgExpression* expr, int level);
	// для print_dot
	string print_subdot(AlgExpression* expr, const std::string& parent_dot_node, int& id);

	// Turns string into lexeme vector
	static vector<Lexeme> parse_string(string);
	bool from_string(const string&);
	// возвращаемый тип нижеперечисленных методов зависит от типа объекта (Regex/BackRefRegex)
	// внутреннее состояние не имеет значения
	AlgExpression* expr(const vector<Lexeme>&, int, int);
	AlgExpression* scan_conc(const vector<Lexeme>&, int, int);
	AlgExpression* scan_star(const vector<Lexeme>&, int, int);
	AlgExpression* scan_alt(const vector<Lexeme>&, int, int);
	AlgExpression* scan_symb(const vector<Lexeme>&, int, int);
	AlgExpression* scan_eps(const vector<Lexeme>&, int, int);
	AlgExpression* scan_par(const vector<Lexeme>&, int, int);

	// список листьев дерева regex
	vector<AlgExpression*> pre_order_travers();

	// Проверяет, входит ли eps в дерево regex (принадлежит ли языку)
	bool contains_eps() const;

	static bool equality_checker(const AlgExpression*, const AlgExpression*);

	// Слово, в котором все итерации Клини раскрыты n раз
	string get_iterated_word(int n) const;

	// начальные состояния для to_glushkov
	vector<Lexeme> first_state() const;
	// конечные состояния для to_glushkov
	vector<Lexeme> end_state() const;
	unordered_map<int, vector<int>> pairs() const;

	void regex_union(AlgExpression* a, AlgExpression* b);
	void regex_alt(AlgExpression* a, AlgExpression* b);
	void regex_star(AlgExpression* a);
	void regex_eps();

  public:
	AlgExpression();
	AlgExpression(shared_ptr<Language>, Type, const Lexeme&, const set<alphabet_symbol>&);
	AlgExpression(set<alphabet_symbol>);

	~AlgExpression();

	AlgExpression* copy() const;
	AlgExpression(const AlgExpression&);
	AlgExpression& operator=(const AlgExpression& other);

	string to_txt() const override;
	// вывод дерева для дебага
	void print_tree();
	void print_dot();

	friend class FiniteAutomaton;
	friend class Tester;
};