#pragma once
#include <string>
#include <variant>
#include <vector>

#include "Objects/MetaInfo.h"

class FiniteAutomaton;
class Regex;
class MemoryFiniteAutomaton;
class BackRefRegex;

class iLogTemplate {
  public:
	struct Table {
		std::vector<std::string> rows;	  // названия строк
		std::vector<std::string> columns; // названия столбцов
		std::vector<std::string> data;	  // данные

		std::string to_csv() {
			std::string table_str;
			for (int i = 0; i < columns.size(); i++) {
				table_str += ";";
				table_str += columns[i] == " " ? "eps" : columns[i];
			}
			for (int i = 0; i < rows.size(); i++) {
				std::string r = rows[i] == " " ? "eps" : rows[i];
				std::string row = r;
				for (int j = 0; j < columns.size(); j++) {
					row += ";" + data[i * columns.size() + j];
				}
				table_str += "\n" + row;
			}
			return table_str;
		}
		bool is_empty() {
			return data.size() + rows.size() == 0;
		}
	};

	struct Point {
		std::string plot_label;
		long x_coord;
		long y_coord;
	};

	struct Plot {
		std::vector<Point> data;
	};

	using LogObject = std::variant<FiniteAutomaton, MemoryFiniteAutomaton, Regex, BackRefRegex,
								   std::string, int, Table, Plot>;

	virtual void set_parameter(const std::string& key, const LogObject& value,
							   const MetaInfo& meta = {}) = 0;
};