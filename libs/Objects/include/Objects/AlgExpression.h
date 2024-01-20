#pragma once

#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "BaseObject.h"

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
		Symbol symbol; // символ алфавита регулярки или ссылка(&i)
		int number = 0; // Для указания номера (при линеаризации) в to_glushkov и to_mfa
						// либо для указания номера ячейки памяти (Type: squareBrL, squareBrL, ref),
						// чтобы использовать в scan_ref и scan_square_br
		Lexeme(Type type = error, const Symbol& symbol = Symbol(),
			   int number = 0); // NOLINT(runtime/explicit)
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
		memoryWriter, // поддерево "записывается" в ячейку памяти
		// Terminal:
		symb,
		ref,
	};

	// множество уникальных символов алфавита в дереве
	std::set<Symbol> alphabet;
	Type type;
	// символ алфавита регулярки или ссылка(&i)
	Symbol symbol;

	AlgExpression* term_l = nullptr;
	AlgExpression* term_r = nullptr;

	// копирует объект (!!! не чистит память !!!)
	virtual void copy(const AlgExpression*) = 0; // NOLINT(build/include_what_you_use)
	// возвращает указатель на 'new' объект своего типа
	virtual AlgExpression* make() const = 0;

	void clear();

	// Рекурсивная генерация алфавита во всех нодах
	void generate_alphabet();
	// Генерация алфавита и создание нового языка
	void make_language();

	std::string _to_txt(bool eps_is_empty) const;

	// для print_tree
	void print_subtree(AlgExpression* expr, int level) const;
	// для print_dot
	std::string print_subdot(AlgExpression* expr, const std::string& parent_dot_node,
							 int& id) const; // NOLINT(runtime/references)
	virtual std::string type_to_str() const;
	static bool is_terminal_type(Type);

	// Turns string into lexeme vector
	static std::vector<Lexeme> parse_string(std::string, bool allow_ref = false,
											bool allow_negation = true);
	bool from_string(const std::string&, bool allow_ref = false, bool allow_negation = true);

	// возвращаемый тип нижеперечисленных методов зависит от типа объекта (Regex/BackRefRegex)
	// внутреннее состояние не имеет значения
	// Построение из вектора лексем дерева регулярного выражения
	// 2 и 3 аргумент - это начальный и конечный индекс рассматриваемых лексем в векторе
	virtual AlgExpression* expr(const std::vector<Lexeme>&, int, int) = 0;
	AlgExpression* scan_conc(const std::vector<Lexeme>&, int, int);
	AlgExpression* scan_star(const std::vector<Lexeme>&, int, int);
	AlgExpression* scan_alt(const std::vector<Lexeme>&, int, int);
	AlgExpression* scan_symb(const std::vector<Lexeme>&, int, int);
	AlgExpression* scan_eps(const std::vector<Lexeme>&, int, int);
	AlgExpression* scan_par(const std::vector<Lexeme>&, int, int);
	static void update_balance(const AlgExpression::Lexeme&, int&);

	// Проверяет, входит ли eps в дерево regex (принадлежит ли языку)
	bool contains_eps() const;

	static bool equality_checker(const AlgExpression*, const AlgExpression*);

	// Слово, в котором все итерации Клини раскрыты n раз
	std::string get_iterated_word(int n) const;

	// возвращает множество нод, с которых может начинаться слово языка объекта
	std::vector<AlgExpression*> get_first_nodes();
	// возвращает множество нод, на которые может заканчиваться слово языка объекта
	std::vector<AlgExpression*> get_last_nodes();

  public:
	AlgExpression();
	AlgExpression(std::shared_ptr<Language>, Type, const Symbol&, const std::set<Symbol>&);
	explicit AlgExpression(std::set<Symbol>);
	// переданные term_l и term_l копируются с помощью make_copy
	explicit AlgExpression(Type type, AlgExpression* = nullptr,
				  AlgExpression* = nullptr);
				  
	virtual ~AlgExpression();

	// возвращает указатель на копию себя
	virtual AlgExpression* make_copy() const = 0;
	AlgExpression(const AlgExpression&);
	AlgExpression& operator=(const AlgExpression& other);

	Symbol get_symbol();
	// Устанавливает новый язык с алфавитом
	void set_language(const std::set<Symbol>& _alphabet);
	// Устанавливает язык
	void set_language(const std::shared_ptr<Language>& _language);

	std::string to_txt() const override;
	// вывод дерева для дебага
	void print_tree() const;
	void print_dot() const;

	friend class FiniteAutomaton;
	friend class Tester;
	friend class UnitTests;
};