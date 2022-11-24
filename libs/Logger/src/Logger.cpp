#include "..\include\Logger\Logger.h"

void Logger::add_log(const LogTemplate& log) {
	logs.push_back(log);
}

void Logger::render_to_file(const string& filename) {
	// TODO
}
