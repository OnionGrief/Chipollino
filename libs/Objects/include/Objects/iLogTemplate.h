#pragma once
#include <string>
#include <variant>

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

  using LogObject = variant<FiniteAutomaton, Regex, string, int, Table, Plot>;

	virtual void set_parameter(const string& key, const LogObject& value,
							   string meta = "") = 0;
};