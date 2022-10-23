#pragma once
#include "AutomatonToImage.h"
#include <string>
using namespace std;

class Logger {
public:
	Logger();
	~Logger();
	static void init();
	// начало шага, передается название
	static void init_step(string step_name);
	// добавление записи
	static void log(string text, string fa1 = "", string fa2 = "");
	// завершение шага
	static void finish_step();
	// завершение записи в файл
	static void finish();
};