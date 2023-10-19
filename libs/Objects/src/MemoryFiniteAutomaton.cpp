#include "Objects/MemoryFiniteAutomaton.h"

template <typename T>
MemoryFiniteAutomaton* MemoryFiniteAutomaton::castToMFA(
	std::unique_ptr<T>&& uniquePtr) {
	auto* fa = static_cast<MemoryFiniteAutomaton*>(uniquePtr.get());

	if (!fa) {
		throw std::runtime_error("Failed to cast to MemoryFiniteAutomaton");
	}

	return fa;
}

string MemoryFiniteAutomaton::to_txt() const {
	return "";
}