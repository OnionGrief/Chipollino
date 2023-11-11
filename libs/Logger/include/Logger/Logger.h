#pragma once
#include <string>
#include <vector>
#include "LogTemplate.h"


class Logger {
  public:
	void add_log(const LogTemplate& log);
	void render_to_file(const string& filename = "./resources/report.tex");
	void enable();
	void disable();

  private:
	bool enabled = true;
	vector<LogTemplate> logs;
};