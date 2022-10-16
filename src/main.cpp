#include <iostream>
#include <FiniteAutomat.h>
#include <arden.h>
using namespace std;

int main() {
    std::vector<char> alphabet = {'a', 'b'};
    std::vector<bool> is_terminal ;
    std::vector<int> t1 = {1}; //a
    std::vector<int> t2 = {2}; //a
    std::vector<int> t3 = {3}; //a
    std::vector<int> t4 = {0}; //a
    std::vector<int> t5 = {0}; //b
    std::vector<int> t6 = {3}; //b
    std::vector<int> t7 = {3}; //b
    std::vector<int> t8 = {3}; //b
    std::vector<std::vector<int>> m1 = {t1,t2};
    std::vector<std::vector<int>> m2 = {t3,t4};
    std::vector<std::vector<int>> m3 = {t5,t6};
    std::vector<std::vector<int>> m4 = {t7,t8};
    std::vector<std::vector<std::vector<int>>> transition_matrix = {m1,m2,m3,m4};
    FiniteAutomat *a = new FiniteAutomat(true,0,
    alphabet,is_terminal,transition_matrix);
    NFA_to_Regex(*a,0);
	//cout << a->to_txt();
	
}