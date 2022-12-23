#pragma once
#include <iostream>
#include <vector>
using namespace std;

// using alphabet_symbol = string;
struct alphabet_symbol {
	string value = "";

	alphabet_symbol();
	alphabet_symbol(const string& s);
	alphabet_symbol(const char* c);
	alphabet_symbol(char c);

	alphabet_symbol(const alphabet_symbol& other);

	const alphabet_symbol& operator=(const string& s);
	const alphabet_symbol& operator=(const char* c);
	const alphabet_symbol& operator=(char c);
	const alphabet_symbol& operator=(const alphabet_symbol& other);
	// многие функции все еще работают с символами алфавита как со строками
	// для них добавлено преобразование типов
	operator string() const;

	bool operator==(const alphabet_symbol& other) const;
	bool operator!=(const alphabet_symbol& other) const;
	bool operator<(const alphabet_symbol& other) const;
	alphabet_symbol operator+(const alphabet_symbol& other) const;
	// возвращает символ эпсилон
	static alphabet_symbol epsilon();
	bool is_epsilon() const;
	// возвращает символ без разметки
	alphabet_symbol remove_numbers();
	// преобразовывает вектор символов в одну строку
	static string vector_to_str(const vector<alphabet_symbol>&);

	int size() const;
};

ostream& operator<<(ostream& os, const alphabet_symbol& item);