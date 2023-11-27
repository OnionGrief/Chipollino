#include <string>
#include <vector>

#include "Typization.h"

using namespace Typization; // NOLINT(build/namespaces)

// Функция, состоит из имени и сигнатуры
// Предикат - тоже функция, но на выходе boolean
struct Function {
	// Имя функции
	std::string name;
	// Типы входных аргументов
	std::vector<ObjectType> input;
	// Тип выходного аргумента
	ObjectType output;
};

// с namespace ругается((
class FuncLib {
  public:
	inline static const std::vector<Function> functions = {
		{"Thompson", {ObjectType::Regex}, ObjectType::NFA},
		{"IlieYu", {ObjectType::Regex}, ObjectType::NFA},
		{"Antimirov", {ObjectType::Regex}, ObjectType::NFA},
		{"Arden", {ObjectType::NFA}, ObjectType::Regex},
		{"Glushkov", {ObjectType::Regex}, ObjectType::NFA},
		{"Determinize", {ObjectType::NFA}, ObjectType::DFA},
		{"Determinize+", {ObjectType::NFA}, ObjectType::DFA},
		{"RemEps", {ObjectType::NFA}, ObjectType::NFA},
		{"Linearize", {ObjectType::Regex}, ObjectType::Regex},
		{"Minimize", {ObjectType::NFA}, ObjectType::DFA},
		{"Minimize+", {ObjectType::NFA}, ObjectType::DFA},
		{"Reverse", {ObjectType::NFA}, ObjectType::NFA},
		{"Annote", {ObjectType::NFA}, ObjectType::DFA},
		{"DeLinearize", {ObjectType::Regex}, ObjectType::Regex},
		{"DeLinearize", {ObjectType::NFA}, ObjectType::NFA},
		{"Complement", {ObjectType::DFA}, ObjectType::DFA},
		{"RemoveTrap", {ObjectType::DFA}, ObjectType::DFA},
		{"DeAnnote", {ObjectType::Regex}, ObjectType::Regex},
		{"DeAnnote", {ObjectType::NFA}, ObjectType::NFA},
		{"MergeBisim", {ObjectType::NFA}, ObjectType::NFA},
		{"Disambiguate", {ObjectType::Regex}, ObjectType::Regex},
		{"Intersect", {ObjectType::NFA, ObjectType::NFA}, ObjectType::NFA},
		{"Union", {ObjectType::NFA, ObjectType::NFA}, ObjectType::NFA},
		{"Difference", {ObjectType::NFA, ObjectType::NFA}, ObjectType::NFA},
		// Многосортные функции
		{"PumpLength", {ObjectType::Regex}, ObjectType::Int},
		{"ClassLength", {ObjectType::NFA}, ObjectType::Int},
		{"Normalize", {ObjectType::Regex, ObjectType::Array}, ObjectType::Regex},
		{"States", {ObjectType::NFA}, ObjectType::Int},
		{"ClassCard", {ObjectType::NFA}, ObjectType::Int},
		{"Ambiguity", {ObjectType::NFA}, ObjectType::AmbiguityValue},
		{"MyhillNerode", {ObjectType::NFA}, ObjectType::Int},
		{"GlaisterShallit", {ObjectType::NFA}, ObjectType::Int},
		{"PrefixGrammar", {ObjectType::NFA}, ObjectType::PrefixGrammar},
		{"PGtoNFA", {ObjectType::PrefixGrammar}, ObjectType::NFA},
		// Предикаты
		{"Bisimilar", {ObjectType::NFA, ObjectType::NFA}, ObjectType::Boolean},
		// для dfa - bool, для nfa - std::optional<bool>
		{"Minimal", {ObjectType::NFA}, ObjectType::OptionalBool},
		{"Deterministic", {ObjectType::NFA}, ObjectType::Boolean},
		{"Subset", {ObjectType::Regex, ObjectType::Regex}, ObjectType::Boolean},
		{"Subset", {ObjectType::NFA, ObjectType::NFA}, ObjectType::Boolean},
		{"Equiv", {ObjectType::Regex, ObjectType::Regex}, ObjectType::Boolean},
		{"Equiv", {ObjectType::NFA, ObjectType::NFA}, ObjectType::Boolean},
		{"Equal", {ObjectType::Regex, ObjectType::Regex}, ObjectType::Boolean},
		{"Equal", {ObjectType::NFA, ObjectType::NFA}, ObjectType::Boolean},
		{"Equal", {ObjectType::Int, ObjectType::Int}, ObjectType::Boolean},
		{"Equal", {ObjectType::AmbiguityValue, ObjectType::AmbiguityValue}, ObjectType::Boolean},
		{"Equal", {ObjectType::Boolean, ObjectType::Boolean}, ObjectType::Boolean},
		{"OneUnambiguity", {ObjectType::Regex}, ObjectType::Boolean},
		{"OneUnambiguity", {ObjectType::NFA}, ObjectType::Boolean},
		{"SemDet", {ObjectType::NFA}, ObjectType::Boolean}};
};