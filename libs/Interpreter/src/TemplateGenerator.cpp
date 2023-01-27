#include "Interpreter/Interpreter.h"
#include <string>

bool types_equiv(vector<ObjectType> input, ObjectType output) {
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
			outfile << "\\begin{frame}{" << function.name << "}" << endl;

			bool equal_types = types_equiv(function.input, function.output);
			if (function.input.size() > 1) {
				equal_types = false;
			}
			//========= Шаблон для одиночных входных данных =========

			if (function.input.size() == 1) {
				//Для автоматов
				if ((function.input[0] == ObjectType::NFA ||
					 function.input[0] == ObjectType::DFA)) {
					outfile << "\tАвтомат";

					if (equal_types) {
						outfile << " до преобразования";
					}

					outfile << ":\n\n\t%template_oldautomaton" << endl << endl;
				}

				//Для префиксной грамматики
				if (function.input[0] == ObjectType::PrefixGrammar) {
					outfile << "\tПрефиксная грамматика:" << endl << endl;
					outfile << "\t%template_grammar" << endl << endl;
				}

				//Для регулярок
				if ((function.input[0] == ObjectType::Regex)) {
					outfile << "\tРегулярное выражение";

					if (equal_types) {
						outfile << " до преобразования";
					}

					outfile << ":\n\t%template_oldregex" << endl << endl;
				}
			} else {

				//========= Шаблон для парных входных данных ===========

				bool input_types_equal =
					types_equiv(function.input, function.input[1]);

				for (int index = 0; index < (function.input.size()); index++) {

					//Для автоматов
					if ((function.input[index] == ObjectType::NFA ||
						 function.input[index] == ObjectType::DFA)) {
						if (input_types_equal) {
							if (index == 0) {
								outfile << "\tПервый автомат:" << endl << endl;
								outfile << "\t%template_automaton" << index + 1
										<< endl
										<< endl;
							} else {
								outfile << "\tВторой автомат:" << endl << endl;
								outfile << "\t%template_automaton" << index + 1
										<< endl
										<< endl;
							}
						} else {
							outfile << "\tАвтомат:" << endl << endl;
							outfile << "\t%template_oldautomaton" << endl;
						}
					}

					//Для префиксной грамматики
					if (function.input[index] == ObjectType::PrefixGrammar) {
						outfile << "\tПрефиксная грамматика:" << endl << endl;
						outfile << "\t%template_grammar" << endl << endl;
					}

					//Для регулярок
					if (function.input[index] == ObjectType::Regex) {
						if (input_types_equal) {
							if (index == 0) {
								outfile << "\tПервое регулярное выражение:"
										<< endl;
								outfile << "\t%template_regex" << index + 1
										<< endl
										<< endl;
							} else {
								outfile << "\tВторое регулярное выражение:"
										<< endl;
								outfile << "\t%template_regex" << index + 1
										<< endl
										<< endl;
							}
						} else {
							outfile << "\tРегулярное выражение:" << endl;
							outfile << "\t%template_oldregex" << endl << endl;
						}
					}

					//Для Array
					if (function.input[index] == ObjectType::Array) {
						outfile << "\tПравила переписывания:" << endl << endl;
						outfile << "\t%template_oldarray" << endl << endl;
					}

					//Для других типов
					if (function.input[index] == ObjectType::Int ||
						function.input[index] == ObjectType::AmbiguityValue ||
						function.input[index] == ObjectType::Boolean ||
						function.input[index] == ObjectType::OptionalBool) {
						if (input_types_equal) {
							outfile << "\t"
									<< types_to_string[function.input[index]]
									<< index + 1 << ":" << endl
									<< endl;
							outfile << "\t%template_value" << index + 1 << endl
									<< endl;
						} else {
							outfile << "\t"
									<< types_to_string[function.input[index]]
									<< endl
									<< endl;
							outfile << "\t%template_value" << endl << endl;
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
					outfile << " после преобразования";
				}

				outfile << ":\n\n\t%template_result" << endl << endl;
			}

			//Для Regex, Int, Bool, optionalbool
			if (function.output == ObjectType::AmbiguityValue ||
				function.output == ObjectType::Int ||
				function.output == ObjectType::Boolean ||
				function.output == ObjectType::OptionalBool) {
				outfile << "\tРезультат:" << endl;

				outfile << "\t%template_result" << endl << endl;
			}

			//Для префиксной грамматики
			if (function.output == ObjectType::PrefixGrammar) {
				outfile << "\tПрефиксная грамматика:" << endl << endl;

				outfile << "\t%template_result" << endl << endl;
			}

			//Для регулярок
			if (function.output == ObjectType::Regex) {
				outfile << "\tРегулярное выражение";

				if (equal_types) {
					outfile << " после преобразования";
				}

				outfile << ":\n\t%template_result" << endl << endl;
			}

			outfile << "\\end{frame}" << endl;
			outfile.close();
		}
	}
	return;
}

void Interpreter::generate_test_for_all_functions() {
	ofstream outfile("./resources/all_functions.txt");
	outfile << "R = {a}" << endl;
	outfile << "A = Determinize.Glushkov R" << endl;
	outfile << "V = Ambiguity A" << endl;
	outfile << "B = Deterministic A" << endl;
	outfile << "P = PrefixGrammar A" << endl;
	for (auto func_item : names_to_functions) {
		for (auto function : func_item.second) {
			string func_id = function.name;
			outfile << "N = " << func_id;
			for (auto arg : function.input) {
				if (arg == ObjectType::NFA || arg == ObjectType::DFA) {
					outfile << " A";
				} else if (arg == ObjectType::AmbiguityValue) {
					outfile << " V";
				} else if (arg == ObjectType::Boolean ||
						   arg == ObjectType::OptionalBool) {
					outfile << " B";
				} else if (arg == ObjectType::PrefixGrammar) {
					outfile << " P";
				} else if (arg == ObjectType::Regex) {
					outfile << " R";
				} else if (arg == ObjectType::Array) {
					outfile << " [[{a} {b}]]";
				} else if (arg == ObjectType::Int) {
					outfile << " 1";
				}
			}
			outfile << endl;
		}
	}
}