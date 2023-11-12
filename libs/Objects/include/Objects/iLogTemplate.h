#pragma once
#include <string>
#include <variant>
#include <vector>

#include "Objects/MetaInfo.h"

class FiniteAutomaton;
class Regex;

class iLogTemplate {
  public:
	struct Table {
		vector<string> rows;	// названия строк
		vector<string> columns; // названия столбцов
		vector<string> data;	// данные
	};

	struct Point {
		string plot_id;
		int x_coord;
		long y_coord;
	};

	struct Plot {
		vector<Point> data;
	};

	using LogObject = std::variant<FiniteAutomaton, Regex, string, int, Table, Plot>;

	virtual void set_parameter(const string& key, const LogObject& value,
							   const MetaInfo& meta = {}) = 0;
};