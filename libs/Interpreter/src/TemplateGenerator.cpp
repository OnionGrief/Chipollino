#include "Interpreter/Interpreter.h"
#include <string>

void Interpreter::generate_brief_templates() {
	for (auto func_item : names_to_functions) {
		for (auto function : func_item.second) {
			string func_id = get_func_id(function);
			string filename = "./resources/template/";
			filename += func_id + ".tex";
			ofstream outfile(filename);
			outfile << "\\section{" << function.name << "}" << endl;
			outfile.close();
		}
	}
	return;
}