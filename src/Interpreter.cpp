#pragma once
#include "Interpreter.h"
#include <string>
#include <map>
using namespace std;

optional<vector<Function>> Interpreter::build_function_sequence(vector<string> function_names) {
	optional<vector<Function>> finalfuncs;
	vector<bool> neededfuncs(function_names.size(), true);
	for (int i = 1; i < function_names.size(); i++) {
		string func = function_names[i];
		string predfunc = function_names[i - 1];
		//check on types
		if (!(func == "DeLinearize" || func == "DeAnnote" ||
			predfunc == "DeLinearize" || predfunc == "DeAnnote")) {
			if (functions[predfunc].output != functions[func].input) {
				if (!(functions[predfunc].output == "DFA" &&
					functions[func].input == {"NFA"})) {
					return nullopt;
				} else {
					if (predfunc == "Determinize" || predfunc == "Annote") {
						if (func == "Determinize" || func == "Minimize" ||
							func == "Annote") {
							needfuncs[i - 1] = false;
						}
					} else if (predfunc == "Minimize" && func == "Minimize") {
						needfuncs[i - 1] = false;
                    }
                }
			} else {
				if (predfunc == func) {
					if (predfunc != "Reverse" || predfunc != "Complement") {
						needfuncs[i - 1] = false;
                    }
				} else {
                    //доработать
                }
            }
		}
    }
	for (int i = 0; i < function_name.size(); i++) {
		if (neededfuncs[i]) {
			finalfuncs.value().push_back(functions[function_names[i]]);
        }
    }
	return finalfuncs;
}