#pragma once
#include "FiniteAutomat.h"
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
	Interpreter();

  private:
	// Применение функции к набору аргументов
	GeneralObject apply_function(Function, vector<ObjectType>); // TODO

	// Тут хранятся объекты по их id
	map<string, GeneralObject> objects;

	// Операция объявления
	// [идентификатор] = ([функция].)*[функция]? [объект]+ (!!)?
	struct Decalaration {
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
		variant<Regex, FiniteAutomat> sample;
		// регулярное выражение без альтернатив(только с итерацией Клини) —
		// тестовый сет;
		Regex test_set;
		// натуральное число — шаг итерации в сете.
		int iterations;
	};

	// Предикат [предикат] [объект]+
	struct Predicate {
		// Функция (предикат)
		Function predicate;
		// Параметры
		vector<GeneralObject> parameters;
	};

	// Общий вид опрерации
	using GeneralOperation = variant<Decalaration, Test, Predicate>;

	struct Lexem;

	// Типизация идентификаторов. Нужна для корректного составления опреаций
	map<string, ObjectType> id_types;
	// Считывание операции из набора лексем
	optional<Decalaration> scan_declaration(vector<Lexem>);	  // TODO
	optional<Test> scan_test(vector<Lexem>);				  // TODO
	optional<Predicate> scan_predicate(vector<Lexem>);		  // TODO
	optional<GeneralOperation> scan_operation(vector<Lexem>); // TODO

	// Построение последовательности функций по их названиям
	optional<vector<Function>> build_function_sequence(
		vector<string> function_names, vector<ObjectType>); // TODO

	// Множество всех функций; TODO: инициализировать его в конструкторе
	// Interpreter()
	set<Function> functions; // TODO: определить operator< для Function
	// Так предлагается сделать мапинг между названиями функций и сигнатурами
	// разумеется, генерировать эту мапу можно при инициализации
	map<string, vector<Function>> names_to_functions;
	// Заполнение мапы names_to_functions по сету functions
	void generate_function_mapping(); // TODO

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
		Lexer();
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
	};
};