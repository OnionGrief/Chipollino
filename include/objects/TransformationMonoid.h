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
	vector<Term> get_Equalence_Classes(); // получаем все термы
	vector<Term> get_Equalence_Classes_VW(
		Term w); //получаем термы, что vw - в языке
	vector<Term> get_Equalence_Classes_WV(
		Term w); //получаем термы, что wv - в языке
	vector<TermDouble> get_Equalence_Classes_VWV(
		Term w); //получаем термы, что vwv - в языке
	map<vector<string>, vector<vector<string>>>
	get_Rewriting_Rules(); //получаем правила переписывания
	string get_Equalence_Classes_Txt(); //вывод эквивалентных классов
	string get_Rewriting_Rules_Txt(); //вывод правил переписывания
	string to_txt() const override;
	int is_Synchronized(Term w); //Вернет	-1	если	не	синхронизирован	или
	//номер состояния	с	которым синхронизирован
	int class_Card(); //Вернет число классов эквивалентности
	Term Class_Length(); //Вернет самое длинное слово в классе
	bool is_minimality(); //Вычисление Минимальности (1 если минимальный)
	int size_MyhillNerode(); //Вычисление размера по М-Н
	string to_Txt_MyhillNerode(); //вывод таблицы М-Н
	static int class_card_typecheker(
		const FiniteAutomaton& in); //Вернет число классов эквивалентности
	static int class_legth_typecheker(
		const FiniteAutomaton&
			in); //Вернет длину самого длинного слова в классе
	static int MyhillNerode_typecheker(
		const FiniteAutomaton& in); //размер по М-Н
									// и тд
  private:
	FiniteAutomaton automat; //Автомат
	vector<Term> terms;		 //Эквивалентные классы
	map<vector<string>, vector<vector<string>>> rules; //Правила переписывания
	vector<vector<bool>> equivalence_class_table; //таблица М-Н
};
