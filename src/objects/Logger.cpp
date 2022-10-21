#include "Logger.h"
#include <sstream>
#include <iostream>
#include <fstream>
using namespace std;

void Logger::init_step(string step_name) {
	ofstream out;
	out.open("Chipollino\resources\report.tex");
	if (out.is_open()){
		out << step_name << endl;
	}
	// return step_name;
}

void Logger::log() {
	stringstream ss;
	ss << "начало шага";
	// return ss.str();
}

void Logger::finish_step() {
	ofstream out;
	out.open("Chipollino\resource\report.tex");
	if (out.is_open()){
		out << "\end{document}"<< endl;
	}
	// return 0;
}