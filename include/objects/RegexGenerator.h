#pragma once
#include "BaseObject.h"
#include <string>
#include <vector>
#include <iostream>

using namespace std;

class RegexGenerator : BaseObject {
private:
	vector<char> alphabet;
	int regex_length = 0;
	int star_num = 0;

public:
	RegexGenerator();
	RegexGenerator(vector<char>, int, int);
	string to_txt() override;

	char rand_symb();
};