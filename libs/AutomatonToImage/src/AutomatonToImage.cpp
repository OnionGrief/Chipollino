#include "AutomatonToImage/AutomatonToImage.h"
#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;

AutomatonToImage::AutomatonToImage() {}

AutomatonToImage::~AutomatonToImage() {}

/*
void AutomatonToImage::to_image(string automat, int name) {
	char cmd[1024];

	// для Linux:

	/*sprintf(cmd,
			"dot -Tpng ./resources/input.dot > "
			"./resources/output%d.png && rm ./resources/input.dot",
			name);

	// для Windows:

	sprintf(cmd,
			"dot -Tpng ./resources/input.dot > "
			"./resources/output%d.png && rm ./resources/input.dot",
			name);
	system(cmd);
*/

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

string AutomatonToImage::to_image(string automat) {
	remove_file("refal", "Aux_input.data", true);
	FILE* fo;
	fo = fopen("./refal/input.dot", "wt");
	fprintf(fo, "%s", automat.c_str());
	fclose(fo);
	system("cd refal && refgo Preprocess+MathMode+FrameFormatter input.dot > "
		   "error_refal0.txt");
	system("cd refal && dot2tex -ftikz -tmath \"Mod_input.dot\" > input.tex");
	system("cd refal && refgo Postprocess+MathMode+FrameFormatter input.tex > "
		   "error_refal.txt "
		   "2>&1");
	remove_file("refal", "Meta_input.data", true);
	remove_file("refal", "Aux_input.data", true);
	// автомат
	ifstream infile_for_R("./refal/R_input.tex");
	stringstream graph;
	string s;
	if (!infile_for_R) return "";

	while (!infile_for_R.eof()) {
		getline(infile_for_R, s);
		graph << s << "\n";
	}
	infile_for_R.close();

	remove_file("refal", "input.dot");
	remove_file("refal", "input.tex");
	remove_file("refal", "Mod_input.dot");
	remove_file("refal", "R_input.tex");

	return graph.str();
}

string AutomatonToImage::colorize(string automaton, string metadata) {

	FILE* fo;
	FILE* md;
	ifstream infile_for_Final;
	fo = fopen("./refal/Col_input.tex", "wt");
	fprintf(fo, "%s", automaton.c_str());
	fclose(fo);
	if (metadata != "") {
		md = fopen("./refal/Meta_input.data", "wt");
		fprintf(md, "%s", metadata.c_str());
		fclose(md);
		system("cd refal && refgo Colorize+MathMode Col_input.tex > "
			   "error_colorize.txt");
		infile_for_Final.open("./refal/Final_input.tex");
		remove_file("refal", "Meta_input.data");
	} else
		infile_for_Final.open("./refal/Col_input.tex");

	// автомат
	stringstream graph;
	string s;
	if (!infile_for_Final) return "";

	while (!infile_for_Final.eof()) {
		getline(infile_for_Final, s);
		graph << s << endl;
	}
	infile_for_Final.close();

	remove_file("refal", "Final_input.tex", true);
	remove_file("refal", "Col_input.tex");
	// таблица
	ifstream infile_for_L("./refal/L_input.tex");

	if (!infile_for_L) return graph.str();

	// the table is adjusted for frames in the general renderer module
	while (!infile_for_L.eof()) {
		getline(infile_for_L, s);
		graph << s << endl;
	}
	infile_for_L.close();

	remove_file("refal", "Aux_input.data", true);
	remove_file("refal", "L_input.tex", true);

	return graph.str();
}
