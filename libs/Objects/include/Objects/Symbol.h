#pragma once
#include <iostream>
#include <string>
#include <vector>

// Символ, по которому осуществляются переходы в автомате.
// Может быть символом-буквой (и входить в алфавит) или ссылкой (&i)
class Symbol {
  private:
	std::vector<int> annote_numbers;
	std::vector<int> linearize_numbers;
	std::string symbol;
	// symbol + разметка
	std::string value;
	void update_value();

  public:
	static const char linearize_marker = '.';
	static const char annote_marker = ',';
	inline static const std::string Epsilon = "_eps_";
	inline static const std::string EpmptySet = "_empty_";

	Symbol() = default;
	Symbol(const std::string& s); // NOLINT(runtime/explicit)
	Symbol(const char* c);		  // NOLINT(runtime/explicit)
	Symbol(char c);				  // NOLINT(runtime/explicit)

	Symbol(const Symbol& other) = default;

	Symbol& operator=(const std::string& s);
	Symbol& operator=(const char* c);
	Symbol& operator=(char c);
	Symbol& operator=(const Symbol& other);
	// многие функции все еще работают с символами алфавита как со строками
	// для них добавлено преобразование типов
	operator std::string() const;

	bool operator==(const Symbol& other) const;
	bool operator!=(const Symbol& other) const;
	bool operator<(const Symbol& other) const;
	// возвращает символ эпсилон
	static Symbol epsilon();
	bool is_epsilon() const;
	// преобразовывает вектор символов в одну строку
	static std::string vector_to_str(const std::vector<Symbol>&);

	void annote(int num);
	void linearize(int num);
	void deannote();
	void delinearize();
	bool is_annotated() const;
	bool is_linearized() const;

	int last_linearization_number();

	struct Hasher {
		std::size_t operator()(const Symbol& s) const;
	};
};

std::ostream& operator<<(std::ostream& os, const Symbol& item);
