#include "Interpreter/Interpreter.h"
#include "Tester/Tester.h"
#include <algorithm>

using namespace std;

bool operator==(const Interpreter::Function& l,
				const Interpreter::Function& r) {
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
		{"Intersect",
		 {{"Intersect", {ObjectType::NFA, ObjectType::NFA}, ObjectType::NFA}}},
		{"Union",
		 {{"Union", {ObjectType::NFA, ObjectType::NFA}, ObjectType::NFA}}},
		{"Difference",
		 {{"Difference", {ObjectType::NFA, ObjectType::NFA}, ObjectType::NFA}}},
		// Многосортные функции
		{"PumpLength", {{"PumpLength", {ObjectType::Regex}, ObjectType::Int}}},
		{"ClassLength", {{"ClassLength", {ObjectType::NFA}, ObjectType::Int}}},
		{"Normalize",
		 {{"Normalize",
		   {ObjectType::Regex, ObjectType::Array},
		   ObjectType::Regex}}},
		{"States", {{"States", {ObjectType::NFA}, ObjectType::Int}}},
		{"ClassCard", {{"ClassCard", {ObjectType::NFA}, ObjectType::Int}}},
		{"Ambiguity",
		 {{"Ambiguity", {ObjectType::NFA}, ObjectType::AmbiguityValue}}},
		{"MyhillNerode",
		 {{"MyhillNerode", {ObjectType::NFA}, ObjectType::Int}}},
		{"GlaisterShallit",
		 {{"GlaisterShallit", {ObjectType::NFA}, ObjectType::Int}}},
		{"PrefixGrammar",
		 {{"PrefixGrammar", {ObjectType::NFA}, ObjectType::PrefixGrammar}}},
		{"PGtoNFA",
		 {{"PGtoNFA", {ObjectType::PrefixGrammar}, ObjectType::NFA}}},
		// Предикаты
		{"Bisimilar",
		 {{"Bisimilar",
		   {ObjectType::NFA, ObjectType::NFA},
		   ObjectType::Boolean}}},
		{"Minimal", {{"Minimal", {ObjectType::NFA}, ObjectType::OptionalBool}}},
		// для dfa - bool, для nfa - optional<bool>
		{"Deterministic",
		 {{"Deterministic", {ObjectType::NFA}, ObjectType::Boolean}}},
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
		 {{"Equal",
		   {ObjectType::Regex, ObjectType::Regex},
		   ObjectType::Boolean},
		  {"Equal", {ObjectType::NFA, ObjectType::NFA}, ObjectType::Boolean},
		  {"Equal", {ObjectType::Int, ObjectType::Int}, ObjectType::Boolean},
		  {"Equal",
		   {ObjectType::AmbiguityValue, ObjectType::AmbiguityValue},
		   ObjectType::Boolean},
		  {"Equal",
		   {ObjectType::Boolean, ObjectType::Boolean},
		   ObjectType::Boolean}}},
		{"OneUnambiguity",
		 {{"OneUnambiguity", {ObjectType::Regex}, ObjectType::Boolean},
		  {"OneUnambiguity", {ObjectType::NFA}, ObjectType::Boolean}}},
		{"SemDet", {{"SemDet", {ObjectType::NFA}, ObjectType::Boolean}}}};
	// generate_brief_templates();
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

void Interpreter::generate_log(const string& filename) {
	tex_logger.render_to_file(filename);
}

bool Interpreter::set_flag(Flag key, bool value) {
	auto logger = init_log();
	if (flags.count(key)) {
		flags[key] = value;
	} else {
		logger.throw_error("set_flag::invalid flag id");
		return false;
	}
	return true;
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

optional<GeneralObject> Interpreter::apply_function_sequence(
	const vector<Function>& functions, vector<GeneralObject> arguments) {

	for (const auto& func : functions) {
		LogTemplate log_template;
		const auto& f = apply_function(func, arguments, log_template);
		if (f.has_value()) {
			arguments = {*f};
		} else {
			return nullopt;
		}
		if (flags[Flag::report]) tex_logger.add_log(log_template);
	}

	return arguments[0];
}

optional<GeneralObject> Interpreter::apply_function(
	const Function& function, const vector<GeneralObject>& arguments,
	LogTemplate& log_template) {

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

	// имя шаблона по умолчанию - название ф/и в интерпретаторе + номер
	// сигнатуры (если их несколько)
	string func_id = get_func_id(function);

	log_template.load_tex_template(func_id);
	log_template.set_theory_flag(flags[Flag::log_theory]);

	if (function.name == "Glushkov") {
		return ObjectNFA(
			get<ObjectRegex>(arguments[0]).value.to_glushkov(&log_template));
	}

	if (function.name == "IlieYu") {
		return ObjectNFA(
			get<ObjectRegex>(arguments[0]).value.to_ilieyu(&log_template));
	}
	if (function.name == "Antimirov") {
		return ObjectNFA(
			get<ObjectRegex>(arguments[0]).value.to_antimirov(&log_template));
	}
	if (function.name == "Thompson") {
		return ObjectNFA(
			get<ObjectRegex>(arguments[0]).value.to_thompson(&log_template));
	}
	if (function.name == "Arden") {
		return ObjectRegex(
			(get_automaton(arguments[0]).to_regex(&log_template)));
	}
	if (function.name == "Bisimilar") {
		return ObjectBoolean(FiniteAutomaton::bisimilar(
			get_automaton(arguments[0]), get_automaton(arguments[1]),
			&log_template));
	}
	if (function.name == "Minimal") {
		FiniteAutomaton a = get_automaton(arguments[0]);
		if (a.is_deterministic())
			return ObjectBoolean(a.is_dfa_minimal(&log_template));
		else
			return ObjectOptionalBool(a.is_nfa_minimal(&log_template));
	}
	if (function.name == "Deterministic") {
		return ObjectBoolean(
			get_automaton(arguments[0]).is_deterministic(&log_template));
	}
	if (function.name == "Subset") {
		if (function.input[0] == ObjectType::NFA) {
			return ObjectBoolean(
				(get_automaton(arguments[0])
					 .subset(get_automaton(arguments[1]), &log_template)));
		} else {
			return ObjectBoolean(
				get<ObjectRegex>(arguments[0])
					.value.subset(get<ObjectRegex>(arguments[1]).value,
								  &log_template));
		}
	}
	if (function.name == "Equiv") {

		if (function.input[0] == ObjectType::NFA) {
			return ObjectBoolean(FiniteAutomaton::equivalent(
				get_automaton(arguments[0]), get_automaton(arguments[1]),
				&log_template));
		} else {
			return ObjectBoolean(Regex::equivalent(
				get<ObjectRegex>(arguments[0]).value,
				get<ObjectRegex>(arguments[1]).value, &log_template));
		}
	}
	if (function.name == "Equal") {
		if (function.input[0] == ObjectType::NFA) {
			return ObjectBoolean(FiniteAutomaton::equal(
				get_automaton(arguments[0]), get_automaton(arguments[1]),
				&log_template));
		} else if (function.input[0] == ObjectType::Regex) {
			return ObjectBoolean(Regex::equal(
				get<ObjectRegex>(arguments[0]).value,
				get<ObjectRegex>(arguments[1]).value, &log_template));
			// TODO для кратких шаблонов
		} else if (function.input[0] == ObjectType::Int) {
			int value1 = get<ObjectInt>(arguments[0]).value;
			int value2 = get<ObjectInt>(arguments[1]).value;
			bool res = (value1 == value2);
			log_template.set_parameter("value1", value1);
			log_template.set_parameter("value2", value2);
			log_template.set_parameter("result", res);
			return ObjectBoolean(res);
		} else if (function.input[0] == ObjectType::Boolean) {
			int value1 = get<ObjectBoolean>(arguments[0]).value;
			int value2 = get<ObjectBoolean>(arguments[1]).value;
			bool res = (value1 == value2);
			log_template.set_parameter("value1", value1);
			log_template.set_parameter("value2", value2);
			log_template.set_parameter("result", res);
			return ObjectBoolean(res);
		} else {
			FiniteAutomaton::AmbiguityValue value1 =
				get<ObjectAmbiguityValue>(arguments[0]).value;
			FiniteAutomaton::AmbiguityValue value2 =
				get<ObjectAmbiguityValue>(arguments[1]).value;
			bool res = (value1 == value2);
			log_template.set_parameter("value1", value1);
			log_template.set_parameter("value2", value2);
			log_template.set_parameter("result", res);
			return ObjectBoolean(res);
		}
	}
	if (function.name == "OneUnambiguity") {
		if (function.input[0] == ObjectType::NFA) {
			return ObjectBoolean(
				get_automaton(arguments[0]).is_one_unambiguous(&log_template));
		} else {
			return ObjectBoolean(get<ObjectRegex>(arguments[0])
									 .value.is_one_unambiguous(&log_template));
		}
	}
	if (function.name == "SemDet") {
		return ObjectBoolean(get_automaton(arguments[0]).semdet(&log_template));
	}
	if (function.name == "PumpLength") {
		log_template.load_tex_template("PumpLength");
		return ObjectInt(
			get<ObjectRegex>(arguments[0]).value.pump_length(&log_template));
	}
	TransformationMonoid trmon;
	if (function.name == "ClassLength") {
		trmon = TransformationMonoid(get_automaton(arguments[0]));
		return ObjectInt(trmon.class_length(&log_template));
	}
	if (function.name == "States") {
		return ObjectInt(
			get_automaton(arguments[0]).states_number(&log_template));
	}
	if (function.name == "ClassCard") {
		trmon = TransformationMonoid(get_automaton(arguments[0]));
		return ObjectInt(trmon.class_card(&log_template));
	}
	if (function.name == "Ambiguity") {
		return ObjectAmbiguityValue(
			get_automaton(arguments[0]).ambiguity(&log_template));
	}
	if (function.name == "MyhillNerode") {
		trmon = TransformationMonoid(get_automaton(arguments[0]));
		return ObjectInt(trmon.get_classes_number_MyhillNerode(&log_template));
	}
	if (function.name == "GlaisterShallit") {
		return ObjectInt(
			get_automaton(arguments[0])
				.get_classes_number_GlaisterShallit(&log_template));
	}
	if (function.name == "PrefixGrammar") {
		Grammar g;
		g.fa_to_prefix_grammar_TM(get_automaton(arguments[0]), &log_template);
		return ObjectPrefixGrammar(g);
	}
	if (function.name == "PGtoNFA") {
		return ObjectNFA(get<ObjectPrefixGrammar>(arguments[0])
							 .value.prefix_grammar_to_automaton(&log_template));
	}

	/*
	* Идёт Глушков по лесу, видит -- регулярка.
	Построил автомат и застрелился.
	*/

	// преобразования внутри класса:

	GeneralObject predres = arguments[0];
	optional<GeneralObject> res;

	if (function.name == "Determinize") {
		res = ObjectDFA(get_automaton(arguments[0]).determinize(&log_template));
	}
	if (function.name == "Determinize+") {
		log_template.load_tex_template("Determinize");
		res = ObjectDFA(
			get_automaton(arguments[0]).determinize(&log_template, false));
	}
	if (function.name == "Minimize") {
		res = ObjectDFA(get_automaton(arguments[0]).minimize(&log_template));
	}
	if (function.name == "Minimize+") {
		log_template.load_tex_template("Minimize");
		res = ObjectDFA(
			get_automaton(arguments[0]).minimize(&log_template, false));
	}
	if (function.name == "Annote") {
		res = ObjectDFA(get_automaton(arguments[0]).annote(&log_template));
	}
	if (function.name == "RemEps") {
		res = ObjectNFA(get_automaton(arguments[0]).remove_eps(&log_template));
	}
	if (function.name == "Linearize") {
		res = ObjectRegex(
			get<ObjectRegex>(arguments[0]).value.linearize(&log_template));
	}
	if (function.name == "Reverse") {
		res = ObjectNFA(get_automaton(arguments[0]).reverse(&log_template));
	}
	if (function.name == "DeLinearize") {
		if (function.output == ObjectType::Regex) {
			res = ObjectRegex(get<ObjectRegex>(arguments[0])
								  .value.delinearize(&log_template));
		} else {
			res = ObjectNFA(
				get_automaton(arguments[0]).delinearize(&log_template));
		}
	}
	if (function.name == "Complement") {
		// FiniteAutomaton fa = get_automaton(arguments[0]);
		// if (fa.is_deterministic())
		res = ObjectDFA(get_automaton(arguments[0]).complement(&log_template));
	}
	if (function.name == "RemoveTrap") {
		res = ObjectDFA(
			get_automaton(arguments[0]).remove_trap_states(&log_template));
	}
	if (function.name == "DeAnnote") {
		if (function.output == ObjectType::NFA) {
			// Пример: (пока в объявлении функции не добавила флаг)
			// res =
			// ObjectNFA(get_automaton(arguments[0]).deannote(&log_template,
			// Flag::auto_remove_trap_states));
			res =
				ObjectNFA(get_automaton(arguments[0]).deannote(&log_template));
		} else {
			res = ObjectRegex(
				get<ObjectRegex>(arguments[0]).value.deannote(&log_template));
		}
	}
	if (function.name == "MergeBisim") {
		res = ObjectNFA(
			get_automaton(arguments[0]).merge_bisimilar(&log_template));
	}
	if (function.name == "Normalize") {
		// Преобразуем array в массив пар
		const auto& arr = get<ObjectArray>(arguments[1]).value;
		vector<pair<Regex, Regex>> rules;
		for (const auto& object : arr) {
			if (holds_alternative<ObjectArray>(object)) {
				const auto& rule = get<ObjectArray>(object).value;
				if (rule.size() == 2 &&
					holds_alternative<ObjectRegex>(rule[0]) &&
					holds_alternative<ObjectRegex>(rule[1])) {
					rules.push_back({get<ObjectRegex>(rule[0]).value,
									 get<ObjectRegex>(rule[1]).value});
				} else {
					logger.throw_error("Normalize: invalid inner array");
					return nullopt;
				}
			} else {
				logger.throw_error("Normalize: invalid array");
				return nullopt;
			}
		}
		res = ObjectRegex(get<ObjectRegex>(arguments[0])
							  .value.normalize_regex(rules, &log_template));
	}
	if (function.name == "Disambiguate") {
		res = ObjectRegex(get<ObjectRegex>(arguments[0])
							  .value.get_one_unambiguous_regex(&log_template));
	}
	if (function.name == "Intersect") {
		res = ObjectNFA(FiniteAutomaton::intersection(
			get_automaton(arguments[0]), get_automaton(arguments[1]),
			&log_template));
	}
	if (function.name == "Union") {
		res = ObjectNFA(FiniteAutomaton::uunion(get_automaton(arguments[0]),
												get_automaton(arguments[1]),
												&log_template));
	}
	if (function.name == "Difference") {
		res = ObjectNFA(FiniteAutomaton::difference(get_automaton(arguments[0]),
													get_automaton(arguments[1]),
													&log_template));
	}

	if (res.has_value()) {
		GeneralObject resval = res.value();
		// Logger::activate_step_counter();

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

		// Logger::deactivate_step_counter();
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
							vector<ObjectType> argument_type) {

	// несовпдаение по кол-ву аргументов
	if (argument_type.size() != func_input_type.size()) return false;
	// сверяем тип каждого аргумента
	for (int i = 0; i < argument_type.size(); i++) {
		// тип либо одинаковый либо NFA<-DFA
		if (!((argument_type[i] == func_input_type[i]) ||
			  (argument_type[i] == ObjectType::DFA &&
			   func_input_type[i] == ObjectType::NFA) ||
			  // если включен флаг динамического тайпчека - принимать DFA<-NFA
			  (flags[Flag::weak_type_comparison] &&
			   argument_type[i] == ObjectType::NFA &&
			   func_input_type[i] == ObjectType::DFA) ||
			  // для верификатора гипотез (на место '*' - ставить Regex)
			  (argument_type[i] == ObjectType::RandomRegex &&
			   func_input_type[i] == ObjectType::Regex))) {
			// несовпадение по типам
			return false;
		}
	}
	return true;
}

optional<int> Interpreter::find_func(string func,
									 vector<ObjectType> argument_type) {
	// проходимся по всем вариантам сигнатуры функции
	for (int j = 0; j < names_to_functions[func].size(); j++) {
		// смотрим что каждый принимает на вход
		auto func_input_type = names_to_functions[func][j].input;
		// нашли совпадение по аргументам - возвращаем номер в массиве вариаций
		if (typecheck(func_input_type, argument_type)) return j;
	}
	return nullopt;
}

string Interpreter::get_func_id(Function function) {
	string id = function.name;
	if (names_to_functions[id].size() > 1) {
		id += to_string(find_func(function.name, function.input).value() + 1);
	}
	return id;
}

optional<vector<Interpreter::Function>> Interpreter::build_function_sequence(
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

	/* содержит информацию о каждой ф/и в посл-ти:
	 либо < 0 - ф/я не входит в итоговую посл-ть (не выполняется)
		"-1" - выполняет тождественное преобразование
		"-2" - излишнее действие в связи с последующей ф/ей
	 либо номер сигнатуры ф/и в таблице 'names_to_functions' */
	vector<int> needed_funcs(function_names.size(), 0);

	string argument_type = "";
	for (int i = 0; i < first_type.size(); i++)
		argument_type += (i == 0 ? "" : ", ") + types_to_string[first_type[i]];

	// устанавливаем тип для 1ой ф/и в посл-ти
	if (auto num = find_func(function_names[0], first_type); num.has_value()) {
		needed_funcs[0] = num.value();
	} else {
		logger.throw_error("mismatch by type of function \"" +
						   function_names[0] + "\": passed {" + argument_type +
						   "}");
		return nullopt;
	}

	string prev_func = function_names[0];
	ObjectType prev_type =
		names_to_functions[prev_func][needed_funcs[0]].output;

	for (int i = 1; i < function_names.size(); i++) {
		// запоминаем предыдущую функцию и ее тип
		if (needed_funcs[i - 1] != -1) {
			prev_func = function_names[i - 1];
			prev_type =
				names_to_functions[prev_func][needed_funcs[i - 1]].output;
		}
		string func = function_names[i];

		// поиск совпадения по типу (не нашли - бан)
		if (auto num = find_func(function_names[i], {prev_type});
			num.has_value()) {
			needed_funcs[i] = num.value();

			// удаление ненужных ф/й из посл-ти:
			if ((func == "Determinize" || func == "Annote") &&
				names_to_functions[prev_func][0].output == ObjectType::DFA) {
				needed_funcs[i] = -1;
				// удаление Annote и Determinize перед DFA
			}
			if (prev_func == "Minimize+" &&
				(func == "Minimize" || func == "Determinize+")) {
				needed_funcs[i] = -1;
			}
			if (prev_func == "Determinize" && func == "Minimize") {
				needed_funcs[i - 1] = -2;
			}
			if ((prev_func == "Determinize" || prev_func == "Determinize+") &&
				func == "Minimize+") {
				needed_funcs[i - 1] = -2;
			}
			if (prev_func == func) {
				if (func != "Reverse" && func != "Complement" &&
					func != "Linearize" && func != "DeLinearize" &&
					func != "DeAnnote") {
					needed_funcs[i] = -1;
					// удаление из последовательности повторений
				}
			}
			/*if (prev_func == "Linearize" &&
				(func == "Glushkov" || func == "IlieYu")) {
				needed_funcs[i - 1] = -1;
				// удаление Linearize перед Glushkov
			}*/
		} else {
			logger.throw_error("mismatch by type of function \"" + func +
							   "\": passed {" + types_to_string[prev_type] +
							   "}");
			return nullopt;
		}
	}

	string output_type;

	// собираем посл-ть
	optional<vector<Function>> finalfuncs = nullopt;
	finalfuncs.emplace() = {};
	for (int i = 0; i < function_names.size(); i++) {
		if (needed_funcs[i] >= 0) {
			Function f = names_to_functions[function_names[i]][needed_funcs[i]];
			finalfuncs.value().push_back(f);
			output_type = types_to_string[f.output];
			logger.log(f.name + " (type: {" + argument_type + "} -> " +
					   output_type + ")"); // можно убрать
			argument_type = output_type;
		} else {
			logger.log(function_names[i] +
					   " is skipped, it performs unnecessary transformation");
		}
	}

	return finalfuncs;
}

optional<GeneralObject> Interpreter::eval_expression(const Expression& expr) {
	auto logger = init_log();
	logger.log("Evaluating expression \"" + expr.to_txt() + "\"");

	if (expr.type == ObjectType::RandomRegex) {
		if (current_random_regex.has_value()) {
			return ObjectRegex(*current_random_regex);
		} else {
			return nullopt;
		}
	}
	if (holds_alternative<int>(expr.value)) {
		return ObjectInt(get<int>(expr.value));
	}
	if (holds_alternative<Regex>(expr.value)) {
		return ObjectRegex(get<Regex>(expr.value));
	}
	if (holds_alternative<Array>(expr.value)) {
		vector<GeneralObject> arr;
		for (const auto& e : get<Array>(expr.value)) {
			if (const auto& object = eval_expression(e); object.has_value()) {
				arr.push_back(*object);
			} else {
				logger.throw_error("invalid array member");
			}
		}
		return ObjectArray(arr);
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
	logger.throw_error("unknown expression type");
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
	/*if (decl.show_result) {
		// Logger::activate();
		logger.log("logger is activated for this task");
	} else {
		// Logger::deactivate();
	}*/
	if (const auto& expr = eval_expression(decl.expr); expr.has_value()) {
		objects[decl.id] = *expr;
	} else {
		logger.throw_error("while running declaration: invalid expression");
		return false;
	}
	logger.log("assigned to " + decl.id);
	// Logger::deactivate();
	return true;
}

bool Interpreter::run_predicate(const Predicate& pred) {
	auto logger = init_log();
	logger.log("");
	logger.log("Running predicate...");
	// Logger::activate();

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

	// Logger::deactivate();
	return success;
}

bool Interpreter::run_test(const Test& test) {
	auto logger = init_log();
	logger.log("");
	logger.log("Running test...");
	// Logger::activate();

	auto language = eval_expression(test.language);
	auto test_set = eval_expression(test.test_set);
	bool success = true;

	LogTemplate log_template;

	if (language.has_value() && test_set.has_value()) {
		auto reg = get<ObjectRegex>(*test_set).value;

		if (holds_alternative<ObjectRegex>(*language)) {
			log_template.load_tex_template("Test1");
			Tester::test(get<ObjectRegex>(*language).value, reg,
						 test.iterations, &log_template);
		} else if (holds_alternative<ObjectNFA>(*language)) {
			log_template.load_tex_template("Test2");
			Tester::test(get<ObjectNFA>(*language).value, reg, test.iterations,
						 &log_template);
		} else if (holds_alternative<ObjectDFA>(*language)) {
			log_template.load_tex_template("Test2");
			Tester::test(get<ObjectDFA>(*language).value, reg, test.iterations,
						 &log_template);
		} else {
			logger.throw_error(
				"while running test: invalid language expression");
			success = false;
		}
	} else {
		logger.throw_error("while running test: invalid arguments");
		success = false;
	}

	tex_logger.add_log(log_template);

	// Logger::deactivate();
	return success;
}

bool Interpreter::run_verification(const Verification& verification) {
	auto logger = init_log();
	logger.log("");
	logger.log("Running verification...");
	bool success = true;
	int results = 0;
	int tests_size = verification.size;
	int tests_false_num = min(10, (int)ceil(verification.size * 0.1));
	vector<string> regex_list;
	RegexGenerator RG; // TODO: менять параметры
	Expression expr = verification.predicate;

	LogTemplate log_template;
	log_template.load_tex_template("Verify");
	log_template.set_theory_flag(flags[Flag::log_theory]);
	log_template.set_parameter("expr", expr.to_txt());

	set_log_mode(LogMode::errors);
	flags[Flag::report] = false;

	for (int i = 0; i < verification.size; i++) {
		// подстановка равных Regex на место '*'
		current_random_regex =
			Regex(RG.generate_regex()); // хз как еще передавать
		auto predicate = eval_expression(expr);

		if (predicate.has_value()) {
			bool res = get<ObjectBoolean>(*predicate).value;
			results += res;
			if (!res && tests_false_num > 0) {
				regex_list.push_back(current_random_regex->to_txt());
				tests_false_num--;
			}
		} else {
			logger.throw_error("while running verification: invalid arguments");
			success = false;
			break;
		}
	}

	flags[Flag::report] = true;
	set_log_mode(LogMode::all);

	current_random_regex = nullopt;

	string res = to_string(100 * results / tests_size);
	logger.log("result: " + res + "%");
	log_template.set_parameter("result", res + +"\\%");

	if (results < tests_size) {
		logger.log("");
		logger.log("Tests with negative result:");
		string neg_tests = "";
		for (string str : regex_list) {
			logger.log(str);
			neg_tests += str + "\\\\";
		}
		log_template.set_parameter("neg tests", neg_tests);
	}

	tex_logger.add_log(log_template);

	return success;
}

bool Interpreter::run_set_flag(const SetFlag& flag) {
	auto logger = init_log();
	logger.log("");
	if (flags_names.count(flag.name)) {
		Flag flag_name = flags_names[flag.name];
		flags[flag_name] = flag.value;
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
	} else if (holds_alternative<SetFlag>(op)) {
		success = run_set_flag(get<SetFlag>(op));
	} else if (holds_alternative<Verification>(op)) {
		success = run_verification(get<Verification>(op));
	}
	return success;
}

string Interpreter::FunctionSequence::to_txt() const {
	string str = "(";
	for (int i = (int)functions.size() - 1; i >= 0; i--) {
		str += functions[i].name + (i == 0 ? " " : ".");
	}
	for (int i = 0; i < parameters.size(); i++) {
		str += parameters[i].to_txt() + (i == parameters.size() - 1 ? "" : " ");
	}
	str += ")";
	return str;
}

string Interpreter::Expression::to_txt() const {
	if (type == ObjectType::RandomRegex) {
		return "*";
	}
	if (const auto* pval = get_if<FunctionSequence>(&value)) {
		return pval->to_txt();
	}
	if (const auto* pval = get_if<int>(&value)) {
		return to_string(*pval);
	}
	if (const auto* pval = get_if<Regex>(&value)) {
		return "{" + pval->to_txt() + "}";
	}
	if (const auto* pval = get_if<string>(&value)) {
		return *pval;
	}
	if (const auto* pval = get_if<Array>(&value)) {
		string str = "[";
		for (int i = 0; i < pval->size(); i++) {
			str += (*pval)[i].to_txt() + (i == pval->size() - 1 ? "" : " ");
		}
		str += "]";
		return str;
	}
	return "";
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
	return (int)pos - 1;
}

optional<Interpreter::Id> Interpreter::scan_id(const vector<Lexem>& lexems,
											   int& pos, size_t end) {
	if (end > pos && lexems[pos].type == Lexem::name) {
		pos += 1;
		return lexems[pos].value;
	}
	return nullopt;
}

optional<Regex> Interpreter::scan_regex(const vector<Lexem>& lexems, int& pos,
										size_t end) {
	if (end > pos && lexems[pos].type == Lexem::regex) {
		pos += 1;
		return Regex(lexems[pos].value);
	}
	return nullopt;
}

optional<Interpreter::FunctionSequence> Interpreter::scan_function_sequence(
	const vector<Lexem>& lexems, int& pos, size_t end) {
	auto logger = init_log();

	int i = pos;

	// !!
	bool show_res = false;
	if (lexems[end - 1].type == Lexem::doubleExclamation) {
		show_res = true;
		end--;
	}

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
		if (const auto& expr = scan_expression(lexems, i, end);
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
		seq.show_result = show_res;
		pos = i + show_res;
		return seq;
	} else {
		logger.throw_error("failed to build function sequence");
	}

	return nullopt;
}

optional<Interpreter::Array> Interpreter::scan_array(
	const vector<Lexem>& lexems, int& pos, size_t end) {
	auto logger = init_log();

	int i = pos;

	// Начинается с [
	if (lexems[i].type == Lexem::bracketL) {
		i++;
	} else {
		return nullopt;
	}

	Array arr;
	while (i < end && lexems[i].type != Lexem::bracketR) {
		if (auto expr = scan_expression(lexems, i, end); expr.has_value()) {
			arr.push_back(*expr);
		} else {
			logger.throw_error("unable to scan expression in array");
			return nullopt;
		}
	}

	// Кончается на ]
	if (i < end && lexems[i].type == Lexem::bracketR) {
		i++;
	} else {
		logger.throw_error("unable to scan array: \"]\" expected");
		return nullopt;
	}

	pos = i;

	return arr;
}

optional<Interpreter::Expression> Interpreter::scan_expression(
	const vector<Lexem>& lexems, int& pos, size_t end) {
	// ( Expr )
	if (end > pos && lexems[pos].type == Lexem::parL) {
		end = find_closing_par(lexems, pos);
		pos++;
		const auto& expr = scan_expression(lexems, pos, end);
		pos++;
		return expr;
	}
	Expression expr;
	// Int
	if (end > pos && lexems[pos].type == Lexem::number) {
		expr.type = ObjectType::Int;
		expr.value = lexems[pos].num;
		pos++;
		return expr;
	}
	// Regex
	if (end > pos && lexems[pos].type == Lexem::regex) {
		expr.type = ObjectType::Regex;
		expr.value = Regex(lexems[pos].value);
		pos++;
		return expr;
	}
	// Id
	if (end > pos && lexems[pos].type == Lexem::name &&
		id_types.count(lexems[pos].value)) {
		expr.type = id_types[lexems[pos].value];
		expr.value = lexems[pos].value;
		pos++;
		return expr;
	}
	// String
	if (end > pos && lexems[pos].type == Lexem::stringval) {
		expr.type = ObjectType::String;
		expr.value = lexems[pos].value;
		pos++;
		return expr;
	}
	// Star (RandomRegex)
	if (end > pos && lexems[pos].type == Lexem::star) {
		expr.type = ObjectType::RandomRegex;
		pos++;
		return expr;
	}
	// FunctionSequence
	int i = pos;
	if (const auto& seq = scan_function_sequence(lexems, i, end);
		seq.has_value()) {
		expr.type = (*seq).functions.back().output;
		expr.value = *seq;
		pos = i;
		return expr;
	}
	i = pos;
	// Array
	if (const auto& arr = scan_array(lexems, i, end); arr.has_value()) {
		expr.type = ObjectType::Array;
		expr.value = *arr;
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
	if (const auto& expr = scan_expression(lexems, i, lexems.size());
		expr.has_value()) {
		decl.expr = *expr;
	} else {
		return nullopt;
	}

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
	if (const auto& expr = scan_expression(lexems, i, lexems.size());
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
	if (const auto& expr = scan_expression(lexems, i, lexems.size());
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

	if (auto seq = scan_function_sequence(lexems, pos, lexems.size());
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

optional<Interpreter::SetFlag> Interpreter::scan_flag(
	const vector<Lexem>& lexems, int& pos) {

	auto logger = init_log();
	int i = pos;

	if (lexems.size() < i + 2 || lexems[i].type != Lexem::name ||
		lexems[i].value != "Set") {
		return nullopt;
	}
	SetFlag flag;
	i++;
	if (lexems[i].type == Lexem::name) {
		flag.name = lexems[i].value;
	} else {
		logger.throw_error("Scan \"Set\": wrong flagName at position 1");
		return nullopt;
	}
	i++;
	if (lexems[i].type == Lexem::name &&
		(lexems[i].value == "true" || lexems[i].value == "false")) {
		if (lexems[i].value == "true")
			flag.value = true;
		else
			flag.value = false;
	} else {
		logger.throw_error(
			"Scan \"Set\": wrong type at position 2, boolean expected");
		return nullopt;
	}
	pos = i + 1;
	return flag;
}

optional<Interpreter::Verification> Interpreter::scan_verification(
	const vector<Lexem>& lexems, int& pos) {

	auto logger = init_log();
	int i = pos;

	if (lexems.size() < i + 1 || lexems[i].type != Lexem::name ||
		lexems[i].value != "Verify") {
		return nullopt;
	}
	i++;

	Verification verification;
	// Predicate
	if (const auto& expr = scan_expression(lexems, i, lexems.size());
		expr.has_value() && ((*expr).type == ObjectType::Boolean ||
							 (*expr).type == ObjectType::OptionalBool)) {
		verification.predicate = *expr;
	} else {
		logger.throw_error(
			"Scan verification: wrong type at position 1, predicate expected");
		return nullopt;
	}
	// tests size
	if (lexems.size() > i)
		if (lexems[i].type == Lexem::number) {
			verification.size = lexems[i].num;
		} else {
			logger.throw_error(
				"Scan verification: wrong type at position 2, number expected");
			return nullopt;
		}
	i++;

	pos = i;
	return verification;
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
	if (auto verification = scan_verification(lexems, pos);
		verification.has_value()) {
		return verification;
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