#include "Logger.h"
#include <iostream>
#include <fstream>
using namespace std;

Logger::Logger() {}

Logger::~Logger() {}

void Logger::activate() {
	active = true;
}

void Logger::deactivate() {
	active = false;
}

void Logger::init() {
	ofstream out;
	out.open("./resources/report.tex", ofstream::trunc);
	if (out.is_open()) {
		out << "\\documentclass[14pt, russion]{article}" << endl;
		out << "\\usepackage[T2A]{fontenc}" << endl;
		out << "\\usepackage[utf8]{inputenc}" << endl;
		out << "\\usepackage[russian]{babel}" << endl;
		out << "\\title{Отчет}" << endl;
		out << "\\author{Чиполлино}" << endl;
		out << "\\date{Москва, 2022}" << endl;
		out << "\\usepackage{graphicx}" << endl;
		out << "\\graphicspath{ {./resources/} }" << endl;
		out << "\\begin{document}" << endl;
		out << "\\maketitle" << endl;
		out << "\\newpage" << endl;
	}
	out.close(); 
}

void Logger::init_step(string step_name) {
	if (!active) return;
	ofstream out("./resources/report.tex", ios::app);
	if (out.is_open()) {
		out << step_name + "\n" << endl;
	}
	out.close(); 
}

void Logger::log(string text) {
	if (!active) return;
	ofstream out("./resources/report.tex", ios::app);
	if (out.is_open()) {
		out << text + "\n" <<endl;
	}
	out.close();
}

void Logger::log(string text, string val) {
	if (!active) return;
	ofstream out("./resources/report.tex", ios::app);
	if (out.is_open()) {
		out << text + ": ";
		out << val + "\n" <<endl;
	}
	out.close();
}

void Logger::log(const FiniteAutomaton& fa1, const FiniteAutomaton& fa2) {
	if (!active) return;
	string f1 = fa1.to_txt();
	string f2 = fa2.to_txt();
	ofstream out("./resources/report.tex", ios::app);
	if (out.is_open()) {
		if (f1 != "") {
			i += 1;
			AutomatonToImage::to_image(f1, i);
			out << "Автомат до преобразования\n" << endl;
			char si[256];
    		sprintf(si, "\\includegraphics[width=5in, keepaspectratio]{./resources/output%d.png}\n", i);
			out << si << endl;
		}
		if (f2 != "") {
			i += 1;
			AutomatonToImage::to_image(f2, i);
			out << "Автомат после преобразования\n" << endl;
			char si[256];
    		sprintf(si, "\\includegraphics[width=5in, keepaspectratio]{./resources/output%d.png}\n", i);
			out << si << endl;
		}
	}
	out.close();
}

void Logger::log(string text, const FiniteAutomaton& fa1, const FiniteAutomaton& fa2, const FiniteAutomaton& fa3) {
	if (!active) return;
	string f1 = fa1.to_txt();
	string f2 = fa2.to_txt();
	string f3 = fa3.to_txt();
	ofstream out("./resources/report.tex", ios::app);
	if (out.is_open()) {
		// out << text + "\n" << endl;
		if (f1 != "") {
			i += 1;
			AutomatonToImage::to_image(f1, i);
			out << "Первый автомат\n" << endl;
			char si[256];
    		sprintf(si, "\\includegraphics[width=5in, keepaspectratio]{./resources/output%d.png}\n", i);
			out << si << endl;
		}
		if (f2 != "") {
			i += 1;
			AutomatonToImage::to_image(f2, i);
			out << "Второй атомат\n" << endl;
			char si[256];
    		sprintf(si, "\\includegraphics[width=5in, keepaspectratio]{./resources/output%d.png}\n", i);
			out << si << endl;
		}
		if (f3 != "") {
			i += 1;
			AutomatonToImage::to_image(f3, i);
			out << text + "\n" << endl;
			char si[256];
    		sprintf(si, "\\includegraphics[width=5in, keepaspectratio]{./resources/output%d.png}\n", i);
			out << si << endl;
		}
	}
	out.close();
}

void Logger::finish_step() {
	ofstream out("./resources/report.tex", ios::app);
	if (out.is_open()) {
		out << "\\newpage\n" << endl;
	}
	out.close(); 
}

void Logger::finish() {
	ofstream out("./resources/report.tex", ios::app);
	if (out.is_open()) {
		out << "\\end{document}" << endl;
	}
	out.close();
	char cmd[1024];
    sprintf(cmd, "pdflatex \"./resources/report.tex\"");
    system(cmd); 
}