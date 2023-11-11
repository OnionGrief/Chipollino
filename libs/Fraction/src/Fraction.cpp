#include "Fraction/Fraction.h"

Fraction::Fraction() {
	numerator = 0;
	denominator = 1;
}

Fraction::Fraction(InfInt n, InfInt d) {
	if (d == 0) throw std::invalid_argument("d");
	numerator = n;
	denominator = d;
	simplify();
}

Fraction::~Fraction() {}

Fraction Fraction::operator+(const Fraction& f) {
	InfInt n = numerator * f.denominator + f.numerator * denominator;
	InfInt d = denominator * f.denominator;

	Fraction ff(n, d);
	return ff;
}

Fraction Fraction::operator-(const Fraction& f) {
	InfInt n = numerator * f.denominator - f.numerator * denominator;
	InfInt d = denominator * f.denominator;
	last_number_of_digits = n.numberOfDigits() + d.numberOfDigits();

	Fraction ff(n, d);
	return ff;
}

Fraction Fraction::operator*(const Fraction& f) {
	InfInt n = numerator * f.numerator;
	InfInt d = denominator * f.denominator;

	Fraction ff(n, d);
	return ff;
}

Fraction Fraction::operator/(const Fraction& f) {
	InfInt n = numerator * f.denominator;
	InfInt d = denominator * f.numerator;

	Fraction ff(n, d);
	return ff;
}

Fraction Fraction::operator+=(const Fraction& f) {
	numerator = numerator * f.denominator + f.numerator * denominator;
	denominator = denominator * f.denominator;
	simplify();

	return *this;
}

Fraction Fraction::operator++() {
	Fraction f(1, 1);
	numerator = numerator * f.denominator + f.numerator * denominator;
	denominator = denominator * f.denominator;
	simplify();

	return *this;
}

Fraction Fraction::operator++(int) {
	Fraction ff(numerator, denominator);
	Fraction f(1, 1);
	numerator = numerator * f.denominator + f.numerator * denominator;
	denominator = denominator * f.denominator;
	simplify();

	return ff;
}

bool Fraction::operator>(const Fraction& f) {
	InfInt n = numerator * f.denominator - f.numerator * denominator;
	return n > 0;
}

bool Fraction::operator==(const Fraction& f) {
	InfInt n = numerator * f.denominator - f.numerator * denominator;
	return n == 0;
}

bool Fraction::operator>=(const Fraction& f) {
	InfInt n = numerator * f.denominator - f.numerator * denominator;
	return n >= 0;
}

std::ostream& operator<<(std::ostream& output, const Fraction& f) {
	output << "(" << f.numerator << "/" << f.denominator << ")";
	return output;
}