#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "Logger/LogTemplate.h"

using std::cout;
using std::ifstream;
using std::ofstream;
using std::string;
using std::stringstream;
using std::to_string;
using std::vector;

string write_to_file(int file_num, string content) {
	if (user_name != "") {
		ofstream out;
		string filename = user_name + "/" + to_string(file_num) + ".txt";
		out.open(filename, ofstream::trunc);
		if (out.is_open())
			out << content;
		out.close();
		return "\n%@" + filename + "\n";
	}
	return "";
}

void LogTemplate::add_parameter(string parameter_name) {
	ifstream infile(template_fullpath);

	if (!infile)
		return; // infile.close();

	string s;
	bool is_exist = false;
	int str_endframe_place, i = 0;
	while (!infile.eof()) {
		getline(infile, s);

		size_t insert_place = s.find("%template_" + parameter_name);
		size_t endframe_place = s.find("\\end{frame}");
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
		string outstr;

		int i = 0;
		while (!infile.eof()) {
			getline(infile, s);
			if (str_endframe_place == i)
				outstr += "\t" + parameter_name + ":\n\n\t%template_" + parameter_name + "\n\n" +
						  s + "\n";
			else
				outstr += s + "\n";
			i++;
		}

		infile.close();
		ofstream outfile(template_fullpath);
		outfile << outstr;
		outfile.close();
	}
}

void LogTemplate::set_parameter(const string& key, const LogObject& value, const MetaInfo& meta) {
	parameters[key] = {value, meta};
}

void LogTemplate::set_theory_flag(bool value) {
	render_theory = value;
}

void LogTemplate::load_tex_template(const string& filename) {
	template_filename = filename;
	template_fullpath = template_path + filename + ".tex";
}

string LogTemplate::get_tex_template() {
	return template_filename;
}

string replace_for_rendering(const string& s) {
	vector<std::pair<string, string>> substrs_to_replace = {{"\\^", "\\textasciicircum "},
															{"&", "\\&"}};

	string result = s;
	for (const auto& [old_substr, new_substr] : substrs_to_replace) {
		std::regex re(old_substr);
		result = std::regex_replace(result, re, new_substr);
	}

	return result;
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

		// Отображаем строчку
		if (show) {
			for (const auto& [key, param] : parameters) {
				string template_mark = "%template_" + key;
				size_t insert_place = s.find(template_mark);
				// Если имя шаблона не заканчивает строку, то это может быть и другой шаблон
				if ((insert_place != s.length() - template_mark.length()) || (insert_place == -1)) {
					continue;
				}

				if (std::holds_alternative<Regex>(param.value)) {
					// Math mode is done in global renderer
					string r = std::get<Regex>(param.value).to_txt();
					s.insert(insert_place, replace_for_rendering(r));
				} else if (std::holds_alternative<BackRefRegex>(param.value)) {
					string r = std::get<BackRefRegex>(param.value).to_txt();
					s.insert(insert_place, replace_for_rendering(r));
				} else if (std::holds_alternative<FiniteAutomaton>(param.value) ||
						   std::holds_alternative<MemoryFiniteAutomaton>(param.value)) {
					std::hash<string> hasher;
					string c_graph;
					string automaton;
					string graph_name;
					iLogTemplate::Table table;
					if (std::holds_alternative<FiniteAutomaton>(param.value)) {
						FiniteAutomaton fa = std::get<FiniteAutomaton>(param.value);
						std::tie(fa, table) = fa.short_labels();
						graph_name = write_to_file(image_number++, fa.to_dsl());
						automaton = fa.to_txt();
					} else {
						MemoryFiniteAutomaton mfa = std::get<MemoryFiniteAutomaton>(param.value);
						std::tie(mfa, table) = mfa.short_labels();
						graph_name = write_to_file(image_number++, mfa.to_dsl());
						automaton = mfa.to_txt();
					}
					size_t hash = hasher(automaton);
					if (cache_automatons.count(hash) != 0) {
						c_graph = cache_automatons[hash];
					} else {
						c_graph = AutomatonToImage::to_image(automaton);
						cache_automatons[hash] = c_graph;
					}
					c_graph = AutomatonToImage::colorize(c_graph, param.meta.to_output());

					if (!table.is_empty()) {
						string table_name = write_to_file(image_number++, table.to_csv());
						c_graph += table_name + log_table(table);
					}
					s.insert(insert_place, graph_name + c_graph);
				} else if (std::holds_alternative<string>(param.value)) {
					string str = std::get<string>(param.value);
					s.insert(insert_place, replace_for_rendering(str));
				} else if (std::holds_alternative<int>(param.value)) {
					s.insert(insert_place, to_string(std::get<int>(param.value)));
				} else if (std::holds_alternative<Table>(param.value)) {
					Table table = std::get<Table>(param.value);
					string table_name = write_to_file(image_number++, table.to_csv());
					s.insert(insert_place, table_name + log_table(table));
				} else if (std::holds_alternative<Plot>(param.value)) {
					s.insert(insert_place, log_plot(std::get<Plot>(param.value)));
				} else {
					cout << "LOGGER ERROR: can not render object";
				}
			}
			outstr += s;
			outstr += "\n";
		}

		if (s.find("%end detailed") != -1)
			show = true;
	}

	return outstr;
}

// Функция заворачивания строки в декорацию (и размер), с учётом того, находится ли среда в мат.
// режиме
string decorate_element(const string& label, Decoration d, TextSize s, bool now_in_math) {
	string mode_switcher = LogTemplate::decor_data.at(d).is_math && !now_in_math ? "$" : "";
	return "{" + LogTemplate::textsize_to_str.at(s) + mode_switcher +
		   LogTemplate::decor_data.at(d).tag + "{" + label + "}" + mode_switcher + "}";
}

// Вычисление шага для делений на графике
int step_size(int maxscale, size_t objsize, size_t datasize) {
	int step = std::ceil((maxscale * (static_cast<int>(objsize) + 3)) /
						 std::max(static_cast<int>(datasize) - 2, 1));
	return (step > 10 ? std::floor(step / 10) * 10 : std::max(step, 1));
}

// Логирование графиков
string LogTemplate::log_plot(Plot p) {
	int max_x = 0, max_y = 0;
	string visualization = "", styling, legenda;
	vector<string> styles;
	for (int i = 0; i < p.data.size(); i++) {
		if (find(styles.begin(), styles.end(), p.data[i].plot_label) == styles.end()) {
			styles.push_back(p.data[i].plot_label);
			styling += (i == 0 ? "" : ", ") + p.data[i].plot_label;
			legenda += p.data[i].plot_label + " = {label in legend={text=" +
					   decorate_element(p.data[i].plot_label, regexstyle, none, false) + "}},\n";
		}
		if (max_x < p.data[i].x_coord)
			max_x = p.data[i].x_coord;
		if (max_y < p.data[i].y_coord)
			max_y = p.data[i].y_coord;
	}
	visualization =
		"\\begin{tikzpicture}\\scriptsize %begin_plot\n "
		"\\datavisualization[scientific axes=clean, visualize as line/.list={" +
		styling +
		"},\n x axis={ticks={step=" + to_string(step_size(max_x, styles.size(), p.data.size())) +
		"}, label=" + decorate_element("длина слова", italic, footnote, false) +
		"}, y axis={ticks={step=" + to_string(step_size(max_y, styles.size(), p.data.size())) +
		"}, label=" + decorate_element("шаги", italic, footnote, false) + "},\n" + legenda +
		"style sheet = vary hue, style sheet = vary dashing]\n "
		"data[headline={x, y, set}] {\n";
	for (auto& i : p.data) {
		visualization +=
			to_string(i.x_coord) + ", " + to_string(i.y_coord) + ", " + i.plot_label + "\n";
	}
	visualization += "};\n \\end{tikzpicture} %end_plot\n\n";
	return visualization;
}

string LogTemplate::log_table(Table t) {
	string table;
	if (!(!t.columns.empty() && !t.rows.empty()))
		return table;
	string format = "c!{\\color{black!80}\\vline width .65pt}";
	string cols = " ";
	string row;
	for (size_t i = 0; i < t.columns.size(); i++) {
		format += "c";
		string c = t.columns[i] == " " ? "eps" : t.columns[i];
		cols += " & " + c;
	}
	table += "$\\begin{array}{" + format + "}\\rowcolor{HeaderColor}\n";
	table += cols + "\\\\\n\\hline\n";
	for (size_t i = 0; i < t.rows.size(); i++) {
		row = t.rows[i] == " " ? "eps" : t.rows[i];
		for (size_t j = 0; j < t.columns.size(); j++) {
			row += " & " + replace_for_rendering(t.data[i * t.columns.size() + j]);
		}
		table += row + "\\\\\n";
	}
	table += "\\end{array}$\n";
	return table;
}

stringstream LogTemplate::expand_includes(string filename) const {
	stringstream outstream;

	ifstream infile(filename);
	if (!infile) {
		std::cerr << "ERROR: while rendering template. Unknown filename " + filename + "\n";
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
