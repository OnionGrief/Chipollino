#include "Interpreter/Interpreter.h"
#include <string>

bool types_equiv(vector<ObjectType> input, ObjectType output) {
	if (input.size() > 1) {
		return false;
	}
	if (!((output == input[0]) ||
		  (output == ObjectType::DFA && input[0] == ObjectType::NFA) ||
		  (output == ObjectType::NFA && input[0] == ObjectType::DFA))) {
		return false;
	}
	return true;
}

void Interpreter::generate_brief_templates() {
	for (auto func_item : names_to_functions) {
		for (auto function : func_item.second) {
			string func_id = get_func_id(function);
			string filename = "./resources/template/";
			filename += func_id + ".tex";
			ofstream outfile(filename);
			outfile << "\\section{" << function.name << "}" << endl;
			outfile << "\\begin{frame}{}" << endl;

			bool equal_types = types_equiv(function.input, function.output);

			//========= Шаблон для входных данных ====================

			if (function.input.size() == 1) {
				//Для автоматов
				if ((function.input[0] == ObjectType::NFA ||
					 function.input[0] == ObjectType::DFA)) {
					outfile << "\tАвтомат";

					if (equal_types) {
						outfile << " до преоброзования";
					}

					outfile << ":\n\t%template_oldautomaton" << endl;
				}

				//Для префиксной грамматики
				if (function.input[0] == ObjectType::PrefixGrammar) {
					outfile << "\tПрефиксная грамматика:" << endl;
					outfile << "\t%template_grammar" << endl;
				}

				//Для регулярок
				if ((function.input[0] == ObjectType::Regex)) {
					outfile << "\tРегулярное выражение";

					if (equal_types) {
						outfile << " до преоброзования";
					}

					outfile << ":\n\t%template_oldregex" << endl;
				}
			} else {
				bool equal_input_types =
					types_equiv(function.input, function.input[1]);
				for (int index = 0; index < (function.input.size() - 1);
					 index++) {
					//Для автоматов
					if ((function.input[index] == ObjectType::NFA ||
						 function.input[index] == ObjectType::DFA)) {
						if (equal_input_types) {
							if (index == 0) {
								outfile << "Первый автомат:";
								outfile << "\t%template_firstautomaton" << endl;
							} else {
								outfile << "Второй автомат:";
								outfile << "\t%template_secondautomaton"
										<< endl;
							}
						} else {
							outfile << "\tАвтомат:";
							outfile << "\t%template_oldautomaton" << endl;
						}
					}

					//Для префиксной грамматики
					if (function.input[index] == ObjectType::PrefixGrammar) {
						outfile << "\tПрефиксная грамматика:";
						outfile << "\t%template_grammar" << endl;
					}

					//Для регулярок
					if (function.input[index] == ObjectType::Regex) {
						if (equal_input_types) {
							if (index == 0) {
								outfile << "Первое регулярное выражение:";
								outfile << "\t%template_firstregex" << endl;
							} else {
								outfile << "Второе регулярное выражение:";
								outfile << "\t%template_secondregex" << endl;
							}
						} else {
							outfile << "\tРегулярное выражение:";
							outfile << "\t%template_oldregex" << endl;
						}
					}

					//Для Array
					if (function.input[index] == ObjectType::Array) {
						outfile << "\tПравила переписования:";
						outfile << "\t%template_oldarray" << endl;
					}

					//Для Int
					if (function.input[index] == ObjectType::Int ||
						function.input[index] == ObjectType::AmbiguityValue ||
						function.input[index] == ObjectType::Boolean ||
						function.input[index] == ObjectType::OptionalBool) {
						if (equal_input_types) {
							if (index == 0) {
								outfile << "Первое значение:";
								outfile << "\t%template_firstvalue" << endl;
							} else {
								outfile << "Второе значение:";
								outfile << "\t%template_secondvalue" << endl;
							}
						} else {
							outfile << "\tЗначение:";
							outfile << "\t%template_value" << endl;
						}
					}
				}
			}

			//========= Шаблон для выходных данных ===================

			//Для автоматов
			if (function.output == ObjectType::NFA ||
				function.output == ObjectType::DFA) {
				outfile << "\tАвтомат";

				if (equal_types) {
					outfile << " после преоброзования";
				}

				outfile << ":\n\t%template_newautomaton" << endl;
			}

			//Для Regex, Int, Bool, optionalbool
			if (function.output == ObjectType::AmbiguityValue ||
				function.output == ObjectType::Int ||
				function.output == ObjectType::Boolean ||
				function.output == ObjectType::OptionalBool) {
				outfile << "\tРезультат:" << endl;

				outfile << "\t%template_result" << endl;
			}

			//Для префиксной грамматики
			if (function.output == ObjectType::PrefixGrammar) {
				outfile << "\tПрефиксная грамматика:" << endl;

				outfile << "\t%template_grammar" << endl;
			}

			//Для регулярок
			if (function.output == ObjectType::Regex) {
				outfile << "\tРегулярное выражение";

				if (equal_types) {
					outfile << " после преоброзования";
				}

				outfile << ":\n\t%template_newregex" << endl;
			}

			outfile << "\\end{frame}" << endl;
			outfile.close();
		}
	}
	return;
}