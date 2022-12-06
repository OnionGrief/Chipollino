#pragma once
#include "InfInt/InfInt.h"
#include <iostream>
using namespace std;

class Fraction {
	InfInt numerator;
	InfInt denominator;

  public:
	inline static unsigned long long last_number_of_digits = 0;
	Fraction();
	Fraction(InfInt n, InfInt d);
	~Fraction();
	Fraction operator+(const Fraction& f);
	Fraction operator-(const Fraction& f);
	Fraction operator*(const Fraction& f);
	Fraction operator/(const Fraction& f);
	Fraction operator+=(const Fraction& f);
	Fraction operator++();
	Fraction operator++(int);
	bool operator>(const Fraction& f);
	bool operator==(const Fraction& f);
	friend ostream& operator<<(ostream& output, const Fraction& f);

  private:
	void simplify() {
		reduction();
		fix_sign();
	}
	void fix_sign() {
		if (denominator < 0) {
			denominator = -denominator;
			numerator = -numerator;
		}
	}
	void reduction() {
		InfInt common = gcd(numerator, denominator);
		numerator /= common;
		denominator /= common;
	}
	InfInt gcd(InfInt x, InfInt y) {
		return y == 0 ? x : gcd(y, x % y);
	}
};