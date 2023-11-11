#pragma once
#include "AlgExpression.h"
using namespace std;

class BackRefRegex : public AlgExpression {
  private:
	int cell_number = 0;

	void copy(const AlgExpression*) override;
	// возвращает указатель на new BackRefRegex
	BackRefRegex* make() const override;

	string type_to_str() const override;

	BackRefRegex* expr(const vector<Lexeme>&, int, int) override;
	BackRefRegex* scan_ref(const vector<Lexeme>&, int, int);
	BackRefRegex* scan_square_br(const vector<Lexeme>&, int, int);

  public:
	BackRefRegex() = default;
	BackRefRegex(const string&);

	BackRefRegex* make_copy() const override;
	BackRefRegex(const BackRefRegex&);

	// dynamic_cast к типу BackRefRegex*
	template <typename T> static BackRefRegex* cast(T* ptr, bool not_null_ptr = true);
	template <typename T> static const BackRefRegex* cast(const T* ptr, bool not_null_ptr = true);

	string to_txt() const override;
};