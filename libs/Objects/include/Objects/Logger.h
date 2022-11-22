#pragma once
#include "AutomatonToImage/AutomatonToImage.h"
#include "Language.h"
#include "Objects/FiniteAutomaton.h"
#include <string>
#include <vector>
using namespace std;

class Logger {
  private:
	// счетчик картинок
	inline static int image_number = 0;
	// флаг включения логгирования
	inline static bool active = false;
	// количество начатых шагов
	inline static int step_counter = 0;

  public:
	Logger();
	~Logger();
	// установка флага логгирования
	static void activate();
	// сброс флага логгирования
	static void deactivate();
	// вкл и выкл логирования вложенных функций
	static void activate_step_counter();
	static void deactivate_step_counter();
	// начало записи в файл
	static void init();
	// начало шага, передается название
	static void init_step(string step_name);
	// добавление записи
	// текстовая запись
	static void log(string text);
	// строковые данные(val) с заголовком(text)
	static void log(string text, string val);
	// для отображения одного автомата, а1 - название автомата
	static void log(string a1, const FiniteAutomaton& fa1);
	// для отображения двух автоматов; а1 и а2 - названия автоматов fa1 и fa2
	// соответственно
	static void log(string a1, string a2, const FiniteAutomaton& fa1,
					const FiniteAutomaton& fa2);
	// для отображения трех автоматов; а1, а2, а3 - названия автоматов fa1, fa2,
	// fa3 соответственно
	static void log(string a1, string a2, string a3, const FiniteAutomaton& fa1,
					const FiniteAutomaton& fa2, const FiniteAutomaton& fa3);
	// для отображения таблицы тестера word
	static void log(int step, vector<int> lengths, vector<double> times,
					vector<bool> belongs);
	// таблицы в общем виде
	static void log_table(vector<string> rows, vector<string> columns,
						  vector<string> data);
	// завершение шага
	static void finish_step();
	// завершение записи в файл
	static void finish();
	static string math_mode(string str);
};