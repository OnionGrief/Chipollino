#pragma once
#include "AlgExpression.h"
using namespace std;

class BackRefRegex : public AlgExpression {
  public:
	template <typename T> static BackRefRegex* castToBRegex(T* ptr);

	string to_txt() const override;
};