#pragma once
#include <map>
#include <sstream>
#include <vector>

#include "BaseObject.h"
#include "FiniteAutomat.h"
using namespace std;
struct Perehod {
    int first;
    int second;
};
struct Term {
    bool isFinal = false;
    string name;
    vector<Perehod> perehods;
};
class TransformationMonoid : public BaseObject {
   private:
    FiniteAutomat *Automat;  //Ссылка на автомат
    vector<Term> Terms;      //Эквивалентные классы
    map<string, vector<string>> Rules;  //Правила переписывания
   public:
    TransformationMonoid();
    TransformationMonoid(FiniteAutomat *in);
    vector<Term> getEqualenseClasses();

    vector<Term> getEqualenseClassesVW(Term w);
    vector<Term> getEqualenseClassesWV(Term w);
    vector<Term> getEqualenseClassesVWV(Term w);
    map<string, vector<string>> getRewritingRules();
    string getEqualenseClassesTxt();
    string getRewritingRulesTxt();
    string to_txt() override;
    // и тд
};