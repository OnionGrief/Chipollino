#include "Logger/Logger.h"
#include <fstream>

void Logger::add_log(const LogTemplate& log) {
	logs.push_back(log);
}

void Logger::render_to_file(const string& filename) {
	// TODO
	ofstream outfile(filename);

	// That's just a demo
	for (const auto& log : logs) {
		outfile << log.render() << "\n";
	}

	outfile.close();
}