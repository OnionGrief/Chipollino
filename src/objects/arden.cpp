//
// Created by xtoter on 03.10.22.
//
#include "arden.h"
#include <iostream>
#include <map>

struct Expression_arden {};
struct elems {
	bool operations;
	std::vector<elems> masselem;
	char symbol;
	char perehod;
};
std::string NFA_to_Regex(FiniteAutomat in, int FinalState) {
	std::map<char, elems> mp;
	// for (int i = 0; i < in.)
	std::cout << "axaxa";
	return "axa";
}
