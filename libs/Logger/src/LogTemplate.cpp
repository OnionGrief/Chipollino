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