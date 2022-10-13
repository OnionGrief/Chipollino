#pragma once
#include "BaseObject.h"
#include "FiniteAutomat.h"
#include <vector>
#include <map>
using namespace std;
struct Perehod
{
    int first;
    int second;
};
struct Term
{
    bool isFinal = false;
    string name;
    vector<Perehod> perehods;
};
class TransformationMonoid : public BaseObject
{
private:
public:
    FiniteAutomat *Automat;            //Ссылка на автомат
    vector<Term> Terms;                //Эквивалентные классы
    map<string, vector<string>> Rules; //Правила переписывания
    TransformationMonoid();
    TransformationMonoid(FiniteAutomat *in);
    string to_txt() override;
    // и тд
};