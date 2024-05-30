#pragma once
#include <cctype>
#include <cmath>
#include <deque>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "AutomataParser/Parser.h"
#include "FuncLib/Functions.h"
#include "FuncLib/Typization.h"
#include "InputGenerator/AutomatonGenerator.h"
#include "InputGenerator/RegexGenerator.h"
#include "Logger/Logger.h"
#include "Objects/BackRefRegex.h"
#include "Objects/FiniteAutomaton.h"
#include "Objects/Grammar.h"
#include "Objects/MemoryFiniteAutomaton.h"
#include "Objects/Regex.h"
#include "Objects/TransformationMonoid.h"

using Typization::GeneralObject;

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
	bool run_file(const std::string& path, const std::string& user_name="");
	std::string user_name = "";
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
		explicit InterpreterLogger(Interpreter& parent) : parent(parent) {
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
	std::unordered_map<std::string, GeneralObject> objects;

	// Выражение для подстановки на место *
	std::unordered_map<ObjectType, std::string> current_random_objects;
	RegexGenerator regex_generator; // TODO: менять параметры
	void generate_automaton(std::string test_path, FA_type fa_type, int states_num = 6);

	//== Элементы грамматики интерпретатора ===================================
	struct Id {
		std::string name;
	};
	struct Expression;

	friend bool operator==(const FuncLib::Function& l, const FuncLib::Function& r);

	// Композиция функций и аргументы к ней
	struct FunctionSequence {
		// Композиция функций
		std::vector<FuncLib::Function> functions;
		// Параметры композиции функций (1 или более)
		std::vector<Expression> parameters;
		// Надо ли отображать результат
		bool show_result = false;
		// Преобразование в текст
		std::string to_txt() const;
	};

	using Array = std::vector<Expression>;

	// Общий вид выражения
	struct Expression {
		Typization::ObjectType type;
		std::variant<int, FunctionSequence, Regex, BackRefRegex, std::string, Array, Id> value;
		// Преобразование в текст
		std::string to_txt() const;
	};

	// Операция объявления
	// [идентификатор] = ([функция].)*[функция]? [объект]+ (!!)?
	struct Declaration {
		// Идентификатор, в который запишется объект
		std::string id;
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

	// SetFlag [flagname] [value]
	struct SetFlag {
		std::string name;
		bool value;
	};

	// Флаги:
	bool in_verification = false;

	std::unordered_map<std::string, Flag> flags_names = {
		{"auto_remove_trap_states", Flag::auto_remove_trap_states},
		{"weak_type_comparison", Flag::weak_type_comparison},
		{"log_theory", Flag::log_theory},
	};

	std::unordered_map<Flag, bool> flags = {
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
	using GeneralOperation = std::variant<Declaration, Test, Expression, SetFlag, Verification>;

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

	// Типизация идентификаторов. Нужна для корректного составления опреаций
	std::unordered_map<std::string, Typization::ObjectType> id_types;
	// Считывание операции из набора лексем
	std::optional<Declaration> scan_declaration(const std::vector<Lexem>&,
												int& pos); // NOLINT(runtime/references)
	std::optional<Test> scan_test(const std::vector<Lexem>&,
								  int& pos); // NOLINT(runtime/references)
	std::optional<Verification> scan_verification(
		const std::vector<Lexem>&, // NOLINT(runtime/references)
		int& pos);				   // NOLINT(runtime/references)
	std::optional<Expression> scan_expression(const std::vector<Lexem>&,
											  int& pos); // NOLINT(runtime/references)
	std::optional<SetFlag> scan_flag(const std::vector<Lexem>&,
									 int& pos); // NOLINT(runtime/references)
	std::optional<GeneralOperation> scan_operation(const std::vector<Lexem>&);

	//== Исполнение комманд ===================================================

	// Применение цепочки функций к набору аргументов
	std::optional<GeneralObject> apply_function_sequence(
		const std::vector<FuncLib::Function>& functions, std::vector<GeneralObject> arguments,
		bool is_logged);

	// Применение функции к набору аргументов
	std::optional<GeneralObject> apply_function(
		const FuncLib::Function& function, const std::vector<GeneralObject>& arguments,
		LogTemplate& log_template); // NOLINT(runtime/references)

	// Вычисление выражения
	std::optional<GeneralObject> eval_expression(const Expression& expr);

	// Вычисление последовательности функций
	std::optional<GeneralObject> eval_function_sequence(const FunctionSequence& seq);

	// Исполнение операций
	bool run_declaration(const Declaration&);
	bool run_expression(const Expression&);
	bool run_test(const Test&);
	bool run_verification(const Verification&);
	bool run_set_flag(const SetFlag&);
	bool run_operation(const GeneralOperation&);

	// Сравнение типов ожидаемых и полученных входных данных
	bool typecheck(std::vector<Typization::ObjectType> func_input_type,
				   std::vector<Typization::ObjectType> input_type);
	// выбрать подходящий вариант функции для данных аргументов (если он есть)
	std::optional<int> find_func(std::string func, std::vector<Typization::ObjectType> input_type);
	std::optional<std::string> get_func_id(FuncLib::Function function);

	// Построение последовательности функций по их названиям
	std::optional<std::vector<FuncLib::Function>> build_function_sequence(
		std::vector<std::string> function_names, std::vector<Typization::ObjectType> first_type);

	// Соответствие между названиями функций и сигнатурами
	std::map<std::string, std::vector<FuncLib::Function>> names_to_functions;

	//== Лексер ===============================================================

	struct Lexem {
		enum Type { // TODO добавить тип строки (для filename)
			error,
			equalSign,
			randomObject,
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
		// Если type = id | function | expr
		std::string value = "";
		// Eсли type = number
		int num = 0;

		Lexem(Type type = error, std::string value = ""); // NOLINT(runtime/explicit)
		explicit Lexem(int num);
	};

	class Lexer {
	  public:
		explicit Lexer(Interpreter& parent) : parent(parent) {}
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