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
				cout << "Unknown id";
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

optional<vector<Function>> Interpreter::build_function_sequence(
	vector<string> function_names, vector<ObjectType>) {
	return optional<vector<Function>>();
}
