#include "Objects/Logger.h"
#include <fstream>
#include <iostream>
using namespace std;

Logger::Logger() {}

Logger::~Logger() {}

void Logger::activate() {
	active = true;
}

void Logger::deactivate() {
	active = false;
}

void Logger::activate_step_counter() {
	step_counter++;
}

void Logger::deactivate_step_counter() {
	step_counter--;
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
	step_counter++;
	if (step_counter > 1) return;
	ofstream out("./resources/report.tex", ios::app);
	if (out.is_open()) {
		out << step_name + "\n" << endl;
	}
	out.close();
}

void Logger::log(string text) {
	if (!active) return;
	if (step_counter > 1) return;
	ofstream out("./resources/report.tex", ios::app);
	if (out.is_open()) {
		out << text + "\n" << endl;
	}
	out.close();
}

void Logger::log(string text, string val) {
	if (!active) return;
	if (step_counter > 1) return;
	ofstream out("./resources/report.tex", ios::app);
	if (out.is_open()) {
		out << text + ": ";
		out << val + "\n" << endl;
	}
	out.close();
}

void Logger::log(string a1, const FiniteAutomaton& fa1) {
	if (!active) return;
	if (step_counter > 1) return;
	string f1 = fa1.to_txt();
	ofstream out("./resources/report.tex", ios::app);
	if (out.is_open()) {
		image_number += 1;
		AutomatonToImage::to_image(f1, image_number);
		out << a1 + ":\n" << endl;
		char si[256];
		sprintf(si,
				"\\includegraphics[width=5in, "
				"keepaspectratio]{output%d.png}\n",
				image_number);
		out << si << endl;
	}
	out.close();
}

void Logger::log(string a1, string a2, const FiniteAutomaton& fa1,
				 const FiniteAutomaton& fa2) {
	if (!active) return;
	if (step_counter > 1) return;
	string f1 = fa1.to_txt();
	string f2 = fa2.to_txt();
	ofstream out("./resources/report.tex", ios::app);
	if (out.is_open()) {
		image_number += 1;
		AutomatonToImage::to_image(f1, image_number);
		out << a1 + ":\n" << endl;
		char si[256];
		sprintf(si,
				"\\includegraphics[width=5in, "
				"keepaspectratio]{output%d.png}\n",
				image_number);
		out << si << endl;

		image_number += 1;
		AutomatonToImage::to_image(f2, image_number);
		out << a2 + ":\n" << endl;
		sprintf(si,
				"\\includegraphics[width=5in, "
				"keepaspectratio]{output%d.png}\n",
				image_number);
		out << si << endl;
	}
	out.close();
}

void Logger::log(string a1, string a2, string a3, const FiniteAutomaton& fa1,
				 const FiniteAutomaton& fa2, const FiniteAutomaton& fa3) {
	if (!active) return;
	if (step_counter > 1) return;
	string f1 = fa1.to_txt();
	string f2 = fa2.to_txt();
	string f3 = fa3.to_txt();
	ofstream out("./resources/report.tex", ios::app);
	if (out.is_open()) {
		image_number += 1;
		AutomatonToImage::to_image(f1, image_number);
		out << a1 + ":\n" << endl;
		char si[256];
		sprintf(si,
				"\\includegraphics[width=5in, "
				"keepaspectratio]{output%d.png}\n",
				image_number);
		out << si << endl;

		image_number += 1;
		AutomatonToImage::to_image(f2, image_number);
		out << a2 + ":\n" << endl;
		sprintf(si,
				"\\includegraphics[width=5in, "
				"keepaspectratio]{output%d.png}\n",
				image_number);
		out << si << endl;

		image_number += 1;
		AutomatonToImage::to_image(f3, image_number);
		out << a3 + ":\n" << endl;
		sprintf(si,
				"\\includegraphics[width=5in, "
				"keepaspectratio]{output%d.png}\n",
				image_number);
		out << si << endl;
	}
	out.close();
}

void Logger::log(int step, vector<int> lengths, vector<double> times,
				 vector<bool> belongs) {
	if (!active) return;
	if (step_counter > 1) return;
	ofstream out("./resources/report.tex", ios::app);
	if (out.is_open()) {
		out << "\\begin{tabular}{llll}\n" << endl;
		string s1 = "Количество итераций";
		s1 = s1 + " & ";
		s1 = s1 + "Длина строки";
		s1 = s1 + " & ";
		s1 = s1 + "Время парсинга";
		s1 = s1 + " & ";
		s1 = s1 + "Принадлежность языку";
		s1 = s1 + "\\\\";
		out << s1 << endl;
		for (int i = 0; i < times.size(); i++) {
			string w1 = to_string(step * i);
			string w2 = to_string(lengths[i]);
			string w3 = to_string(times[i]);
			w3 = w3.substr(0, 5);
			string w4;
			if (belongs[i]) {
				w4 = "true";
			} else {
				w4 = "false";
			}
			if (i != times.size() - 1) {
				out << w1 + " & " + w2 + " & " + w3 + " & " + w4 + " \\\\"
					<< endl;
			} else {
				out << w1 + " & " + w2 + " & " + w3 + " & " + w4 << endl;
			}
		}
		out << "\\end{tabular}\n" << endl;
	}
	out.close();
}

void Logger::finish_step() {
	if (!active) return;
	step_counter--;
	if (step_counter > 0) return;
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
	sprintf(cmd, "pdflatex \"./resources/report.tex\" > pdflatex.log");
	system(cmd);
}
