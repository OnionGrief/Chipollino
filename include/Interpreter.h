#pragma once
#include <fstream>
#include <string>
#include <set>
#include "Regex.h"
using namespace std;

class Interpreter {
private:
	struct Lexem {
		enum Type {
			error,
			equalSign,
			doubleExclamation,
			function,
			id,
			regex,
			number,
			predicate,
			test
		};

		Type type = error;
		string value = "";
		int num = 0;

		Lexem(Type type = error, string value = "");
		Lexem(int num);
	};

	// «десь храним строку и место, откуда еЄ читаем
	struct {
		string str = "";
		int pos = 0;
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
	Lexem scan_object();
	Lexem scan_id_lvalue(); // объ€вление нового ID
	Lexem scan_id_rvalue(); // считывание известного ID
	Lexem scan_regex();
	Lexem scan_number();
	Lexem scan_predicate();
	Lexem scan_test();

	// TODO: сделать класс хранени€ функции с входным и выходным типами
	// где-то надо хранить map с названи€ми вместо этого set
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

	set<string> ids = {};

public:
	Interpreter();
	void load_file(string path);
	void run_operation();
	void run_all();
};