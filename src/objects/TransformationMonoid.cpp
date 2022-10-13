#include "TransformationMonoid.h"

#include <iostream>
using namespace std;

//получаем все перестановки алфавита длины len
vector<string> get_comb_alphabet(int len, vector<char> alphabet) {
    vector<string> newcomb;
    if (len == 0) {
        return newcomb;
    }
    for (int i = 0; i < alphabet.size(); i++) {
        string temp;
        temp += alphabet[i];
        newcomb.push_back(temp);
    }
    if (len == 1) {
        return newcomb;
    }
    vector<string> comb;
    vector<string> oldcomb = get_comb_alphabet(len - 1, alphabet);
    for (int i = 0; i < newcomb.size(); i++) {
        for (int j = 0; j < oldcomb.size(); j++) {
            comb.push_back(newcomb[i] + oldcomb[j]);
        }
    }
    return comb;
}
//Проверяем встречался ли терм раньше
string wasTerm(vector<Term> allTerms, vector<Perehod> curperehod) {
    bool cond = true;
    for (int i = 0; i < allTerms.size(); i++) {
        cond = true;
        if (allTerms[i].perehods.size() != curperehod.size()) {
            continue;
        }
        for (int j = 0; j < allTerms[i].perehods.size(); j++) {
            if ((allTerms[i].perehods[j].first == curperehod[j].first) &&
                (allTerms[i].perehods[j].second == curperehod[j].second)) {
                // continue;
            } else {
                cond = false;
            }
        }
        if (cond) {
            return allTerms[i].name;
        }
    }
    return "";
}
TransformationMonoid::TransformationMonoid(){};
TransformationMonoid::TransformationMonoid(FiniteAutomat *in) {
    Automat = in;
    for (int i = 1; i <= 3; i++) {
        vector<string> various = get_comb_alphabet(i, Automat->get_alphabet());
        for (int j = 0; j < various.size(); j++)  //Для всех комбинаций
        {
            Term current;
            current.name = various[j];
            for (int t = 0; t < Automat->get_states_size();
                 t++)  //Для всех состояний
            {
                // cout << current.name << " ";
                int endsost = t;

                Perehod g;
                g.first = endsost;
                bool cond = true;
                for (int k = 0; k < current.name.size();
                     k++)  //Для каждого символа перехода
                {
                    State a = Automat->get_state(endsost);
                    if (a.transitions.count(current.name[k])) {
                        vector<int> temp = a.transitions.at(current.name[k]);
                        endsost = temp[0];
                    } else {
                        cond = false;
                    }
                }
                g.second = endsost;
                if (cond) {
                    current.perehods.push_back(g);
                }
            }

            string eqv = wasTerm(Terms, current.perehods);
            if (eqv == "")  //Если не встретился в Эквивалентных классах
            {
                bool cond = true;
                for (int i = 0; i < current.perehods.size(); i++) {
                    if (!Automat->get_state(current.perehods[i].second)
                             .is_terminal) {
                        cond = false;
                    }
                }
                current.isFinal = cond;
                // cout << current.name << " " << current.isFinal << "\n";
                Terms.push_back(current);
            } else {
                Rules[eqv].push_back(current.name);
                // cout << current.name << "->" << eqv << "\n";
            }
        }
    }
}
string TransformationMonoid::to_txt() { cout << Automat->to_txt(); }