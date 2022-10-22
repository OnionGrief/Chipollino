#include "Logger.h"
#include <iostream>
#include <fstream>
using namespace std;

Logger::Logger() {}

Logger::~Logger() {}

void Logger::init() {
	char cmd[1024];
    sprintf(cmd, "mkdir \"./../resources/images\"");
    system(cmd);
}

void Logger::init_step(string step_name) {
	ofstream out("./../resources/report.tex", ios::app);
	if (out.is_open()) {
		out << step_name + "\n" << endl;
	}
	out.close(); 
}

void Logger::log(string text, string fa1, string fa2) {
	ofstream out("./../resources/report.tex", ios::app);
	if (out.is_open()) {
		out << text + "\n" << endl;
		if (fa1 != "") {
			AutomatonToImage a;
			a.to_image(fa1, "1");
			out << "Автомат до преобразования\n" << endl;
			out << "\\includegraphics{output1.png}\n" << endl;
		}
		if (fa2 != "") {
			AutomatonToImage a;
			a.to_image(fa2, "2");
			out << "Автомат после преобразования\n" << endl;
			out << "\\includegraphics{output2.png}\n" << endl;
		}
	}
	out.close();
}

void Logger::finish_step() {
	ofstream out("./../resources/report.tex", ios::app);
	if (out.is_open()) {
		out << "\\newpage\n" << endl;
	}
	out.close(); 
}

void Logger::finish() {
	ofstream out("./../resources/report.tex", ios::app);
	if (out.is_open()) {
		out << "\\end{document}" << endl;
	}
	out.close(); 
}

string Logger::to_txt() {}