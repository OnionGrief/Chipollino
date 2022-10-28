#pragma once
#include "Interpreter.h"
#include <map>
#include <string>
using namespace std;

Interpreter::Interpreter() {
	names_to_functions = {
		{"Thompson", {{"Thompson", {ObjectType::Regex}, ObjectType::NFA}}},

		{"IlieYu", {{"IlieYu", {ObjectType::Regex}, ObjectType::NFA}}},
		{"Antimirov", {{"Antimirov", {ObjectType::Regex}, ObjectType::NFA}}},
		{"Arden", {{"Arden", {ObjectType::NFA}, ObjectType::Regex}}},
		{"Glushkov", {{"Glushkov", {ObjectType::Regex}, ObjectType::NFA}}},
		{"Determinize", {{"Determinize", {ObjectType::NFA}, ObjectType::DFA}}},
		{"RemEps", {{"RemEps", {ObjectType::NFA}, ObjectType::NFA}}},
		{"Linearize", {{"Linearize", {ObjectType::Regex}, ObjectType::Regex}}},
		{"Minimize", {{"Minimize", {ObjectType::NFA}, ObjectType::DFA}}},
		{"Reverse", {{"Reverse", {ObjectType::NFA}, ObjectType::NFA}}},
		{"Annote", {{"Annote", {ObjectType::NFA}, ObjectType::DFA}}},
		{"DeLinearize",
		 {{"DeLinearize", {ObjectType::NFA}, ObjectType::NFA},
		  {"DeLinearize", {ObjectType::Regex}, ObjectType::Regex}}},
		{"Complement", {{"Complement", {ObjectType::DFA}, ObjectType::DFA}}},
		{"DeAnnote",
		 {{"DeAnnote", {ObjectType::NFA}, ObjectType::NFA},
		  {"DeAnnote", {ObjectType::Regex}, ObjectType::Regex}}},
		{"MergeBisim", {{"MergeBisim", {ObjectType::NFA}, ObjectType::NFA}}},
		//Многосортные функции
		{"PumpLength", {{"PumpLength", {ObjectType::Regex}, ObjectType::Int}}},
		{"ClassLength", {{"ClassLength", {ObjectType::DFA}, ObjectType::Int}}},
		{"KSubSet",
		 {{"KSubSet", {ObjectType::Int, ObjectType::NFA}, ObjectType::NFA}}},
		{"Normalize",
		 {{"Normalize",
		   {ObjectType::Regex, ObjectType::FileName},
		   ObjectType::Regex}}},
		{"States", {{"States", {ObjectType::NFA}, ObjectType::Int}}},
		{"ClassCard", {{"ClassCard", {ObjectType::DFA}, ObjectType::Int}}},
		{"Ambiguity", {{"Ambiguity", {ObjectType::NFA}, ObjectType::Value}}},
		{"Width", {{"Width", {ObjectType::NFA}, ObjectType::Int}}},
		{"MyhillNerode",
		 {{"MyhillNerode", {ObjectType::DFA}, ObjectType::Int}}},
		{"Simplify", {{"Simplify", {ObjectType::Regex}, ObjectType::Regex}}},
		//Предикаты
		{"Bisimilar",
		 {{"Bisimilar",
		   {ObjectType::NFA, ObjectType::NFA},
		   ObjectType::Boolean}}},
		{"Minimal", {{"Minimal", {ObjectType::DFA}, ObjectType::Boolean}}},
		{"Subset",
		 {{"Subset",
		   {ObjectType::Regex, ObjectType::Regex},
		   ObjectType::Boolean}}},
		{"Equiv",
		 {{"Equiv", {ObjectType::NFA, ObjectType::NFA}, ObjectType::Boolean}}},
		{"Minimal", {{"Minimal", {ObjectType::NFA}, ObjectType::Boolean}}},
		{"Equal",
		 {{"Equal", {ObjectType::NFA, ObjectType::NFA}, ObjectType::Boolean}}},
		{"SemDet", {{"SemDet", {ObjectType::NFA}, ObjectType::Boolean}}}};
}

optional<vector<Function>> Interpreter::build_function_sequence(
	vector<string> function_names) {
	vector<bool> neededfuncs(function_names.size(), true);
	for (int i = 1; i < function_names.size(); i++) {
		string func = function_names[i];
		string predfunc = function_names[i - 1];
		// check on types
		if (names_to_functions[func].size() == 1 ||
			names_to_functions[predfunc].size() == 1) {
			if (names_to_functions[func][0].input.size() == 1 &&
				names_to_functions[predfunc][0].output !=
					names_to_functions[func][0].input[0]) {
				vector<ObjectType> v = {ObjectType::NFA};
				if (!(names_to_functions[predfunc][0].output ==
						  ObjectType::DFA &&
					  names_to_functions[func][0].input == v)) {
					return nullopt;
				} else {
					if (predfunc == "Determinize" || predfunc == "Annote") {
						if (func == "Determinize" || func == "Minimize" ||
							func == "Annote") {
							neededfuncs[i - 1] = false;
						}
					} else if (predfunc == "Minimize" && func == "Minimize") {
						neededfuncs[i - 1] = false;
					}
				}
			} else {
				if (predfunc == func) {
					if (predfunc != "Reverse" || predfunc != "Complement") {
						neededfuncs[i - 1] = false;
					}
				} else {
					//доработать
				}
			}
		}
	}
	optional<vector<Function>> finalfuncs = nullopt;
	finalfuncs.emplace() = {};
	for (int i = 0; i < function_names.size(); i++) {
		if (neededfuncs[i]) {
			//cout << function_names[i];
			Function f = names_to_functions[function_names[i]][0];
			finalfuncs.value().push_back(f);
		}
	}
	return finalfuncs;
}