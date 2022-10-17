//
// Created by xtoter on 03.10.22.
//
#include "Regex.h"
#include <FiniteAutomat.h>
#include <algorithm>
#include <iostream>
#include <map>
#include <string>

struct expression_arden {
	int condition;
	Regex temp_regex;
};
Regex nfa_to_regex(FiniteAutomat in);

#ifndef CHIPOLLINO_ARDEN_H
#define CHIPOLLINO_ARDEN_H
#endif // CHIPOLLINO_ARDEN_H