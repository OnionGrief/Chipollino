#pragma once

#include "BaseObject.h"
#include "iLogTemplate.h"
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

using std::cout;
using std::endl;
using std::map;
using std::set;
using std::string;
using std::unordered_map;
using std::vector;

class AlgExpression : public BaseObject {
  protected:
	struct Lexeme {
		enum Type {
			error,
			parL,	   // (
			parR,	   // )
			alt,	   // |
			conc,	   // .
			star,	   // *
			negative,  // ^
			symb,	   // alphabet symbol
			eps,	   // Epsilon
			squareBrL, // [
			squareBrR, // ]
			ref,	   // &
		};

		Type type = error;
		alphabet_symbol symbol = "";
		int number = 0; // Для линеаризации Type::symb (возможно это применение неактуально)
						// либо для указания номера ячейки памяти
						// Type::squareBrL, Type::squareBrL, Type::ref
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
		negative,
		// Terminal:
		symb,
		// [i]
		cellWriter,
		// Reference
		ref,
	};

	set<alphabet_symbol> alphabet;
	Type type;
	Lexeme value;

	AlgExpression* term_l = nullptr;
	AlgExpression* term_r = nullptr;

	// копирует объект (!!! не чистит память !!!)
	virtual void copy(const AlgExpression*) = 0;
	// возвращает указатель на 'new' объект соответствующего типа
	virtual AlgExpression* make() const = 0;

	void clear();

	// Создает новый язык с алфавитом
	void set_language(const set<alphabet_symbol>& _alphabet);
	// Рекурсивная генерация алфавита
	void generate_alphabet(set<alphabet_symbol>& _alphabet); // NOLINT(runtime/references)
	// Генерация языка из алфавита
	void make_language();

	// для print_tree
	void print_subtree(AlgExpression* expr, int level) const;
	// для print_dot
	string print_subdot(AlgExpression* expr, const std::string& parent_dot_node,
						int& id) const; // NOLINT(runtime/references)
	virtual string type_to_str() const;
	static bool is_terminal_type(Type);

	// Turns string into lexeme vector
	static vector<Lexeme> parse_string(string, bool allow_ref = false, bool allow_negation = true);
	bool from_string(const string&, bool allow_ref = false, bool allow_negation = true);

	// возвращаемый тип нижеперечисленных методов зависит от типа объекта (Regex/BackRefRegex)
	// внутреннее состояние не имеет значения
	// Построение из вектора лексем дерева регулярного выражения
	// 2 и 3 аргумент - это начальный и конечный индекс рассматриваемых лексем в векторе
	virtual AlgExpression* expr(const vector<Lexeme>&, int, int) = 0;
	AlgExpression* scan_conc(const vector<Lexeme>&, int, int);
	AlgExpression* scan_star(const vector<Lexeme>&, int, int);
	AlgExpression* scan_alt(const vector<Lexeme>&, int, int);
	AlgExpression* scan_symb(const vector<Lexeme>&, int, int);
	AlgExpression* scan_eps(const vector<Lexeme>&, int, int);
	AlgExpression* scan_par(const vector<Lexeme>&, int, int);
	static void update_balance(const AlgExpression::Lexeme&, int&);

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
	AlgExpression(std::shared_ptr<Language>, Type, const Lexeme&, const set<alphabet_symbol>&);
	AlgExpression(set<alphabet_symbol>); // NOLINT(runtime/explicit)

	virtual ~AlgExpression();

	// возвращает указатель на копию себя
	virtual AlgExpression* make_copy() const = 0;
	AlgExpression(const AlgExpression&);
	AlgExpression& operator=(const AlgExpression& other);

	string to_txt() const override;
	// вывод дерева для дебага
	void print_tree() const;
	void print_dot() const;

	friend class FiniteAutomaton;
	friend class Tester;
	friend class UnitTests;
};