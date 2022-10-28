#pragma once
#include "Interpreter.h"
#include <algorithm>
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
		 {{"DeLinearize", {ObjectType::Regex}, ObjectType::Regex},
		  {"DeLinearize", {ObjectType::NFA}, ObjectType::NFA}}},
		{"Complement", {{"Complement", {ObjectType::DFA}, ObjectType::DFA}}},
		{"DeAnnote",
		 {{"DeAnnote", {ObjectType::Regex}, ObjectType::Regex},
		  {"DeAnnote", {ObjectType::NFA}, ObjectType::NFA}}},
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

void Interpreter::load_file(const string& filename) {
	Lexer lexer;
	auto lexem_strings = lexer.load_file(filename);
	operations = {};
	for (const auto& lexems : lexem_strings) {
		if (auto op = scan_operation(lexems); op.has_value()) {
			operations.push_back(*op);
		}
	}
}

optional<vector<Function>> Interpreter::build_function_sequence(
	vector<string> function_names, vector<ObjectType> first_type) {
	// 0 - функцию надо исключить из последовательности
	// 1 - функция остается в последовательности
	// 2 - функция(Delinearize или DeAnnote) принимает на вход Regex
	// 3 - функция(Delinearize или DeAnnote) принимает на вход NFA
	vector<int> neededfuncs(function_names.size(), 1);
	if (first_type == names_to_functions[function_names[0]][0].input) {
		if (names_to_functions[function_names[0]].size() == 2) {
			neededfuncs[0] = 2;
		} else {
			neededfuncs[0] = 1;
		}
	} else {
		if (names_to_functions[function_names[0]].size() == 2) {
			if (first_type == names_to_functions[function_names[0]][1].input) {
				neededfuncs[0] = 3;
			}
		} else {
			return nullopt;
		}
	}
	for (int i = 1; i < function_names.size(); i++) {
		string func = function_names[i];
		string predfunc = function_names[i - 1];
		// check on types
		if (names_to_functions[func].size() == 1 &&
			names_to_functions[predfunc].size() == 1) {
			if (names_to_functions[func][0].input.size() == 1) {
				if (names_to_functions[predfunc][0].output !=
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
								neededfuncs[i - 1] = 0;
							}
						} else if (predfunc == "Minimize" &&
								   func == "Minimize") {
							neededfuncs[i - 1] = 0;
						}
					}
				} else {
					if (predfunc == func) {
						if (predfunc != "Reverse" || predfunc != "Complement") {
							neededfuncs[i - 1] = 0;
						}
					} else {
						if (predfunc == "Linearize" &&
							(func == "Glushkov" || func == "IlieYu")) {
							neededfuncs[i - 1] = 0;
						}
					}
				}
			} else {
				return nullopt;
			}
		} else {
			vector<ObjectType> r = {ObjectType::Regex};
			vector<ObjectType> n = {ObjectType::NFA};
			if (names_to_functions[predfunc].size() == 2 &&
				names_to_functions[func].size() == 2) {
				if (predfunc != func) {
					if (neededfuncs[i - 1] > 1) {
						neededfuncs[i] = neededfuncs[i - 1];
					} else {
						return nullopt;
					}
				} else {
					neededfuncs[i - 1] = 0;
				}
			} else if (names_to_functions[predfunc].size() == 2) {
				if (neededfuncs[i - 1] < 2) {
					if (names_to_functions[func][0].input == r) {
						neededfuncs[i - 1] = 2;
					} else if (names_to_functions[func][0].input == n) {
						neededfuncs[i - 1] = 3;
					} else {
						return nullopt;
					}
				} else {
					if (names_to_functions[func][0].input == r) {
						if (neededfuncs[i - 1] != 2) {
							return nullopt;
						}
					} else if (names_to_functions[func][0].input == n) {
						if (neededfuncs[i - 1] != 3) {
							return nullopt;
						}
					} else {
						return nullopt;
					}
				}
			} else {
				if (names_to_functions[predfunc][0].input == r) {
					neededfuncs[i] = 2;
				} else if (names_to_functions[predfunc][0].input == n) {
					neededfuncs[i] = 3;
				} else {
					return nullopt;
				}
			}
		}
	}
	optional<vector<Function>> finalfuncs = nullopt;
	finalfuncs.emplace() = {};
	for (int i = 0; i < function_names.size(); i++) {
		if (neededfuncs[i] > 0) {
			// cout << function_names[i];
			if (neededfuncs[i] == 1 || neededfuncs[i] == 2) {
				Function f = names_to_functions[function_names[i]][0];
				finalfuncs.value().push_back(f);
			} else {
				Function f = names_to_functions[function_names[i]][1];
				finalfuncs.value().push_back(f);
			}
		}
	}

	return finalfuncs;
}

optional<Interpreter::Decalaration> Interpreter::scan_declaration(
	vector<Lexem> lexems) {

	Decalaration decl;

	// [идентификатор]
	if (lexems[0].type != Lexem::id) {
		cout << "id expected\n";
		return nullopt;
	}
	decl.id = lexems[0].value;

	// =
	if (lexems[1].type != Lexem::equalSign) {
		cout << "= expected\n";
		return nullopt;
	}

	// ([функция].)*[функция]?
	vector<string> func_names;
	int i = 2;
	for (; i < lexems.size() && lexems[i].type == Lexem::function; i++) {
		func_names.push_back(lexems[i].value);
		i++;
		if (lexems[i].type != Lexem::dot) {
			break;
		}
	}
	reverse(func_names.begin(), func_names.end());

	// [объект]+
	vector<ObjectType> argument_types;
	vector<variant<string, GeneralObject>> arguments;
	for (; i < lexems.size(); i++) {
		if (lexems[i].type == Lexem::id) {
			if (id_types.count(lexems[i].value)) {
				argument_types.push_back(id_types[lexems[i].value]);
				arguments.push_back(lexems[i].value);
			} else {
				cout << "Unknown id\n";
				return nullopt;
			}
		} else if (lexems[i].type == Lexem::regex) {
			argument_types.push_back(ObjectType::Regex);
			arguments.push_back(ObjectRegex(lexems[i].reg));
		} else {
			break;
		}
	}

	// (!!)
	if (i < lexems.size() && lexems[i].type == Lexem::doubleExclamation) {
		decl.show_result = true;
	}

	cout << "id\n";

	if (auto seq = build_function_sequence(func_names, argument_types);
		seq.has_value()) {

		decl.function_sequence = *seq;
		decl.parameters = arguments;
		id_types[decl.id] =
			(*seq).size() ? (*seq).back().output : argument_types[0];
		return decl;
	}
	return nullopt;
}

optional<Interpreter::Predicate> Interpreter::scan_predicate(
	vector<Lexem> lexems) {

	Predicate pred;

	// [предикат]
	if (lexems[0].type != Lexem::predicate) {
		return nullopt;
	}
	auto prdeicat_name = lexems[0].value;

	// [объект]+
	vector<ObjectType> argument_types;
	vector<variant<string, GeneralObject>> arguments;
	for (int i = 1; i < lexems.size(); i++) {
		if (lexems[i].type == Lexem::id) {
			if (id_types.count(lexems[i].value)) {
				argument_types.push_back(id_types[lexems[i].value]);
			} else {
				cout << "Unknown id";
				return nullopt;
			}
		} else if (lexems[i].type == Lexem::regex) {
			arguments.push_back(ObjectRegex(lexems[i].reg));
		} else if (lexems[i].type == Lexem::regex) {
			arguments.push_back(ObjectRegex(lexems[i].reg));
		}
	}

	cout << "pred\n";

	if (auto seq = build_function_sequence({prdeicat_name}, argument_types);
		seq.has_value()) {

		pred.predicate = (*seq)[0];
		pred.parameters = arguments;
		return pred;
	}
	return optional<Predicate>();
}

optional<Interpreter::GeneralOperation> Interpreter::scan_operation(
	vector<Lexem> lexems) {

	if (auto declaration = scan_declaration(lexems); declaration.has_value()) {
		return declaration;
	}
	if (auto predicate = scan_predicate(lexems); predicate.has_value()) {
		return predicate;
	}
	return nullopt;
}
