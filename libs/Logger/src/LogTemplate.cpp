#include "Logger/LogTemplate.h"
#include <variant>
#include <algorithm>
#include <cmath>

void LogTemplate::add_parameter(string parameter_name) {

	ifstream infile(template_fullpath);

	if (!infile) return; // infile.close();

	string s;
	bool is_exist = false;
	int str_endframe_place, i = 0;
	while (!infile.eof()) {
		getline(infile, s);

		int insert_place = s.find("%template_" + parameter_name);
		int endframe_place = s.find("\\end{frame}");
		if (endframe_place != -1) {
			str_endframe_place = i;
		}
		i++;

		if (insert_place != -1) {
			is_exist = true;
			break;
		}
	}

	infile.close();

	if (!is_exist) {
		infile.open(template_fullpath);
		string outstr = "";

		int i = 0;
		while (!infile.eof()) {
			getline(infile, s);
			if (str_endframe_place == i) {
				outstr += "\t" + parameter_name + ":\n\n\t%template_" + parameter_name + "\n\n" +
						  s + "\n";
			} else {
				outstr += s + "\n";
			}
			i++;
		}

		infile.close();
		ofstream outfile(template_fullpath);
		outfile << outstr;
		outfile.close();
	}
}

void LogTemplate::set_parameter(const string& key, const FiniteAutomaton& value, string meta) {
	parameters[key].value = value;
	parameters[key].meta = meta;
}

void LogTemplate::set_parameter(const string& key, Regex value, string meta) {
	parameters[key].value = value;
	parameters[key].meta = meta;
}

void LogTemplate::set_parameter(const string& key, string value, string meta) {
	parameters[key].value = value;
	parameters[key].meta = meta;
}

void LogTemplate::set_parameter(const string& key, int value, string meta) {
	parameters[key].value = value;
	parameters[key].meta = meta;
}

void LogTemplate::set_parameter(const string& key, Table value, string meta) {
	parameters[key].value = value;
	parameters[key].meta = meta;
}

void LogTemplate::set_parameter(const string& key, Plot value, string meta) {
	parameters[key].value = value;
	parameters[key].meta = meta;
	add_parameter(key);
}

void LogTemplate::set_theory_flag(bool value) {
	render_theory = value;
}

void LogTemplate::load_tex_template(string filename) {
	template_filename = filename;
	template_fullpath = template_path + filename + ".tex";
}

string LogTemplate::get_tex_template() {
	return template_filename;
}

string LogTemplate::render() const {
	stringstream infile = expand_includes(template_fullpath);

	// Строка-аккумулятор
	string outstr = "";

	// Если false, отображение отключается, скипаем строчки, пока не станет true
	bool show = true;
	while (!infile.eof()) {
		// Сюда записываем строчку
		string s;
		getline(infile, s);

		// Проверка на detailed-блоки
		if (s.find("%begin detailed") != -1) {
			if (!render_theory) {
				show = false;
			}
		}
		if (s.find("%end detailed") != -1) {
			show = true;
		}

		// Отображаем строчку
		if (show) {
			for (const auto& p : parameters) {
				int insert_place = s.find("%template_" + p.first);
				if (insert_place == -1) {
					continue;
				}

				if (holds_alternative<Regex>(p.second.value)) {
					s.insert(insert_place,
							 get<Regex>(p.second.value).to_txt()); /* Math mode is done in global
																	  renderer */
				} else if (holds_alternative<FiniteAutomaton>(p.second.value)) {
					hash<string> hasher;
					string c_graph;
					string automaton =
						get<FiniteAutomaton>(p.second.value).to_txt();
					size_t hash = hasher(automaton);
					if (cache_automatons.count(hash) != 0) {
						c_graph = cache_automatons[hash];
					} else {
						c_graph = AutomatonToImage::to_image(automaton);
						cache_automatons[hash] = c_graph;
					}
					c_graph =
						AutomatonToImage::colorize(c_graph, p.second.meta);
					s.insert(insert_place, c_graph);
				} else if (holds_alternative<string>(p.second.value)) {
					s.insert(insert_place, get<string>(p.second.value));
				} else if (holds_alternative<int>(p.second.value)) {
					s.insert(insert_place, to_string(get<int>(p.second.value)));
				} else if (holds_alternative<Table>(p.second.value)) {
					s.insert(insert_place,
							 log_table(get<Table>(p.second.value)));
				} else if (holds_alternative<Plot>(p.second.value)) {
					s.insert(insert_place, log_plot(get<Plot>(p.second.value)));
				}
			}
			outstr += s;
			outstr += "\n";
		}
	}

	return outstr;
}

string LogTemplate::math_mode(string str) {
	if (str.empty()) {
		return str;
	}
	string str_math = "";
	bool flag = true;
	auto is_number = [](char c) { return c >= '0' && c <= '9'; };
	auto is_symbol = [](char c) { return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z'; };
	for (size_t index = 0; index < str.size(); index++) {
		char c = str[index];
		if (c == ' ' && index != str.size() - 1) {
			if (!flag) {
				str_math += "}";
				flag = true;
			}
			str_math += ", ";
		} else if (c == '*') {
			if (!flag) {
				str_math += "}";
				flag = true;
			}
			str_math += "\\star ";
		} else if (c == '|') {
			if (!flag) {
				str_math += "}";
				flag = true;
			}
			str_math += "\\alter ";
		} else if (is_number(c)) {
			string num = "";
			for (index; index < str.size() && is_number(str[index]); index++) {
				num += str[index];
			}
			num = "_{" + num + "}";
			str_math += num;
			index--;
		} else if (is_symbol(c)) {
			string sym = "";
			if (flag) {
				str_math += "\\regexpstr{";
				flag = false;
			}
			for (index; index < str.size() && is_symbol(str[index]); index++) {
				sym += str[index];
			}
			str_math += sym;
			index--;
		} else {

			if (!flag) {
				str_math += "}";
				flag = true;
			}
			str_math += c;
		}
	}
	if (!flag) {
		str_math += "}";
	}
	str_math = "$" + str_math + "$";
	return str_math;
}

string LogTemplate::log_plot(Plot p) {
	size_t max_x, max_y;
	string visualization = "", styling, legenda;
	vector<string> styles;
	if (p.data.size()) {
		styles.push_back(p.data[0].second);
		styling = p.data[0].second;
		legenda = p.data[0].second + " = {label in legend={text=$\\regexpstr{" +
				  p.data[0].second + "}$}},\n";
		max_x = unsigned(p.data[0].first.first);
		max_y = unsigned(p.data[0].first.second);
	}
	for (int i = 1; i < p.data.size(); i++) {
		if (find(styles.begin(), styles.end(), p.data[i].second) ==
			styles.end()) {
			styles.push_back(p.data[i].second);
			styling += ", " + p.data[i].second;
			legenda += p.data[i].second +
					   " = {label in legend={text=$\\regexpstr{" +
					   p.data[i].second + "}$}},\n";
		}
		if (max_x < p.data[i].first.first) max_x = p.data[i].first.first;
		if (max_y < p.data[i].first.second) max_y = p.data[i].first.second;
	}
	max_x =
		std::ceil((max_x * (styles.size() + 3)) / max(p.data.size() - 2, size_t(1)));
	if (max_x > 10) {
		max_x = std::floor(max_x / 10) * 10;
	}
	max_y =
		std::ceil((max_y * (styles.size() + 3)) / max(p.data.size() - 2, size_t(1)));
	if (max_y > 10) {
		max_y = std::floor(max_y / 10) * 10;
	}
	visualization =
		"\\begin{tikzpicture}\\scriptsize \%begin_plot\n "
		"\\datavisualization[scientific axes=clean, visualize as line/.list={" +
		styling + "},\n x axis={ticks={step=" + to_string(max_x) +
		"}, label=\\footnotesize\\textit{длина слова}}, y axis={ticks={step=" +
		to_string(max_y) + "}, label=\\footnotesize\\textit{шаги}},\n" +
		legenda +
		"style sheet = vary hue, style sheet = vary dashing]\n "
		"data[headline={x, y, set}] {\n";
	for (int i = 0; i < p.data.size(); i++) {
		visualization += to_string(p.data[i].first.first) + ", " +
						 to_string(p.data[i].first.second) + ", " +
						 p.data[i].second + "\n";
	}
	visualization += "};\n \\end{tikzpicture} \%end_plot\n\n";
	return visualization;
}

string LogTemplate::log_table(Table t) {
	string table = "";
	if (!(t.columns.size() && t.rows.size())) return table;
	string format = "c!{\\color{black!80}\\vline width .65pt}";
	string cols = "  &";
	string row = "";
	for (int i = 0; i < t.columns.size(); i++) {
		format += "c";
		string c = t.columns[i] == " " ? "eps" : t.columns[i];
		if (i != t.columns.size() - 1) {
			cols += c + " &";
		} else {
			cols += c + "\\\\";
		}
	}
	table += "$\\begin{array}{" + format + "}\\rowcolor{HeaderColor}\n";
	table += cols + "\\hline\n";
	for (int i = 0; i < t.rows.size(); i++) {
		string r = t.rows[i] == " " ? "eps" : t.rows[i];
		row = r + " & ";
		for (int j = 0; j < t.columns.size(); j++) {
			if (j != t.columns.size() - 1) {
				row = row + t.data[i * t.columns.size() + j] + " &";
			} else {
				row = row + t.data[i * t.columns.size() + j] + "\\\\";
			}
		}
		table += row + "\n";
	}
	table += "\\end{array}$\n";
	return table;
}

stringstream LogTemplate::expand_includes(string filename) const {
	stringstream outstream;

	ifstream infile(filename);
	if (!infile) {
		cerr << "ERROR: while rendering template. Unknown filename " + filename + "\n";
		return outstream;
	}

	while (!infile.eof()) {
		// Сюда записываем строчку
		string s;
		getline(infile, s);

		// Проверка include
		if (s.find("%include") != -1) {
			int l_bound = s.find("\"");
			int r_bound = s.find("\"", l_bound + 1);
			if (l_bound != -1 && r_bound != -1) {
				string input_file_name = s.substr(l_bound + 1, r_bound - l_bound - 1);

				outstream << expand_includes(template_path + input_file_name).str();
			} else {
				cout << "ERROR: Expected quotes \"\" after %include statement";
			}
		} else {
			outstream << s << "\n";
		}
	}

	return outstream;
}
