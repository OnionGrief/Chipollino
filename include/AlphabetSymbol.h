#pragma once
#include <iostream>
using namespace std;

using alphabet_symbol = string;
alphabet_symbol epsilon();
bool is_epsilon(alphabet_symbol);
string to_string(alphabet_symbol);
alphabet_symbol char_to_alphabet_symbol(char);