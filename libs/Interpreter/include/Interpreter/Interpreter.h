#pragma once
#include "Interpreter/Typization.h"
#include "Objects/FiniteAutomaton.h"
#include "Objects/Regex.h"
#include "Objects/TransformationMonoid.h"
#include <deque>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <variant>

using namespace Typization;

class Interpreter {
  public:
	enum class LogMode {
		all,
		errors,
		nothing
	};
	Interpreter();
	// Интерпретация строчки, возвращает true в случае успеха
	bool run_line(const string& line);
	// Интерпретация файла построчно
	bool run_file(const string& path);
	// Установит режим логгирования в консоль
	void set_log_mode(LogMode mode);

  private:
	// true, если во время исполнения произошла ошибка
	bool error = false;

	// Вывод
	LogMode log_mode = LogMode::all;
	int log_nesting = 0;

	class InterpreterLogger {
	  public:
		InterpreterLogger(Interpreter& parent) : parent(parent) {
			parent.log_nesting++;
		}
		~InterpreterLogger() {
			parent.log_nesting--;
		}
		void log(const string& str);
		void throw_error(const string& str);

	  private:
		Interpreter& parent;
	};

	InterpreterLogger init_log();

	// Применение цепочки функций к набору аргументов
	GeneralObject apply_function_sequence(const vector<Function>& functions,
										  vector<GeneralObject> arguments);

	// Применение функции к набору аргументов
	GeneralObject apply_function(const Function& function,
								 const vector<GeneralObject>& arguments);

	// Тут хранятся объекты по их id
	map<string, GeneralObject> objects;

	//== Элементы грамматики интерпретатора ===================================
	using Id = string;
	struct Expression;

	// Композиция функций и аргументы к ней
	struct FunctionSequence {
		// Композиция функций
		vector<Function> functions;
		// Параметры композиции функций (1 или более)
		vector<Expression> parameters;
	};

	// Общий вид выражения
	struct Expression {
		ObjectType type;
		variant<FunctionSequence, int, Regex, Id> value;
	};

	// Операция объявления
	// [идентификатор] = ([функция].)*[функция]? [объект]+ (!!)?
	struct Declaration {
		// Идентификатор, в который запишется объект
		Id id;
		// Выражение
		Expression expr;
		// Надо ли отображать результат
		bool show_result = 0;
	};

	// Специальная форма test
	struct Test {
		// Аргументы:
		// НКА или регулярное выражение;
		Expression language;
		// регулярное выражение без альтернатив(только с итерацией Клини) —
		// тестовый сет;
		Expression test_set;
		// натуральное число — шаг итерации в сете.
		int iterations = 1;
	};

	// Предикат [предикат] [объект]+
	struct Predicate {
		// Функция (предикат)
		Function predicate;
		// Параметры (могут быть идентификаторами)
		vector<Expression> arguments;
	};

	// SetFlag [flagname] [value]
	struct Flag {
		string name;
		bool value;
	};

	// Общий вид опрерации
	using GeneralOperation = variant<Declaration, Test, Predicate, Flag>;

	//== Парсинг ==============================================================

	struct Lexem;

	// Находит парную закрывающую скобку
	int find_closing_par(const vector<Lexem>&, size_t pos);

	optional<Id> scan_Id(const vector<Lexem>&, int& pos, size_t end);
	optional<Regex> scan_Regex(const vector<Lexem>&, int& pos, size_t end);
	optional<FunctionSequence> scan_FunctionSequence(const vector<Lexem>&,
													 int& pos, size_t end);
	optional<Expression> scan_Expression(const vector<Lexem>&, int& pos,
										 size_t end);

	// Типизация идентификаторов. Нужна для корректного составления опреаций
	map<string, ObjectType> id_types;
	// Считывание операции из набора лексем
	optional<Declaration> scan_declaration(const vector<Lexem>&, int& pos);
	optional<Test> scan_test(const vector<Lexem>&, int& pos);
	optional<Predicate> scan_predicate(const vector<Lexem>&, int& pos);
	optional<Flag> scan_flag(const vector<Lexem>&, int& pos);
	optional<GeneralOperation> scan_operation(const vector<Lexem>&);

	//== Исполнение комманд ===================================================

	// Вычисление выражения
	optional<GeneralObject> eval_expression(const Expression& expr);

	// Вычисление последовательности функций
	optional<GeneralObject> eval_function_sequence(const FunctionSequence& seq);

	// Исполнение операций
	bool run_declaration(const Declaration&);
	bool run_predicate(const Predicate&);
	bool run_test(const Test&);
	bool set_flag(const Flag&);
	bool run_operation(const GeneralOperation&);

	// глобальный флаг автоматов (отвечает за удаление ловушек)
	// Если режим isTrim включён (т.е. по умолчанию), то на всех подозрительных
	// преобразованиях всегда удаляем в конце ловушки.
	// Если isTrim = false, тогда после удаления ловушки в результате
	// преобразований добавляем её обратно
	bool is_trim = true;

	// Список опреаций для последовательного выполнения
	vector<GeneralOperation> operations;

	// Сравнение типов ожидаемых и полученных входных данных
	bool typecheck(vector<ObjectType> func_input_type,
				   vector<ObjectType> input_type);
	// Построение последовательности функций по их названиям
	optional<vector<Function>> build_function_sequence(
		vector<string> function_names, vector<ObjectType> first_type);

	// Соответствие между названиями функций и сигнатурами
	map<string, vector<Function>> names_to_functions;

	//== Лексер ===============================================================

	struct Lexem {
		enum Type { // TODO добавить тип строки (для filename)
			error,
			equalSign,
			doubleExclamation,
			parL,
			parR,
			dot,
			number,
			regex,
			name
		};

		Type type = error;
		// Если type = id | function | predicate
		string value = "";
		// Eсли type = number
		int num = 0;

		Lexem(Type type = error, string value = "");
		Lexem(int num);
	};

	class Lexer {
	  public:
		Lexer(Interpreter& parent) : parent(parent) {}
		// Возвращает лексемы, разбитые по строчкам
		vector<vector<Lexem>> load_file(string path);
		// Бьёт строку на лексемы (без перевода строки)
		vector<Lexem> parse_string(string);

	  private:
		// Здесь храним строку и место, откуда её читаем
		struct {
		  public:
			string str = "";
			int pos = 0;

			void save() {
				saves.push_back(pos);
			}
			void restore() {
				pos = saves.back();
				saves.pop_back();
			}

		  private:
			deque<int> saves;

		} input;

		bool eof();
		char current_symbol();
		void next_symbol();
		void skip_spaces();
		bool scan_word(string);
		string scan_until_space();
		string scan_until(char symbol);

		Lexem scan_equalSign();
		Lexem scan_doubleExclamation();
		Lexem scan_parL();
		Lexem scan_parR();
		Lexem scan_dot();
		Lexem scan_number();
		Lexem scan_name();
		Lexem scan_regex();
		Lexem scan_lexem();

		Interpreter& parent;
	};
};