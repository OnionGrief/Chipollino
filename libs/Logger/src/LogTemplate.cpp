#include "Logger/LogTemplate.h"

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

string LogTemplate::render() const {
	// TODO: заполнять здесь шаблон
	// That's just a demo
	string outstr = "";
	for (const auto& p : parameters) {
		outstr += "    " + p.first + " : ";
		if (holds_alternative<string>(p.second.value)) {
			outstr += get<string>(p.second.value);
		}
		if (holds_alternative<int>(p.second.value)) {
			outstr += to_string(get<int>(p.second.value));
		}
		if (holds_alternative<FiniteAutomaton>(p.second.value)) {
			outstr += get<FiniteAutomaton>(p.second.value).to_txt();
		}
		if (holds_alternative<Regex>(p.second.value)) {
			outstr += get<Regex>(p.second.value).to_txt();
		}
		outstr += "\n";
	}
	return outstr;
}

string LogTemplate::math_mode(string str) {
	if (str.empty()) {
		return str;
	}
	// cout << str << endl;
	string str_math = "";
	auto is_number = [](char c) { return c >= '0' && c <= '9'; };
	for (size_t index = 0; index < str.size(); index++) {
		char c = str[index];
		// cout << "-------------" + index << endl;
		// cout << c << endl;
		if (c == ' ' && index != str.size() - 1) {
			str_math += ", ";
		} else if (c == '*') {
			str_math += "^*";
			// cout << "c is *" << endl;
			// cout << "str_math " + str_math << endl;
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
		} else {
			str_math += c;
			cout << "c else" << endl;
			cout << "str_math " + str_math << endl;
		}
	}
	// cout << str_math << endl;
	str_math = "$" + str_math + "$";
	return str_math;
}