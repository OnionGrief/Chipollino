#pragma once
#include <iostream>
#include <optional>
#include <set>
#include <string>
#include <vector>

// Символ, по которому осуществляются переходы в автомате.
// Может быть символом-буквой (и входить в алфавит) или ссылкой (&i)
class Symbol {
  private:
	std::vector<int> annote_numbers;
	std::vector<int> linearize_numbers;
	std::string symbol;
	std::optional<int> reference;
	// symbol + разметка
	std::string value;

	void update_value();

  public:
	static const char linearize_marker = '.';
	static const char annote_marker = ',';
	static const Symbol Epsilon;
	static const Symbol EmptySet;

	Symbol() = default;
	explicit Symbol(const std::string& s);
	explicit Symbol(const char* cstr);
	explicit Symbol(char c);

	Symbol(const Symbol& other) = default;

	static Symbol Ref(int number);

	Symbol& operator=(const std::string& s);
	Symbol& operator=(const char* cstr);
	Symbol& operator=(char c);
	Symbol& operator=(const Symbol& other) = default;
	// многие функции все еще работают с символами алфавита, как со строками
	// для них добавлено преобразование типов
	explicit operator std::string() const;

	size_t size() const;

	bool operator==(const Symbol& other) const;
	bool operator==(const char* cstr) const;
	bool operator==(char c) const;
	bool operator!=(const Symbol& other) const;
	bool operator<(const Symbol& other) const;

	bool is_epsilon() const;
	// преобразовывает вектор символов в одну строку
	static std::string vector_to_str(const std::vector<Symbol>&);

	void annote(int num);
	void linearize(int num);
	void deannote();
	void delinearize();
	bool is_annotated() const;
	bool is_linearized() const;
	bool is_ref() const;
	int get_ref() const;

	int last_linearization_number() const;

	struct Hasher {
		std::size_t operator()(const Symbol&) const;
	};
};

std::ostream& operator<<(std::ostream& os, const Symbol& item);

using Alphabet = std::set<Symbol>;
