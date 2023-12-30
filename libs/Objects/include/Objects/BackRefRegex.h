#pragma once
#include <string>
#include <unordered_set>
#include <vector>

#include "AlgExpression.h"

class MemoryFiniteAutomaton;
class MFAState;

class BackRefRegex : public AlgExpression {
  private:
	// номер ячейки памяти (используется при Type: ref, memoryWriter)
	int cell_number = 0;
	// номера ячеек, в которых "находится" нода (лежит в поддеревьях memoryWriter с этими номерами)
	std::unordered_set<int> in_cells;

	void copy(const AlgExpression*) override; // NOLINT(build/include_what_you_use)
	// возвращает указатель на new BackRefRegex
	BackRefRegex* make() const override;

	std::string type_to_str() const override;

	BackRefRegex* expr(const std::vector<Lexeme>&, int, int) override;
	BackRefRegex* scan_ref(const std::vector<Lexeme>&, int, int);
	BackRefRegex* scan_square_br(const std::vector<Lexeme>&, int, int);

	// возвращает вектор состояний без индексов и идентификаторов
	// 0-e состояние начальное
	std::vector<MFAState> _to_mfa() const;

  public:
	BackRefRegex() = default;
	explicit BackRefRegex(const std::string&);
	explicit BackRefRegex(Type type, AlgExpression* = nullptr, AlgExpression* = nullptr);

	BackRefRegex* make_copy() const override;
	BackRefRegex(const BackRefRegex&);

	// dynamic_cast к типу BackRefRegex*
	template <typename T> static BackRefRegex* cast(T* ptr, bool not_null_ptr = true);
	template <typename T> static const BackRefRegex* cast(const T* ptr, bool not_null_ptr = true);
	template <typename T>
	static std::vector<BackRefRegex*> cast(std::vector<T*> ptr, bool not_null_ptr = true);

	std::string to_txt(bool eps_is_empty = true) const override;

	MemoryFiniteAutomaton to_mfa(iLogTemplate* log = nullptr) const;
};