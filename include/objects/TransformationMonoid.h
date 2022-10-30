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

struct Transition {
	int first;
	int second;
};
struct Term {
	bool isFinal = false;
	vector<alphabet_symbol> name;
	vector<Transition> Transitions;
};
struct TermDouble {
	Term first;
	Term second;
};
class TransformationMonoid : public BaseObject {
  private:
	FiniteAutomaton automat; //Автомат
	vector<Term> terms;		 //Эквивалентные классы
	map<vector<string>, vector<vector<string>>> rules; //Правила переписывания
	vector<vector<bool>> equivalence_class_table;

  public:
	TransformationMonoid();
	TransformationMonoid(FiniteAutomaton* in,
						 int transferlen); //Автомат и макс длина перехода
	vector<Term> get_Equalence_Classes();
	vector<Term> get_Equalence_Classes_VW(Term w);
	vector<Term> get_Equalence_Classes_WV(Term w);
	vector<TermDouble> get_Equalence_Classes_VWV(Term w);
	map<vector<string>, vector<vector<string>>> get_Rewriting_Rules();
	string get_Equalence_Classes_Txt(); //вывод эквивалентных классов
	string get_Rewriting_Rules_Txt(); //вывод правил переписывания
	string to_txt() const override;
	int is_Synchronized(Term w);
	int class_Card();
	Term Class_Length();
	bool is_minimality();
	int size_MyhillNerode();
	string to_Txt_MyhillNerode();
	// и тд
};
int class_card_typecheker(FiniteAutomaton* in);
int class_legth_typecheker(FiniteAutomaton* in);
int MyhillNerode_typecheker(FiniteAutomaton* in);