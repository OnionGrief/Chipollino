#pragma once
#include <iostream>
#include <string>
#include <vector>

using std::cout;
using std::string;
using std::vector;

// using alphabet_symbol = string;
struct alphabet_symbol {
  private:
	vector<string> annote_numbers;
	vector<string> linearize_numbers;
	string symbol;
	string value;
	void update_value();

  public:
	static const char linearize_marker = '.';
	static const char annote_marker = ',';
	inline static const string Epsilon = "_eps_";
	inline static const string EpmptySet = "_empty_";

	alphabet_symbol();
	alphabet_symbol(const string& s); // NOLINT(runtime/explicit)
	alphabet_symbol(const char* c);	  // NOLINT(runtime/explicit)
	alphabet_symbol(char c);		  // NOLINT(runtime/explicit)

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
	// возвращает символ эпсилон
	static alphabet_symbol epsilon();
	bool is_epsilon() const;
	// преобразовывает вектор символов в одну строку
	static string vector_to_str(const vector<alphabet_symbol>&);

	void annote(int num);
	void linearize(int num);
	void deannote();
	void delinearize();
	bool is_annotated() const;
	bool is_linearized() const;
};

std::ostream& operator<<(std::ostream& os, const alphabet_symbol& item);