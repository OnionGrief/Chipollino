#pragma once
#include "AutomatonToImage.h"
#include "FiniteAutomaton.h"
#include <string>
using namespace std;

class Logger {
private:
	// счетчик картинок
	inline static int i = 0;
	inline static bool active = false;
public:
	Logger();
	~Logger();
	static void activate();
	static void deactivate();
	// начало записи в файл
	static void init();
	// начало шага, передается название
	static void init_step(string step_name);
	// добавление записи
	static void log(string text);
	static void log(string text, string val);
	static void log(const FiniteAutomaton& fa1, const FiniteAutomaton& fa2);
	static void log(string text, const FiniteAutomaton& fa1, const FiniteAutomaton& fa2, const FiniteAutomaton& fa3);
	// завершение шага
	static void finish_step();
	// завершение записи в файл
	static void finish();
};