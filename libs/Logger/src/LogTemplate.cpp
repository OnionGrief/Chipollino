#include "Logger/LogTemplate.h"
#include <variant>

void LogTemplate::add_parameter(string parameter_name) {

	ifstream infile(tex_template);

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
		infile.open(tex_template);
		string outstr = "";

		int i = 0;
		while (!infile.eof()) {
			getline(infile, s);
			if (str_endframe_place == i) {
				outstr += "\t" + parameter_name + ":\n\n\t%template_" +
						  parameter_name + "\n\n" + s + "\n";
			} else {
				outstr += s + "\n";
			}
			i++;
		}

		infile.close();
		ofstream outfile(tex_template);
		outfile << outstr;
		outfile.close();
	}
}

void LogTemplate::set_parameter(const string& key, FiniteAutomaton value) {
	parameters[key].value = value;
	add_parameter(key);
}

void LogTemplate::set_parameter(const string& key, Regex value) {
	parameters[key].value = value;
	add_parameter(key);
}

void LogTemplate::set_parameter(const string& key, string value) {
	parameters[key].value = value;
	add_parameter(key);
}

void LogTemplate::set_parameter(const string& key, int value) {
	parameters[key].value = value;
	add_parameter(key);
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
	while (!infile.eof()) {
		getline(infile, s);
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
			}
		}
		outstr += s;
		outstr += "\n";
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