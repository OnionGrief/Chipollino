#include "Logger/Logger.h"
#include <fstream>

void Logger::add_log(const LogTemplate& log) {
	logs.push_back(log);
}

void Logger::render_to_file(const string& filename) {
	// TODO
	ifstream infile("./resources/template/head.tex");
	ofstream outfile(filename);

	string s;
	for (; !infile.eof();) {
		getline(infile, s);
		outfile << s << endl;
	}
	infile.close();

	// That's just a demo
	for (const auto& log : logs) {
		outfile << log.render() << "\n";
	}
	outfile << "\\end{document}" << endl;
	outfile.close();

	char cmd[1024];
	// sprintf(cmd, "cd resources && pdflatex report.tex > pdflatex.log");
	sprintf(cmd, "pdflatex ./resources/report.tex > pdflatex.log");
	system(cmd);
}
