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

	virtual void set_parameter(const string& key,
							   const FiniteAutomaton& value) = 0;
	virtual void set_parameter(const string& key, Regex value) = 0;
	virtual void set_parameter(const string& key, std::string value) = 0;
	virtual void set_parameter(const string& key, int value) = 0;
	virtual void set_parameter(const string& key, Table value) = 0;
};