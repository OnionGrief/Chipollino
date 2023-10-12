#pragma once
#include <string>

using namespace std;
class FiniteAutomaton;
class Regex;

class iLogTemplate {
  public:
	struct Table {
		vector<string> rows;	// названия строк
		vector<string> columns; // названия столбцов
		vector<string> data;	// данные
	};

	struct Plot {
		vector<pair<pair<int, long>, string>> data;
	};

	virtual void set_parameter(const string& key, const FiniteAutomaton& value,
							   string meta = "") = 0;
	virtual void set_parameter(const string& key, Regex value,
							   string meta = "") = 0;
	virtual void set_parameter(const string& key, std::string value,
							   string meta = "") = 0;
	virtual void set_parameter(const string& key, int value,
							   string meta = "") = 0;
	virtual void set_parameter(const string& key, Table value,
							   string meta = "") = 0;
	virtual void set_parameter(const string& key, Plot value,
							   string meta = "") = 0;
};