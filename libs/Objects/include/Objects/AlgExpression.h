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
	Alphabet alphabet;
	Type type;
	// символ алфавита регулярки или ссылка(&i)
	Symbol symbol;

	AlgExpression* term_l = nullptr;
	AlgExpression* term_r = nullptr;

	// копирует объект в себя (!!! не чистит память !!!)
	virtual void copy(const AlgExpression*) = 0; // NOLINT(build/include_what_you_use)
	// возвращает указатель на 'new' объект своего типа
	virtual AlgExpression* make() const = 0;

	void clear();

	// Рекурсивная генерация алфавита во всех нодах
	void generate_alphabet();
	// Генерация алфавита и создание нового языка
	void make_language();
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

	virtual bool equals(const AlgExpression* other) const = 0;

	// проверяет, содержит ли выражение пустое слово (принадлежит ли языку пустое слово)
	virtual bool contains_eps() const = 0;

	static bool equality_checker(const AlgExpression*, const AlgExpression*);

	// Слово, в котором все итерации Клини раскрыты n раз
	std::string get_iterated_word(int n) const;

	// возвращает множество нод, с которых может начинаться слово языка выражения
	std::vector<AlgExpression*> get_first_nodes();
	// возвращает множество нод, на которые может заканчиваться слово языка выражения
	std::vector<AlgExpression*> get_last_nodes();

	static void sort_alts(std::vector<AlgExpression*>& alts, // NOLINT(runtime/references)
						  bool erase_alts = true);
	// возвращает указатели на альтернативы в регулярке, построенной от переданного корня
	std::vector<AlgExpression*> join_alts(std::vector<AlgExpression*>, AlgExpression*) const;
	// аргументы: выражения под альтернативами (собираются для ноды выше)
	// признак from_alt указывает на то, что вызов был из ноды с типом Alt (избавляет от лишних
	// операций)
	// признак erase_alts указывает на то, что при переписывании нужно убирать лишние альтернативы
	void _rewrite_aci(std::vector<AlgExpression*>& alts, // NOLINT(runtime/references)
					  bool from_alt, bool erase_alts);

  public:
	AlgExpression();
	AlgExpression(std::shared_ptr<Language>, Type, const Symbol&, Alphabet);
	AlgExpression(Type, const Symbol&);
	explicit AlgExpression(Alphabet);
	// переданные term_l и term_l копируются с помощью make_copy
	explicit AlgExpression(Type type, AlgExpression* = nullptr,
				  AlgExpression* = nullptr);

	virtual ~AlgExpression();

	// возвращает указатель на копию себя
	virtual AlgExpression* make_copy() const = 0;
	AlgExpression(const AlgExpression&);

	Symbol get_symbol() const;
	Type get_type() const;
	AlgExpression* get_term_l() const;
	AlgExpression* get_term_r() const;
	// Устанавливает новый язык с алфавитом
	void set_language(const Alphabet& _alphabet);
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