#include <algorithm>
#include <cmath>
#include <regex>
#include <variant>

#include "Logger/LogTemplate.h"

using std::cout;
using std::ifstream;
using std::ofstream;
using std::string;
using std::stringstream;
using std::to_string;
using std::vector;

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
			for (const auto& [key, param] : parameters) {
				int insert_place = s.find("%template_" + key);
				// Если имя шаблона не заканчивает строку, то это может быть и другой шаблон
				if ((insert_place != s.length() - 10 - key.length()) || (insert_place == -1)) {
					continue;
				}

				if (std::holds_alternative<Regex>(param.value)) {
					// Math mode is done in global renderer
					string r0 = std::get<Regex>(param.value).to_txt();
					string r = std::regex_replace(r0, std::regex("\\^"), "\\textasciicircum ");
					s.insert(insert_place, r);
				} else if (std::holds_alternative<FiniteAutomaton>(param.value)) {
					std::hash<string> hasher;
					string c_graph;
					string automaton0 = std::get<FiniteAutomaton>(param.value).to_txt();
					string automaton = std::regex_replace(automaton0, std::regex("\\^"), "\\textasciicircum ");
					size_t hash = hasher(automaton);
					if (cache_automatons.count(hash) != 0) {
						c_graph = cache_automatons[hash];
					} else {
						c_graph = AutomatonToImage::to_image(automaton);
						cache_automatons[hash] = c_graph;
					}
					c_graph = AutomatonToImage::colorize(c_graph, param.meta.to_output());
					s.insert(insert_place, "\n" + c_graph);
				} else if (std::holds_alternative<string>(param.value)) {
					string s0 = std::get<string>(param.value);
					string str = std::regex_replace(s0, std::regex("\\^"), "\\textasciicircum ");
					s.insert(insert_place, str);
				} else if (std::holds_alternative<int>(param.value)) {
					s.insert(insert_place, to_string(std::get<int>(param.value)));
				} else if (std::holds_alternative<Table>(param.value)) {
					s.insert(insert_place, log_table(std::get<Table>(param.value)));
				} else if (std::holds_alternative<Plot>(param.value)) {
					s.insert(insert_place, log_plot(std::get<Plot>(param.value)));
				}
			}
			outstr += s;
			outstr += "\n";
		}
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
		"\\begin{tikzpicture}\\scriptsize \%begin_plot\n " // NOLINT(build/printf_format)
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
	visualization += "};\n \\end{tikzpicture} \%end_plot\n\n"; // NOLINT(build/printf_format)
	return visualization;
}

string LogTemplate::log_table(Table t) {
	string table;
	if (!(!t.columns.empty() && !t.rows.empty()))
		return table;
	string format = "c!{\\color{black!80}\\vline width .65pt}";
	string cols = "  &";
	string row;
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
