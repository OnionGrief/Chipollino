#pragma once
#include "BaseObject.h"
#include "Logger.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <math.h>
#include <sstream>
#include <vector>
using namespace std;

class Language;
class FiniteAutomaton;

class TransformationMonoid : public BaseObject {
  public:
	struct Transition { //переход (индекс состояния - индекс состояния)
		int first;
		int second;
	};
	struct Term {
		bool isFinal = false;
		vector<alphabet_symbol> name;
		vector<Transition> transitions;
	};
	struct TermDouble { //двойной терм
		Term first;
		Term second;
	};
	TransformationMonoid();
	TransformationMonoid(
		const FiniteAutomaton& in); //Автомат и макс длина перехода
	vector<Term> get_equalence_classes(); // получаем все термы
	vector<Term> get_equalence_classes_vw(
		const Term& w); //получаем термы, что vw - в языке
	vector<Term> get_equalence_classes_wv(
		const Term& w); //получаем термы, что wv - в языке
	vector<TermDouble> get_equalence_classes_vwv(
		const Term& w); //получаем термы, что vwv - в языке
	map<vector<string>, vector<vector<string>>>
	get_rewriting_rules(); //получаем правила переписывания
	string get_equalence_classes_txt(); //вывод эквивалентных классов
	string get_rewriting_rules_txt(); //вывод правил переписывания
	string to_txt() const override;
	int is_synchronized(
		const Term& w); //Вернет	-1	если	не	синхронизирован	или
	//номер состояния	с	которым синхронизирован
	int class_card(); //Вернет число классов эквивалентности
	int class_length(); //Вернет самое длинное слово в классе
	bool is_minimal(); //Вычисление Минимальности по М-Н(1 если минимальный)
	int classes_number_MyhillNerode(); //Вычисление размера по М-Н
	string to_txt_MyhillNerode(); //вывод таблицы М-Н
	const vector<vector<bool>>& get_equivalence_classes_table(); // возвращает
																 // таблицу М-Н

  private:
	FiniteAutomaton automat; //Автомат
	vector<Term> terms;		 //Эквивалентные классы
	map<vector<string>, vector<vector<string>>> rules; //Правила переписывания
	vector<vector<bool>> equivalence_classes_table; //таблица М-Н
};
