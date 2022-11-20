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
		{"Normalize",
		 {{"Normalize",
		   {ObjectType::Regex, ObjectType::FileName},
		   ObjectType::Regex}}},
		{"States", {{"States", {ObjectType::NFA}, ObjectType::Int}}},
		{"ClassCard", {{"ClassCard", {ObjectType::DFA}, ObjectType::Int}}},
		{"Ambiguity", {{"Ambiguity", {ObjectType::NFA}, ObjectType::Value}}},
		{"MyhillNerode",
		 {{"MyhillNerode", {ObjectType::DFA}, ObjectType::Int}}},
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
		{"Equal",
		 {{"Equal", {ObjectType::NFA, ObjectType::NFA}, ObjectType::Boolean}}},
		{"SemDet", {{"SemDet", {ObjectType::NFA}, ObjectType::Boolean}}}};
}

bool Interpreter::run_line(const string& line) {
	Lexer lexer(*this);
	log("running \"" + line + "\"");
	auto lexems = lexer.parse_string(line);
 	if (const auto op = scan_operation(lexems); op.has_value()) {
		run_operation(*op);
	} else {
		throw_error("failed to scan operation");
		return false;
	}
	return true;
}

bool Interpreter::run_file(const string& path) {
	log("opening file " + path);
	ifstream input_file(path);
	if (!input_file) {
		throw_error("failed to open " + to_string(path));
		return false;
	}
	log("file opened");

	string str = "";
	while (getline(input_file, str)) {
		if (!run_line(str)) {
			throw_error("failed to run string \"" + str + "\"");
			return false;
		}
	}

	input_file.close();
	log("successfully interpreted " + path);

	return true;
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
	TransformationMonoid trmon;
	if (function.name == "Minimal") {
		trmon = TransformationMonoid(get_automaton(arguments[0]));
		return ObjectBoolean(trmon.is_minimal());
	}
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
	if (function.name == "ClassLength") {
		trmon = TransformationMonoid(get_automaton(arguments[0]));
		return ObjectInt(trmon.class_length());
	}
	if (function.name == "States") {
		return ObjectInt(get_automaton(arguments[0]).states_number());
	}
	if (function.name == "ClassCard") {
		trmon = TransformationMonoid(get_automaton(arguments[0]));
		return ObjectInt(trmon.class_card());
	}
	if (function.name == "Ambiguity") {
		return ObjectValue(get_automaton(arguments[0]).ambiguity());
	}
	/*if (function.name == "Width") {
		return ObjectInt(get<ObjectNFA>(arguments[0]).value.);
	}*/
	if (function.name == "MyhillNerode") {
		trmon = TransformationMonoid(get_automaton(arguments[0]));
		return ObjectInt(trmon.classes_number_MyhillNerode());
	}

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
		} else {
			// пусть будет так
			res = ObjectNFA(get<ObjectNFA>(arguments[0]).value.deannote());
		}
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
			} else {
				return nullopt;
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
						// Determinize добавляет ловушку после Annote
						// if ((func == "Determinize" || func == "Annote") &&
						if (func == "Annote" &&
							names_to_functions[predfunc][0].output ==
								ObjectType::DFA) {
							neededfuncs[i] = 0;
						}
						if (predfunc == "Minimize" &&
							(func == "Minimize" || func == "Determinize")) {
							neededfuncs[i] = 0;
						}
						if (predfunc == "Determinize" &&
							func == "Determinize") {
							neededfuncs[i] = 0;
						}
						if (predfunc == "Determinize" && func == "Minimize") {
							neededfuncs[i - 1] = 0;
						}
					}
				} else {
					if (predfunc == func) {
						if (predfunc != "Reverse" && predfunc != "Complement") {
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

			vector<ObjectType> regex_type = {ObjectType::Regex};
			vector<ObjectType> nfa_type = {ObjectType::NFA};
			vector<ObjectType> dfa_type = {ObjectType::DFA};

			if (names_to_functions[func].size() == 2) {
				// DeLinearize ~ DeAnnote
				if (names_to_functions[predfunc].size() != 2) {
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
				} else {
					neededfuncs[i] = neededfuncs[i - 1];
					neededfuncs[i - 1] = 0;
				}
			} else if (names_to_functions[predfunc].size() == 2) {
				if (names_to_functions[func][0].input == regex_type &&
						neededfuncs[i - 1] == 2 ||
					names_to_functions[func][0].input == nfa_type &&
						neededfuncs[i - 1] == 3) {
					neededfuncs[i] = 1;
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
	/*if (decl.show_result) {
		Logger::activate();
		log("    logger is activated for this task");
	} else {
		Logger::deactivate();
	}
	objects[decl.id] = apply_function_sequence(
		decl.function_sequence, parameters_to_arguments(decl.));
	log("    assigned to " + to_string(decl.id));*/
	Logger::deactivate();
}

void Interpreter::run_predicate(const Predicate& pred) {
	log("Running predicate...");
	/*Logger::activate();
	auto res = apply_function(pred.predicate,
							  parameters_to_arguments(pred.parameters));
	log("    result: " + to_string(get<ObjectBoolean>(res).value));*/
	Logger::deactivate();
}

void Interpreter::run_test(const Test& test) {
	log("Running test...");
	/*Logger::activate();
	const Regex& reg =
		holds_alternative<Regex>(test.test_set)
			? get<Regex>(test.test_set)
			: get<ObjectRegex>(objects[get<string>(test.test_set)]).value;

	if (holds_alternative<Regex>(test.language)) {
		Tester::test(get<Regex>(test.language), reg, test.iterations);
	}*/
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

optional<Interpreter::Id> Interpreter::scan_Id(const vector<Lexem>& lexems,
											   int& pos) {
	if (lexems.size() > pos && lexems[pos].type == Lexem::name) {
		pos += 1;
		return lexems[pos].value;
	}
	return nullopt;
}

optional<Regex> Interpreter::scan_Regex(const vector<Lexem>& lexems, int& pos) {
	if (lexems.size() > pos && lexems[pos].type == Lexem::regex) {
		pos += 1;
		return Regex(lexems[pos].value);
	}
	return nullopt;
}

optional<Interpreter::FunctionSequence> Interpreter::scan_FunctionSequence(
	const vector<Lexem>& lexems, int& pos) {

	int i = pos;

	// Открывающая скобка
	if (lexems.size() <= i || lexems[i].type != Lexem::parL) {
		return nullopt;
	}
	i++;

	// Функции
	// ([функция].)*[функция]?
	vector<string> func_names;
	for (; i < lexems.size() && lexems[i].type == Lexem::name; i++) {
		func_names.push_back(lexems[i].value);
		i++;
		if (lexems[i].type != Lexem::dot) {
			break;
		}
	}

	reverse(func_names.begin(), func_names.end());

	// Аргументы
	// [объект]+
	vector<ObjectType> argument_types;
	vector<Expression> arguments;
	while (i < lexems.size() && lexems[i].type != Lexem::parR) {
		if (const auto& expr = scan_Expression(lexems, i); expr.has_value()) {
			argument_types.push_back((*expr).type);
			arguments.push_back(*expr);
		}
	}

	// Если аргументов нет - ошибка
	if (arguments.size() == 0) {
		return nullopt;
	}

	// Построение функциональной последовательности
	if (const auto& functions =
			build_function_sequence(func_names, argument_types);
		functions.has_value()) {
		FunctionSequence seq;
		seq.functions = *functions;
		seq.parameters = arguments;
		i++; // Закрывающая скобка
		pos = i;
		return seq;
	}

	return nullopt;
}

optional<Interpreter::Expression> Interpreter::scan_Expression(
	const vector<Lexem>& lexems, int& pos) {
	// Int
	if (lexems.size() > pos && lexems[pos].type == Lexem::number) {
		Expression expr;
		expr.type = ObjectType::Int;
		expr.value = lexems[pos].num;
		pos++;
		return expr;
	}
	// Regex
	if (lexems.size() > pos && lexems[pos].type == Lexem::regex) {
		Expression expr;
		expr.type = ObjectType::Regex;
		expr.value = Regex(lexems[pos].value);
		pos++;
		return expr;
	}
	// Id
	if (lexems.size() > pos && lexems[pos].type == Lexem::name &&
		id_types.count(lexems[pos].value)) {
		Expression expr;
		expr.type = id_types[lexems[pos].value];
		expr.value = lexems[pos].value;
		pos++;
		return expr;
	}
	// FunctionSequence
	int i = pos;
	if (const auto& seq = scan_FunctionSequence(lexems, i); seq.has_value()) {
		Expression expr;
		expr.type = (*seq).functions[0].output;
		expr.value = *seq;
		pos = i;
		return expr;
	}
	return nullopt;
}

optional<Interpreter::Declaration> Interpreter::scan_declaration(
	const vector<Lexem>& lexems, int& pos) {

	if (lexems.size() < pos + 3) {
		return nullopt;
	}

	int i = pos;

	Declaration decl;
	// [идентификатор]
	if (lexems.size() < i || lexems[i].type != Lexem::name) {
		return nullopt;
	}
	decl.id = lexems[i].value;
	i++;

	// =
	if (lexems.size() < i || lexems[i].type != Lexem::equalSign) {
		return nullopt;
	}
	i++;

	// Expression
	if (const auto& expr = scan_Expression(lexems, i); expr.has_value()) {
		decl.expr = *expr;
	} else {
		return nullopt;
	}

	// (!!)
	if (i < lexems.size() && lexems[i].type == Lexem::doubleExclamation) {
		decl.show_result = true;
	}
	i++;

	id_types[decl.id] = decl.expr.type;

  	pos = i;
	return decl;
}

optional<Interpreter::Test> Interpreter::scan_test(const vector<Lexem>& lexems,
												   int& pos) {
	int i = pos;

	if (lexems.size() < i + 1 || lexems[i].type != Lexem::name ||
		lexems[i].value != "Test") {
		return nullopt;
	}

	Test test;
	// Language
	if (const auto& expr = scan_Expression(lexems, i);
		expr.has_value() &&
		((*expr).type == ObjectType::Regex || (*expr).type == ObjectType::DFA ||
		 (*expr).type == ObjectType::NFA)) {
		test.language = *expr;
	} else {
		throw_error(
			"Scan test: wrong type at position 1, nfa or regex expected");
		return nullopt;
	}

	// Test set
	if (const auto& expr = scan_Expression(lexems, i);
		expr.has_value() && (*expr).type == ObjectType::Regex) {
		test.test_set = *expr;
	} else {
		throw_error("Scan test: wrong type at position 2, regex expected");
		return nullopt;
	}

	if (lexems.size() > i && lexems[i].type == Lexem::number) {
		test.iterations = lexems[i].num;
	} else {
		log("Scan test: wrong type at position 3, number expected");
		return nullopt;
	}
	i++;

	pos = i;
	return test;
}

optional<Interpreter::Predicate> Interpreter::scan_predicate(
	const vector<Lexem>& lexems, int& pos) {
	// TODO scan predicate
	return nullopt;
}

optional<Interpreter::GeneralOperation> Interpreter::scan_operation(
	const vector<Lexem>& lexems) {
	int pos = 0;
	if (auto declaration = scan_declaration(lexems, pos);
		declaration.has_value()) {
		return declaration;
	}
	if (auto predicate = scan_predicate(lexems, pos); predicate.has_value()) {
		return predicate;
	}
	if (auto test = scan_test(lexems, pos); test.has_value()) {
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