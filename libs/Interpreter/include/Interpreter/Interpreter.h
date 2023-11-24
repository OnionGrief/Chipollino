#pragma once
#include <cctype>
#include <cmath>
#include <deque>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <variant>
#include <vector>

#include "InputGenerator/RegexGenerator.h"
#include "Interpreter/Typization.h"
#include "Logger/Logger.h"
#include "Objects/FiniteAutomaton.h"
#include "Objects/Regex.h"
#include "Objects/TransformationMonoid.h"

using namespace Typization; // NOLINT(build/namespaces)

class Interpreter {
  public:
	enum class LogMode {
		all,
		errors,
		nothing
	};
	Interpreter();
	// Интерпретация строчки, возвращает true в случае успеха
	bool run_line(const std::string& line);
	// Интерпретация файла построчно
	bool run_file(const std::string& path);
	// Установит режим логгирования в консоль
	void set_log_mode(LogMode mode);
	// Выгружает лог в файл
	void generate_log(const std::string& filename);

	enum class Flag {
		auto_remove_trap_states,
		weak_type_comparison,
		log_theory
	};
	bool set_flag(Flag key, bool value);

  private:
	// Логгер для преобразований
	Logger tex_logger;
	// автогенерация кратких шаблонов
	void generate_brief_templates();

	// генерация теста для всех функций
	void generate_test_for_all_functions();

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
		InterpreterLogger(Interpreter& parent) : parent(parent) { // NOLINT(runtime/explicit)
			parent.log_nesting++;
		}
		~InterpreterLogger() {
			parent.log_nesting--;
		}
		void log(const std::string& str);
		void throw_error(const std::string& str);

	  private:
		Interpreter& parent;
	};

	// Инициалиризирует внутренний логгер
	InterpreterLogger init_log();

	// Тут хранятся объекты по их id
	std::map<std::string, GeneralObject> objects;

	//== Элементы грамматики интерпретатора ===================================
	using Id = std::string;
	struct Expression;

	// Функция, состоит из имени и сигнатуры
	// Предикат - тоже функция, но на выходе boolean
	struct Function {
		// Имя функции
		std::string name;
		// Типы входных аргументов
		std::vector<ObjectType> input;
		// Тип выходного аргумента
		ObjectType output;
		Function() {}
		Function(std::string name, std::vector<ObjectType> input, ObjectType output)
			: name(name), input(input), output(output) {}
	};

	friend bool operator==(const Function& l, const Function& r);

	// Композиция функций и аргументы к ней
	struct FunctionSequence {
		// Композиция функций
		std::vector<Function> functions;
		// Параметры композиции функций (1 или более)
		std::vector<Expression> parameters;
		// Надо ли отображать результат
		bool show_result = 0;
		// Преобразование в текст
		std::string to_txt() const;
	};

	using Array = std::vector<Expression>;

	// Общий вид выражения
	struct Expression {
		ObjectType type;
		std::variant<int, FunctionSequence, Regex, std::string, Array> value;
		// Преобразование в текст
		std::string to_txt() const;
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
		std::vector<Expression> arguments;
	};

	// SetFlag [flagname] [value]
	struct SetFlag {
		std::string name;
		bool value;
	};

	// Флаги:

	std::map<std::string, Flag> flags_names = {
		{"auto_remove_trap_states", Flag::auto_remove_trap_states},
		{"weak_type_comparison", Flag::weak_type_comparison},
		{"log_theory", Flag::log_theory},
	};

	std::map<Flag, bool> flags = {
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
	};

	// Общий вид опрерации
	using GeneralOperation = std::variant<Declaration, Test, Predicate, SetFlag, Verification>;

	//== Парсинг ==============================================================

	struct Lexem;

	// Находит парную закрывающую скобку
	int find_closing_par(const std::vector<Lexem>&, size_t pos);

	std::optional<Id> scan_id(const std::vector<Lexem>&, int& pos, // NOLINT(runtime/references)
							  size_t end);						   // NOLINT(runtime/references)
	std::optional<Regex> scan_regex(const std::vector<Lexem>&,
									int& pos,	 // NOLINT(runtime/references)
									size_t end); // NOLINT(runtime/references)
	std::optional<FunctionSequence> scan_function_sequence(
		const std::vector<Lexem>&, int& pos, size_t end); // NOLINT(runtime/references)
	std::optional<Array> scan_array(const std::vector<Lexem>&,
									int& pos,	 // NOLINT(runtime/references)
									size_t end); // NOLINT(runtime/references)
	std::optional<Expression> scan_expression(const std::vector<Lexem>&,
											  int& pos,	   // NOLINT(runtime/references)
											  size_t end); // NOLINT(runtime/references)

	// перевод ObjectType в std::string (для логирования и дебага)
	std::map<ObjectType, std::string> types_to_string = {
		{ObjectType::NFA, "NFA"},
		{ObjectType::DFA, "DFA"},
		{ObjectType::Regex, "Regex"},
		{ObjectType::RandomRegex, "RandomRegex"},
		{ObjectType::Int, "Int"},
		{ObjectType::String, "std::string"},
		{ObjectType::Boolean, "Boolean"},
		{ObjectType::OptionalBool, "OptionalBool"},
		{ObjectType::AmbiguityValue, "AmbiguityValue"},
		{ObjectType::PrefixGrammar, "PrefixGrammar"},
		{ObjectType::Array, "Array"},
	}; // не додумалась как по другому(не ручками) (((

	// Типизация идентификаторов. Нужна для корректного составления опреаций
	std::map<std::string, ObjectType> id_types;
	// Считывание операции из набора лексем
	std::optional<Declaration> scan_declaration(const std::vector<Lexem>&,
												int& pos); // NOLINT(runtime/references)
	std::optional<Test> scan_test(const std::vector<Lexem>&,
								  int& pos); // NOLINT(runtime/references)
	std::optional<Verification> scan_verification(
		const std::vector<Lexem>&, // NOLINT(runtime/references)
		int& pos);				   // NOLINT(runtime/references)
	std::optional<Predicate> scan_predicate(const std::vector<Lexem>&,
											int& pos); // NOLINT(runtime/references)
	std::optional<SetFlag> scan_flag(const std::vector<Lexem>&,
									 int& pos); // NOLINT(runtime/references)
	std::optional<GeneralOperation> scan_operation(const std::vector<Lexem>&);

	//== Исполнение комманд ===================================================

	// Выражение для подстановки на место *
	std::optional<Regex> current_random_regex;

	// Применение цепочки функций к набору аргументов
	std::optional<GeneralObject> apply_function_sequence(const std::vector<Function>& functions,
														 std::vector<GeneralObject> arguments);

	// Применение функции к набору аргументов
	std::optional<GeneralObject> apply_function(
		const Function& function, const std::vector<GeneralObject>& arguments,
		LogTemplate& log_template); // NOLINT(runtime/references)

	// Вычисление выражения
	std::optional<GeneralObject> eval_expression(const Expression& expr);

	// Вычисление последовательности функций
	std::optional<GeneralObject> eval_function_sequence(const FunctionSequence& seq);

	// Исполнение операций
	bool run_declaration(const Declaration&);
	bool run_predicate(const Predicate&);
	bool run_test(const Test&);
	bool run_verification(const Verification&);
	bool run_set_flag(const SetFlag&);
	bool run_operation(const GeneralOperation&);

	// Сравнение типов ожидаемых и полученных входных данных
	bool typecheck(std::vector<ObjectType> func_input_type, std::vector<ObjectType> input_type);
	// выбрать подходящий вариант функции для данных аргументов (если он есть)
	std::optional<int> find_func(std::string func, std::vector<ObjectType> input_type);
	std::optional<std::string> get_func_id(Function function);

	// Построение последовательности функций по их названиям
	std::optional<std::vector<Function>> build_function_sequence(
		std::vector<std::string> function_names, std::vector<ObjectType> first_type);

	// Соответствие между названиями функций и сигнатурами
	std::map<std::string, std::vector<Function>> names_to_functions;

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
		std::string value = "";
		// Eсли type = number
		int num = 0;

		Lexem(Type type = error, std::string value = ""); // NOLINT(runtime/explicit)
		Lexem(int num);									  // NOLINT(runtime/explicit)
	};

	class Lexer {
	  public:
		Lexer(Interpreter& parent) : parent(parent) {} // NOLINT(runtime/explicit)
		// Возвращает лексемы, разбитые по строчкам
		std::vector<std::vector<Lexem>> load_file(std::string path);
		// Бьёт строку на лексемы (без перевода строки)
		std::vector<Lexem> parse_string(std::string);

	  private:
		// Здесь храним строку и место, откуда её читаем
		struct {
		  public:
			std::string str = "";
			int pos = 0;

			void save() {
				saves.push_back(pos);
			}
			void restore() {
				pos = saves.back();
				saves.pop_back();
			}

		  private:
			std::deque<int> saves;

		} input;

		bool eof();
		char current_symbol();
		void next_symbol();
		void skip_spaces();
		bool scan_word(std::string);
		std::string scan_until_space();
		std::string scan_until(char symbol);

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