#include <fstream>

#include "Logger/Logger.h"

using std::cout;
using std::ifstream;
using std::ofstream;
using std::string;

void Logger::add_log(const LogTemplate& log) {
	if (enabled) {
		logs.push_back(log);
	}
}

void Logger::render_to_file(const string& filename) {
	ifstream infile("./resources/template/head.tex");
	ofstream outfile(filename);

	string s;
	while (!infile.eof()) {
		getline(infile, s);
		outfile << s << "\n";
	}
	infile.close();

	// может позже добавить логгер для логгера
	cout << "\nCreating report...\n\n";

	size_t logs_size = logs.size();
	// Генерация каждого лога
	for (size_t i = 0; i < logs_size; i++) {
		outfile << logs[i].render() << "\n";
		cout << 100 * (i + 1) / logs_size << "% (template \"" << logs[i].get_tex_template()
			 << "\" is completed)\n";
	}
	outfile << "\\end{document}\n";
	outfile.close();

	cout << "\nConverting to PDF 1...\n";
	system("pdflatex ./resources/report.tex > pdflatex.log");

	cout << "\nFrameFormatter + MathMode...\n";

	system("cd refal && refgo RunFormatter+FrameFormatter+MathMode 2>error_FrameFormatter.raux");

	cout << "\nConverting to PDF 2...\n";
	system("pdflatex ./resources/rendered_report.tex > pdflatex2.log");

	cout << "successfully created report\n";
}

void Logger::enable() {
	enabled = true;
}

void Logger::disable() {
	enabled = false;
}