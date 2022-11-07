#pragma once
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class LongDouble {
	const size_t divDigits = 1000;
	const size_t sqrtDigits = 100;

	int sign;
	std::vector<int> digits;
	long exponent;

	void initFromString(const std::string& s);
	void removeZeroes();
	void normalize();

  public:
	LongDouble();
	LongDouble(const LongDouble& x);
	LongDouble(long double value);
	LongDouble(const std::string& s);

	LongDouble& operator=(const LongDouble& x);

	bool operator>(const LongDouble& x) const;
	bool operator<(const LongDouble& x) const;
	bool operator>=(const LongDouble& x) const;
	bool operator<=(const LongDouble& x) const;
	bool operator==(const LongDouble& x) const;
	bool operator!=(const LongDouble& x) const;

	LongDouble operator-() const;

	LongDouble operator+(const LongDouble& x) const;
	LongDouble operator-(const LongDouble& x) const;
	LongDouble operator*(const LongDouble& x) const;
	LongDouble operator/(const LongDouble& x) const;

	LongDouble& operator+=(const LongDouble& x);
	LongDouble& operator-=(const LongDouble& x);
	LongDouble& operator*=(const LongDouble& x);
	LongDouble& operator/=(const LongDouble& x);

	LongDouble operator++(int);
	LongDouble operator--(int);

	LongDouble& operator++();
	LongDouble& operator--();

	LongDouble inverse() const;
	bool isZero() const;

	friend std::ostream& operator<<(std::ostream& os, const LongDouble& value);
};