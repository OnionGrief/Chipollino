#pragma once
#include "Interpreter.h"
#include <algorithm>
#include <map>
#include <string>
using namespace std;

bool operator==(const Function& l, const Function& r) {
	return l.name == r.name && l.input == r.input && l.output == r.output;
}

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
	Lexer lexer(*this);
	auto lexem_strings = lexer.load_file(filename);
	operations = {};
	int line_number = 0;
	for (const auto& lexems : lexem_strings) {
		if (auto op = scan_operation(lexems); op.has_value()) {
			operations.push_back(*op);
		} else {
			throw_error("Error: cannot identify operation in line " +
						to_string(line_number));
		}
		line_number++;
	}
}

void Interpreter::run_all() {
	for (const auto& op : operations) {
		run_operation(op);
	}
}

void Interpreter::set_log_mode(LogMode mode) {
	log_mode = mode;
}

void Interpreter::log(const string& str) {
	if (log_mode == LogMode::all) {
		cout << str << "\n";
	}
}

void Interpreter::throw_error(const string& str) {
	if (log_mode != LogMode::nothing) {
		cout << "ERROR: " << str << "\n";
	}
	error = true;
}

GeneralObject Interpreter::apply_function_sequence(
	const vector<Function>& functions, vector<GeneralObject> arguments) {

	for (const auto& func : functions) {
		arguments = {apply_function(func, arguments)};
	}

	return arguments[0];
}

GeneralObject Interpreter::apply_function(
	const Function& function, const vector<GeneralObject>& arguments) {
	// Можээт прыгодыытся
	const auto nfa = ObjectType::NFA;
	const auto dfa = ObjectType::DFA;
	const auto regex = ObjectType::Regex;
	const auto integer = ObjectType::Int;
	const auto filename = ObjectType::FileName;
	const auto boolean = ObjectType::Boolean;
	const auto value = ObjectType::Value;

	if (function.name == "Glushkov") {
		return ObjectNFA(get<ObjectRegex>(arguments[0]).value.to_glushkov());
	}

	if (function.name == "IlieYu") {
		return ObjectNFA(get<ObjectRegex>(arguments[0]).value.to_ilieyu());
	}
	if (function.name == "Antimirov") {
		return ObjectNFA(get<ObjectRegex>(arguments[0]).value.to_antimirov());
	}
	/*if (function.name == "Arden") {
		return ObjectRegex(nfa_to_regex(get<ObjectNFA>(arguments[0]).value));
	}*/
	if (function.name == "Thompson") {
		return ObjectNFA(get<ObjectRegex>(arguments[0]).value.to_tompson());
	}
	if (function.name == "Bisimilar") {
		return ObjectBoolean(
			FiniteAutomaton::bisimilar(get<ObjectNFA>(arguments[0]).value,
									   get<ObjectNFA>(arguments[1]).value));
	}
	//неясно чье
	/*if (function.name == "Minimal") {

		return ObjectBoolean(get<ObjectDFA>(arguments[0]).value.);
	}*/
	if (function.name == "Subset") {
		return ObjectBoolean(
			get<ObjectNFA>(arguments[1])
				.value.subset(get<ObjectNFA>(arguments[1]).value));
	}
	if (function.name == "Equiv") {
		vector<ObjectType> n = {nfa, nfa};
		if (function.input == n) {
			return ObjectBoolean(FiniteAutomaton::equivalent(
				get<ObjectNFA>(arguments[0]).value,
				get<ObjectNFA>(arguments[1]).value));
		} else {
			return ObjectBoolean(
				Regex::equivalent(get<ObjectRegex>(arguments[0]).value,
								  get<ObjectRegex>(arguments[1]).value));
		}
	}
	if (function.name == "Equal") {
		return ObjectBoolean(
			FiniteAutomaton::equal(get<ObjectNFA>(arguments[0]).value,
								   get<ObjectNFA>(arguments[1]).value));
	}
	//пока не работает
	/*(if (function.name == "SemDet") {
		return ObjectBoolean(get<ObjectNFA>(arguments[0]).value.);
	}*/
	if (function.name == "Determinize") {
		return ObjectDFA(get<ObjectNFA>(arguments[0]).value.determinize());
	}
	if (function.name == "Minimize") {
		return ObjectDFA(get<ObjectNFA>(arguments[0]).value.minimize());
	}
	if (function.name == "Annote") {
		return ObjectDFA(get<ObjectNFA>(arguments[0]).value.annote());
	}
	if (function.name == "PumpLength") {
		return ObjectInt(get<ObjectRegex>(arguments[0]).value.pump_length());
	}
	//Миша
	/*if (function.name == "ClassLength") {
		return ObjectInt(get<ObjectDFA>(arguments[0]).value.);
	}*/
	//у нас нет
	/*if (function.name == "KSubSet") {
		return ObjectInt(get<ObjectDFA>(arguments[0]).value.);
	}*/
	//Саша вроде сделает
	/*if (function.name == "States") {
		return ObjectInt(get<ObjectNFA>(arguments[0]).value.);
	}*/
	//Мишино вроде
	/*if (function.name == "ClassCard") {
		return ObjectInt(get<ObjectDFA>(arguments[0]).value.);
	}*/
	//пока не реализовано
	/*if (function.name == "Ambiguity") {
		return ObjectValue(get<ObjectNFA>(arguments[0]).value.);
	}*/
	/*if (function.name == "Width") {
		return ObjectInt(get<ObjectNFA>(arguments[0]).value.);
	}*/
	//Миша
	/*if (function.name == "MyhillNerode") {
		return ObjectInt(get<ObjectDFA>(arguments[0]).value.);
	}*/

	//пошла жопа

	/*
	* Идёт Глушков по лесу, видит -- регулярка.
	Построил автомат и застрелился.
	*/

	GeneralObject predres = arguments[0];
	optional<GeneralObject> res;

	if (function.name == "RemEps") {
		res = ObjectNFA(get<ObjectNFA>(arguments[0]).value.remove_eps());
	}
	/*if (function.name == "Linearize") {
		res = ObjectRegex(get<ObjectRegex>(arguments[0]).value.);
	}*/
	if (function.name == "Reverse") {
		res = ObjectNFA(get<ObjectNFA>(arguments[0]).value.reverse());
	}
	/*if (function.name == "Delinearize") {
		if (function.output == regex){
			res =  ObjectRegex(get<ObjectRegex>(arguments[0]).value.);
		} else {
			res =  ObjectNFA(get<ObjectNFA>(arguments[0]).value.);
		}
	}*/
	if (function.name == "Complement") {
		res = ObjectDFA(get<ObjectDFA>(arguments[0]).value.complement());
	}
	if (function.name == "DeAnnote") {
		if (function.output == nfa) {
			res = ObjectNFA(get<ObjectNFA>(arguments[0]).value.deannote());
		} else {
			// res =
			// ObjectRegex(get<ObjectRegex>(arguments[0]).value.deannote());
		}
	}
	if (function.name == "MergeBisim") {
		res = ObjectNFA(get<ObjectNFA>(arguments[0]).value.merge_bisimilar());
	}
	if (function.name == "Normalize") {
		res = ObjectRegex(get<ObjectRegex>(arguments[0])
							  .value.normalize_regex(
								  get<ObjectFileName>(arguments[0]).value));
	}
	/*if (function.name == "Simplify") {
		res =  ObjectRegex(get<ObjectRegex>(arguments[0]).value.);
	}*/

	if (res.has_value()) {
		GeneralObject resval = res.value();

		if (holds_alternative<ObjectRegex>(resval)) {
			if (Regex::equal(get<ObjectRegex>(resval).value,
							 get<ObjectRegex>(predres).value))
				cerr << "Функция " + function.name + " ниче не меняет. Грусть("
					 << endl;
		}

		if (holds_alternative<ObjectDFA>(resval)) {
			if (FiniteAutomaton::equal(get<ObjectDFA>(resval).value,
									   get<ObjectDFA>(predres).value))
				cerr << "Функция " + function.name + " ниче не меняет. Грусть("
					 << endl;
		}

		if (holds_alternative<ObjectNFA>(resval)) {
			if (FiniteAutomaton::equal(get<ObjectNFA>(resval).value,
									   get<ObjectNFA>(predres).value))
				cerr << "Функция " + function.name + " ниче не меняет. Грусть("
					 << endl;
		}

		return res.value();
	}

	cerr << "Функция " + function.name + " страшная и мне не известная O_O"
		 << endl;

	// FIXME: Ошибка *намеренно* вызывает сегфолт.
	//          Придумай что-нибудь!

	cerr << *((int*)0);

	return GeneralObject();
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

vector<GeneralObject> Interpreter::parameters_to_arguments(
	const vector<variant<string, GeneralObject>>& parameters) {

	vector<GeneralObject> arguments;
	for (const auto& p : parameters) {
		if (holds_alternative<GeneralObject>(p)) {
			arguments.push_back(get<GeneralObject>(p));
		} else {
			arguments.push_back(objects[get<string>(p)]);
		}
	}
	return arguments;
}

void Interpreter::run_declaration(const Declaration& decl) {
	log("Running declaration...");
	// TODO activate/disactivate logger here
	objects[decl.id] = apply_function_sequence(
		decl.function_sequence, parameters_to_arguments(decl.parameters));
	log("    assigned to " + to_string(decl.id));
}

void Interpreter::run_predicate(const Predicate& pred) {
	log("Running predicate...");
	auto res = apply_function(pred.predicate,
							  parameters_to_arguments(pred.parameters));
	log("    result: " + to_string(get<ObjectBoolean>(res).value));
}

void Interpreter::run_operation(const GeneralOperation& op) {
	if (holds_alternative<Declaration>(op)) {
		run_declaration(get<Declaration>(op));
	} else if (holds_alternative<Predicate>(op)) {
		run_predicate(get<Predicate>(op));
	} else if (holds_alternative<Test>(op)) {
		throw_error("Tests are not supported yet :-(\n"); // TODO run tests
	}
}

optional<Interpreter::Declaration> Interpreter::scan_declaration(
	vector<Lexem> lexems) {

	Declaration decl;

	// [идентификатор]
	if (lexems[0].type != Lexem::id) {
		return nullopt;
	}
	decl.id = lexems[0].value;

	// =
	if (lexems[1].type != Lexem::equalSign) {
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

/*
Был такой анекдот: человек приходит к врачу. У него депрессия. Говорит, жизнь
жестока и несправедлива. Говорит, он один-одинешенек в этом ужасном и мрачном
мире, где будущее вечно скрыто во мраке. Врач говорит: «Лекарство очень простое.
Сегодня в цирке выступает великий клоун Пальяччи. Сходите, посмотрите на него.
Это вам поможет.» Человек разражается слезами. И говорит: «Но, доктор… … я и
есть Пальяччи». Хороший анекдот. Всем смеяться.
*/