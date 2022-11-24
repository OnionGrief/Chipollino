#pragma once
#include <string>

using namespace std;
class FiniteAutomaton;
class Regex;

class iLogTemplate {
  public:
	virtual void set_parameter(const string& key, FiniteAutomaton value) = 0;
	virtual void set_parameter(const string& key, Regex value) = 0;
	virtual void set_parameter(const string& key, std::string value) = 0;
	virtual void set_parameter(const string& key, int value) = 0;
};