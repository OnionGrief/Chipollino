#include "Objects/BackRefRegex.h"

template <typename T> BackRefRegex* BackRefRegex::cast(T* ptr) {
	auto* r = static_cast<BackRefRegex*>(ptr);
	if (!r) {
		throw std::runtime_error("Failed to cast to BackRefRegex");
	}

	return r;
}

string BackRefRegex::to_txt() const {
	return "";
}