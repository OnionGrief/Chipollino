#include "Logger/Logger.h"
#include <fstream>

void Logger::add_log(const LogTemplate& log) {
	logs.push_back(log);
}

void Logger::render_to_file(const string& filename) {
	ifstream infile("./resources/template/head.tex");
	ofstream outfile(filename);

	string s;
	for (; !infile.eof();) {
		getline(infile, s);
		outfile << s << "\n";
	}
	infile.close();

	// может позже добавить логгера для логгера
	cout << "\nCreating report...\n\n";
	size_t logs_size = logs.size();

	// That's just a demo
	for (size_t i = 0; i < logs.size(); i++) {
		outfile << logs[i].render() << "\n";
		cout << 100 * (i + 1) / logs_size << "% (template \""
			 << logs[i].get_tex_template() << "\" is completed)\n";
	}
	outfile << "\\end{document}\n";
	outfile.close();

	cout << "\nFrameFormatter + MathMode...\n";

	system("cd refal && refgo FrameFormatter+MathMode 2>err.txt");

	cout << "\nConverting to PDF 1...\n";

	// sprintf(cmd, "cd resources && pdflatex report.tex > pdflatex.log");
	system("pdflatex ./resources/report.tex > pdflatex.log");

	cout << "\nConverting to PDF 2...\n";
	system("pdflatex ./resources/rendered_report.tex > pdflatex2.log");

	cout << "successfully created report\n";
}
