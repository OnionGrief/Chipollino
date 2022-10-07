#pragma once
#include <fstream>
#include <string>
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
			predicate
		};

		Type type = error;
		string value = "";
	};

	// Здесь храним строку и место, откуда её читаем
	struct {
		string str = "";
		int pos = 0;
	} input;

	
	Lexem scan_equalSign();
	Lexem scan_doubleExclamation();
	Lexem scan_function();
	Lexem scan_object();
	Lexem scan_id();
	Lexem scan_regex();
	Lexem scan_number();
	Lexem scan_predicate();

public:
	Interpreter();
	void load_file(string path);
	void run_operation();
	void run_all();
};