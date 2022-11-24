#include "LogTemplate.h"
#include "..\include\Logger\LogTemplate.h"

void LogTemplate::set_parameter(const string& key, FiniteAutomaton value) {
	parameters[key] = value;
}

void LogTemplate::set_parameter(const string& key, Regex value) {
	parameters[key] = value;
}

void LogTemplate::set_parameter(const string& key, string value) {
	parameters[key] = value;
}

void LogTemplate::set_parameter(const string& key, int value) {
	parameters[key] = value;
}

void LogTemplate::render() {
	// TODO: заполнять здесь шаблон
	return "";
}