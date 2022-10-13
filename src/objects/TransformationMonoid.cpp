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
TransformationMonoid::TransformationMonoid(FiniteAutomat* in) {
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
string TransformationMonoid::to_txt() { return Automat->to_txt(); }

vector<Term> TransformationMonoid::getEqualenseClasses() { return Terms; }

map<string, vector<string>> TransformationMonoid::getRewritingRules() {
    return Rules;
}

string TransformationMonoid::getEqualenseClassesTxt() {
    stringstream ss;
    for (int i = 0; i < Terms.size(); i++) {
        ss << "Term " << Terms[i].name << " in language " << Terms[i].isFinal
           << "\n";
        for (int j = 0; j < Terms[i].perehods.size(); j++) {
            ss << Terms[i].perehods[j].first << " -> "
               << Terms[i].perehods[j].second << "\n";
        }
    }
    return ss.str();
}
string TransformationMonoid::getRewritingRulesTxt() {
    stringstream ss;
    for (auto& item : Rules) {
        for (int i = 0; i < item.second.size(); i++) {
            ss << item.first << " -> " << item.second[i] << "\n";
        }
    }
    return ss.str();
}

vector<Term> TransformationMonoid::getEqualenseClassesVW(Term w) {
    vector<Term> out;
    for (int i = 0; i < Terms.size(); i++) {
        vector<Perehod> perehods;

        for (int j = 0; j < Terms[i].perehods.size(); j++) {
            for (int k = 0; k < w.perehods.size(); k++) {
                if (Terms[i].perehods[j].second == w.perehods[k].first) {
                    Perehod temp;
                    temp.first = Terms[i].perehods[j].first;
                    temp.second = w.perehods[k].second;
                    perehods.push_back(temp);
                }
            }
        }
        if (perehods.size() > 0) {
            bool cond = true;
            for (int j = 0; j < perehods.size(); j++) {
                if (!Automat->get_state(perehods[j].second).is_terminal) {
                    cond = false;
                }
            }
            if (cond) {
                out.push_back(Terms[i]);
                cout << Terms[i].name << " ";
            }
        }
    }
    return out;
}
vector<Term> TransformationMonoid::getEqualenseClassesWV(Term w) {
    vector<Term> out;
    for (int i = 0; i < Terms.size(); i++) {
        vector<Perehod> perehods;

        for (int j = 0; j < Terms[i].perehods.size(); j++) {
            for (int k = 0; k < w.perehods.size(); k++) {
                if (Terms[i].perehods[j].first == w.perehods[k].second) {
                    Perehod temp;
                    temp.first = w.perehods[k].first;
                    temp.second = Terms[i].perehods[j].second;
                    perehods.push_back(temp);
                }
            }
        }
        if (perehods.size() > 0) {
            bool cond = true;
            for (int j = 0; j < perehods.size(); j++) {
                if (!Automat->get_state(perehods[j].second).is_terminal) {
                    cond = false;
                }
            }
            if (cond) {
                out.push_back(Terms[i]);
                cout << Terms[i].name << " ";
            }
        }
    }
    return out;
}
vector<TermDouble> TransformationMonoid::getEqualenseClassesVWV(Term w) {
    vector<TermDouble> out;
    for (int i1 = 0; i1 < Terms.size(); i1++) {
        for (int i2 = 0; i2 < Terms.size(); i2++) {
            vector<Perehod> perehods;

            for (int j1 = 0; j1 < Terms[i1].perehods.size(); j1++) {
                for (int j2 = 0; j2 < Terms[i1].perehods.size(); j2++) {
                    for (int k = 0; k < w.perehods.size(); k++) {
                        if ((Terms[i1].perehods[j1].second ==
                             w.perehods[k].first) &&
                            (w.perehods[k].second ==
                             Terms[i2].perehods[j2].first)) {
                            Perehod temp;
                            temp.first = Terms[i1].perehods[j1].first;
                            temp.second = Terms[i2].perehods[j2].second;
                            perehods.push_back(temp);
                        }
                    }
                }
            }
            if (perehods.size() > 0) {
                bool cond = true;
                for (int j = 0; j < perehods.size(); j++) {
                    if (!Automat->get_state(perehods[j].second).is_terminal) {
                        cond = false;
                    }
                }
                if (cond) {
                    TermDouble temp1;
                    temp1.first = Terms[i1];
                    temp1.second = Terms[i2];
                    out.push_back(temp1);
                    cout << temp1.first.name << " " << temp1.second.name
                         << "\n";
                }
            }
        }
    }
    return out;
}