#pragma once
#include <fstream>
#include <string>
#include <variant>
#include <set>
#include <map>
#include <deque>
#include "Regex.h"
#include "FiniteAutomat.h"

class Interpreter {
public:

	Interpreter() {
		Lexer s;
		auto lexems = s.load_file("test.txt");
	}
private:
	// Перечисление типов объектов
	enum class ObjectType {
		NFA,      // недетерминированный КА
		DFA,      // детерминированный КА
		Regex,    // регулярное выражение
		Int,      // целое число
		Value,    // строковое значение
		FileName, // имя файла для чтения
		Boolean   // true/false
	};

	// Структуры объектов для хранения в интерпретаторе
	template <ObjectType T, class V>
	struct ObjectHolder {
		V value;
		ObjectType type() { return  T };
	};

	// Универсальный объект
	using GeneralObject = variant<
		ObjectHolder<ObjectType::NFA, FiniteAutomat>,
		ObjectHolder<ObjectType::DFA, FiniteAutomat>,
		ObjectHolder<ObjectType::Regex, Regex>,
		ObjectHolder<ObjectType::Int, int>,
		ObjectHolder<ObjectType::Value, string>,
		ObjectHolder<ObjectType::FileName, string>,
		ObjectHolder<ObjectType::Boolean, bool>
	>;

	// Тут хранятся объекты по их id
	map<string, GeneralObject> objects;

	// Функция, состоит из имени и сигнатуры
	// Предикат - тоже функция, но на выходе boolean
	struct Function {
		// Имя функции
		string name;
		// Типы воходных аргументов
		vector<ObjectType> input;
		// Тип выходного аргументов
		vector<ObjectType> output;
	};
	
	// Применение функции к набору аргументов
	GeneralObject apply_function(Function, vector<ObjectType>); // TODO

	// Операция объявления
	// [идентификатор] = ([функция].)*[функция]? [объект]+ (!!)?
	struct DecalarationOp {
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
		set<string> functions = {
			"Thompson",
			"IlieYu",
			"Antimirov",
			"Arden",
			"Glushkov",
			"Determinize",
			"RemEps",
			"Linearize",
			"Minimize",
			"Reverse",
			"Annote",
			"DeLinearize",
			"Complement",
			"MergeBisim",
			"PumpLength",
			"ClassLength",
			"KSubSet",
			"Normalize",
			"States",
			"ClassCard",
			"Ambiguity",
			"Width",
			"MyhillNerode",
			"Simplify",
		};

		set<string> predicates = {
			"Bisimilar",
			"Minimal",
			"Subset",
			"Equiv",
			"Minimal",
			"Equal",
			"SemDet",
		};
	};
};