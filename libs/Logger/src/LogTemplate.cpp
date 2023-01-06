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

void LogTemplate::load_tex_template(string filename) {
	tex_template = "./resources/template/" + filename + ".tex";
}

string LogTemplate::render() const {
	// TODO: заполнять здесь шаблон
	ifstream infile(tex_template);
	string outstr = "";
	string s;
	for (; !infile.eof();) {
		getline(infile, s);
		for (const auto& p : parameters) {
			if (s.find("template_" + p.first) == -1) {
				continue;
			}
			if (holds_alternative<Regex>(p.second.value)) {
				outstr += math_mode(get<Regex>(p.second.value).to_txt());
			} else if (holds_alternative<FiniteAutomaton>(p.second.value)) {
				image_number += 1;
				AutomatonToImage::to_image(
					get<FiniteAutomaton>(p.second.value).to_txt(),
										   image_number);
				char si[256];
				sprintf(si,
						"\\includegraphics[width=2in, "
						"keepaspectratio]{output%d.png}\n",
						image_number);
				outstr += si;
			} else if (holds_alternative<string>(p.second.value)) {
				outstr += get<string>(p.second.value);
			} else if (holds_alternative<int>(p.second.value)) {
				outstr += to_string(get<int>(p.second.value));
			}
		}
		outstr += s;
		outstr += "\n";
	}
	infile.close();
	// That's just a demo
	// for (const auto& p : parameters) {
	// 	// outstr += "    " + p.first + " : ";
	// 	if (holds_alternative<string>(p.second.value)) {
	// 		if (p.first == "first") {
	// 			outstr += "First: " + math_mode(get<string>(p.second.value));
	// 		} else if (p.first == "end") {
	// 			outstr += "End: " + math_mode(get<string>(p.second.value));
	// 		} else if (p.first == "pairs") {
	// 			outstr += "Pairs: " + math_mode(get<string>(p.second.value));
	// 		} else if (p.first == "state") {
	// 			outstr += "States: " + math_mode(get<string>(p.second.value));
	// 		} else if (p.first == "follow") {
	// 			outstr += "Follow-отношения: " +
	// 					  math_mode(get<string>(p.second.value));
	// 		} else if (p.first == "deverative") {
	// 			outstr +=
	// 				"Производные:\n" + math_mode(get<string>(p.second.value));
	// 		} else if (p.first == "prefix") {
	// 			outstr += "Префикс: " + math_mode(get<string>(p.second.value));
	// 		} else if (p.first == "result") {
	// 			outstr += "Результат: " + get<string>(p.second.value);
	// 		} else if (p.first == "equivclasses") {
	// 			outstr +=
	// 				"Классы эквивалентности: " + get<string>(p.second.value);
	// 		}

	// 		// outstr += get<string>(p.second.value);
	// 	}
	// 	if (holds_alternative<int>(p.second.value)) {
	// 		if (p.first == "pumplength") {
	// 			outstr +=
	// 				"Длина накачки: " + to_string(get<int>(p.second.value));
	// 		} else if (p.first == "result") {
	// 			outstr += "Результат: " + get<string>(p.second.value);
	// 		} else if (p.first == "state") {
	// 			outstr += "State: " + get<string>(p.second.value);
	// 		} else if (p.first == "statesnum") {
	// 			outstr +=
	// 				"Количество состояний: " + get<string>(p.second.value);
	// 		}
	// 		// outstr += to_string(get<int>(p.second.value));
	// 	}
	// 	if (holds_alternative<FiniteAutomaton>(p.second.value)) {
	// 		// outstr += get<FiniteAutomaton>(p.second.value).to_txt();
	// 		if (p.first == "automaton") {
	// 			outstr += "Автомат: \n";
	// 		} else if (p.first == "automaton1") {
	// 			outstr += "Первый автомат: \n";
	// 		} else if (p.first == "automaton2") {
	// 			outstr += "Второй автомат: \n";
	// 		} else if (p.first == "oldautomaton") {
	// 			outstr += "Автомат до преобразования: \n";
	// 		} else if (p.first == "newautomaton") {
	// 			outstr += "Автомат после преобразования: \n";
	// 		} else if (p.first == "result") {
	// 			outstr += "Результат: \n";
	// 		}
	// 		image_number += 1;
	// 		AutomatonToImage::to_image(
	// 			get<FiniteAutomaton>(p.second.value).to_txt(), image_number);
	// 		char si[256];
	// 		sprintf(si,
	// 				"\\includegraphics[width=5in, "
	// 				"keepaspectratio]{output%d.png}\n",
	// 				image_number);
	// 		outstr += si;
	// 	}
	// 	if (holds_alternative<Regex>(p.second.value)) {
	// 		if (p.first == "oldregex") {
	// 			outstr += "Регулярное выражение до преобразования: \n";
	// 		} else if (p.first == "newregex") {
	// 			outstr += "Регулярное выражение после преобразования: \n";
	// 		} else if (p.first == "regex") {
	// 			outstr += "Регулярное выражение: \n";
	// 		} else if (p.first == "regex1") {
	// 			outstr += "Первое регулярное выражение: \n";
	// 		} else if (p.first == "regex2") {
	// 			outstr += "Второе регулярное выражение: \n";
	// 		} else if (p.first == "result") {
	// 			outstr += "Результат: \n";
	// 		}
	// 		outstr += math_mode(get<Regex>(p.second.value).to_txt());
	// 	}
	// 	outstr += "\n";
	// }
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