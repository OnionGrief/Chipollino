#pragma once
#include "Interpreter/Interpreter.h"
#include <string>

void Interpreter::generate_brief_templates() {
    for (auto func_vector : names_to_functions) {
        // for для каждой сигнатуры
        /*string func_id = function.name;
	if (names_to_functions[func_id].size() > 1)
		func_id +=
			to_string(find_func(function.name, function.input).value() + 1);*/
    string filename = "./resources/template/";
    filename += ".tex"; // func_id
	ofstream outfile(filename); 
    outfile << "\\section{"<</*function.name<<*/"}" << endl;
	outfile.close();
    }
	return;
}