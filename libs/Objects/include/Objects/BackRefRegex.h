#pragma once
#include "AlgExpression.h"
using namespace std;

class BackRefRegex : public AlgExpression {
  private:
	// возвращает указатель на new BackRefRegex
	AlgExpression* make() const override;

  public:
	// dynamic_cast к типу BackRefRegex*
	template <typename T> static BackRefRegex* cast(T* ptr);
};