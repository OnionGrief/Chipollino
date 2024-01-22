#pragma once
#include <fstream>
#include <string>
#include <vector>

#include "Typization.h"

using namespace Typization; // NOLINT(build/namespaces)

namespace FuncLib {

// Функция, состоит из имени и сигнатуры
struct Function {
	// Имя функции
	std::string name;
	// Типы входных аргументов
	std::vector<ObjectType> input;
	// Тип выходного аргумента
	ObjectType output;
};

// список всех доступных функций интерпретатора
inline static const std::vector<Function> functions = {
	{"Thompson", {ObjectType::Regex}, ObjectType::NFA},
	{"IlieYu", {ObjectType::Regex}, ObjectType::NFA},
	{"Antimirov", {ObjectType::Regex}, ObjectType::NFA},
	{"Arden", {ObjectType::NFA}, ObjectType::Regex},
	{"Glushkov", {ObjectType::Regex}, ObjectType::NFA},
	{"MFA", {ObjectType::BRefRegex}, ObjectType::MFA},
	{"MFAexpt", {ObjectType::BRefRegex}, ObjectType::MFA},
	{"Determinize", {ObjectType::NFA}, ObjectType::DFA},
	{"Determinize+", {ObjectType::NFA}, ObjectType::DFA},
	{"RemEps", {ObjectType::NFA}, ObjectType::NFA},
	{"RemEps", {ObjectType::MFA}, ObjectType::MFA},
	{"Linearize", {ObjectType::Regex}, ObjectType::Regex},
	{"Minimize", {ObjectType::NFA}, ObjectType::DFA},
	{"Minimize+", {ObjectType::NFA}, ObjectType::DFA},
	{"Reverse", {ObjectType::NFA}, ObjectType::NFA},
	{"Reverse", {ObjectType::BRefRegex}, ObjectType::BRefRegex},
	{"Annote", {ObjectType::NFA}, ObjectType::DFA},
	{"DeLinearize", {ObjectType::Regex}, ObjectType::Regex},
	{"DeLinearize", {ObjectType::NFA}, ObjectType::NFA},
	{"AddTrap", {ObjectType::MFA}, ObjectType::MFA},
	{"Complement", {ObjectType::DFA}, ObjectType::DFA},
	{"Complement", {ObjectType::MFA}, ObjectType::MFA},
	{"RemoveTrap", {ObjectType::DFA}, ObjectType::DFA},
	{"DeAnnote", {ObjectType::Regex}, ObjectType::Regex},
	{"DeAnnote", {ObjectType::NFA}, ObjectType::NFA},
	{"MergeBisim", {ObjectType::NFA}, ObjectType::NFA},
	{"Disambiguate", {ObjectType::Regex}, ObjectType::Regex},
	{"Intersect", {ObjectType::NFA, ObjectType::NFA}, ObjectType::NFA},
	{"Union", {ObjectType::NFA, ObjectType::NFA}, ObjectType::NFA},
	{"Difference", {ObjectType::NFA, ObjectType::NFA}, ObjectType::NFA},
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
	{"Bisimilar", {ObjectType::NFA, ObjectType::NFA}, ObjectType::Boolean},
	{"Minimal", {ObjectType::NFA}, ObjectType::OptionalBool},
	{"Deterministic", {ObjectType::NFA}, ObjectType::Boolean},
	{"Deterministic", {ObjectType::MFA}, ObjectType::Boolean},
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
	{"SemDet", {ObjectType::NFA}, ObjectType::Boolean},
	{"IsAcreg", {ObjectType::BRefRegex}, ObjectType::Boolean},
};

// вспомогательная функция для Ани и ее курсача
static void create_yaml() {
	std::ofstream outfile("funcs.yaml");
	outfile << "functions:\n";
	for (int j = 0; j < functions.size(); j++) {
		Function func = functions[j];
		outfile << " - name: " << func.name << "\n";
		outfile << "   prog_name: \n";
		outfile << "   return_type: " << types_to_string.at(func.output) << "\n";
		outfile << "   arguments: [";
		for (int i = 0; i < func.input.size(); i++) {
			if (i != 0)
				outfile << ", ";
			outfile << "" << types_to_string.at(func.input[i]) << "";
		}
		outfile << "]\n\n";
	}
}
}; // namespace FuncLib

/*
Считаем дедлайном 6 часов утра 24 октября. Успехов!
*/