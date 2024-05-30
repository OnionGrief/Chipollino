#pragma once
#include <string>
#include <vector>

#include "LogTemplate.h"

class Logger {
  public:
	Logger();
	void add_log(const LogTemplate& log);
	void render_to_file(const std::string& filename = "./resources/report.tex",
						const std::string& user_name = "");
	void enable();
	void disable();

  private:
	bool enabled = true;
	std::vector<LogTemplate> logs;
};