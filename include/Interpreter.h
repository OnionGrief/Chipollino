#pragma once
#include <fstream>
#include <string>
#include <variant>
#include <set>
#include <map>
#include <deque>
#include "Regex.h"
#include "Typization.h"
#include "FiniteAutomat.h"

using namespace Typization;

class Interpreter {
public:

	Interpreter() {
		Lexer s;
		auto lexems = s.load_file("test.txt");
		functions = {{"Thompson", {"Thompson", {"Regex"}, "NFA"}},
			{"IlieYu", {"IlieYu", {"Regex"}, "NFA"}},
			{"Antimirov", {"Antimirov", {"Regex"}, "NFA"}},
			{"Arden", {"Arden", {"NFA"}, "Regex"}},
			{"Glushkov", {"Glushkov", {"Regex"}, "NFA"}},
			//преобразования внутри класса
			{"Determinize", {"Determinize", {"NFA"}, "DFA"}},
			{"RemEps", {"RemEps", {"NFA"}, "NFA"}},
			{"Linearize", {"Linearize", {"Regex"}, "Regex"}},
			{"Minimize", {"Minimize", {"NFA"}, "DFA"}},
			{"Reverse", {"Reverse", {"NFA"}, "NFA"}},
			{"Annote", {"Annote", {"NFA"}, "DFA"}},
			{"DeLinearizeNFA", {"DeLinearize", {"NFA"}, "NFA"}},
			{"DeLinearizeRegex", {"DeLinearize", {"Regex"}, "Regex"}},
			{"Complement", {"Complement", {"DFA"}, "DFA"}},
			{"DeAnnoteNFA", {"DeAnnote", {"NFA"}, "NFA"}},
			{"DeAnnoteRegex", {"DeAnnote", {"Regex"}, "Regex"}},
			{"MergeBisim", {"MergeBisim", {"NFA"}, "NFA"}},
			//Многосортные функции
			{"PumpLength", {"PumpLength", {"Regex"}, "Int"}},
			{"ClassLength", {"ClassLength", {"DFA"}, "Int"}},
			{"KSubSet", {"KSubSet", {"Int", "NFA"}, "NFA"}},
			{"Normalize", {"Normalize", {"Regex", "FileName"}, "Regex"}},
			{"States", {"States", {"NFA"}, "Int"}},
			{"ClassCard", {"ClassCard", {"DFA"}, "Int"}},
			{"Ambiguity", {"Ambiguity", {"NFA"}, "Value"}},
			{"Width", {"Width", {"NFA"}, "Int"}},
			{"MyhillNerode", {"MyhillNerode", {"DFA"}, "Int"}},
			{"Simplify", {"Simplify", {"Regex"}, "Regex"}},
			//Предикаты
			{"Bisimilar", {"Bisimilar", {"NFA", "NFA"}, "Boolean"}},
			{"Minimal", {"Minimal", {"DFA"}, "Boolean"}},
			{"Subset", {"Subset", {"Regex", "Regex"}, "Boolean"}},
			{"Equiv", {"Equiv", {"NFA", "NFA"}, "Boolean"}},
			{"Minimal", {"Minimal", {"NFA"}, "Boolean"}},
			{"Equal", {"Equal", {"NFA", "NFA"}, "Boolean"}},
			{"SemDet", {"SemDet", {"NFA"}, "Boolean"}}};
	}
private:
	// Применение функции к набору аргументов
	GeneralObject apply_function(Function, vector<ObjectType>); // TODO

	// Тут хранятся объекты по их id
	map<string, GeneralObject> objects;

	// Операция объявления
	// [идентификатор] = ([функция].)*[функция]? [объект]+ (!!)?
	struct Decalaration {
		// Идентификатор, в который запишется объект
		string  id;
		// Композиция функций
		vector<Function> function_sequence;
		// Параметры композиции функций (1 или более)
		vector<GeneralObject> parameters;
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

	// Считывание операции из набора лексем
	Decalaration scan_declaration(vector<Lexem>); // TODO
	Test scan_test(vector<Lexem>); // TODO
	Predicate scan_predicate(vector<Lexem>); // TODO
	GeneralOperation scan_operation(vector<Lexem>); // TODO

	// Построение последовательности функций по их названиям
	optional<vector<Function>> build_function_sequence(vector<string> function_names); // TODO

	// Множество всех функций; TODO: инициализировать его в конструкторе Interpreter()
	map<string, Function> functions; // TODO: определить operator< для Function
	// Так предлагается сделать мапинг между названиями функций и сигнатурами
	// разумеется, генерировать эту мапу можно при инициализации
	map<string, vector<Function>> names_to_functions;
	// Заполнение мапы names_to_functions по сету functions
	void generate_function_mapping(); // TODO

	class Lexer {
	public:
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

			Lexem(Type type = error, string value = "");
			Lexem(int num);
		};

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
	};
};