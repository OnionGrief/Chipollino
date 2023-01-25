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

	// может позже добавить логгера для логгера
	cout << "\nCreating report...\n\n";
	size_t logs_size = logs.size();

	// That's just a demo
	for (size_t i = 0; i < logs.size(); i++) {
		outfile << logs[i].render() << "\n";
		cout << 100 * (i + 1) / logs_size << "% (template \""
			 << logs[i].get_tex_template() << "\" is completed)\n";
	}
	outfile << "\\end{document}" << endl;
	outfile.close();

	cout << "\nConverting to PDF...\n";

	char cmd[1024];
	// sprintf(cmd, "cd resources && pdflatex report.tex > pdflatex.log");
	sprintf(cmd, "pdflatex ./resources/report.tex > pdflatex.log");
	system(cmd);

	cout << "successfully created report\n";
}
