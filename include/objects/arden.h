//
// Created by xtoter on 03.10.22.
//
#pragma once
#include "Regex.h"
#include <FiniteAutomaton.h>
#include <algorithm>
#include <iostream>
#include <map>
#include <string>

struct expression_arden {
	int condition;
	Regex* temp_regex;
};
Regex nfa_to_regex(FiniteAutomaton in);