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
		{"Determinize+",
		 {{"Determinize+", {ObjectType::NFA}, ObjectType::DFA}}},
		{"RemEps", {{"RemEps", {ObjectType::NFA}, ObjectType::NFA}}},
		{"Linearize", {{"Linearize", {ObjectType::Regex}, ObjectType::Regex}}},
		{"Minimize", {{"Minimize", {ObjectType::NFA}, ObjectType::DFA}}},
		{"Minimize+", {{"Minimize+", {ObjectType::NFA}, ObjectType::DFA}}},
		{"Reverse", {{"Reverse", {ObjectType::NFA}, ObjectType::NFA}}},
		{"Annote", {{"Annote", {ObjectType::NFA}, ObjectType::DFA}}},
		{"DeLinearize",
		 {{"DeLinearize", {ObjectType::Regex}, ObjectType::Regex},
		  {"DeLinearize", {ObjectType::NFA}, ObjectType::NFA}}},
		{"Complement", {{"Complement", {ObjectType::DFA}, ObjectType::DFA}}},
		{"RemoveTrap", {{"RemoveTrap", {ObjectType::DFA}, ObjectType::DFA}}},
		{"DeAnnote",
		 {{"DeAnnote", {ObjectType::Regex}, ObjectType::Regex},
		  {"DeAnnote", {ObjectType::NFA}, ObjectType::NFA}}},
		{"MergeBisim", {{"MergeBisim", {ObjectType::NFA}, ObjectType::NFA}}},
		{"Disambiguate",
		 {{"Disambiguate", {ObjectType::Regex}, ObjectType::Regex}}},
		// Многосортные функции
		{"PumpLength", {{"PumpLength", {ObjectType::Regex}, ObjectType::Int}}},
		{"ClassLength", {{"ClassLength", {ObjectType::DFA}, ObjectType::Int}}},
		{"Normalize",
		 {{"Normalize",
		   {ObjectType::Regex, ObjectType::FileName},
		   ObjectType::Regex}}},
		{"States", {{"States", {ObjectType::NFA}, ObjectType::Int}}},
		{"ClassCard", {{"ClassCard", {ObjectType::DFA}, ObjectType::Int}}},
		{"Ambiguity",
		 {{"Ambiguity", {ObjectType::NFA}, ObjectType::AmbiguityValue}}},
		{"MyhillNerode",
		 {{"MyhillNerode", {ObjectType::DFA}, ObjectType::Int}}},
		{"GlaisterShallit",
		 {{"GlaisterShallit", {ObjectType::NFA}, ObjectType::Int}}},
		{"PrefixGrammar",
		 {{"PrefixGrammar", {ObjectType::NFA}, ObjectType::PrefixGrammar}}},
		// Предикаты
		{"Bisimilar",
		 {{"Bisimilar",
		   {ObjectType::NFA, ObjectType::NFA},
		   ObjectType::Boolean}}},
		{"Minimal", {{"Minimal", {ObjectType::NFA}, ObjectType::OptionalBool}}},
		// для dfa - bool, для nfa - optional<bool>
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
		{"OneUnambiguity",
		 {{"OneUnambiguity", {ObjectType::Regex}, ObjectType::Boolean},
		  {"OneUnambiguity", {ObjectType::NFA}, ObjectType::Boolean}}},
		{"SemDet", {{"SemDet", {ObjectType::NFA}, ObjectType::Boolean}}}};
}

bool Interpreter::run_line(const string& line) {
	auto logger = init_log();
	Lexer lexer(*this);
	logger.log("running \"" + line + "\"");
	auto lexems = lexer.parse_string(line);
	bool success = false;
	if (const auto op = scan_operation(lexems); op.has_value()) {
		success = run_operation(*op);
	} else {
		logger.throw_error("failed to scan operation");
		success = false;
	}
	logger.log("");
	return success;
}

bool Interpreter::run_file(const string& path) {
	auto logger = init_log();
	logger.log("opening file " + path);
	ifstream input_file(path);
	if (!input_file) {
		logger.throw_error("failed to open " + path);
		return false;
	}
	logger.log("file opened");

	string str = "";
	while (getline(input_file, str)) {
		if (!run_line(str)) {
			logger.throw_error("failed to run string \"" + str + "\"");
			return false;
		}
	}

	input_file.close();
	logger.log("successfully interpreted " + path);

	return true;
}

void Interpreter::set_log_mode(LogMode mode) {
	log_mode = mode;
}

void Interpreter::InterpreterLogger::log(const string& str) {
	if (parent.log_mode == LogMode::all) {
		for (int i = 1; i < parent.log_nesting; i++) {
			cout << "|  ";
		}
	}
	if (parent.log_mode == LogMode::all) {
		cout << str << "\n";
	}
}

void Interpreter::InterpreterLogger::throw_error(const string& str) {
	if (parent.log_mode == LogMode::all) {
		for (int i = 1; i < parent.log_nesting; i++) {
			cout << "|  ";
		}
	}
	if (parent.log_mode != LogMode::nothing) {
		cout << "ERROR: " << str << "\n";
	}
	parent.error = true;
}

Interpreter::InterpreterLogger Interpreter::init_log() {
	return InterpreterLogger(*this);
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

	auto logger = init_log();
	logger.log("running function \"" + function.name + "\"");

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
		return ObjectRegex((get_automaton(arguments[0]).to_regex()));
	}
	if (function.name == "Thompson") {
		return ObjectNFA(get<ObjectRegex>(arguments[0]).value.to_thompson());
	}
	if (function.name == "Bisimilar") {
		return ObjectBoolean(FiniteAutomaton::bisimilar(
			get_automaton(arguments[0]), get_automaton(arguments[1])));
	}
	if (function.name == "Minimal") {
		FiniteAutomaton a = get_automaton(arguments[0]);
		if (a.is_deterministic())
			return ObjectBoolean(a.is_dfa_minimal());
		else
			return ObjectOptionalBool(a.is_nfa_minimal());
	}
	if (function.name == "Subset") {
		if (vector<ObjectType> sign = {ObjectType::NFA, ObjectType::NFA};
			function.input == sign) {
			return ObjectBoolean((get_automaton(arguments[0])
									  .subset(get_automaton(arguments[1]))));
		} else {
			return ObjectBoolean(
				get<ObjectRegex>(arguments[0])
					.value.subset(get<ObjectRegex>(arguments[1]).value));
		}
	}
	if (function.name == "Equiv") {
		vector<ObjectType> n = {ObjectType::NFA, ObjectType::NFA};
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
		if (vector<ObjectType> sign = {ObjectType::NFA, ObjectType::NFA};
			function.input == sign) {
			return ObjectBoolean(FiniteAutomaton::equal(
				get_automaton(arguments[0]), get_automaton(arguments[1])));
		} else {
			return ObjectBoolean(
				Regex::equal(get<ObjectRegex>(arguments[0]).value,
							 get<ObjectRegex>(arguments[1]).value));
		}
	}
	if (function.name == "OneUnambiguity") {
		if (vector<ObjectType> sign = {ObjectType::NFA};
			function.input == sign) {
			return ObjectBoolean(
				get_automaton(arguments[0]).is_one_unambiguous());
		} else {
			return ObjectBoolean(
				get<ObjectRegex>(arguments[0]).value.is_one_unambiguous());
		}
	}
	if (function.name == "SemDet") {
		return ObjectBoolean(get_automaton(arguments[0]).semdet());
	}
	if (function.name == "PumpLength") {
		return ObjectInt(get<ObjectRegex>(arguments[0]).value.pump_length());
	}
	TransformationMonoid trmon;
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
		return ObjectAmbiguityValue(get_automaton(arguments[0]).ambiguity());
	}
	/*if (function.name == "Width") {
		return ObjectInt(get<ObjectNFA>(arguments[0]).value.);
	}*/
	if (function.name == "MyhillNerode") {
		trmon = TransformationMonoid(get_automaton(arguments[0]));
		return ObjectInt(trmon.get_classes_number_MyhillNerode());
	}
	if (function.name == "GlaisterShallit") {
		return ObjectInt(
			get_automaton(arguments[0]).get_classes_number_GlaisterShallit());
	}
	if (function.name == "PrefixGrammar") {
		Grammar g;
		g.fa_to_prefix_grammar(get_automaton(arguments[0]));
		return ObjectPrefixGramnar(g);
	}

	/*
	* Идёт Глушков по лесу, видит -- регулярка.
	Построил автомат и застрелился.
	*/

	// преобразования внутри класса:

	GeneralObject predres = arguments[0];
	optional<GeneralObject> res;

	if (function.name == "Determinize") {
		res = ObjectDFA(get_automaton(arguments[0]).determinize());
	}
	if (function.name == "Determinize+") {
		res = ObjectDFA(get_automaton(arguments[0]).determinize(false));
	}
	if (function.name == "Minimize") {
		res = ObjectDFA(get_automaton(arguments[0]).minimize());
	}
	if (function.name == "Minimize+") {
		res = ObjectDFA(get_automaton(arguments[0]).minimize(false));
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
		if (function.output == ObjectType::Regex) {
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
	if (function.name == "RemoveTrap") {
		res =
			ObjectDFA(get<ObjectDFA>(arguments[0]).value.remove_trap_states());
	}
	if (function.name == "DeAnnote") {
		if (function.output == ObjectType::NFA) {
			// Пример: (пока в объявлении функции не добавила флаг)
			// res = ObjectNFA(get_automaton(arguments[0]).deannote(is_trim));
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
	if (function.name == "Disambiguate") {
		res = ObjectRegex(
			get<ObjectRegex>(arguments[0]).value.get_one_unambiguous_regex());
	}

	if (res.has_value()) {
		GeneralObject resval = res.value();
		Logger::activate_step_counter();

		if (holds_alternative<ObjectRegex>(resval) &&
			holds_alternative<ObjectRegex>(predres)) {
			if (Regex::equal(get<ObjectRegex>(resval).value,
							 get<ObjectRegex>(predres).value))
				logger.log("function \"" + function.name +
						   "\" has left regex unchanged");
		}

		if (is_automaton(resval) && is_automaton(predres))
			if (FiniteAutomaton::equal(get_automaton(resval),
									   get_automaton(predres))) {
				logger.log("function \"" + function.name +
						   "\" has left automaton unchanged");
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

	auto logger = init_log();

	// Если функций нет - возвращаем пустой вектор
	if (!function_names.size()) {
		return vector<Function>();
	}

	// Проверка корректности названий
	for (const auto& func : function_names) {
		if (!names_to_functions.count(func)) {
			logger.throw_error("unknown function name \"" + func + "\"");
			return nullopt;
		}
	}

	// TODO: переписать всё что ниже начисто
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
						// Determinize+ добавляет ловушку после Annote
						if ((func == "Determinize" || func == "Annote") &&
							names_to_functions[predfunc][0].output ==
								ObjectType::DFA) {
							neededfuncs[i] = 0;
							// удаление Annote и Determinize перед DFA
						}
						if (predfunc == "Minimize" && func == "Minimize") {
							neededfuncs[i] = 0;
						}
						if (predfunc == "Minimize+" &&
							(func == "Minimize" || func == "Minimize+" ||
							 func == "Determinize+")) {
							neededfuncs[i] = 0;
						}
						if (predfunc == "Determinize+" &&
							func == "Determinize+") {
							neededfuncs[i] = 0;
						}
						if (predfunc == "Determinize" && func == "Minimize") {
							neededfuncs[i - 1] = 0;
						}
						if ((predfunc == "Determinize" ||
							 predfunc == "Determinize+") &&
							func == "Minimize+") {
							neededfuncs[i - 1] = 0;
						}
					}
				} else {
					if (predfunc == func) {
						if (func != "Reverse" && func != "Complement" &&
							func != "Linearize") {
							neededfuncs[i - 1] = 0;
						}
					} /*else {
						if (predfunc == "Linearize" &&
							(func == "Glushkov" || func == "IlieYu")) {
							neededfuncs[i - 1] = 0;
							// удаление Linearize перед Glushkov
						}
					}*/
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
					// neededfuncs[i - 1] = 0;
					// удаление из последовательности повторений:
					// DeLinearize.DeLinearize.DeAnnote == DeLinearize
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

optional<GeneralObject> Interpreter::eval_expression(const Expression& expr) {

	if (holds_alternative<int>(expr.value)) {
		return ObjectInt(get<int>(expr.value));
	}
	if (holds_alternative<Regex>(expr.value)) {
		return ObjectRegex(get<Regex>(expr.value));
	}
	if (holds_alternative<Id>(expr.value)) {
		Id id = get<Id>(expr.value);
		if (objects.count(id)) {
			return objects[id];
		} else {
			auto logger = init_log();
			logger.throw_error("evaluating expression: unknown id \"" + id +
							   "\"");
		}
		return nullopt;
	}
	if (holds_alternative<FunctionSequence>(expr.value)) {
		return eval_function_sequence(get<FunctionSequence>(expr.value));
	}
	return nullopt;
}

optional<GeneralObject> Interpreter::eval_function_sequence(
	const FunctionSequence& seq) {

	auto logger = init_log();

	logger.log("evaluating function sequence");

	vector<GeneralObject> args;
	for (const auto& expr : seq.parameters) {
		if (const auto& arg = eval_expression(expr); arg.has_value()) {
			args.push_back(*arg);
		} else {
			logger.throw_error(
				"while evaluating function sequence: invalid expression");
			return nullopt;
		}
	}

	const auto& expr = apply_function_sequence(seq.functions, args);
	logger.log("function sequence evaluated");
	return expr;
}

bool Interpreter::run_declaration(const Declaration& decl) {
	auto logger = init_log();
	logger.log("");
	logger.log("Running declaration...");
	if (decl.show_result) {
		Logger::activate();
		logger.log("logger is activated for this task");
	} else {
		Logger::deactivate();
	}
	if (const auto& expr = eval_expression(decl.expr); expr.has_value()) {
		objects[decl.id] = *expr;
	} else {
		logger.throw_error("while running declaration: invalid expression");
		return false;
	}
	logger.log("assigned to " + decl.id);
	Logger::deactivate();
	return true;
}

bool Interpreter::run_predicate(const Predicate& pred) {
	auto logger = init_log();
	logger.log("");
	logger.log("Running predicate...");
	Logger::activate();

	FunctionSequence seq;
	seq.functions = {pred.predicate};
	seq.parameters = pred.arguments;

	auto res = eval_function_sequence(seq);
	bool success = false;

	if (res.has_value() && holds_alternative<ObjectBoolean>(*res)) {
		logger.log("result: " + to_string(get<ObjectBoolean>(*res).value));
		success = true;
	} else if (res.has_value() && holds_alternative<ObjectOptionalBool>(*res)) {
		string result = "Unknown";
		if (get<ObjectOptionalBool>(*res).value)
			result = "1"; // true
		else if (get<ObjectOptionalBool>(*res).value.has_value())
			result = "0"; // false
		logger.log("result: " + result);
		success = true;
	} else {
		logger.throw_error("while running predicate: invalid expression");
		success = false;
	}

	Logger::deactivate();
	return success;
}

bool Interpreter::run_test(const Test& test) {
	auto logger = init_log();
	logger.log("");
	logger.log("Running test...");
	Logger::activate();

	auto language = eval_expression(test.language);
	auto test_set = eval_expression(test.test_set);
	bool success = true;

	if (language.has_value() && test_set.has_value()) {
		auto reg = get<ObjectRegex>(*test_set).value;

		if (holds_alternative<ObjectRegex>(*language)) {
			Tester::test(get<ObjectRegex>(*language).value, reg,
						 test.iterations);
		} else if (holds_alternative<ObjectNFA>(*language)) {
			Tester::test(get<ObjectNFA>(*language).value, reg, test.iterations);
		} else if (holds_alternative<ObjectDFA>(*language)) {
			Tester::test(get<ObjectDFA>(*language).value, reg, test.iterations);
		} else {
			logger.throw_error(
				"while running test: invalid language expression");
			success = false;
		}
	} else {
		logger.throw_error("while running test: invalid arguments");
		success = false;
	}

	Logger::deactivate();
	return success;
}

bool Interpreter::set_flag(const Flag& flag) {
	auto logger = init_log();
	logger.log("");
	if (flag.name == "trim") {
		is_trim = flag.value;
	} else {
		logger.throw_error("while setting flag: wrong name \"" + flag.name +
						   "\"");
		return false;
	}
	logger.log("set flag \"" + flag.name + "\" = " + to_string(flag.value));
	return true;
}

bool Interpreter::run_operation(const GeneralOperation& op) {
	bool success = false;
	if (holds_alternative<Declaration>(op)) {
		success = run_declaration(get<Declaration>(op));
	} else if (holds_alternative<Predicate>(op)) {
		success = run_predicate(get<Predicate>(op));
	} else if (holds_alternative<Test>(op)) {
		success = run_test(get<Test>(op));
	} else if (holds_alternative<Flag>(op)) {
		success = set_flag(get<Flag>(op));
	}
	return success;
}

int Interpreter::find_closing_par(const vector<Lexem>& lexems, size_t pos) {
	size_t balance = 1;
	pos++;
	for (; pos < lexems.size() && balance > 0; pos++) {
		if (lexems[pos].type == Lexem::parL) {
			balance++;
		}
		if (lexems[pos].type == Lexem::parR) {
			balance--;
		}
	}
	return pos - 1;
}

optional<Interpreter::Id> Interpreter::scan_Id(const vector<Lexem>& lexems,
											   int& pos, size_t end) {
	if (end > pos && lexems[pos].type == Lexem::name) {
		pos += 1;
		return lexems[pos].value;
	}
	return nullopt;
}

optional<Regex> Interpreter::scan_Regex(const vector<Lexem>& lexems, int& pos,
										size_t end) {
	if (end > pos && lexems[pos].type == Lexem::regex) {
		pos += 1;
		return Regex(lexems[pos].value);
	}
	return nullopt;
}

optional<Interpreter::FunctionSequence> Interpreter::scan_FunctionSequence(
	const vector<Lexem>& lexems, int& pos, size_t end) {
	auto logger = init_log();

	int i = pos;

	// Функции
	// ([функция].)*[функция]?
	vector<string> func_names;
	for (; i < end && lexems[i].type == Lexem::name; i++) {
		func_names.push_back(lexems[i].value);
		i++;
		if (i >= end || lexems[i].type != Lexem::dot) {
			break;
		}
	}

	// Если функций нет - ошибка
	if (func_names.size() == 0) {
		return nullopt;
	}

	reverse(func_names.begin(), func_names.end());

	// Аргументы
	// [объект]+
	vector<ObjectType> argument_types;
	vector<Expression> arguments;
	while (i < end && lexems[i].type) {
		if (const auto& expr = scan_Expression(lexems, i, end);
			expr.has_value()) {
			argument_types.push_back((*expr).type);
			arguments.push_back(*expr);
		} else {
			return nullopt;
		}
	}

	// Если аргументов нет - ошибка
	if (arguments.size() == 0) {
		return nullopt;
	}

	// Построение функциональной последовательности
	logger.log("building function sequence");
	if (const auto& functions =
			build_function_sequence(func_names, argument_types);
		functions.has_value()) {
		FunctionSequence seq;
		seq.functions = *functions;
		seq.parameters = arguments;
		pos = i;
		return seq;
	} else {
		logger.throw_error("failed to build function sequence");
	}

	return nullopt;
}

optional<Interpreter::Expression> Interpreter::scan_Expression(
	const vector<Lexem>& lexems, int& pos, size_t end) {
	// ( Expr )
	if (end > pos && lexems[pos].type == Lexem::parL) {
		end = find_closing_par(lexems, pos);
		pos++;
		const auto& expr = scan_Expression(lexems, pos, end);
		pos++;
		return expr;
	}
	// Int
	if (end > pos && lexems[pos].type == Lexem::number) {
		Expression expr;
		expr.type = ObjectType::Int;
		expr.value = lexems[pos].num;
		pos++;
		return expr;
	}
	// Regex
	if (end > pos && lexems[pos].type == Lexem::regex) {
		Expression expr;
		expr.type = ObjectType::Regex;
		expr.value = Regex(lexems[pos].value);
		pos++;
		return expr;
	}
	// Id
	if (end > pos && lexems[pos].type == Lexem::name &&
		id_types.count(lexems[pos].value)) {
		Expression expr;
		expr.type = id_types[lexems[pos].value];
		expr.value = lexems[pos].value;
		pos++;
		return expr;
	}
	// FunctionSequence
	int i = pos;
	if (const auto& seq = scan_FunctionSequence(lexems, i, end);
		seq.has_value()) {
		Expression expr;
		expr.type = (*seq).functions.back().output;
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

	auto end = lexems.size();
	if (lexems[end - 1].type == Lexem::doubleExclamation) {
		end--;
	}

	// Expression
	if (const auto& expr = scan_Expression(lexems, i, end); expr.has_value()) {
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

	auto logger = init_log();
	int i = pos;

	if (lexems.size() < i + 1 || lexems[i].type != Lexem::name ||
		lexems[i].value != "Test") {
		return nullopt;
	}
	i++;

	Test test;
	// Language
	if (const auto& expr = scan_Expression(lexems, i, lexems.size());
		expr.has_value() &&
		((*expr).type == ObjectType::Regex || (*expr).type == ObjectType::DFA ||
		 (*expr).type == ObjectType::NFA)) {
		test.language = *expr;
	} else {
		logger.throw_error(
			"Scan test: wrong type at position 1, nfa or regex expected");
		return nullopt;
	}

	// Test set
	if (const auto& expr = scan_Expression(lexems, i, lexems.size());
		expr.has_value() && (*expr).type == ObjectType::Regex) {
		test.test_set = *expr;
	} else {
		logger.throw_error(
			"Scan test: wrong type at position 2, regex expected");
		return nullopt;
	}

	if (lexems.size() > i && lexems[i].type == Lexem::number) {
		test.iterations = lexems[i].num;
	} else {
		logger.log("Scan test: wrong type at position 3, number expected");
		return nullopt;
	}
	i++;

	pos = i;
	return test;
}

optional<Interpreter::Predicate> Interpreter::scan_predicate(
	const vector<Lexem>& lexems, int& pos) {

	int i = pos;
	Predicate pred;

	if (auto seq = scan_FunctionSequence(lexems, pos, lexems.size());
		seq.has_value() && (*seq).functions.size() == 1 &&
		((*seq).functions[0].output == ObjectType::Boolean ||
		 (*seq).functions[0].output == ObjectType::OptionalBool)) {

		pred.predicate = (*seq).functions[0];
		pred.arguments = (*seq).parameters;
		pos = i + 1;
		return pred;
	}

	return nullopt;
}

optional<Interpreter::Flag> Interpreter::scan_flag(const vector<Lexem>& lexems,
												   int& pos) {

	auto logger = init_log();
	int i = pos;

	if (lexems.size() < i + 2 || lexems[i].type != Lexem::name ||
		lexems[i].value != "SetFlag") {
		return nullopt;
	}
	Flag flag;
	i++;
	if (lexems[i].type == Lexem::name) {
		flag.name = lexems[i].value;
	} else {
		logger.throw_error("Scan SetFlag: wrong flagName at position 1");
		return nullopt;
	}
	i++;
	if (lexems[i].type == Lexem::name &&
		(lexems[i].value == "true" || lexems[i].value == "false")) {
		if (lexems[i - 1].value == "true")
			flag.value = true;
		else
			flag.value = false;
	} else {
		logger.throw_error(
			"Scan SetFlag: wrong type at position 2, boolean expected");
		return nullopt;
	}
	pos = i + 1;
	return flag;
}

optional<Interpreter::GeneralOperation> Interpreter::scan_operation(
	const vector<Lexem>& lexems) {

	auto logger = init_log();
	logger.log("scanning");

	int pos = 0;
	if (auto test = scan_test(lexems, pos); test.has_value()) {
		return test;
	}
	if (auto flag = scan_flag(lexems, pos); flag.has_value()) {
		return flag;
	}
	if (auto declaration = scan_declaration(lexems, pos);
		declaration.has_value()) {
		return declaration;
	}
	if (auto predicate = scan_predicate(lexems, pos); predicate.has_value()) {
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