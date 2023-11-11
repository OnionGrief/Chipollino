#include <fstream>
#include <iostream>
#include <sstream>
#include "AutomatonToImage/AutomatonToImage.h"

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

string AutomatonToImage::to_image(string automat) {
#ifdef _WIN32
	system("cd refal && IF EXIST Meta_input.data del Meta_input.data && IF "
		   "EXIST Aux_input.data del Aux_input.data");
#elif __unix || __unix__ || __linux__
	system("cd refal && rm -f Meta_input.data && rm -f Aux_input.data");
#endif

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

#ifdef _WIN32
	system("cd refal && IF EXIST Meta_input.data del Meta_input.data && IF "
		   "EXIST Aux_input.data del Aux_input.data");
#elif __unix || __unix__ || __linux__
	system("cd refal && rm -f Meta_input.data && rm -f Aux_input.data");
#endif

	// автомат
	std::ifstream infile_for_R("./refal/R_input.tex");
	std::stringstream graph;
	string s;
	if (!infile_for_R) return "";

	while (!infile_for_R.eof()) {
		getline(infile_for_R, s);
		graph << s << "\n";
	}
	infile_for_R.close();

#ifdef _WIN32
	system("cd refal && del input.dot && del input.tex && del Mod_input.dot && "
		   "del "
		   "R_input.tex");
#elif __unix || __unix__ || __linux__
	system("cd refal && rm input.dot && rm input.tex && rm Mod_input.dot && rm "
		   "R_input.tex");
#endif

	// таблица
	std::ifstream infile_for_L("./refal/L_input.tex");

	if (!infile_for_L) return graph.str();

	// the table is adjusted for frames in the general renderer module
	while (!infile_for_L.eof()) {
		getline(infile_for_L, s);
		graph << s << "\n";
	}
	infile_for_L.close();

#ifdef _WIN32
	system("cd refal && del L_input.tex");
#elif __unix || __unix__ || __linux__
	system("cd refal && rm L_input.tex");
#endif

	return graph.str();
}