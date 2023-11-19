#pragma once
#include <iostream>
#include <string>
#include <vector>

// Символ, по которому осуществляются переходы в автомате.
// Может быть символом-буквой (и входить в алфавит) или ссылкой (&i)
struct alphabet_symbol {
  private:
	std::vector<std::string> annote_numbers;
	std::vector<std::string> linearize_numbers;
	std::string symbol;
	// symbol + разметка
	std::string value;
	void update_value();

  public:
	static const char linearize_marker = '.';
	static const char annote_marker = ',';

	alphabet_symbol();
	alphabet_symbol(const std::string& s); // NOLINT(runtime/explicit)
	alphabet_symbol(const char* c);		   // NOLINT(runtime/explicit)
	alphabet_symbol(char c);			   // NOLINT(runtime/explicit)

	alphabet_symbol(const alphabet_symbol& other);

	const alphabet_symbol& operator=(const std::string& s);
	const alphabet_symbol& operator=(const char* c);
	const alphabet_symbol& operator=(char c);
	const alphabet_symbol& operator=(const alphabet_symbol& other);
	// многие функции все еще работают с символами алфавита как со строками
	// для них добавлено преобразование типов
	operator std::string() const;

	bool operator==(const alphabet_symbol& other) const;
	bool operator!=(const alphabet_symbol& other) const;
	bool operator<(const alphabet_symbol& other) const;
	// возвращает символ эпсилон
	static alphabet_symbol epsilon();
	bool is_epsilon() const;
	// преобразовывает вектор символов в одну строку
	static std::string vector_to_str(const std::vector<alphabet_symbol>&);

	void annote(int num);
	void linearize(int num);
	void deannote();
	void delinearize();
	bool is_annotated() const;
	bool is_linearized() const;

	friend struct AlphabetSymbolHasher;
};

std::ostream& operator<<(std::ostream& os, const alphabet_symbol& item);

struct AlphabetSymbolHasher {
	std::size_t operator()(const alphabet_symbol& symbol) const;
};