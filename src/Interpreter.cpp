#include "Interpreter.h"

Interpreter::Interpreter() {}

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

	// [объект]+
	vector<ObjectType> argument_types;
	vector<variant<string, GeneralObject>> arguments;
	for (; i < lexems.size(); i++) {
		if (lexems[i].type == Lexem::id) {
			if (id_types.count(lexems[i].value)) {
				argument_types.push_back(id_types[lexems[i].value]);
			} else {
				cout << "Unknown id\n";
				return nullopt;
			}
		} else if (lexems[i].type == Lexem::regex) {
			arguments.push_back(ObjectRegex(lexems[i].reg));
		} else if (lexems[i].type == Lexem::regex) {
			arguments.push_back(ObjectRegex(lexems[i].reg));
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

optional<vector<Function>> Interpreter::build_function_sequence(
	vector<string> function_names, vector<ObjectType>) {
	return optional<vector<Function>>();
}
