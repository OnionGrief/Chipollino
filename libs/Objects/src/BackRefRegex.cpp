#include "Objects/BackRefRegex.h"

AlgExpression* BackRefRegex::make() const {
	return new BackRefRegex;
}

template <typename T> BackRefRegex* BackRefRegex::cast(T* ptr) {
	auto* r = dynamic_cast<BackRefRegex*>(ptr);
	if (!r) {
		throw std::runtime_error("Failed to cast to BackRefRegex");
	}

	return r;
}