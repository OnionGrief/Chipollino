#include "Objects/MemoryFiniteAutomaton.h"

template <typename T>
MemoryFiniteAutomaton* MemoryFiniteAutomaton::castToMFA(unique_ptr<T>&& uptr) {
	auto* mfa = static_cast<MemoryFiniteAutomaton*>(uptr.get());

	if (!mfa) {
		throw std::runtime_error("Failed to cast to MemoryFiniteAutomaton");
	}

	return mfa;
}

string MemoryFiniteAutomaton::to_txt() const {
	return "";
}