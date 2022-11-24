#pragma once
#include "LogTemplate.h"
#include <string>
#include <vector>

class Logger {
  public:
	void add_log(const LogTemplate& log);
	void render_to_file(const string& filename);

  private:
	vector<LogTemplate> logs;
};