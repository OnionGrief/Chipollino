#pragma once
#include <string>
#include <variant>
#include <vector>

#include "Objects/MetaInfo.h"

class FiniteAutomaton;
class Regex;
class MemoryFiniteAutomaton;
class BackRefRegex;
class PushdownAutomaton;

class iLogTemplate {
  public:
	struct Table {
		std::vector<std::string> rows;	  // названия строк
		std::vector<std::string> columns; // названия столбцов
		std::vector<std::string> data;	  // данные
	};

	struct Point {
		std::string plot_label;
		long x_coord;
		long y_coord;
	};

	struct Plot {
		std::vector<Point> data;
	};

	using LogObject = std::variant<FiniteAutomaton, MemoryFiniteAutomaton, PushdownAutomaton, Regex, BackRefRegex,
								   std::string, int, Table, Plot>;

	virtual void set_parameter(const std::string& key, const LogObject& value,
							   const MetaInfo& meta = {}) = 0;
};