#pragma once
#include "InputGenerator/RegexGenerator.h"
#include "Interpreter/Typization.h"
#include "Logger/Logger.h"
#include "Objects/FiniteAutomaton.h"
#include "Objects/Regex.h"
#include "Objects/TransformationMonoid.h"
#include <cmath>
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
	// Выгружает лог в файл
	void generate_log(const string& filename);

	enum class Flag {
		auto_remove_trap_states,
		weak_type_comparison,
		log_theory,
		report,
	};
	bool set_flag(Flag key, bool value);

  private:
	// Логгер для преобразований
	Logger tex_logger;
	// автогенерация кратких шаблонов
	void generate_brief_templates();

	//== Внутреннее логгирование ==============================================
	// true, если во время исполнения произошла ошибка
	bool error = false;

	// Режим вывода
	LogMode log_mode = LogMode::all;

	// Уровень вложенности логов
	int log_nesting = 0;

	// Внутренний логгер. Контролирует уровень вложенности с учётом скопа
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

	// Инициалиризирует внутренний логгер
	InterpreterLogger init_log();

	// Тут хранятся объекты по их id
	map<string, GeneralObject> objects;

	//== Элементы грамматики интерпретатора ===================================
	using Id = string;
	struct Expression;

	// Функция, состоит из имени и сигнатуры
	// Предикат - тоже функция, но на выходе boolean
	struct Function {
		// Имя функции
		string name;
		// Типы входных аргументов
		vector<ObjectType> input;
		// Тип выходного аргумента
		ObjectType output;
		Function(){};
		Function(string name, vector<ObjectType> input, ObjectType output)
			: name(name), input(input), output(output){};
	};

	friend bool operator==(const Function& l, const Function& r);

	// Композиция функций и аргументы к ней
	struct FunctionSequence {
		// Композиция функций
		vector<Function> functions;
		// Параметры композиции функций (1 или более)
		vector<Expression> parameters;
		// Надо ли отображать результат
		bool show_result = 0;
		// Преобразование в текст
		string to_txt() const;
	};

	using Array = vector<Expression>;

	// Общий вид выражения
	struct Expression {
		ObjectType type;
		variant<int, FunctionSequence, Regex, string, Array> value;
		// Преобразование в текст
		string to_txt() const;
	};

	// Операция объявления
	// [идентификатор] = ([функция].)*[функция]? [объект]+ (!!)?
	struct Declaration {
		// Идентификатор, в который запишется объект
		Id id;
		// Выражение
		Expression expr;
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

	// Специальная форма Verify
	struct Verification {
		// Аргументы:
		// Предикат
		Expression predicate;
		// натуральное число — размер тестов.
		int size = 20;
		// Regex random_regex;
	};

	// Предикат [предикат] [объект]+
	struct Predicate {
		// Функция (предикат)
		Function predicate;
		// Параметры (могут быть идентификаторами)
		vector<Expression> arguments;
	};

	// SetFlag [flagname] [value]
	struct SetFlag {
		string name;
		bool value;
	};

	// Флаги:

	map<string, Flag> flags_names = {
		{"auto_remove_trap_states", Flag::auto_remove_trap_states},
		{"weak_type_comparison", Flag::weak_type_comparison},
		{"log_theory", Flag::log_theory},
		// Андрей, придумай сам названия
	};

	map<Flag, bool> flags = {
		/* глобальный флаг автоматов (отвечает за удаление ловушек)
		Если режим isTrim включён (т.е. по умолчанию), то на всех подозрительных
		преобразованиях всегда удаляем в конце ловушки.
		Если isTrim = false, тогда после удаления ловушки в результате
		преобразований добавляем её обратно */
		{Flag::auto_remove_trap_states, true},
		// флаг динамического тайпчекера
		{Flag::weak_type_comparison, false},
		// флаг добавления теоретического блока к ф/ям в логгере
		{Flag::log_theory, false},
		// временный флаг для логирования в отчет, Андрей исправь
		{Flag::report, true},
	};

	// Общий вид опрерации
	using GeneralOperation =
		variant<Declaration, Test, Predicate, SetFlag, Verification>;

	//== Парсинг ==============================================================

	struct Lexem;

	// Находит парную закрывающую скобку
	int find_closing_par(const vector<Lexem>&, size_t pos);

	optional<Id> scan_id(const vector<Lexem>&, int& pos, size_t end);
	optional<Regex> scan_regex(const vector<Lexem>&, int& pos, size_t end);
	optional<FunctionSequence> scan_function_sequence(const vector<Lexem>&,
													  int& pos, size_t end);
	optional<Array> scan_array(const vector<Lexem>&, int& pos, size_t end);
	optional<Expression> scan_expression(const vector<Lexem>&, int& pos,
										 size_t end);

	// перевод ObjectType в String (для логирования и дебага)
	map<ObjectType, string> types_to_string = {
		{ObjectType::NFA, "NFA"},
		{ObjectType::DFA, "DFA"},
		{ObjectType::Regex, "Regex"},
		{ObjectType::RandomRegex, "RandomRegex"},
		{ObjectType::Int, "Int"},
		{ObjectType::String, "String"},
		{ObjectType::Boolean, "Boolean"},
		{ObjectType::OptionalBool, "OptionalBool"},
		{ObjectType::AmbiguityValue, "AmbiguityValue"},
		{ObjectType::PrefixGrammar, "PrefixGrammar"},
		{ObjectType::Array, "Array"},
	}; // не додумалась как по другому(не ручками) (((

	// Типизация идентификаторов. Нужна для корректного составления опреаций
	map<string, ObjectType> id_types;
	// Считывание операции из набора лексем
	optional<Declaration> scan_declaration(const vector<Lexem>&, int& pos);
	optional<Test> scan_test(const vector<Lexem>&, int& pos);
	optional<Verification> scan_verification(const vector<Lexem>&, int& pos);
	optional<Predicate> scan_predicate(const vector<Lexem>&, int& pos);
	optional<SetFlag> scan_flag(const vector<Lexem>&, int& pos);
	optional<GeneralOperation> scan_operation(const vector<Lexem>&);

	//== Исполнение комманд ===================================================

	// Выражение для подстановки на место *
	optional<Regex> current_random_regex;

	// Применение цепочки функций к набору аргументов
	optional<GeneralObject> apply_function_sequence(
		const vector<Function>& functions, vector<GeneralObject> arguments);

	// Применение функции к набору аргументов
	optional<GeneralObject> apply_function(
		const Function& function, const vector<GeneralObject>& arguments,
		LogTemplate& log_template);

	// Вычисление выражения
	optional<GeneralObject> eval_expression(const Expression& expr);

	// Вычисление последовательности функций
	optional<GeneralObject> eval_function_sequence(const FunctionSequence& seq);

	// Исполнение операций
	bool run_declaration(const Declaration&);
	bool run_predicate(const Predicate&);
	bool run_test(const Test&);
	bool run_verification(const Verification&);
	bool run_set_flag(const SetFlag&);
	bool run_operation(const GeneralOperation&);

	// Список опреаций для последовательного выполнения
	// vector<GeneralOperation> operations;

	// Сравнение типов ожидаемых и полученных входных данных
	bool typecheck(vector<ObjectType> func_input_type,
				   vector<ObjectType> input_type);
	// выбрать подходящий вариант функции для данных аргументов (если он есть)
	optional<int> find_func(string func, vector<ObjectType> input_type);
	string get_func_id(Function function);

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
			star,
			doubleExclamation,
			parL,
			parR,
			bracketL,
			bracketR,
			dot,
			number,
			regex,
			stringval,
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
		Lexem scan_star();
		Lexem scan_doubleExclamation();
		Lexem scan_parL();
		Lexem scan_parR();
		Lexem scan_bracketL();
		Lexem scan_bracketR();
		Lexem scan_dot();
		Lexem scan_number();
		Lexem scan_stringval();
		Lexem scan_name();
		Lexem scan_regex();
		Lexem scan_lexem();

		Interpreter& parent;
	};
};