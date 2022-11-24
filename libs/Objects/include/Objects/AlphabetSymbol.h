#pragma once
#include <iostream>
using namespace std;

// using alphabet_symbol = string;
struct alphabet_symbol {
	string value = "";

	alphabet_symbol();
	alphabet_symbol(const string& value);
	alphabet_symbol(const char* c);
	alphabet_symbol(char c);

	bool is_epsilon() const;
	operator string() const;

	bool operator==(const alphabet_symbol& other) const;
	bool operator!=(const alphabet_symbol& other) const;
	bool operator<(const alphabet_symbol& other) const;
	alphabet_symbol& operator=(const alphabet_symbol& other);
	alphabet_symbol operator+(const alphabet_symbol& other) const;

	static alphabet_symbol epsilon();

	alphabet_symbol remove_numbers();
	int size() const;
};

ostream& operator<<(ostream& os, const alphabet_symbol& item);
ostream& operator<<(ostream& os, const alphabet_symbol& item);