#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <vector>

#include "AutomatonToImage/AutomatonToImage.h"

using std::cout;
using std::ifstream;
using std::ofstream;
using std::string;
using std::stringstream;
using std::vector;

AutomatonToImage::AutomatonToImage() {}

AutomatonToImage::~AutomatonToImage() {}

string replace_before_dot2tex(const string& s) {
	vector<std::pair<string, string>> substrs_to_replace = {{"\\^", "#^ "}, {"&", "#&"}};

	string result = s;
	for (const auto& [old_substr, new_substr] : substrs_to_replace) {
		std::regex re(old_substr);
		result = std::regex_replace(result, re, new_substr);
	}

	return result;
}

void write_to_file(const string& file_name, const string& content) {
	ofstream file;
	file.open(file_name, ofstream::trunc);
	if (file.is_open())
		file << replace_before_dot2tex(content);
	file.close();
}

void remove_file(string dir, string file, bool guarded = false) {
	stringstream command;
	command << "cd " << dir;
#ifdef _WIN32
	if (guarded)
		command << " && IF EXIST " << file << " DEL " << file;
	else
		command << " && DEL " << file;
#elif __unix || __unix__ || __linux__
	if (guarded)
		command << " && rm -f " << file;
	else
		command << " && rm " << file;
#endif
	system(command.str().c_str());
}

string AutomatonToImage::to_image(string automaton) {
	remove_file("refal", "Meta_log.raux", true);
	remove_file("refal", "Aux_input.raux", true);
	write_to_file("./refal/input.dot", replace_before_dot2tex(automaton));

	system("cd refal && dot2tex -ftikz -tmath \"input.dot\" > input.tex");

	system("cd refal && refgo Postprocess+MathMode+FrameFormatter input.tex > "
		   "error_Postprocess.raux "
		   "2>&1");
	remove_file("refal", "Meta_input.raux", true);
	remove_file("refal", "Aux_input.raux", true);
	// автомат
	ifstream infile_for_R("./refal/R_input.tex");
	stringstream graph;
	string s;
	if (!infile_for_R)
		return "";

	while (!infile_for_R.eof()) {
		getline(infile_for_R, s);
		graph << s << "\n";
	}
	infile_for_R.close();

	remove_file("refal", "input.dot");
	remove_file("refal", "input.tex");
	// remove_file("refal", "Mod_input.dot");
	remove_file("refal", "R_input.tex");

	return graph.str();
}

string AutomatonToImage::colorize(string automaton, string metadata) {

	ifstream infile_for_Final;
	write_to_file("./refal/Col_input.tex", automaton);
	if (metadata != "") {
		write_to_file("./refal/Meta_input.raux", metadata);
		system("cd refal && refgo Colorize+MathMode Col_input.tex > "
			   "error_Colorize.raux");
		infile_for_Final.open("./refal/Final_input.tex");
		remove_file("refal", "Meta_input.raux");
	} else {
		infile_for_Final.open("./refal/Col_input.tex");
	}

	// автомат
	stringstream graph;
	string s;
	if (!infile_for_Final)
		return "";

	while (!infile_for_Final.eof()) {
		getline(infile_for_Final, s);
		graph << s << "\n";
	}
	infile_for_Final.close();

	remove_file("refal", "Final_input.tex", true);
	remove_file("refal", "Col_input.tex");
	// таблица
	ifstream infile_for_L("./refal/L_input.tex");

	if (!infile_for_L)
		return graph.str();

	// the table is adjusted for frames in the general renderer module
	while (!infile_for_L.eof()) {
		getline(infile_for_L, s);
		graph << s << "\n";
	}
	infile_for_L.close();

	remove_file("refal", "Aux_input.raux", true);
	remove_file("refal", "L_input.tex", true);

	return graph.str();
}
