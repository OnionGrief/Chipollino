#pragma once
#include "BaseObject.h"
#include "AutomatonToImage.h"
#include <string>
using namespace std;

class Logger: public BaseObject {
public:
	Logger();
	~Logger();
	void init();
	// начало шага, передается название
	void init_step(string step_name);
	// добавление записи
	void log(string text, string fa1, string fa2);
	// завершение шага
	void finish_step();
	// завершение записи в файл
	void finish();
	string to_txt();
};