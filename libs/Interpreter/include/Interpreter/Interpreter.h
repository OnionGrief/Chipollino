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
	enum class LogMode { all, errors, nothing };
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
	void log(const string& str);
	void throw_error(const string& str);

	// Применение цепочки функций к набору аргументов
	static GeneralObject apply_function_sequence(
		const vector<Function>& functions, vector<GeneralObject> arguments);

	// Применение функции к набору аргументов
	static GeneralObject apply_function(const Function& function,
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
		// Параметры композиции функций (1 или более). Могут содержать ID
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

	// Общий вид опрерации
	using GeneralOperation = variant<Declaration, Test, Predicate>;

	//== Парсинг ==============================================================

	struct Lexem;

	optional<Id> scan_Id(const vector<Lexem>&, int& pos);
	optional<Regex> scan_Regex(const vector<Lexem>&, int& pos);
	optional<FunctionSequence> scan_FunctionSequence(const vector<Lexem>&,
													 int& pos);
	optional<Expression> scan_Expression(const vector<Lexem>&, int& pos);

	// Типизация идентификаторов. Нужна для корректного составления опреаций
	map<string, ObjectType> id_types;
	// Считывание операции из набора лексем
	optional<Declaration> scan_declaration(const vector<Lexem>&, int& pos);
	optional<Test> scan_test(const vector<Lexem>&, int& pos);
	optional<Predicate> scan_predicate(const vector<Lexem>&, int& pos);
	optional<GeneralOperation> scan_operation(const vector<Lexem>&);

	//== Исполнение комманд ===================================================

	// Преобразование списка параметров в список аргументов
	vector<GeneralObject> parameters_to_arguments(
		const vector<variant<string, GeneralObject>>& parameters);

	// Исполнение операций
	void run_declaration(const Declaration&);
	void run_predicate(const Predicate&);
	void run_test(const Test&);
	void run_operation(const GeneralOperation&);

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
		// Усли type = number
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