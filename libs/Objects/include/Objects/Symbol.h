#pragma once
#include <iostream>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace SymbolErrors {
static constexpr const char* EmptyString = "Invalid Symbol format: string cannot be empty";
static constexpr const char* NullString = "Invalid Symbol format: null string";
static constexpr const char* InvalidFormat =
	"Invalid Symbol format: must start with a single letter, 'eps', or '-empty-'";
static constexpr const char* ExpectedNumber = "Invalid Symbol format: number expected after marker";
static constexpr const char* UnexpectedCharacters = "Invalid Symbol format: unexpected characters";
} // namespace SymbolErrors

// Символ, по которому осуществляются переходы в автомате.
// Может быть символом-буквой (и входить ТОЛЬКО в алфавит FA) или ссылкой (&i)
class Symbol {
  private:
	std::vector<int> annote_numbers;
	std::vector<int> linearize_numbers;
	std::string symbol;
	std::optional<int> reference;
	// symbol + разметка
	std::string value;

	void initialize(const std::string& s);
	void parse_markup(const std::string& s, size_t pos);
	void update_value();

  public:
	static const char linearize_marker = '.';
	static const char annote_marker = ',';

	// нужно добавлять в prefixes в Symbol::initialize
	static constexpr const char* Epsilon = "eps";
	static constexpr const char* EmptySet = "-empty-";

	Symbol() = default;
	Symbol(const std::string& s); // NOLINT(runtime/explicit)
	Symbol(const char* c);		  // NOLINT(runtime/explicit)
	Symbol(char c);				  // NOLINT(runtime/explicit)

	Symbol(const Symbol& other) = default;

	static Symbol Ref(int number);

	Symbol& operator=(const std::string& s);
	Symbol& operator=(const char* c);
	Symbol& operator=(char c);
	Symbol& operator=(const Symbol& other) = default;
	// многие функции все еще работают с символами алфавита, как со строками
	// для них добавлено преобразование типов
	operator std::string() const;

	bool operator==(const Symbol& other) const;
	bool operator==(char c) const;
	bool operator!=(const Symbol& other) const;
	bool operator<(const Symbol& other) const;

	bool empty() const;
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

	friend class MemorySymbols;
};

std::ostream& operator<<(std::ostream& os, const Symbol& item);

using Alphabet = std::set<Symbol>;

// специальные символы переходов в Symbolic-NFA
class MemorySymbols {
  public:
	static const char CloseChar = 'C';
	static const char ResetChar = 'R';
	static const char OpenChar = 'O';

	static Symbol Close(int number);
	static Symbol Reset(int number);
	static Symbol Open(int number);

	static std::optional<std::string> is_memory_string(const std::string& s);
	static bool is_memory_symbol(const Symbol& s);
	static bool is_close(const Symbol& s);
	static bool is_reset(const Symbol& s);
	static bool is_open(const Symbol& s);

	static int get_cell_number(const Symbol& s);
};

bool is_special_symbol(const Symbol& s);
