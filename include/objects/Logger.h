#pragma once
#include "BaseObject.h"
#include <string>
#include <iostream>
using namespace std;

class Logger: public BaseObject {
public:
	Logger();
	Logger(int);
	// начало шага, передается название
	void init_step(string step_name);
	// добавление записи
	void log();
	// завершение шага
	void finish_step();
	// string to_txt();
};