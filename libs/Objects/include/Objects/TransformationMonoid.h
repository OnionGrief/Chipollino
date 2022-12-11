#pragma once
#include "BaseObject.h"
#include "FiniteAutomaton.h"
#include "Language.h"
#include "Logger.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <math.h>
#include <sstream>
#include <vector>
using namespace std;

class TransformationMonoid : public BaseObject {

  public:
	struct Transition { // переход (индекс состояния - индекс состояния)
		int first;
		int second;
	};
	struct Term {
		bool isFinal = false;
		vector<alphabet_symbol> name;
		vector<Transition> transitions;
	};

	struct TermDouble { // двойной терм
		Term first;
		Term second;
	};
	TransformationMonoid();
	TransformationMonoid(
		const FiniteAutomaton& in); // Автомат и макс длина перехода
	vector<Term> get_equalence_classes(); // получаем все термы
	vector<Term> get_equalence_classes_vw(
		const Term& w); // получаем термы, что vw - в языке
	vector<Term> get_equalence_classes_wv(
		const Term& w); // получаем термы, что wv - в языке
	vector<TermDouble> get_equalence_classes_vwv(
		const Term& w); // получаем термы, что vwv - в языке
	map<vector<alphabet_symbol>, vector<vector<alphabet_symbol>>>
	get_rewriting_rules(); // получаем правила переписывания
	string get_equalence_classes_txt(); // вывод эквивалентных классов
	string get_rewriting_rules_txt(); // вывод правил переписывания
	string to_txt() const;
	int is_synchronized(
		const Term& w); // Вернет	-1	если	не	синхронизирован	или
	// номер состояния	с	которым синхронизирован
	int class_card(); // Вернет число классов эквивалентности
	int class_length(); // Вернет самое длинное слово в классе
	bool is_minimal(); // Вычисление Минимальности по М-Н(1 если минимальный)
	int get_classes_number_MyhillNerode(); // Вычисление размера по М-Н
	string to_txt_MyhillNerode(); // вывод таблицы М-Н
	vector<vector<bool>> get_equivalence_classes_table(
		vector<string>& table_rows,
		vector<string>& table_columns); // возвращает
										// таблицу М-Н

  private:
	FiniteAutomaton automat; // Автомат
	vector<Term> terms;		 // Эквивалентные классы
	map<vector<alphabet_symbol>, vector<vector<alphabet_symbol>>>
		rules; // Правила переписывания
	vector<vector<bool>> equivalence_classes_table_bool; // Taблица М-Н
	vector<string> equivalence_classes_table_left; // Левая часть таблицы
	vector<string> equivalence_classes_table_top; // шапка таблицы

	//   | t o p
	// l |--------
	// e | 0 1 0 0
	// f | 0 bool0
	// t | 1 0 1 1
};