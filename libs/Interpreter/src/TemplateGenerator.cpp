#include <string>

#include "Interpreter/Interpreter.h"

using std::cout;
using std::ofstream;
using std::string;
using std::to_string;
using std::vector;

bool types_equiv(const vector<ObjectType>& input, const ObjectType& output) {
	if ((output == input[0]) || (output == ObjectType::DFA && input[0] == ObjectType::NFA) ||
		(output == ObjectType::NFA && input[0] == ObjectType::DFA))
		return true;
	return false;
}

void Interpreter::generate_brief_templates() {
	for (const auto& func_item : names_to_functions) {
		for (const auto& function : func_item.second) {
			string func_id = get_func_id(function).value();
			string filename = "./resources/template/";
			filename += func_id + ".tex";
			ofstream outfile(filename);
			outfile << "\\section{" << function.name << "}\n";
			outfile << "\\begin{frame}{" << function.name << "}\n";

			bool equal_types = types_equiv(function.input, function.output);
			if (function.input.size() > 1) {
				equal_types = false;
			}
			// ========= Шаблон для одиночных входных данных =========

			if (function.input.size() == 1) {
				// Для автоматов
				if ((function.input[0] == ObjectType::NFA ||
					 function.input[0] == ObjectType::DFA)) {
					outfile << "\tАвтомат";

					if (equal_types) {
						outfile << " до преобразования";
					}

					outfile << ":\n\n\t%template_oldautomaton\n\n";
				}

				// Для префиксной грамматики
				if (function.input[0] == ObjectType::PrefixGrammar) {
					outfile << "\tПрефиксная грамматика:\n\n";
					outfile << "\t%template_grammar\n\n";
				}

				// Для регулярок
				if ((function.input[0] == ObjectType::Regex)) {
					outfile << "\tРегулярное выражение";

					if (equal_types) {
						outfile << " до преобразования";
					}

					outfile << ":\n\t%template_oldregex\n\n";
				}
			} else {

				// ========= Шаблон для парных входных данных ===========

				bool input_types_equal = types_equiv(function.input, function.input[1]);

				for (int index = 0; index < function.input.size(); index++) {

					// Для автоматов
					if ((function.input[index] == ObjectType::NFA ||
						 function.input[index] == ObjectType::DFA)) {
						if (input_types_equal) {
							if (index == 0) {
								outfile << "\tПервый автомат:\n\n";
								outfile << "\t%template_automaton" << index + 1 << "\n\n";
							} else {
								outfile << "\tВторой автомат:\n\n";
								outfile << "\t%template_automaton" << index + 1 << "\n\n";
							}
						} else {
							outfile << "\tАвтомат:\n\n";
							outfile << "\t%template_oldautomaton\n";
						}
					}

					// Для префиксной грамматики
					if (function.input[index] == ObjectType::PrefixGrammar) {
						outfile << "\tПрефиксная грамматика:\n\n";
						outfile << "\t%template_grammar\n\n";
					}

					// Для регулярок
					if (function.input[index] == ObjectType::Regex) {
						if (input_types_equal) {
							if (index == 0) {
								outfile << "\tПервое регулярное выражение:\n";
								outfile << "\t%template_regex" << index + 1 << "\n\n";
							} else {
								outfile << "\tВторое регулярное выражение:\n";
								outfile << "\t%template_regex" << index + 1 << "\n\n";
							}
						} else {
							outfile << "\tРегулярное выражение:\n";
							outfile << "\t%template_oldregex\n\n";
						}
					}

					// Для Array
					if (function.input[index] == ObjectType::Array) {
						outfile << "\tПравила переписывания:\n\n";
						outfile << "\t%template_oldarray\n\n";
					}

					// Для других типов
					if (function.input[index] == ObjectType::Int ||
						function.input[index] == ObjectType::AmbiguityValue ||
						function.input[index] == ObjectType::Boolean ||
						function.input[index] == ObjectType::OptionalBool) {
						if (input_types_equal) {
							outfile << "\t" << types_to_string.at(function.input[index]) << index + 1
									<< ":\n\n";
							outfile << "\t%template_value" << index + 1 << "\n\n";
						} else {
							outfile << "\t" << types_to_string.at(function.input[index]) << "\n\n";
							outfile << "\t%template_value\n\n";
						}
					}
				}
			}

			// ========= Шаблон для выходных данных ===================

			// Для автоматов
			if (function.output == ObjectType::NFA || function.output == ObjectType::DFA) {
				outfile << "\tАвтомат";

				if (equal_types) {
					outfile << " после преобразования";
				}

				outfile << ":\n\n\t%template_result\n\n";
			}

			// Для Regex, Int, Bool, optionalbool
			if (function.output == ObjectType::AmbiguityValue ||
				function.output == ObjectType::Int ||
				function.output == ObjectType::Boolean ||
				function.output == ObjectType::OptionalBool) {
				outfile << "\tРезультат:\n";

				outfile << "\t%template_result\n\n";
			}

			// Для префиксной грамматики
			if (function.output == ObjectType::PrefixGrammar) {
				outfile << "\tПрефиксная грамматика:\n\n";

				outfile << "\t%template_result\n\n";
			}

			// Для регулярок
			if (function.output == ObjectType::Regex) {
				outfile << "\tРегулярное выражение";

				if (equal_types) {
					outfile << " после преобразования";
				}

				outfile << ":\n\t%template_result\n\n";
			}

			outfile << "\\end{frame}\n";
			outfile.close();
		}
	}
	return;
}