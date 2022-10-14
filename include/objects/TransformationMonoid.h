#pragma once
#include "BaseObject.h"
#include "FiniteAutomat.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
using namespace std;

struct Transition {
	int first;
	int second;
};
struct Term {
	bool isFinal = false;
	string name;
	vector<Transition> Transitions;
};
struct TermDouble {
	Term first;
	Term second;
};
class TransformationMonoid : public BaseObject {
  private:
	FiniteAutomat *automat;			   //Ссылка на автомат
	vector<Term> terms;				   //Эквивалентные классы
	map<string, vector<string>> rules; //Правила переписывания
  public:
	TransformationMonoid();
	TransformationMonoid(FiniteAutomat *in);
	vector<Term> get_Equalence_Classes();
	vector<Term> get_Equalence_Classes_VW(Term w);
	vector<Term> get_Equalence_Classes_WV(Term w);
	vector<TermDouble> get_Equalence_Classes_VWV(Term w);
	map<string, vector<string>> get_Rewriting_Rules();
	string get_Equalence_Classes_Txt();
	string get_Rewriting_Rules_Txt();
	string to_txt() override;
	int is_Synchronized(Term w);
	int classCard();
	// и тд
};