#pragma once
#include "Interpreter/Interpreter.h"
#include "Tester/Tester.h"
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
		   ObjectType::Boolean},
		  {"Subset", {ObjectType::NFA, ObjectType::NFA}, ObjectType::Boolean}}},
		{"Equiv",
		 {{"Equiv",
		   {ObjectType::Regex, ObjectType::Regex},
		   ObjectType::Boolean},
		  {"Equiv", {ObjectType::NFA, ObjectType::NFA}, ObjectType::Boolean}}},
		{"Minimal", {{"Minimal", {ObjectType::NFA}, ObjectType::Boolean}}},
		{"Equal",
		 {{"Equal", {ObjectType::NFA, ObjectType::NFA}, ObjectType::Boolean}}},
		{"SemDet", {{"SemDet", {ObjectType::NFA}, ObjectType::Boolean}}}};
}

void Interpreter::load_file(const string& filename) {
	log("Interpreter: loading file " + filename);
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
	log("Interpreter: file loaded " + filename);
}

void Interpreter::run_all() {
	log("Running all");
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

	auto get_automaton =
		[](const GeneralObject& obj) -> const FiniteAutomaton& {
		if (holds_alternative<ObjectNFA>(obj)) {
			return get<ObjectNFA>(obj).value;
		}
		if (holds_alternative<ObjectDFA>(obj)) {
			return get<ObjectDFA>(obj).value;
		}
	};

	auto is_automaton = [](const GeneralObject& obj) -> const bool {
		return holds_alternative<ObjectNFA>(obj) ||
			   holds_alternative<ObjectDFA>(obj);
	};

	if (function.name == "Glushkov") {
		return ObjectNFA(get<ObjectRegex>(arguments[0]).value.to_glushkov());
	}

	if (function.name == "IlieYu") {
		return ObjectNFA(get<ObjectRegex>(arguments[0]).value.to_ilieyu());
	}
	if (function.name == "Antimirov") {
		return ObjectNFA(get<ObjectRegex>(arguments[0]).value.to_antimirov());
	}
	if (function.name == "Arden") {
		return ObjectRegex((get_automaton(arguments[0]).nfa_to_regex()));
	}
	if (function.name == "Thompson") {
		return ObjectNFA(get<ObjectRegex>(arguments[0]).value.to_tompson());
	}
	if (function.name == "Bisimilar") {
		return ObjectBoolean(FiniteAutomaton::bisimilar(
			get_automaton(arguments[0]), get_automaton(arguments[1])));
	}
	//мое
	/*if (function.name == "Minimal") {

		return ObjectBoolean(get<ObjectDFA>(arguments[0]).value.);
	}*/
	if (function.name == "Subset") {
		if (vector<ObjectType> sign = {nfa, nfa}; function.input == sign) {
			return ObjectBoolean((get_automaton(arguments[0])
									  .subset(get_automaton(arguments[1]))));
		} else {
			return ObjectBoolean(
				get<ObjectRegex>(arguments[0])
					.value.subset(get<ObjectRegex>(arguments[1]).value));
		}
	}
	if (function.name == "Equiv") {
		vector<ObjectType> n = {nfa, nfa};
		if (function.input == n) {
			return ObjectBoolean(FiniteAutomaton::equivalent(
				get_automaton(arguments[0]), get_automaton(arguments[1])));
		} else {
			return ObjectBoolean(
				Regex::equivalent(get<ObjectRegex>(arguments[0]).value,
								  get<ObjectRegex>(arguments[1]).value));
		}
	}
	if (function.name == "Equal") {
		if (vector<ObjectType> sign = {nfa, nfa}; function.input == sign) {
			return ObjectBoolean(FiniteAutomaton::equal(
				get_automaton(arguments[0]), get_automaton(arguments[1])));
		} else {
			return ObjectBoolean(
				Regex::equal(get<ObjectRegex>(arguments[0]).value,
							 get<ObjectRegex>(arguments[1]).value));
		}
	}
	if (function.name == "SemDet") {
		return ObjectBoolean(get_automaton(arguments[0]).semdet());
	}
	if (function.name == "PumpLength") {
		return ObjectInt(get<ObjectRegex>(arguments[0]).value.pump_length());
	}
	//Миша
	/*if (function.name == "ClassLength") {
		return
	ObjectInt(class_legth_typecheker(get<ObjectDFA>(arguments[0]).value));
	}
	//у нас нет
	/*if (function.name == "KSubSet") {
		return ObjectInt(get<ObjectDFA>(arguments[0]).value.);
	}*/
	if (function.name == "States") {
		return ObjectInt(get_automaton(arguments[0]).states_number());
	}
	//Мишино вроде
	/*if (function.name == "ClassCard") {
		return ObjectInt(get<ObjectDFA>(arguments[0]).value.);
	}*/
	if (function.name == "Ambiguity") {
		return ObjectValue(get_automaton(arguments[0]).ambiguity());
	}
	/*if (function.name == "Width") {
		return ObjectInt(get<ObjectNFA>(arguments[0]).value.);
	}*/
	//Миша
	/*if (function.name == "MyhillNerode") {
		return ObjectInt(get<ObjectDFA>(arguments[0]).value.);
	}*/

	/*
	* Идёт Глушков по лесу, видит -- регулярка.
	Построил автомат и застрелился.
	*/

	GeneralObject predres = arguments[0];
	optional<GeneralObject> res;

	if (function.name == "Determinize") {
		res = ObjectDFA(get_automaton(arguments[0]).determinize());
	}
	if (function.name == "Minimize") {
		res = ObjectDFA(get_automaton(arguments[0]).minimize());
	}
	if (function.name == "Annote") {
		res = ObjectDFA(get_automaton(arguments[0]).annote());
	}
	if (function.name == "RemEps") {
		res = ObjectNFA(get_automaton(arguments[0]).remove_eps());
	}
	if (function.name == "Linearize") {
		res = ObjectRegex(get<ObjectRegex>(arguments[0]).value.linearize());
	}
	if (function.name == "Reverse") {
		res = ObjectNFA(get_automaton(arguments[0]).reverse());
	}
	if (function.name == "DeLinearize") {
		if (function.output == regex) {
			res =
				ObjectRegex(get<ObjectRegex>(arguments[0]).value.delinearize());
		} /* else {
			 // res =  ObjectNFA(get<ObjectNFA>(arguments[0]).value.);
		 }*/
	}
	if (function.name == "Complement") {
		// FiniteAutomaton fa = get_automaton(arguments[0]);
		// if (fa.is_deterministic())
		res = ObjectDFA(get<ObjectDFA>(arguments[0]).value.complement());
	}
	if (function.name == "DeAnnote") {
		if (function.output == nfa) {
			res = ObjectNFA(get_automaton(arguments[0]).deannote());
		} else {
			res = ObjectRegex(get<ObjectRegex>(arguments[0]).value.deannote());
		}
	}
	if (function.name == "MergeBisim") {
		res = ObjectNFA(get_automaton(arguments[0]).merge_bisimilar());
	}
	if (function.name == "Normalize") {
		res = ObjectRegex(get<ObjectRegex>(arguments[0])
							  .value.normalize_regex(
								  get<ObjectFileName>(arguments[1]).value));
	}
	/*if (function.name == "Simplify") {
		res =  ObjectRegex(get<ObjectRegex>(arguments[0]).value.);
	}*/

	if (res.has_value()) {
		GeneralObject resval = res.value();
		Logger::activate_step_counter();

		if (holds_alternative<ObjectRegex>(resval) &&
			holds_alternative<ObjectRegex>(predres)) {
			if (Regex::equal(get<ObjectRegex>(resval).value,
							 get<ObjectRegex>(predres).value))
				cerr << "Function " + function.name + " do nothing. Sadness("
					 << endl;
		}

		if (is_automaton(resval) && is_automaton(predres))
			if (FiniteAutomaton::equal(get_automaton(resval),
									   get_automaton(predres))) {
				cerr << "Function " + function.name + " do nothing. Sadness("
					 << endl;
			}

		Logger::deactivate_step_counter();
		return res.value();
	}

	cerr << "Функция " + function.name + " страшная и мне не известная O_O"
		 << endl;

	// FIXME: Ошибка *намеренно* вызывает сегфолт.
	//          Придумай что-нибудь!

	cerr << *((int*)0);

	return GeneralObject();
}

bool Interpreter::typecheck(vector<ObjectType> func_input_type,
							vector<ObjectType> input_type) {
	if (input_type.size() != func_input_type.size()) return false;
	for (int i = 0; i < input_type.size(); i++) {
		if (!((input_type[i] == func_input_type[i]) ||
			  (input_type[i] == ObjectType::DFA &&
			   func_input_type[i] == ObjectType::NFA)))
			return false;
	}
	return true;
}

optional<vector<Function>> Interpreter::build_function_sequence(
	vector<string> function_names, vector<ObjectType> first_type) {
	if (!function_names.size()) {
		return vector<Function>();
	}

	// 0 - функцию надо исключить из последовательности
	// 1 - функция остается в последовательности
	// 2 - функция(Delinearize или DeAnnote) принимает на вход Regex
	// 3 - функция(Delinearize или DeAnnote) принимает на вход NFA/DFA
	vector<int> neededfuncs(function_names.size(), 1);
	if (typecheck(names_to_functions[function_names[0]][0].input, first_type)) {
		if (names_to_functions[function_names[0]].size() == 2) {
			neededfuncs[0] = 2;
		} else {
			neededfuncs[0] = 1;
		}
	} else {
		if (names_to_functions[function_names[0]].size() == 2) {
			if (typecheck(names_to_functions[function_names[0]][1].input,
						  first_type)) {
				neededfuncs[0] = 3;
			}
		} else {
			return nullopt;
		}
	}

	string predfunc = function_names[0];
	for (int i = 1; i < function_names.size(); i++) {
		if (neededfuncs[i - 1] != 0) predfunc = function_names[i - 1];
		string func = function_names[i];
		// check on types

		if (names_to_functions[func].size() == 1 &&
			names_to_functions[predfunc].size() == 1) {
			if (names_to_functions[func][0].input.size() == 1) {
				if (names_to_functions[predfunc][0].output !=
					names_to_functions[func][0].input[0]) {
					vector<ObjectType> nfa_type = {ObjectType::NFA};
					if (!(names_to_functions[predfunc][0].output ==
							  ObjectType::DFA &&
						  names_to_functions[func][0].input == nfa_type)) {
						return nullopt;
					} else {
						if (func == "Determinize" || func == "Annote") {
							if (predfunc == "Determinize" ||
								predfunc == "Minimize" ||
								predfunc == "Annote") {
								neededfuncs[i] = 0;
							}
						} else if (predfunc == "Minimize" &&
								   func == "Minimize") {
							neededfuncs[i - 1] = 0;
						}
					}
				} else {
					if (predfunc == func) {
						if (predfunc != "Reverse" && predfunc != "Complement") {
							neededfuncs[i - 1] = 0;
						}
					} else {
						if (func == "Linearize" &&
							(predfunc == "Glushkov" || predfunc == "IlieYu")) {
							neededfuncs[i - 1] = 0;
						}
					}
				}
			} else {
				return nullopt;
			}
		} else {

			vector<ObjectType> regex_type = {ObjectType::Regex};
			vector<ObjectType> nfa_type = {ObjectType::NFA};
			vector<ObjectType> dfa_type = {ObjectType::DFA};

			if (names_to_functions[predfunc].size() == 2 &&
				names_to_functions[func].size() == 2) {
				if (predfunc != func) {
					if (neededfuncs[i - 1] > 1) {
						neededfuncs[i] = neededfuncs[i - 1];
					} else {
						return nullopt;
					}
				} else {
					neededfuncs[i] = 0;
				}
			} else if (names_to_functions[predfunc].size() == 2) {
				if (neededfuncs[i - 1] < 2) {
					if (names_to_functions[func][0].input == regex_type) {
						neededfuncs[i] = 2;
					} else if (names_to_functions[func][0].input == nfa_type ||
							   names_to_functions[func][0].input == dfa_type) {
						neededfuncs[i] = 3;
					} else {
						return nullopt;
					}
				} else {
					if (names_to_functions[func][0].input == regex_type) {
						if (neededfuncs[i - 1] != 2) {
							return nullopt;
						}
					} else if (names_to_functions[func][0].input == nfa_type ||
							   names_to_functions[func][0].input == dfa_type) {
						if (neededfuncs[i - 1] != 3) {
							return nullopt;
						}
					} else {
						return nullopt;
					}
				}
			} else {
				if (names_to_functions[predfunc][0].output ==
					ObjectType::Regex) {
					neededfuncs[i] = 2;
				} else if (names_to_functions[predfunc][0].output ==
							   ObjectType::NFA ||
						   names_to_functions[predfunc][0].output ==
							   ObjectType::DFA) {
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
				// тип NFA для DeAnnote
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
	if (decl.show_result) {
		Logger::activate();
		log("    logger is activated for this task");
	} else {
		Logger::deactivate();
	}
	objects[decl.id] = apply_function_sequence(
		decl.function_sequence, parameters_to_arguments(decl.parameters));
	log("    assigned to " + to_string(decl.id));
	Logger::deactivate();
}

void Interpreter::run_predicate(const Predicate& pred) {
	log("Running predicate...");
	Logger::activate();
	auto res = apply_function(pred.predicate,
							  parameters_to_arguments(pred.parameters));
	log("    result: " + to_string(get<ObjectBoolean>(res).value));
	Logger::deactivate();
}

void Interpreter::run_test(const Test& test) {
	log("Running test...");
	Logger::activate();
	const Regex& reg =
		holds_alternative<Regex>(test.test_set)
			? get<Regex>(test.test_set)
			: get<ObjectRegex>(objects[get<string>(test.test_set)]).value;

	if (holds_alternative<Regex>(test.language)) {
		Tester::test(get<Regex>(test.language), reg, test.iterations);
	}
	Logger::deactivate();
}

void Interpreter::run_operation(const GeneralOperation& op) {
	if (holds_alternative<Declaration>(op)) {
		run_declaration(get<Declaration>(op));
	} else if (holds_alternative<Predicate>(op)) {
		run_predicate(get<Predicate>(op));
	} else if (holds_alternative<Test>(op)) {
		run_test(get<Test>(op));
	}
}

optional<Interpreter::Declaration> Interpreter::scan_declaration(
	vector<Lexem> lexems) {

	if (lexems.size() < 3) {
		return nullopt;
	}

	Declaration decl;
	// [идентификатор]
	if (lexems.size() < 1 ||
		lexems[0].type != Lexem::id && lexems[0].type != Lexem::regex) {
		return nullopt;
	}
	decl.id = lexems[0].value;

	// =
	if (lexems.size() < 2 || lexems[1].type != Lexem::equalSign) {
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
		if (lexems[i].type == Lexem::id || // TODO: аццкий костыль
			lexems[i].type == Lexem::regex && id_types.count(lexems[i].value)) {
			if (id_types.count(lexems[i].value)) {
				argument_types.push_back(id_types[lexems[i].value]);
				arguments.push_back(lexems[i].value);
			} else {
				log("Scan declaration: unknown id: " + lexems[i].value);
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

	if (arguments.size() == 0) {
		log("Scan declaration: no arguments given");
		return nullopt;
	}

	if (auto seq = build_function_sequence(func_names, argument_types);
		seq.has_value()) {

		decl.function_sequence = *seq;
		decl.parameters = arguments;
		id_types[decl.id] =
			(*seq).size() ? (*seq).back().output : argument_types[0];
		return decl;
	}

	log("Scan declaration: failed to build function sequence");
	return nullopt;
}

optional<Interpreter::Test> Interpreter::scan_test(vector<Lexem> lexems) {
	if (lexems.size() < 4) {
		return nullopt;
	}

	if (lexems[0].type != Lexem::test) {
		return nullopt;
	}

	Test test;
	if (lexems[1].type == Lexem::regex) {
		test.language = lexems[1].reg;
	} else if (lexems[1].type == Lexem::id) {
		test.language = lexems[1].value;
	} else {
		log("Scan test: wrong type at position 1, id or regex expeccted");
		return nullopt;
	}

	if (lexems[2].type == Lexem::regex) {
		test.test_set = lexems[2].reg;
	} else if (lexems[2].type == Lexem::id) {
		test.test_set = lexems[2].value;
	} else {
		log("Scan test: wrong type at position 2, id or regex expeccted");
		return nullopt;
	}

	if (lexems[3].type == Lexem::number) {
		test.iterations = lexems[3].num;
	} else {
		log("Scan test: wrong type at position 3, number expected");
		return nullopt;
	}

	return test;
}

optional<Interpreter::Predicate> Interpreter::scan_predicate(
	vector<Lexem> lexems) {
	if (lexems.size() < 2) {
		return nullopt;
	}

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
		if (lexems[i].type == Lexem::id || // TODO: аццкий костыль
			lexems[i].type == Lexem::regex && id_types.count(lexems[i].value)) {
			if (id_types.count(lexems[i].value)) {
				argument_types.push_back(id_types[lexems[i].value]);
				arguments.push_back(lexems[i].value);
			} else {
				log("Scan test: unknown id: " + lexems[i].value);
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
	log("Scan predicate: failed to build function sequence");
	return nullopt;
}

optional<Interpreter::GeneralOperation> Interpreter::scan_operation(
	vector<Lexem> lexems) {

	if (auto declaration = scan_declaration(lexems); declaration.has_value()) {
		return declaration;
	}
	if (auto predicate = scan_predicate(lexems); predicate.has_value()) {
		return predicate;
	}
	if (auto test = scan_test(lexems); test.has_value()) {
		return test;
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