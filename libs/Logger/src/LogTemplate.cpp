#include "Logger/LogTemplate.h"
#include <variant>

void LogTemplate::set_parameter(const string& key, FiniteAutomaton value) {
	parameters[key].value = value;
}

void LogTemplate::set_parameter(const string& key, Regex value) {
	parameters[key].value = value;
}

void LogTemplate::set_parameter(const string& key, string value) {
	parameters[key].value = value;
}

void LogTemplate::set_parameter(const string& key, int value) {
	parameters[key].value = value;
}

void LogTemplate::set_parameter(const string& key, Table value) {
	parameters[key].value = value;
}

void LogTemplate::set_theory_flag(bool value) {
	render_theory = value;
}

void LogTemplate::load_tex_template(string filename) {
	tex_template = "./resources/template/" + filename + ".tex";
}

string LogTemplate::render() const {
	// TODO: заполнять здесь шаблон
	ifstream infile(tex_template);
	// если шаблона не нашлось
	if (!infile) return ""; // infile.close();

	string outstr = "";
	string s;
	bool detailed = true;
	while (!infile.eof()) {
		getline(infile, s);
		if (s.find("%begin detailed") != -1) {
			if (!render_theory) {
				detailed = false;
			}
		}
		if (s.find("%end detailed") != -1) {
			detailed = true;
		}
		if (detailed) {
			for (const auto& p : parameters) {
				int insert_place = s.find("%template_" + p.first);
				if (insert_place == -1) {
					continue;
				}

				if (holds_alternative<Regex>(p.second.value)) {
					s.insert(insert_place,
							 math_mode(get<Regex>(p.second.value).to_txt()));
				} else if (holds_alternative<FiniteAutomaton>(p.second.value)) {
					image_number += 1;
					string graph = AutomatonToImage::to_image(
						get<FiniteAutomaton>(p.second.value).to_txt());
					/*char si[256];
					sprintf(si,
							"\\includegraphics[height=1.3in, "
							"keepaspectratio]{output%d.png}\n",
							image_number);*/
					s.insert(insert_place, graph);
				} else if (holds_alternative<string>(p.second.value)) {
					s.insert(insert_place, get<string>(p.second.value));
				} else if (holds_alternative<int>(p.second.value)) {
					s.insert(insert_place, to_string(get<int>(p.second.value)));
				} else if (holds_alternative<Table>(p.second.value)) {
					s.insert(insert_place,
							 log_table(get<Table>(p.second.value)));
				}
			}
			outstr += s;
			outstr += "\n";
		}
	}
	infile.close();
	return outstr;
}

string LogTemplate::math_mode(string str) {
	if (str.empty()) {
		return str;
	}
	// cout << str << endl;
	string str_math = "";
	bool flag = true;
	auto is_number = [](char c) { return c >= '0' && c <= '9'; };
	auto is_symbol = [](char c) {
		return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
	};
	for (size_t index = 0; index < str.size(); index++) {
		char c = str[index];
		// cout << "-------------" + index << endl;
		// cout << c << endl;
		if (c == ' ' && index != str.size() - 1) {
			if (!flag) {
				str_math += "}";
				flag = true;
			}
			str_math += ", ";
		} else if (c == '*') {
			// str_math += "^*";
			if (!flag) {
				str_math += "}";
				flag = true;
			}
			str_math += "\\star ";
			// cout << "c is *" << endl;
			// cout << "str_math " + str_math << endl;
		} else if (c == '|') {
			if (!flag) {
				str_math += "}";
				flag = true;
			}
			str_math += "\\alter ";
		} else if (is_number(c)) {
			// cout << "c number" << endl;
			string num = "";
			for (index; index < str.size() && is_number(str[index]); index++) {
				num += str[index];
				// cout << "num " + num << endl;
			}
			num = "_{" + num + "}";
			str_math += num;
			index--;
			// cout << "index " + to_string(index) << endl;
			// cout << "str_math " + str_math << endl;
		} else if (is_symbol(c)) {
			string sym = "";
			if (flag) {
				str_math += "\\regexpstr{";
				flag = false;
			}
			for (index; index < str.size() && is_symbol(str[index]); index++) {
				sym += str[index];
				// cout << "num " + num << endl;
			}
			// sym = "\\regexpstr{" + sym + "}";
			str_math += sym;
			index--;
		} else {

			if (!flag) {
				str_math += "}";
				flag = true;
			}
			str_math += c;
			// cout << "c else" << endl;
			// cout << "str_math " + str_math << endl;
		}
	}
	// cout << str_math << endl;
	if (!flag) {
		str_math += "}";
	}
	str_math = "$" + str_math + "$";
	return str_math;
}

string LogTemplate::log_table(Table t/*vector<string> rows, vector<string> columns,
							  vector<string> data*/) {
	string table = "";
	string format = "|l|";
	string cols = "  & ";
	string row = "";
	for (int i = 0; i < t.columns.size(); i++) {
		format += "l|";
		if (i != t.columns.size() - 1) {
			cols += t.columns[i] + " & ";
		} else {
			cols += t.columns[i] + "\\\\";
		}
	}
	table += "\\begin{tabular}{" + format + "}\n";
	table += "\\hline\n";
	table += cols + "\n";
	table += "\\hline\n";
	int k = 0;
	int j;
	for (int i = 0; i < t.rows.size(); i++) {
		row = t.rows[i] + " & ";
		for (j = 0; j < t.columns.size(); j++) {
			if (j != t.columns.size() - 1) {
				row = row + t.data[k + j] + " & ";
			} else {
				row = row + t.data[k + j] + "\\\\";
			}
		}
		k += j;
		table += row + "\n";
		table += "\\hline\n";
	}
	table += "\\end{tabular}\n";
	return table;
}