#pragma once
#include "FiniteAutomaton.h"
#include "Regex.h"
#include "Typization.h"
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
	// Загрузить программу из файла
	void load_file(const string& filename);
	// Выполнить все опреации
	void run_all();
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

	// Операция объявления
	// [идентификатор] = ([функция].)*[функция]? [объект]+ (!!)?
	struct Declaration {
		// Идентификатор, в который запишется объект
		string id;
		// Композиция функций
		vector<Function> function_sequence;
		// Параметры композиции функций (1 или более). Могут содержать ID
		vector<variant<string, GeneralObject>> parameters;
		// Надо ли отображать результат
		bool show_result = 0;
	};

	// Специальная форма test
	struct Test {
		// Аргументы:
		// НКА или регулярное выражение;
		variant<Regex, string> sample;
		// регулярное выражение без альтернатив(только с итерацией Клини) —
		// тестовый сет;
		variant<Regex, string> test_set;
		// натуральное число — шаг итерации в сете.
		int iterations;
	};

	// Предикат [предикат] [объект]+
	struct Predicate {
		// Функция (предикат)
		Function predicate;
		// Параметры (могут быть идентификаторами)
		vector<variant<string, GeneralObject>> parameters;
	};

	// Общий вид опрерации
	using GeneralOperation = variant<Declaration, Test, Predicate>;

	vector<GeneralObject> parameters_to_arguments(
		const vector<variant<string, GeneralObject>>& parameters);
	void run_declaration(const Declaration&);
	void run_predicate(const Predicate&);
	void run_operation(const GeneralOperation&);

	struct Lexem;

	// Типизация идентификаторов. Нужна для корректного составления опреаций
	map<string, ObjectType> id_types;
	// Считывание операции из набора лексем
	optional<Declaration> scan_declaration(vector<Lexem>);
	optional<Test> scan_test(vector<Lexem>); // TODO
	optional<Predicate> scan_predicate(vector<Lexem>);
	optional<GeneralOperation> scan_operation(vector<Lexem>);

	// Список опреаций для последовательного выполнения
	vector<GeneralOperation> operations;

	// Построение последовательности функций по их названиям
	optional<vector<Function>> build_function_sequence(
		vector<string> function_names, vector<ObjectType> first_type); // TODO
	// Множество всех функций; TODO: инициализировать его в конструкторе
	// Interpreter()
	set<Function> functions; // TODO: определить operator< для Function
	// Так предлагается сделать мапинг между названиями функций и сигнатурами
	// разумеется, генерировать эту мапу можно при инициализации
	map<string, vector<Function>> names_to_functions;
	// Заполнение мапы names_to_functions по сету functions
	// void generate_function_mapping(); // TODO

	struct Lexem {
		enum Type { // TODO добавить тип строки (для filename)
			error,
			equalSign,
			doubleExclamation,
			function,
			id,
			dot,
			regex,
			number,
			predicate,
			test
		};

		Type type = error;
		// Если type = id | function | predicate
		string value = "";
		// Усли type = number
		int num = 0;
		// Если type = regex
		Regex reg;

		Lexem(Type type = error, string value = "");
		Lexem(int num);
	};

	class Lexer {
	  public:
		Lexer(Interpreter& parent) : parent(parent) {}
		vector<vector<Lexem>> load_file(string path);
		// Возвращает лексемы, разбитые по строчкам
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

		Lexem scan_equalSign();
		Lexem scan_doubleExclamation();
		Lexem scan_function();
		Lexem scan_id();
		Lexem scan_dot();
		Lexem scan_regex();
		Lexem scan_number();
		Lexem scan_predicate();
		Lexem scan_test();
		Lexem scan_lexem();

		// TODO: сделать класс хранения функции с входным и выходным типами
		// где-то надо хранить map с названиями вместо этого set
		set<string> functions = {
			"Thompson",	  "IlieYu",		 "Antimirov",	 "Arden",
			"Glushkov",	  "Determinize", "RemEps",		 "Linearize",
			"Minimize",	  "Reverse",	 "Annote",		 "DeLinearize",
			"Complement", "MergeBisim",	 "PumpLength",	 "ClassLength",
			"KSubSet",	  "Normalize",	 "States",		 "ClassCard",
			"Ambiguity",  "Width",		 "MyhillNerode", "Simplify",
		};

		set<string> predicates = {
			"Bisimilar", "Minimal", "Subset", "Equiv",
			"Minimal",	 "Equal",	"SemDet",
		};

		// Высокоточный костыль
		set<string> ids;

		Interpreter& parent;
	};
};