#pragma once
#include "LogTemplate.h"
#include <string>
#include <vector>

using namespace std;

class Logger {
  public:
	void add_log(const LogTemplate& log);
	void render_to_file(const string& filename = "./resources/report.tex");

  private:
	vector<LogTemplate> logs;
};